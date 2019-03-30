// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    machine.c

    Controls execution of the core MAME system.

****************************************************************************

    Since there has been confusion in the past over the order of
    initialization and other such things, here it is, all spelled out
    as of January, 2008:

    main()
        - does platform-specific init
        - calls mame_execute() [mame.c]

        mame_execute() [mame.c]
            - calls mame_validitychecks() [validity.c] to perform validity checks on all compiled drivers
            - begins resource tracking (level 1)
            - calls create_machine [mame.c] to initialize the running_machine structure
            - calls init_machine() [mame.c]

            init_machine() [mame.c]
                - calls fileio_init() [fileio.c] to initialize file I/O info
                - calls config_init() [config.c] to initialize configuration system
                - calls input_init() [input.c] to initialize the input system
                - calls output_init() [output.c] to initialize the output system
                - calls state_init() [state.c] to initialize save state system
                - calls state_save_allow_registration() [state.c] to allow registrations
                - calls palette_init() [palette.c] to initialize palette system
                - calls render_init() [render.c] to initialize the rendering system
                - calls ui_init() [ui.c] to initialize the user interface
                - calls generic_machine_init() [machine/generic.c] to initialize generic machine structures
                - calls timer_init() [timer.c] to reset the timer system
                - calls osd_init() [osdepend.h] to do platform-specific initialization
                - calls input_port_init() [inptport.c] to set up the input ports
                - calls rom_init() [romload.c] to load the game's ROMs
                - calls memory_init() [memory.c] to process the game's memory maps
                - calls the driver's DRIVER_INIT callback
                - calls device_list_start() [devintrf.c] to start any devices
                - calls video_init() [video.c] to start the video system
                - calls tilemap_init() [tilemap.c] to start the tilemap system
                - calls crosshair_init() [crsshair.c] to configure the crosshairs
                - calls sound_init() [sound.c] to start the audio system
                - calls debugger_init() [debugger.c] to set up the debugger
                - calls the driver's MACHINE_START, SOUND_START, and VIDEO_START callbacks
                - calls cheat_init() [cheat.c] to initialize the cheat system
                - calls image_init() [image.c] to initialize the image system

            - calls config_load_settings() [config.c] to load the configuration file
            - calls nvram_load [machine/generic.c] to load NVRAM
            - calls ui_display_startup_screens() [ui.c] to display the startup screens
            - begins resource tracking (level 2)
            - calls soft_reset() [mame.c] to reset all systems

                -------------------( at this point, we're up and running )----------------------

            - calls scheduler->timeslice() [schedule.c] over and over until we exit
            - ends resource tracking (level 2), freeing all auto_mallocs and timers
            - calls the nvram_save() [machine/generic.c] to save NVRAM
            - calls config_save_settings() [config.c] to save the game's configuration
            - calls all registered exit routines [mame.c]
            - ends resource tracking (level 1), freeing all auto_mallocs and timers

        - exits the program

***************************************************************************/

#include "nsm_common.h"
#include "nsm_server.h"
#include "nsm_client.h"

#include <boost/circular_buffer.hpp>

#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "emuconfig.h"
#include "debugger.h"
#include "render.h"
#include "cheat.h"
#include "ui/selgame.h"
#include "ui/ui.h"
#include "uiinput.h"
#include "crsshair.h"
#include "validity.h"
#include "unzip.h"
#include "debug/debugcon.h"
#include "debug/debugvw.h"

#include <time.h>

#ifdef SDLMAME_EMSCRIPTEN
#include <emscripten.h>

void js_set_main_loop(running_machine * machine);
#endif


using namespace std;
using namespace nsm;
using boost::circular_buffer;

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// a giant string buffer for temporary strings
static char giant_string_buffer[65536] = { 0 };



//**************************************************************************
//  RUNNING MACHINE
//**************************************************************************

osd_interface &running_machine::osd() const
{
  return m_manager.osd();
}

//-------------------------------------------------
//  running_machine - constructor
//-------------------------------------------------

running_machine::running_machine(const machine_config &_config, machine_manager &manager)
  : firstcpu(NULL),
    primary_screen(NULL),
    debug_flags(0),
    romload_data(NULL),
    ui_input_data(NULL),
    debugcpu_data(NULL),
    generic_machine_data(NULL),
    m_config(_config),
    m_system(_config.gamedrv()),
    m_manager(manager),
    m_current_phase(MACHINE_PHASE_PREINIT),
    m_paused(false),
    m_hard_reset_pending(false),
    m_exit_pending(false),
    m_soft_reset_timer(NULL),
    m_rand_seed(0x9d14abd7),
    m_ui_active(_config.options().ui_active()),
    m_basename(_config.gamedrv().name),
    m_sample_rate(_config.options().sample_rate()),
    m_saveload_schedule(SLS_NONE),
    m_saveload_schedule_time(attotime::zero),
    m_saveload_searchpath(NULL),

    m_save(*this),
    m_memory(*this),
    m_ioport(*this),
    m_parameters(*this),
    m_scheduler(*this)
{
  memset(&m_base_time, 0, sizeof(m_base_time));

  // set the machine on all devices
  device_iterator iter(root_device());
  for (device_t *device = iter.first(); device != NULL; device = iter.next())
    device->set_machine(*this);

  // find devices
  for (device_t *device = iter.first(); device != NULL; device = iter.next())
    if (dynamic_cast<cpu_device *>(device) != NULL)
    {
      firstcpu = downcast<cpu_device *>(device);
      break;
    }
  screen_device_iterator screeniter(root_device());
  primary_screen = screeniter.first();

  // fetch core options
  if (options().debug())
    debug_flags = (DEBUG_FLAG_ENABLED | DEBUG_FLAG_CALL_HOOK) | (DEBUG_FLAG_OSD_ENABLED);
}


//-------------------------------------------------
//  ~running_machine - destructor
//-------------------------------------------------

running_machine::~running_machine()
{
}

//-------------------------------------------------
//  describe_context - return a string describing
//  which device is currently executing and its
//  PC
//-------------------------------------------------

const char *running_machine::describe_context()
{
  device_execute_interface *executing = m_scheduler.currently_executing();
  if (executing != NULL)
  {
    cpu_device *cpu = dynamic_cast<cpu_device *>(&executing->device());
    if (cpu != NULL)
      m_context.printf("'%s' (%s)", cpu->tag(), core_i64_format(cpu->pc(), cpu->space(AS_PROGRAM).logaddrchars(), cpu->is_octal()));
  }
  else
    m_context.cpy("(no context)");

  return m_context;
}

TIMER_CALLBACK_MEMBER(running_machine::autoboot_callback)
{
}

//-------------------------------------------------
//  start - initialize the emulated machine
//-------------------------------------------------

void running_machine::start()
{
  // initialize basic can't-fail systems here
  config_init(*this);
  m_input.reset(global_alloc(input_manager(*this)));
  output_init(*this);
  m_render.reset(global_alloc(render_manager(*this)));
  generic_machine_init(*this);

  // allocate a soft_reset timer
  m_soft_reset_timer = m_scheduler.timer_alloc(timer_expired_delegate(FUNC(running_machine::soft_reset), this));

  // init the osd layer
  m_manager.osd().init(*this);

  // create the video manager
  m_video.reset(global_alloc(video_manager(*this)));
  m_ui.reset(global_alloc(ui_manager(*this)));

  if(options().server()||options().client())
  {
    // Make the base time a constant for MAMEHub consistency
    m_base_time = 946080000ULL;
  }
  else
  {
  // initialize the base time (needed for doing record/playback)
  ::time(&m_base_time);
  }

  // initialize the input system and input ports for the game
  // this must be done before memory_init in order to allow specifying
  // callbacks based on input port tags
  time_t newbase = m_ioport.initialize();
  if(!options().server() && !options().client() && newbase != 0)
  {
    m_base_time = newbase;
  }

  // intialize UI input
  ui_input_init(*this);

  // initialize the streams engine before the sound devices start
  m_sound.reset(global_alloc(sound_manager(*this)));

  // first load ROMs, then populate memory, and finally initialize CPUs
  // these operations must proceed in this order
  rom_init(*this);
  m_memory.initialize();

  // initialize the watchdog
  m_watchdog_timer = m_scheduler.timer_alloc(timer_expired_delegate(FUNC(running_machine::watchdog_fired), this));
  if (config().m_watchdog_vblank_count != 0 && primary_screen != NULL)
    primary_screen->register_vblank_callback(vblank_state_delegate(FUNC(running_machine::watchdog_vblank), this));
  save().save_item(NAME(m_watchdog_enabled));
  save().save_item(NAME(m_watchdog_counter));

  // save the random seed or save states might be broken in drivers that use the rand() method
  save().save_item(NAME(m_rand_seed));

  // initialize image devices
  image_init(*this);
  m_tilemap.reset(global_alloc(tilemap_manager(*this)));
  crosshair_init(*this);
  network_init(*this);

  // initialize the debugger
  if ((debug_flags & DEBUG_FLAG_ENABLED) != 0)
    debugger_init(*this);

  // call the game driver's init function
  // this is where decryption is done and memory maps are altered
  // so this location in the init order is important
  ui().set_startup_text("Initializing...", true);

  // register callbacks for the devices, then start them
  add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(running_machine::reset_all_devices), this));
  add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(running_machine::stop_all_devices), this));
  save().register_presave(save_prepost_delegate(FUNC(running_machine::presave_all_devices), this));
  start_all_devices();
  save().register_postload(save_prepost_delegate(FUNC(running_machine::postload_all_devices), this));

  m_machine_time = attotime(0,0);
  isRollback = false;

  // if we're coming in with a savegame request, process it now
  const char *savegame = options().state();
  if (savegame[0] != 0)
    schedule_load(savegame);

  // if we're in autosave mode, schedule a load
  else if (options().autosave() && (m_system.flags & GAME_SUPPORTS_SAVE) != 0)
    schedule_load("auto");

  // set up the cheat engine
  m_cheat.reset(global_alloc(cheat_manager(*this)));

  // allocate autoboot timer
  m_autoboot_timer = scheduler().timer_alloc(timer_expired_delegate(FUNC(running_machine::autoboot_callback), this));

  manager().update_machine();
}


//-------------------------------------------------
//  add_dynamic_device - dynamically add a device
//-------------------------------------------------

device_t &running_machine::add_dynamic_device(device_t &owner, device_type type, const char *tag, UINT32 clock)
{
  // add the device in a standard manner
  device_t *device = const_cast<machine_config &>(m_config).device_add(&owner, tag, type, clock);

  // notify this device and all its subdevices that they are now configured
  device_iterator iter(root_device());
  for (device_t *device = iter.first(); device != NULL; device = iter.next())
    if (!device->configured())
      device->config_complete();
  return *device;
}

extern char CORE_SEARCH_PATH[4096];
extern int doCatchup;
bool catchingUp = false;

bool doRollback = false;
attotime rollbackTime;

//-------------------------------------------------
//  run - execute the machine
//-------------------------------------------------

extern list< ChatLog > chatLogs;
extern circular_buffer<std::pair<attotime,InputState> > playerInputData[MAX_PLAYERS];

void running_machine::processNetworkBuffer(PeerInputData *inputData,int peerID)
{
  if(inputData == NULL)
  {
    return;
  }

  if(inputData->inputtype() == PeerInputData::INPUT)
  {
    //printf("GOT INPUT\n");
    attotime tmptime(inputData->time().seconds(), inputData->time().attoseconds());

    for(int a=0;a<inputData->inputstate().players_size();a++) {
      //int player = inputData->inputstate().players(a);
      //cout << "Peer " << peerID << " has input for player " << inputData->inputstate().players(a) << " at time " << tmptime.seconds << "." << tmptime.attoseconds << endl;
      circular_buffer<pair<attotime,InputState> > &onePlayerInputData = playerInputData[inputData->inputstate().players(a)];
      if(onePlayerInputData.empty()) {
        onePlayerInputData.insert(onePlayerInputData.begin(),pair<attotime,InputState>(tmptime,inputData->inputstate()));
      } else {
        //TODO: Re-think this and clean it up
        if (netCommon->isRollback()) {
          circular_buffer<pair<attotime,InputState> >::reverse_iterator it = onePlayerInputData.rbegin();
          attotime lastInputTime = it->first;
          if (lastInputTime == tmptime) {
            return;
          }
          if (lastInputTime > tmptime) {
            cout << "unexpected time " << lastInputTime << " " << tmptime << "\n";
            exit(1);
          }

          // Check if the input states are equal.
          std::string s1string;
          std::string s2string;
          //cout << "First serialize\n";
          ::nsm::InputState s1 = inputData->inputstate();
          ::nsm::InputState s2 = it->second;
          s1.set_framecount(0);
          s2.set_framecount(0);
          s1.SerializeToString(&s1string);
          //cout << "Second serialize\n";
          s2.SerializeToString(&s2string);
          if (s1string != s2string) {
            attotime currentMachineTime = machine_time();
            if (currentMachineTime > tmptime) {
              if (doRollback) {
                if (rollbackTime > tmptime) {
                  // Roll back further
                  rollbackTime = tmptime;
                  cout << "Rolling back further from " << currentMachineTime << " to " << tmptime << endl;
                }
              } else {
                // Roll back
                cout << "Rolling back from " << currentMachineTime << " to " << tmptime << endl;
                rollbackTime = tmptime;
                doRollback=true;
              }
            }
          }

          onePlayerInputData.push_back(make_pair(tmptime,inputData->inputstate()));
        } else { // no rollback
          for(circular_buffer<pair<attotime,InputState> >::reverse_iterator it = onePlayerInputData.rbegin();
              it != onePlayerInputData.rend();
              it++) {
            //cout << "IN INPUT LOOP\n";
            if(it->first < tmptime) {
              onePlayerInputData.insert(
                it.base(), // NOTE: base() returns the iterator 1 position in the past
                pair<attotime,InputState>(tmptime,inputData->inputstate()));
              break;
            } else if(it->first == tmptime) {
              //TODO: If two peers send inputs at the same time for the same player, break ties with peer id.
              break;
            } else if(it == onePlayerInputData.rend()) {
              onePlayerInputData.insert(
                onePlayerInputData.begin(),
                pair<attotime,InputState>(tmptime,inputData->inputstate()));
              break;
            }
          }
        }
      }
    }
  }
  else if(inputData->inputtype() == PeerInputData::CHAT)
  {
    string buffer = inputData->inputbuffer();
    cout << "GOT CHAT " << buffer << endl;
    char buf[4096];
    sprintf(buf,"%s: %s",netCommon->getPeerNameFromID(peerID).c_str(),buffer.c_str());
    astring chatAString = astring(buf);
    //Figure out the index of who spoke and send that
    chatLogs.push_back(ChatLog(peerID,::time(NULL),chatAString));
  }
  else if(inputData->inputtype() == PeerInputData::FORCE_VALUE)
  {
    const string &buffer = inputData->inputbuffer();
    cout << "FORCING VALUE\n";
    int blockIndex,memoryStart,memorySize,value;
    unsigned char ramRegion,memoryMask;
    memcpy(&ramRegion,&buffer[0]+1,sizeof(int));
    memcpy(&blockIndex,&buffer[0]+2,sizeof(int));
    memcpy(&memoryStart,&buffer[0]+6,sizeof(int));
    memcpy(&memorySize,&buffer[0]+10,sizeof(int));
    memcpy(&memoryMask,&buffer[0]+14,sizeof(unsigned char));
    memcpy(&value,&buffer[0]+15,sizeof(int));
    // New force
    netCommon->forceLocation(BlockValueLocation(ramRegion,blockIndex,memoryStart,memorySize,memoryMask),value);
    ui().popup_time(3, "Server created new cheat");
  }
  else
  {
    printf("UNKNOWN INPUT BUFFER PACKET!!!\n");
  }
}

RakNet::Time emulationStartTime=0;
UINT64 inputFrameNumber = 0;
UINT64 trackFrameNumber = 0;
attotime largestEmulationTime(0,0);
vector<int> peerIDs;

void running_machine::mainLoop()
{
  attotime timeBefore = m_scheduler.time();
  attotime machineTimeBefore = machine_time();

  if (!m_paused)
  {
    // execute CPUs if not paused
    m_scheduler.timeslice();
  }
  else
  {
    // otherwise, just pump video updates through
    m_video->frame_update();
  }

  attotime timeAfter = m_scheduler.time();
  // cout << "time: " << timeAfter.as_double() << endl;

  if (timeBefore > timeAfter)
  {
    cout << "OOPS! WE WENT BACK IN TIME SOMEHOW\n";
    exit(1);
  }

  if (timeAfter > largestEmulationTime)
  {
    largestEmulationTime = timeAfter;
    catchingUp = false;
  }

  bool timePassed = (timeBefore != timeAfter);
  bool secondPassed = false;
  bool tenthSecondPassed = false;

  if (timePassed)
  {
    //cout << "TIME MOVED FROM " << timeBefore << " TO " << timeAfter << endl;
    m_machine_time += (timeAfter - timeBefore);
    attotime machineTimeAfter = machine_time();
    secondPassed = machineTimeBefore.seconds != machineTimeAfter.seconds;
    tenthSecondPassed = secondPassed ||
      ((machineTimeBefore.attoseconds/(ATTOSECONDS_PER_SECOND/10ULL)) != (machineTimeAfter.attoseconds/(ATTOSECONDS_PER_SECOND/10ULL)));

    if (netCommon)
    {
      // Process any remaining packets.
      if(!netCommon->update(this))
      {
        cout << "NETWORK FAILED\n";
        ::exit(1);
      }

      netCommon->getPeerIDs(peerIDs);
      
      for (int a = 0; a<peerIDs.size(); a++)
      {
        while(true)
        {
          nsm::PeerInputData input = netCommon->popInput(peerIDs[a]);
          if (!input.has_time())
          {
            break;
          }

          processNetworkBuffer(&input, peerIDs[a]);
        }
      }
    }
  }

  //printf("EMULATION FINISHED\n");
  static int lastSyncSecond = 0;
  static int firstTimeAtSecond = 0;

  if(m_machine_time.seconds > 0 && m_scheduler.can_save() && timePassed && !firstTimeAtSecond)
  {
    firstTimeAtSecond = 1;
    if (netServer)
    {
      // Initial sync
      netServer->sync(this);
    }

    if (netClient)
    {
      // Load initial data
      netClient->createInitialBlocks(this);
    }
  }
  else if(m_machine_time.seconds > 0 && m_scheduler.can_save() && timePassed)
  {
    if(
      netServer &&
      lastSyncSecond != m_machine_time.seconds &&
      netServer->getSecondsBetweenSync()>0 &&
      !netCommon->isRollback() &&
      (m_machine_time.seconds%netServer->getSecondsBetweenSync())==0
      )
    {
      lastSyncSecond = m_machine_time.seconds;
      printf("SERVER SYNC AT TIME: %d\n",int(::time(NULL)));
      if (!m_scheduler.can_save())
      {
        printf("ANONYMOUS TIMER! COULD NOT DO FULL SYNC\n");
      }
      else
      {
        netServer->sync(this);
        //nvram_save(*this);
        cout << "RAND/TIME AT SYNC: " << m_rand_seed << ' ' << machine_time().seconds << '.' << machine_time().attoseconds << endl;
      }
    }

    if(
      netClient &&
      lastSyncSecond != m_machine_time.seconds &&
      netClient->getSecondsBetweenSync()>0 &&
      !netCommon->isRollback() &&
      (m_machine_time.seconds%netClient->getSecondsBetweenSync())==0
      )
    {
      lastSyncSecond = m_machine_time.seconds;
      if (!m_scheduler.can_save())
      {
        printf("ANONYMOUS TIMER! THIS COULD BE BAD (BUT HOPEFULLY ISN'T)\n");
      }
      else
      {
        //The client should update sync check just in case the server didn't have an anon timer
        m_save.dispatch_presave();
        netClient->updateSyncCheck();
        cout << "RAND/TIME AT SYNC: " << m_rand_seed << ' ' << machine_time().seconds << '.' << machine_time().attoseconds << endl;
        m_save.dispatch_postload();
      }
    }

    static clock_t lastSyncTime = clock();
    if(netCommon)
    {
      //printf("IN NET LOOP\n");
      lastSyncTime = clock();
      //TODO: Fix forces
      //netCommon->updateForces(getRawMemoryRegions());
      if(netServer)
      {
        netServer->update(this);
      }

      if(netClient)
      {
        //printf("IN CLIENT LOOP\n");
        pair<bool,bool> survivedAndGotSync;
        survivedAndGotSync.first = netClient->update(this);
        //printf("CLIENT UPDATED\n");
        if(survivedAndGotSync.first==false)
        {
          m_exit_pending = true;
          return;
        }

        // Don't try to resync on the same frame that you created
        // the sync check.
        if (lastSyncSecond != m_machine_time.seconds) {
          bool gotSync = netClient->sync(this);
          if(gotSync)
          {
            if (!m_scheduler.can_save())
            {
              printf("ANONYMOUS TIMER! THIS COULD BE BAD (BUT HOPEFULLY ISN'T)\n");
            }
            cout << "GOT SYNC FROM SERVER\n";
            cout << "RAND/TIME AT SYNC: " << m_rand_seed << ' ' << m_base_time << endl;
          }
        }
      }
    }
  }

  return;

  // handle save/load
  if (timePassed && m_saveload_schedule != SLS_NONE)
  {
    handle_saveload();
  }
  else if (timePassed && netCommon && netCommon->isRollback())
  {
    if (trackFrameNumber>0 && m_scheduler.can_save() && trackFrameNumber != inputFrameNumber)
    {
      isRollback = true;
      immediate_save("test");
      cout << "SAVING AT TIME " << m_machine_time << endl;
      isRollback = false;
      trackFrameNumber=0;
    }

    if(m_machine_time.seconds>0 && m_scheduler.can_save() && tenthSecondPassed)
    {
      cout << "Tenth second passed: " << m_machine_time << endl;
      if (secondPassed)
      {
        cout << "Second passed" << endl;
      }
      trackFrameNumber = inputFrameNumber;

      /*
      static int biggestSecond=0;
      if (time().seconds > biggestSecond) {
        biggestSecond = time().seconds;
        if (biggestSecond>10) {
          cout << "Rolling back" << endl;
          isRollback = true;
          immediate_load("test");
          isRollback = false;
          catchingUp = true;
        }
      }
      */
    }
    
    if(m_machine_time.seconds>0 && m_scheduler.can_save()) {
      if (doRollback) {
        doRollback = false;
        isRollback = true;
        immediate_load("test");
        isRollback = false;
        catchingUp = true;
      }
    }
  }
}

int running_machine::run(bool firstrun)
{
  //JJG: Add media path to mess search path
  strcpy(CORE_SEARCH_PATH,options().media_path());

  int error = MAMERR_NONE;

  // move to the init phase
  m_current_phase = MACHINE_PHASE_INIT;

  // if we have a logfile, set up the callback
  if (options().log())
  {
    m_logfile.reset(global_alloc(emu_file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS)));
    file_error filerr = m_logfile->open("error.log");
    assert_always(filerr == FILERR_NONE, "unable to open log file");
    add_logerror_callback(logfile_callback);
  }

  // then finish setting up our local machine
  start();

  // load the configuration settings and NVRAM
  bool settingsloaded = config_load_settings(*this);

  if((system().flags & GAME_SUPPORTS_SAVE) == 0)
  {
    ui().popup_time(10, "This game does not have complete save state support, desyncs may not be resolved correctly.");
  }

  //After loading config but before loading nvram, initialize the network
  if(netServer)
  {
    netServer->setSecondsBetweenSync(options().secondsBetweenSync());

    if(!netServer->initializeConnection())
    {
      return MAMERR_NETWORK;
    }
  }

  if(netClient)
  {
    /* specify the filename to save or load */
    //set_saveload_filename(machine, "1");
    //handle_load(machine);
    //if(netClient->getSecondsBetweenSync())
    //doPreSave(this);
    bool retval = netClient->initializeConnection(
      (unsigned short)options().selfport(),
      options().hostname(),
      (unsigned short)options().port(),
      this
      );
    printf("LOADED CLIENT\n");
    cout << "RAND/TIME AT INITIAL SYNC: " << m_rand_seed << ' ' << m_base_time << endl;
    if(!retval)
    {
      exit(MAMERR_NETWORK);
    }
    //if(netClient->getSecondsBetweenSync())
    //doPostLoad(this);
  }

  // disallow save state registrations starting here.
  // Don't do it earlier, config load can create network
  // devices with timers.
  m_save.allow_registration(false);

  nvram_load();
  sound().ui_mute(false);

  // initialize ui lists
  ui().initialize(*this);

  // display the startup screens
  ui().display_startup_screens(firstrun, !settingsloaded);

  // perform a soft reset -- this takes us to the running phase
  soft_reset();

#ifdef MAME_DEBUG
  g_tagmap_finds = 0;
  if (strcmp(config().m_gamedrv.name, "___empty") != 0)
    g_tagmap_counter_enabled = true;
#endif
  // handle initial load
  if (m_saveload_schedule != SLS_NONE)
    handle_saveload();

  printf("SOFT RESET FINISHED\n");

  emulationStartTime = RakNet::GetTimeMS();

  m_hard_reset_pending = false;
  //break out to our async javascript loop and halt
  js_set_main_loop(this);
  return error;
}


//-------------------------------------------------
//  schedule_exit - schedule a clean exit
//-------------------------------------------------

void running_machine::schedule_exit()
{
  m_exit_pending = true;

  // if we're executing, abort out immediately
  m_scheduler.eat_all_cycles();

#ifdef MAME_DEBUG
  if (g_tagmap_counter_enabled)
  {
    g_tagmap_counter_enabled = false;
    if (*(options().command()) == 0)
      osd_printf_info("%d tagmap lookups\n", g_tagmap_finds);
  }
#endif

  // if we're autosaving on exit, schedule a save as well
  if (options().autosave() && (m_system.flags & GAME_SUPPORTS_SAVE) && this->time() > attotime::zero)
    schedule_save("auto");
}


//-------------------------------------------------
//  schedule_hard_reset - schedule a hard-reset of
//  the machine
//-------------------------------------------------

void running_machine::schedule_hard_reset()
{
  m_hard_reset_pending = true;

  // if we're executing, abort out immediately
  m_scheduler.eat_all_cycles();
}


//-------------------------------------------------
//  schedule_soft_reset - schedule a soft-reset of
//  the system
//-------------------------------------------------

void running_machine::schedule_soft_reset()
{
  m_soft_reset_timer->adjust(attotime::zero);

  // we can't be paused since the timer needs to fire
  resume();

  // if we're executing, abort out immediately
  m_scheduler.eat_all_cycles();
}


//-------------------------------------------------
//  get_statename - allow to specify a subfolder of
//  the state directory for state loading/saving,
//  very useful for MESS and consoles or computers
//  where you can have separate folders for diff
//  software
//-------------------------------------------------

astring running_machine::get_statename(const char *option)
{
  astring statename_str("");
  if (option == NULL || option[0] == 0)
    statename_str.cpy("%g");
  else
    statename_str.cpy(option);

  // strip any extension in the provided statename
  int index = statename_str.rchr(0, '.');
  if (index != -1)
    statename_str.substr(0, index);

  // handle %d in the template (for image devices)
  astring statename_dev("%d_");
  int pos = statename_str.find(0, statename_dev);

  if (pos != -1)
  {
    // if more %d are found, revert to default and ignore them all
    if (statename_str.find(pos + 3, statename_dev) != -1)
      statename_str.cpy("%g");
    // else if there is a single %d, try to create the correct snapname
    else
    {
      int name_found = 0;

      // find length of the device name
      int end1 = statename_str.find(pos + 3, "/");
      int end2 = statename_str.find(pos + 3, "%");
      int end = -1;

      if ((end1 != -1) && (end2 != -1))
        end = MIN(end1, end2);
      else if (end1 != -1)
        end = end1;
      else if (end2 != -1)
        end = end2;
      else
        end = statename_str.len();

      if (end - pos < 3)
        fatalerror("Something very wrong is going on!!!\n");

      // copy the device name to an astring
      astring devname_str;
      devname_str.cpysubstr(statename_str, pos + 3, end - pos - 3);
      //printf("check template: %s\n", devname_str.cstr());

      // verify that there is such a device for this system
      image_interface_iterator iter(root_device());
      for (device_image_interface *image = iter.first(); image != NULL; image = iter.next())
      {
        // get the device name
        astring tempdevname(image->brief_instance_name());
        //printf("check device: %s\n", tempdevname.cstr());

        if (devname_str.cmp(tempdevname) == 0)
        {
          // verify that such a device has an image mounted
          if (image->basename_noext() != NULL)
          {
            astring filename(image->basename_noext());

            // setup snapname and remove the %d_
            statename_str.replace(0, devname_str, filename);
            statename_str.del(pos, 3);
            //printf("check image: %s\n", filename.cstr());

            name_found = 1;
          }
        }
      }

      // or fallback to default
      if (name_found == 0)
        statename_str.cpy("%g");
    }
  }

  // substitute path and gamename up front
  statename_str.replace(0, "/", PATH_SEPARATOR);
  statename_str.replace(0, "%g", basename());

  return statename_str;
}

//-------------------------------------------------
//  set_saveload_filename - specifies the filename
//  for state loading/saving
//-------------------------------------------------

void running_machine::set_saveload_filename(const char *filename)
{
  // free any existing request and allocate a copy of the requested name
  if (osd_is_absolute_path(filename))
  {
    m_saveload_searchpath = NULL;
    m_saveload_pending_file.cpy(filename);
  }
  else
  {
    m_saveload_searchpath = options().state_directory();
    // take into account the statename option
    const char *stateopt = options().state_name();
    astring statename = get_statename(stateopt);
    m_saveload_pending_file.cpy(statename.cstr()).cat(PATH_SEPARATOR).cat(filename).cat(".sta");
  }
}


//-------------------------------------------------
//  schedule_save - schedule a save to occur as
//  soon as possible
//-------------------------------------------------

void running_machine::schedule_save(const char *filename)
{
  // specify the filename to save or load
  set_saveload_filename(filename);

  // note the start time and set a timer for the next timeslice to actually schedule it
  m_saveload_schedule = SLS_SAVE;
  m_saveload_schedule_time = this->time();

  // we can't be paused since we need to clear out anonymous timers
  resume();
}


//-------------------------------------------------
//  immediate_save - save state.
//-------------------------------------------------

void running_machine::immediate_save(const char *filename)
{
  // specify the filename to save or load
  set_saveload_filename(filename);

  // set up some parameters for handle_saveload()
  m_saveload_schedule = SLS_SAVE;
  m_saveload_schedule_time = this->time();

  // jump right into the save, anonymous timers can't hurt us!
  handle_saveload();
}


//-------------------------------------------------
//  schedule_load - schedule a load to occur as
//  soon as possible
//-------------------------------------------------

void running_machine::schedule_load(const char *filename)
{
  // specify the filename to save or load
  set_saveload_filename(filename);

  // note the start time and set a timer for the next timeslice to actually schedule it
  m_saveload_schedule = SLS_LOAD;
  m_saveload_schedule_time = this->time();

  // we can't be paused since we need to clear out anonymous timers
  resume();
}


//-------------------------------------------------
//  immediate_load - load state.
//-------------------------------------------------

void running_machine::immediate_load(const char *filename)
{
  // specify the filename to save or load
  set_saveload_filename(filename);

  // set up some parameters for handle_saveload()
  m_saveload_schedule = SLS_LOAD;
  m_saveload_schedule_time = this->time();

  // jump right into the load, anonymous timers can't hurt us
  handle_saveload();
}


//-------------------------------------------------
//  pause - pause the system
//-------------------------------------------------

void running_machine::pause()
{
  if(netCommon)
  {
    //Can't pause during netplay
    return;
  }
  // ignore if nothing has changed
  if (m_paused)
    return;
  m_paused = true;

  // call the callbacks
  call_notifiers(MACHINE_NOTIFY_PAUSE);
}


//-------------------------------------------------
//  resume - resume the system
//-------------------------------------------------

void running_machine::resume()
{
  // ignore if nothing has changed
  if (!m_paused)
    return;
  m_paused = false;

  // call the callbacks
  call_notifiers(MACHINE_NOTIFY_RESUME);
}


//-------------------------------------------------
//  toggle_pause - toggles the pause state
//-------------------------------------------------

void running_machine::toggle_pause()
{
  if (paused())
    resume();
  else
    pause();
}


//-------------------------------------------------
//  add_notifier - add a notifier of the
//  given type
//-------------------------------------------------

void running_machine::add_notifier(machine_notification event, machine_notify_delegate callback)
{
  assert_always(m_current_phase == MACHINE_PHASE_INIT, "Can only call add_notifier at init time!");

  // exit notifiers are added to the head, and executed in reverse order
  if (event == MACHINE_NOTIFY_EXIT)
    m_notifier_list[event].prepend(*global_alloc(notifier_callback_item(callback)));

  // all other notifiers are added to the tail, and executed in the order registered
  else
    m_notifier_list[event].append(*global_alloc(notifier_callback_item(callback)));
}


//-------------------------------------------------
//  add_logerror_callback - adds a callback to be
//  called on logerror()
//-------------------------------------------------

void running_machine::add_logerror_callback(logerror_callback callback)
{
  assert_always(m_current_phase == MACHINE_PHASE_INIT, "Can only call add_logerror_callback at init time!");
  m_logerror_list.append(*global_alloc(logerror_callback_item(callback)));
}


//-------------------------------------------------
//  vlogerror - vprintf-style error logging
//-------------------------------------------------

void CLIB_DECL running_machine::vlogerror(const char *format, va_list args)
{
  // process only if there is a target
  if (m_logerror_list.first() != NULL)
  {
    g_profiler.start(PROFILER_LOGERROR);

    // dump to the buffer
    vsnprintf(giant_string_buffer, ARRAY_LENGTH(giant_string_buffer), format, args);

    // log to all callbacks
    for (logerror_callback_item *cb = m_logerror_list.first(); cb != NULL; cb = cb->next())
      (*cb->m_func)(*this, giant_string_buffer);

    g_profiler.stop();
  }
}


//-------------------------------------------------
//  base_datetime - retrieve the time of the host
//  system; useful for RTC implementations
//-------------------------------------------------

void running_machine::base_datetime(system_time &systime)
{
  systime.set(m_base_time);
}


//-------------------------------------------------
//  current_datetime - retrieve the current time
//  (offset by the base); useful for RTC
//  implementations
//-------------------------------------------------

void running_machine::current_datetime(system_time &systime)
{
  systime.set(m_base_time + this->time().seconds);
}


//-------------------------------------------------
//  rand - standardized random numbers
//-------------------------------------------------

UINT32 running_machine::rand()
{
  m_rand_seed = 1664525 * m_rand_seed + 1013904223;

  // return rotated by 16 bits; the low bits have a short period
  // and are frequently used
  return (m_rand_seed >> 16) | (m_rand_seed << 16);
}


//-------------------------------------------------
//  call_notifiers - call notifiers of the given
//  type
//-------------------------------------------------

void running_machine::call_notifiers(machine_notification which)
{
  for (notifier_callback_item *cb = m_notifier_list[which].first(); cb != NULL; cb = cb->next())
    cb->m_func();
}


//-------------------------------------------------
//  handle_saveload - attempt to perform a save
//  or load
//-------------------------------------------------

const int MAX_STATES = 10 * 5;
pair<attotime, vector<unsigned char> > states[MAX_STATES];
int onState = 0;

void running_machine::handle_saveload()
{
  if (!m_scheduler.can_save()) {
    throw emu_fatalerror("CANNOT SAVE!");
  }

  UINT32 openflags = (m_saveload_schedule == SLS_LOAD) ? OPEN_FLAG_READ : (OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
  const char *opnamed = (m_saveload_schedule == SLS_LOAD) ? "loaded" : "saved";
  const char *opname = (m_saveload_schedule == SLS_LOAD) ? "load" : "save";
  file_error filerr = FILERR_NONE;
  int nextBestState = -1;
  int bestState = -1;

  // if no name, bail
  emu_file file(m_saveload_searchpath, openflags);
  if (!m_saveload_pending_file)
    goto cancel;

  // if there are anonymous timers, we can't save just yet, and we can't load yet either
  // because the timers might overwrite data we have loaded
  if (!m_scheduler.can_save())
  {
    // if more than a second has passed, we're probably screwed
    if ((this->time() - m_saveload_schedule_time) > attotime::from_seconds(1))
    {
      popmessage("Unable to %s due to pending anonymous timers. See error.log for details.", opname);
      goto cancel;
    }
    return;
  }

  static void *statePtr = NULL;
  static int stateLength = 0;

  // open the file
  if (!isRollback) {
  filerr = file.open(m_saveload_pending_file);
  } else {
  if (m_saveload_schedule == SLS_LOAD) {
    for (int a=0;a<MAX_STATES;a++) {
      attotime stateTime = states[a].first;
      if (stateTime.seconds == 0 && stateTime.attoseconds == 0) {
        // Empty state
        continue;
      }
      if (stateTime >= rollbackTime) {
        // State in the future
        continue;
      }
      if (bestState != -1 && states[bestState].first > stateTime) {
        // We already found a better state
        if (nextBestState == -1 || states[nextBestState].first < stateTime) {
          nextBestState = a;
        }
        continue;
      }
      nextBestState = bestState;
      bestState = a;
    }
    if (nextBestState == -1) {
      cout << "ERROR: COULD NOT FIND ROLLBACK STATE FOR TIME " << rollbackTime << " " << machine_time() << endl;
      exit(1);
    }
    cout << "Opening save file: " << states[bestState].first.seconds << "." << states[bestState].first.attoseconds << " < " << this->machine_time().seconds << "." << this->machine_time().attoseconds << endl;
    cout << "ROLLBACK TIME: " << rollbackTime << endl;
    vector<unsigned char> &v = states[bestState].second;
    filerr = file.open_ram(&v[0],v.size());
  } else {
    filerr = file.open_ram(NULL,0);
  }
  }

  if (filerr == FILERR_NONE)
  {
    // read/write the save state
    if (isRollback && m_saveload_schedule == SLS_LOAD) {
      cout << "Time rolled back from " << this->machine_time().seconds << "." << this->machine_time().attoseconds;
    }
    save_error saverr = (m_saveload_schedule == SLS_LOAD) ? m_save.read_file(file) : m_save.write_file(file);
    if (isRollback && m_saveload_schedule == SLS_LOAD) {
      m_machine_time = states[bestState].first;
      cout << " to " << this->machine_time().seconds << "." << this->machine_time().attoseconds << endl;
    }

    // handle the result
    switch (saverr)
    {
      case STATERR_ILLEGAL_REGISTRATIONS:
        popmessage("Error: Unable to %s state due to illegal registrations. See error.log for details.", opname);
        break;

      case STATERR_INVALID_HEADER:
        popmessage("Error: Unable to %s state due to an invalid header. Make sure the save state is correct for this game.", opname);
        break;

      case STATERR_READ_ERROR:
        popmessage("Error: Unable to %s state due to a read error (file is likely corrupt).", opname);
        break;

      case STATERR_WRITE_ERROR:
        popmessage("Error: Unable to %s state due to a write error. Verify there is enough disk space.", opname);
        break;

      case STATERR_NONE:
                if (!isRollback) {
        if (!(m_system.flags & GAME_SUPPORTS_SAVE))
          popmessage("State successfully %s.\nWarning: Save states are not officially supported for this game.", opnamed);
        else
          popmessage("State successfully %s.", opnamed);
                }
        break;

      default:
        popmessage("Error: Unknown error during state %s.", opnamed);
        break;
    }

    if (isRollback) {
      if (saverr == STATERR_NONE && m_saveload_schedule == SLS_SAVE) {
        if (stateLength != file.size()) {
          stateLength = file.size();
          if (statePtr) statePtr = realloc(statePtr,stateLength);
          else statePtr = malloc(file.size());
        }
        file.seek(0,SEEK_SET);
        file.read(statePtr,file.size());
        states[onState].first = this->machine_time();
        states[onState].second.clear();
        states[onState].second.insert(
          states[onState].second.begin(),
          (unsigned char*)statePtr,
          ((unsigned char*)statePtr) + stateLength);

        vector<unsigned char> &v = states[onState].second;
        emu_file file2(m_saveload_searchpath, OPEN_FLAG_READ);
        filerr = file2.open_ram(&v[0],v.size());
        save_error saverr2 = m_save.read_file(file2);
        if (saverr2 != STATERR_NONE) {
          cout << "OOPS: " << saverr2 << endl;
          exit(1);
        }

        onState = (onState+1)%MAX_STATES;
      }

      // Destroy any state saves after this point
      if (saverr == STATERR_NONE && m_saveload_schedule == SLS_LOAD) {
        for (int a=0;a<MAX_STATES;a++) {
          if (states[a].first > this->machine_time()) {
            cout << "Destroying state at time " << states[a].first << endl;
            states[a].first.seconds = 0;
            states[a].first.attoseconds = 0;
          }
        }
      }
    }


    // close and perhaps delete the file
    if (saverr != STATERR_NONE && m_saveload_schedule == SLS_SAVE)
      file.remove_on_close();
  }
  else
    popmessage("Error: Failed to open file for %s operation.", opname);

  // unschedule the operation
cancel:
  m_saveload_pending_file.reset();
  m_saveload_searchpath = NULL;
  m_saveload_schedule = SLS_NONE;
}


//-------------------------------------------------
//  soft_reset - actually perform a soft-reset
//  of the system
//-------------------------------------------------

void running_machine::soft_reset(void *ptr, INT32 param)
{
  logerror("Soft reset\n");

  // temporarily in the reset phase
  m_current_phase = MACHINE_PHASE_RESET;

  // set up the watchdog timer; only start off enabled if explicitly configured
  m_watchdog_enabled = (config().m_watchdog_vblank_count != 0 || config().m_watchdog_time != attotime::zero);
  watchdog_reset();
  m_watchdog_enabled = true;

  // call all registered reset callbacks
  call_notifiers(MACHINE_NOTIFY_RESET);

  // setup autoboot if needed
  m_autoboot_timer->adjust(attotime(options().autoboot_delay(),0),0);

  // now we're running
  m_current_phase = MACHINE_PHASE_RUNNING;
}


//-------------------------------------------------
//  watchdog_reset - reset the watchdog timer
//-------------------------------------------------

void running_machine::watchdog_reset()
{
  // if we're not enabled, skip it
  if (!m_watchdog_enabled)
    m_watchdog_timer->adjust(attotime::never);

  // VBLANK-based watchdog?
  else if (config().m_watchdog_vblank_count != 0)
    m_watchdog_counter = config().m_watchdog_vblank_count;

  // timer-based watchdog?
  else if (config().m_watchdog_time != attotime::zero)
    m_watchdog_timer->adjust(config().m_watchdog_time);

  // default to an obscene amount of time (3 seconds)
  else
    m_watchdog_timer->adjust(attotime::from_seconds(3));
}


//-------------------------------------------------
//  watchdog_enable - reset the watchdog timer
//-------------------------------------------------

void running_machine::watchdog_enable(bool enable)
{
  // when re-enabled, we reset our state
  if (m_watchdog_enabled != enable)
  {
    m_watchdog_enabled = enable;
    watchdog_reset();
  }
}


//-------------------------------------------------
//  watchdog_fired - watchdog timer callback
//-------------------------------------------------

void running_machine::watchdog_fired(void *ptr, INT32 param)
{
  logerror("Reset caused by the watchdog!!!\n");

  bool verbose = options().verbose();
#ifdef MAME_DEBUG
  verbose = true;
#endif
  if (verbose)
    popmessage("Reset caused by the watchdog!!!\n");

  schedule_soft_reset();
}


//-------------------------------------------------
//  watchdog_vblank - VBLANK state callback for
//  watchdog timers
//-------------------------------------------------

void running_machine::watchdog_vblank(screen_device &screen, bool vblank_state)
{
  // VBLANK starting
  if (vblank_state && m_watchdog_enabled)
  {
    // check the watchdog
    if (config().m_watchdog_vblank_count != 0)
      if (--m_watchdog_counter == 0)
        watchdog_fired();
  }
}


//-------------------------------------------------
//  logfile_callback - callback for logging to
//  logfile
//-------------------------------------------------

void running_machine::logfile_callback(running_machine &machine, const char *buffer)
{
  if (machine.m_logfile != NULL)
    machine.m_logfile->puts(buffer);
}


//-------------------------------------------------
//  start_all_devices - start any unstarted devices
//-------------------------------------------------

void running_machine::start_all_devices()
{
  // iterate through the devices
  int last_failed_starts = -1;
  while (last_failed_starts != 0)
  {
    // iterate over all devices
    int failed_starts = 0;
    device_iterator iter(root_device());
    for (device_t *device = iter.first(); device != NULL; device = iter.next())
      if (!device->started())
      {
        // attempt to start the device, catching any expected exceptions
        try
        {
          // if the device doesn't have a machine yet, set it first
          if (device->m_machine == NULL)
            device->set_machine(*this);

          // now start the device
          osd_printf_verbose("Starting %s '%s'\n", device->name(), device->tag());
          device->start();
        }

        // handle missing dependencies by moving the device to the end
        catch (device_missing_dependencies &)
        {
          // if we're the end, fail
          osd_printf_verbose("  (missing dependencies; rescheduling)\n");
          failed_starts++;
        }
      }

    // each iteration should reduce the number of failed starts; error if
    // this doesn't happen
    if (failed_starts == last_failed_starts)
      throw emu_fatalerror("Circular dependency in device startup!");
    last_failed_starts = failed_starts;
  }
}


//-------------------------------------------------
//  reset_all_devices - reset all devices in the
//  hierarchy
//-------------------------------------------------

void running_machine::reset_all_devices()
{
  // reset the root and it will reset children
  root_device().reset();
}


//-------------------------------------------------
//  stop_all_devices - stop all the devices in the
//  hierarchy
//-------------------------------------------------

void running_machine::stop_all_devices()
{
  // first let the debugger save comments
  if ((debug_flags & DEBUG_FLAG_ENABLED) != 0)
    debug_comment_save(*this);

  // iterate over devices and stop them
  device_iterator iter(root_device());
  for (device_t *device = iter.first(); device != NULL; device = iter.next())
    device->stop();
}


//-------------------------------------------------
//  presave_all_devices - tell all the devices we
//  are about to save
//-------------------------------------------------

void running_machine::presave_all_devices()
{
  device_iterator iter(root_device());
  for (device_t *device = iter.first(); device != NULL; device = iter.next())
  {
    if (device == NULL)
    {
      std::cout << "DEVICE IS NULL" << std::endl;
      std::cout.flush();
    }

    device->pre_save();
  }
}


//-------------------------------------------------
//  postload_all_devices - tell all the devices we
//  just completed a load
//-------------------------------------------------

void running_machine::postload_all_devices()
{
  device_iterator iter(root_device());
  for (device_t *device = iter.first(); device != NULL; device = iter.next())
    device->post_load();
}


/***************************************************************************
    NVRAM MANAGEMENT
***************************************************************************/

const char *running_machine::image_parent_basename(device_t *device)
{
  device_t *dev = device;
  while(dev != &root_device())
  {
    device_image_interface *intf = NULL;
    if (dev!=NULL && dev->interface(intf))
    {
      return intf->basename_noext();
    }
    dev = dev->owner();
  }
  return NULL;
}

/*-------------------------------------------------
    nvram_filename - returns filename of system's
    NVRAM depending of selected BIOS
-------------------------------------------------*/

astring &running_machine::nvram_filename(astring &result, device_t &device)
{
  // start with either basename or basename_biosnum
  result.cpy(basename());
  if (root_device().system_bios() != 0 && root_device().default_bios() != root_device().system_bios())
    result.catprintf("_%d", root_device().system_bios() - 1);

  // device-based NVRAM gets its own name in a subdirectory
  if (&device != &root_device())
  {
    // add per software nvrams into one folder
    const char *software = image_parent_basename(&device);
    if (software!=NULL && strlen(software)>0)
    {
      result.cat('\\').cat(software);
    }
    astring tag(device.tag());
    tag.del(0, 1).replacechr(':', '_');
    result.cat('\\').cat(tag);
  }
  return result;
}

extern Common *netCommon;

int nvram_size(running_machine &machine) {
  int retval=0;

  nvram_interface_iterator iter(machine.root_device());
  
  for (device_nvram_interface *nvram = iter.first(); nvram != NULL; nvram = iter.next())
  {
    astring filename;
    emu_file file(machine.options().nvram_directory(), OPEN_FLAG_READ);
    if (file.open(machine.nvram_filename(filename, nvram->device())) == FILERR_NONE)
    {
      retval += file.size();
    }
  }

  return retval;
}

/*-------------------------------------------------
    nvram_load - load a system's NVRAM
-------------------------------------------------*/

void running_machine::nvram_load()
{
  int overrideNVram = 0;
  if(netCommon) {
    if(nvram_size(*this)>=32*1024*1024) {
      overrideNVram=1;
      ui().popup_time(3, "The NVRAM for this game is too big, not loading NVRAM.");
    }
  }

  nvram_interface_iterator iter(root_device());
  for (device_nvram_interface *nvram = iter.first(); nvram != NULL; nvram = iter.next())
  {
    astring filename;
    emu_file file(options().nvram_directory(), OPEN_FLAG_READ);
    if (!overrideNVram && file.open(nvram_filename(filename, nvram->device())) == FILERR_NONE)
    {
      nvram->nvram_load(file);
      file.close();
    }
    else
      nvram->nvram_reset();
  }
}

/*-------------------------------------------------
    nvram_save - save a system's NVRAM
-------------------------------------------------*/

void running_machine::nvram_save()
{
  static bool first=true;

  if(netCommon) {
    if(nvram_size(*this)>=32*1024*1024) {
      if(first) {
        ui().popup_time(3, "The NVRAM for this game is too big, not saving NVRAM.");
        first = false;
      }

      return;
    }
  }

  nvram_interface_iterator iter(root_device());
  for (device_nvram_interface *nvram = iter.first(); nvram != NULL; nvram = iter.next())
  {
    astring filename;
    emu_file file(options().nvram_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
    if (file.open(nvram_filename(filename, nvram->device())) == FILERR_NONE)
    {
      nvram->nvram_save(file);
      file.close();
    }
  }
}

//**************************************************************************
//  CALLBACK ITEMS
//**************************************************************************

//-------------------------------------------------
//  notifier_callback_item - constructor
//-------------------------------------------------

running_machine::notifier_callback_item::notifier_callback_item(machine_notify_delegate func)
  : m_next(NULL),
    m_func(func)
{
}

//-------------------------------------------------
//  logerror_callback_item - constructor
//-------------------------------------------------

running_machine::logerror_callback_item::logerror_callback_item(logerror_callback func)
  : m_next(NULL),
    m_func(func)
{
}

//**************************************************************************
//  SYSTEM TIME
//**************************************************************************

//-------------------------------------------------
//  system_time - constructor
//-------------------------------------------------

system_time::system_time()
{
  set(0);
}

//-------------------------------------------------
//  set - fills out a system_time structure
//-------------------------------------------------

void system_time::set(time_t t)
{
  time = t;
  local_time.set(*localtime(&t));
  utc_time.set(*gmtime(&t));
}

//-------------------------------------------------
//  get_tm_time - converts a tm struction to a
//  MAME mame_system_tm structure
//-------------------------------------------------

void system_time::full_time::set(struct tm &t)
{
  //JJG: Force clock to 1/1/2000.
  if(netCommon)
  {
    second  = 0;
    minute  = 0;
    hour  = 0;
    mday  = 0;
    month = 0;
    year  = 2000;
    weekday = 6;
    day   = 0;
    is_dst  = 0;
  }
  else
{
  second  = t.tm_sec;
  minute  = t.tm_min;
  hour    = t.tm_hour;
  mday    = t.tm_mday;
  month   = t.tm_mon;
  year    = t.tm_year + 1900;
  weekday = t.tm_wday;
  day     = t.tm_yday;
  is_dst  = t.tm_isdst;
}
}

//**************************************************************************
//  JAVASCRIPT PORT-SPECIFIC
//**************************************************************************

#ifdef SDLMAME_EMSCRIPTEN

static running_machine * jsmess_machine;

void js_main_loop() {
  device_scheduler * scheduler;
	scheduler = &(jsmess_machine->scheduler());
	attotime stoptime = scheduler->time() + attotime(0,HZ_TO_ATTOSECONDS(60));
	while (scheduler->time() < stoptime) {
    jsmess_machine->mainLoop();
	}
}

void js_set_main_loop(running_machine * machine) {
  jsmess_machine = machine;
  EM_ASM (
    JSMESS.running = true;
  );
  emscripten_set_main_loop(&js_main_loop, 0, 1);
}

running_machine * js_get_machine() {
  return jsmess_machine;
}

ui_manager * js_get_ui() {
  return &(jsmess_machine->ui());
}

sound_manager * js_get_sound() {
  return &(jsmess_machine->sound());
}

#endif /* SDLMAME_EMSCRIPTEN */