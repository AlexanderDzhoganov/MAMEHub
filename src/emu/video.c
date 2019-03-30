// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    video.c

    Core MAME video routines.

***************************************************************************/

#include "debugger.h"
#include "emu.h"
#include "emuopts.h"
#include "nsm_client.h"
#include "nsm_server.h"
#include "output.h"
#include "rendersw.inc"
#include "ui/ui.h"
#include "crsshair.h"

#include "osdepend.h"

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_THROTTLE (0)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// frameskipping tables
const UINT8 video_manager::s_skiptable[FRAMESKIP_LEVELS][FRAMESKIP_LEVELS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
    {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1}, {0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1},
    {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1}, {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1},
    {0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1}, {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

//**************************************************************************
//  VIDEO MANAGER
//**************************************************************************

static void video_notifier_callback(const char *outname, INT32 value,
                                    void *param) {
  video_manager *vm = (video_manager *)param;

  vm->set_output_changed();
}

//-------------------------------------------------
//  video_manager - constructor
//-------------------------------------------------

video_manager::video_manager(running_machine &machine)
    : m_machine(machine), m_screenless_frame_timer(NULL),
      m_output_changed(false), m_throttle_last_ticks(0),
      m_throttle_realtime(attotime::zero), m_throttle_emutime(attotime::zero),
      m_throttle_history(0), m_speed_last_realtime(0),
      m_speed_last_emutime(attotime::zero), m_speed_percent(1.0),
      m_overall_real_seconds(0), m_overall_real_ticks(0),
      m_overall_emutime(attotime::zero), m_overall_valid_counter(0),
      m_throttled(machine.options().throttle()), m_throttle_rate(1.0f),
      m_fastforward(false),
      m_seconds_to_run(machine.options().seconds_to_run()),
      m_auto_frameskip(machine.options().auto_frameskip()),
      m_speed(original_speed_setting()), m_empty_skip_count(0),
      m_frameskip_level(machine.options().frameskip()), m_frameskip_counter(0),
      m_frameskip_adjust(0), m_skipping_this_frame(false),
      m_average_oversleep(0)
    {
  // request a callback upon exiting
 
  machine.save().register_postload(
      save_prepost_delegate(FUNC(video_manager::postload), this));

  // extract initial execution state from global configuration settings
  update_refresh_speed();

  // if no screens, create a periodic timer to drive updates
  if (machine.first_screen() == NULL) {
    m_screenless_frame_timer =
        machine.scheduler().timer_alloc(timer_expired_delegate(
            FUNC(video_manager::screenless_update_callback), this));
    m_screenless_frame_timer->adjust(screen_device::DEFAULT_FRAME_PERIOD, 0,
                                     screen_device::DEFAULT_FRAME_PERIOD);
    output_set_notifier(NULL, video_notifier_callback, this);
  }
}

//-------------------------------------------------
//  set_frameskip - set the current actual
//  frameskip (-1 means autoframeskip)
//-------------------------------------------------

void video_manager::set_frameskip(int frameskip) {
  // Can't skip frames in client-server mode
  if (netCommon) {
    m_auto_frameskip = false;
    m_frameskip_level = 0;
  }
  // -1 means autoframeskip
  else if (frameskip == -1) {
    m_auto_frameskip = true;
    m_frameskip_level = 0;
  }

  // any other level is a direct control
  else if (frameskip >= 0 && frameskip <= MAX_FRAMESKIP) {
    m_auto_frameskip = false;
    m_frameskip_level = frameskip;
  }
}

extern bool waitingForClientCatchup;

//-------------------------------------------------
//  frame_update - handle frameskipping and UI,
//  plus updating the screen during normal
//  operations
//-------------------------------------------------

bool SKIP_OSD = false;
extern bool catchingUp;

void video_manager::frame_update(bool debug) {
  // only render sound and video if we're in the running phase
  int phase = machine().phase();
  bool skipped_it = m_skipping_this_frame;
  if (phase == MACHINE_PHASE_RUNNING &&
      (!machine().paused() || machine().options().update_in_pause())) {
    bool anything_changed = finish_screen_updates();

    // if none of the screens changed and we haven't skipped too many frames in
    // a row, mark this frame as skipped to prevent throttling; this helps for
    // games that don't update their screen at the monitor refresh rate
    if (!anything_changed && !m_auto_frameskip && m_frameskip_level == 0 &&
        m_empty_skip_count++ < 3)
      skipped_it = true;
    else
      m_empty_skip_count = 0;
  }

  if (netCommon) {
    // Can't skip frames in network mode
    skipped_it = false;
  }
  // draw the user interface
  machine().ui().update_and_render(&machine().render().ui_container());

  // if we're throttling, synchronize before rendering

  attotime current_time = machine().machine_time();
  // Don't throttle if you are a network client
  //if (!debug && !skipped_it && effective_throttle())
  //  update_throttle(current_time);

  // ask the OSD to update
  g_profiler.start(PROFILER_BLIT);
  machine().osd().update(!debug && (skipped_it));
  g_profiler.stop();

  // perform tasks for this frame
  if (!debug)
    machine().call_notifiers(MACHINE_NOTIFY_FRAME);

  // update frameskipping
  if (!netCommon && !debug)
    update_frameskip();

  // update speed computations
  if (!debug)
    recompute_speed(current_time);

  // call the end-of-frame callback
  if (phase == MACHINE_PHASE_RUNNING) {
    // reset partial updates if we're paused or if the debugger is active
    if (machine().first_screen() != NULL &&
        (machine().paused() || debug ||
         debugger_within_instruction_hook(machine())))
      machine().first_screen()->reset_partial_updates();
  }
}

//-------------------------------------------------
//  speed_text - print the text to be displayed
//  into a string buffer
//-------------------------------------------------

astring &video_manager::speed_text(astring &string) {
  string.reset();

  // if we're paused, just display Paused
  bool paused = machine().paused();
  if (paused)
    string.cat("paused");

  // if we're fast forwarding, just display Fast-forward
  else if (m_fastforward)
    string.cat("fast ");

  // if we're auto frameskipping, display that plus the level
  else if (effective_autoframeskip())
    string.catprintf("auto%2d/%d", effective_frameskip(), MAX_FRAMESKIP);

  // otherwise, just display the frameskip plus the level
  else
    string.catprintf("skip %d/%d", effective_frameskip(), MAX_FRAMESKIP);

  // append the speed for all cases except paused
  if (!paused)
    string.catprintf("%4d%%", (int)(100 * m_speed_percent + 0.5));

  // display the number of partial updates as well
  int partials = 0;
  screen_device_iterator iter(machine().root_device());
  for (screen_device *screen = iter.first(); screen != NULL;
       screen = iter.next())
    partials += screen->partial_updates();
  if (partials > 1)
    string.catprintf("\n%d partial updates", partials);

  return string;
}

//-------------------------------------------------
//  screenless_update_callback - update generator
//  when there are no screens to drive it
//-------------------------------------------------

void video_manager::screenless_update_callback(void *ptr, int param) {
  // force an update
  frame_update(false);
}

//-------------------------------------------------
//  postload - callback for resetting things after
//  state has been loaded
//-------------------------------------------------

void video_manager::postload() {
}

//-------------------------------------------------
//  effective_autoframeskip - return the effective
//  autoframeskip value, accounting for fast
//  forward
//-------------------------------------------------

inline int video_manager::effective_autoframeskip() const {
  // if we're fast forwarding or paused, autoframeskip is disabled
  if (m_fastforward || machine().paused())
    return false;

  // otherwise, it's up to the user
  return m_auto_frameskip;
}

//-------------------------------------------------
//  effective_frameskip - return the effective
//  frameskip value, accounting for fast
//  forward
//-------------------------------------------------

inline int video_manager::effective_frameskip() const {
  // if we're fast forwarding, use the maximum frameskip
  if (m_fastforward)
    return FRAMESKIP_LEVELS - 1;

  // otherwise, it's up to the user
  return m_frameskip_level;
}

//-------------------------------------------------
//  effective_throttle - return the effective
//  throttle value, accounting for fast
//  forward and user interface
//-------------------------------------------------

inline bool video_manager::effective_throttle() const {
  // if we're paused, or if the UI is active, we always throttle
  if (machine().paused() || machine().ui().is_menu_active())
    return true;

  // if we're fast forwarding, we don't throttle
  if (m_fastforward)
    return false;

  // otherwise, it's up to the user
  return throttled();
}

//-------------------------------------------------
//  original_speed_setting - return the original
//  speed setting
//-------------------------------------------------

inline int video_manager::original_speed_setting() const {
  return machine().options().speed() * 1000.0 + 0.5;
}

//-------------------------------------------------
//  finish_screen_updates - finish updating all
//  the screens
//-------------------------------------------------

bool video_manager::finish_screen_updates() {
  // finish updating the screens
  screen_device_iterator iter(machine().root_device());

  for (screen_device *screen = iter.first(); screen != NULL;
       screen = iter.next())
    screen->update_partial(screen->visible_area().max_y);

  // now add the quads for all the screens
  bool anything_changed = m_output_changed;
  m_output_changed = false;
  for (screen_device *screen = iter.first(); screen != NULL;
       screen = iter.next())
    if (screen->update_quads())
      anything_changed = true;

  // update burn-in state
  if (!machine().paused()) {
    // iterate over screens and update the burnin for the ones that care
    for (screen_device *screen = iter.first(); screen != NULL;
         screen = iter.next())
      screen->update_burnin();
  }

  // draw any crosshairs
  for (screen_device *screen = iter.first(); screen != NULL;
       screen = iter.next())
    crosshair_render(*screen);

  return anything_changed;
}

//-------------------------------------------------
//  update_throttle - throttle to the game's
//  natural speed
//-------------------------------------------------

INT64 realtimeEmulationShift = 0;
extern RakNet::Time emulationStartTime;

void video_manager::update_throttle(attotime emutime) {
  using namespace std;

  // For mamehub we need to do something different
  const attoseconds_t attoseconds_per_tick =
      ATTOSECONDS_PER_SECOND / osd_ticks_per_second();
  {
    bool printed = false;

    while (true) {
      // Get current ticks
      RakNet::Time curTime = RakNet::GetTimeMS() - emulationStartTime;
      if (netClient) {
        curTime = netClient->getCurrentServerTime();
      }
      // cout << "Current time is: " << curTime << endl;
      // osd_ticks_t currentTicks = osd_ticks() - realtimeEmulationShift;

      // Convert ticks to emulation time
      attotime expectedEmulationTime(
          curTime / 1000, // milliseconds to seconds
          (curTime % 1000) *
              ATTOSECONDS_PER_MILLISECOND); // milliseconds to attoseconds
      // currentTicks/osd_ticks_per_second(),
      //(currentTicks%osd_ticks_per_second())*attoseconds_per_tick);

      if (expectedEmulationTime < emutime) {
        if (SKIP_OSD) {
          SKIP_OSD = false;
          cout << "We are caught up "
               << ((emutime - expectedEmulationTime).attoseconds /
                   ATTOSECONDS_PER_MILLISECOND)
               << "ms" << endl;
        }
        if (!printed) {
          printed = true;
        }

        attotime tolerance(0, 16 * ATTOSECONDS_PER_MILLISECOND);
        if ((emutime - expectedEmulationTime) < tolerance) {
          // cout << "Returning " << ((emutime -
          // expectedEmulationTime).attoseconds/ATTOSECONDS_PER_MILLISECOND) <<
          // endl;
          return;
        }

        if (netClient) {
          // Sleep for 15 ms and return
          osd_sleep((osd_ticks_per_second() / 1000) * 15);
          return;
        } else {
          // Sleep the processor 1ms and check again
          osd_sleep((osd_ticks_per_second() / 1000));
          continue;
        }
      } else {
        attotime diffTime = expectedEmulationTime - emutime;

        int msBehind = (diffTime.attoseconds / ATTOSECONDS_PER_MILLISECOND) +
                       diffTime.seconds * 1000;

        if (msBehind > 100 && emutime.seconds > 0) {
          static int lastSecondBehind = 0;
          if (lastSecondBehind < emutime.seconds) {
            cout << "We are behind " << msBehind << "ms.  Skipping video."
                 << endl;
            lastSecondBehind = emutime.seconds;
          }
          SKIP_OSD = true;
        }
        return;
      }
    }
  }
}

void video_manager::rollback(attotime rollbackAmount) {
  osd_ticks_t ticks_per_second = osd_ticks_per_second();
  attoseconds_t attoseconds_per_tick =
      ATTOSECONDS_PER_SECOND / ticks_per_second * m_throttle_rate;
  realtimeEmulationShift -= rollbackAmount.seconds * ticks_per_second;
  realtimeEmulationShift -= rollbackAmount.attoseconds / attoseconds_per_tick;
}

//-------------------------------------------------
//  throttle_until_ticks - spin until the
//  specified target time, calling the OSD code
//  to sleep if possible
//-------------------------------------------------

osd_ticks_t video_manager::throttle_until_ticks(osd_ticks_t target_ticks) {
  // we're allowed to sleep via the OSD code only if we're configured to do so
  // and we're not frameskipping due to autoframeskip, or if we're paused
  bool allowed_to_sleep = false;
  if (machine().options().sleep() &&
      (!effective_autoframeskip() || effective_frameskip() == 0))
    allowed_to_sleep = true;
  if (machine().paused())
    allowed_to_sleep = true;

  // loop until we reach our target
  g_profiler.start(PROFILER_IDLE);
  osd_ticks_t minimum_sleep = osd_ticks_per_second() / 1000;
  osd_ticks_t current_ticks = osd_ticks();
  while (current_ticks < target_ticks) {
    // compute how much time to sleep for, taking into account the average
    // oversleep
    osd_ticks_t delta =
        (target_ticks - current_ticks) * 1000 / (1000 + m_average_oversleep);

    // see if we can sleep
    bool slept = false;
    if (allowed_to_sleep && delta >= minimum_sleep) {
      osd_sleep(delta);
      slept = true;
    }

    // read the new value
    osd_ticks_t new_ticks = osd_ticks();

    // keep some metrics on the sleeping patterns of the OSD layer
    if (slept) {
      // if we overslept, keep an average of the amount
      osd_ticks_t actual_ticks = new_ticks - current_ticks;
      if (actual_ticks > delta) {
        // take 90% of the previous average plus 10% of the new value
        osd_ticks_t oversleep_milliticks =
            1000 * (actual_ticks - delta) / delta;
        m_average_oversleep =
            (m_average_oversleep * 99 + oversleep_milliticks) / 100;

        if (LOG_THROTTLE)
          logerror("Slept for %d ticks, got %d ticks, avgover = %d\n",
                   (int)delta, (int)actual_ticks, (int)m_average_oversleep);
      }
    }
    current_ticks = new_ticks;
  }
  g_profiler.stop();

  return current_ticks;
}

//-------------------------------------------------
//  update_frameskip - update frameskipping
//  counters and periodically update autoframeskip
//-------------------------------------------------

void video_manager::update_frameskip() {
  // if we're throttling and autoframeskip is on, adjust
  if (effective_throttle() && effective_autoframeskip() &&
      m_frameskip_counter == 0) {
    // calibrate the "adjusted speed" based on the target
    double adjusted_speed_percent = m_speed_percent / m_throttle_rate;

    // if we're too fast, attempt to increase the frameskip
    double speed = m_speed * 0.001;
    if (adjusted_speed_percent >= 0.995 * speed) {
      // but only after 3 consecutive frames where we are too fast
      if (++m_frameskip_adjust >= 3) {
        m_frameskip_adjust = 0;
        if (m_frameskip_level > 0)
          m_frameskip_level--;
      }
    }

    // if we're too slow, attempt to increase the frameskip
    else {
      // if below 80% speed, be more aggressive
      if (adjusted_speed_percent < 0.80 * speed)
        m_frameskip_adjust -= (0.90 * speed - m_speed_percent) / 0.05;

      // if we're close, only force it up to frameskip 8
      else if (m_frameskip_level < 8)
        m_frameskip_adjust--;

      // perform the adjustment
      while (m_frameskip_adjust <= -2) {
        m_frameskip_adjust += 2;
        if (m_frameskip_level < MAX_FRAMESKIP)
          m_frameskip_level++;
      }
    }
  }

  // increment the frameskip counter and determine if we will skip the next
  // frame
  m_frameskip_counter = (m_frameskip_counter + 1) % FRAMESKIP_LEVELS;
  m_skipping_this_frame =
      s_skiptable[effective_frameskip()][m_frameskip_counter];
}

//-------------------------------------------------
//  update_refresh_speed - update the m_speed
//  based on the maximum refresh rate supported
//-------------------------------------------------

void video_manager::update_refresh_speed() {
  // only do this if the refreshspeed option is used
  if (machine().options().refresh_speed() && !machine().options().client() &&
      !machine().options().server()) {
    float minrefresh = machine().render().max_update_rate();
    if (minrefresh != 0) {
      // find the screen with the shortest frame period (max refresh rate)
      // note that we first check the token since this can get called before all
      // screens are created
      attoseconds_t min_frame_period = ATTOSECONDS_PER_SECOND;
      screen_device_iterator iter(machine().root_device());
      for (screen_device *screen = iter.first(); screen != NULL;
           screen = iter.next()) {
        attoseconds_t period = screen->frame_period().attoseconds;
        if (period != 0)
          min_frame_period = MIN(min_frame_period, period);
      }

      // compute a target speed as an integral percentage
      // note that we lop 0.25Hz off of the minrefresh when doing the
      // computation to allow for the fact that most refresh rates are not
      // accurate to 10 digits...
      UINT32 target_speed = floor((minrefresh - 0.25f) * 1000.0 /
                                  ATTOSECONDS_TO_HZ(min_frame_period));
      UINT32 original_speed = original_speed_setting();
      target_speed = MIN(target_speed, original_speed);

      // if we changed, log that verbosely
      if (target_speed != m_speed) {
        osd_printf_verbose(
            "Adjusting target speed to %.1f%% (hw=%.2fHz, game=%.2fHz, "
            "adjusted=%.2fHz)\n",
            target_speed / 10.0, minrefresh,
            ATTOSECONDS_TO_HZ(min_frame_period),
            ATTOSECONDS_TO_HZ(min_frame_period * 1000.0 / target_speed));
        m_speed = target_speed;
      }
    }
  }
}

//-------------------------------------------------
//  recompute_speed - recompute the current
//  overall speed; we assume this is called only
//  if we did not skip a frame
//-------------------------------------------------

void video_manager::recompute_speed(const attotime &emutime) {
  // if we don't have a starting time yet, or if we're paused, reset our
  // starting point
  if (m_speed_last_realtime == 0 || machine().paused()) {
    m_speed_last_realtime = osd_ticks();
    m_speed_last_emutime = emutime;
  }

  // if it has been more than the update interval, update the time
  attotime delta_emutime = emutime - m_speed_last_emutime;
  if (delta_emutime > attotime(0, ATTOSECONDS_PER_SPEED_UPDATE)) {
    // convert from ticks to attoseconds
    osd_ticks_t realtime = osd_ticks();
    osd_ticks_t delta_realtime = realtime - m_speed_last_realtime;
    osd_ticks_t tps = osd_ticks_per_second();
    m_speed_percent =
        delta_emutime.as_double() * (double)tps / (double)delta_realtime;

    // remember the last times
    m_speed_last_realtime = realtime;
    m_speed_last_emutime = emutime;

    // if we're throttled, this time period counts for overall speed; otherwise,
    // we reset the counter
    if (!m_fastforward)
      m_overall_valid_counter++;
    else
      m_overall_valid_counter = 0;

    // if we've had at least 4 consecutive valid periods, accumulate stats
    if (m_overall_valid_counter >= 4) {
      m_overall_real_ticks += delta_realtime;
      while (m_overall_real_ticks >= tps) {
        m_overall_real_ticks -= tps;
        m_overall_real_seconds++;
      }
      m_overall_emutime += delta_emutime;
    }
  }

  // if we're past the "time-to-execute" requested, signal an exit
  if (m_seconds_to_run != 0 && emutime.seconds >= m_seconds_to_run) {
#ifdef MAME_DEBUG
    if (g_tagmap_counter_enabled) {
      g_tagmap_counter_enabled = false;
      if (*(machine().options().command()) == 0)
        osd_printf_info("%d tagmap lookups\n", g_tagmap_finds);
    }
#endif

    // printf("Scheduled exit at %f\n", emutime.as_double());
    // schedule our demise
    machine().schedule_exit();
  }
}

//-------------------------------------------------
//  toggle_throttle
//-------------------------------------------------

void video_manager::toggle_throttle() { set_throttled(!throttled()); }
