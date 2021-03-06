#include <stdexcept>

#include "raknet_shim.h"

#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

#include "nsm_client.h"

#include "nsm_server.h"

#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

#define NO_MEM_TRACKING
#include "emu.h"
#include "emuopts.h"
#include "osdcore.h"
#include "osdepend.h"

#include "osdcore.h"
#include "ui/ui.h"
#include "unicode.h"

using namespace std;
using namespace nsm;
using namespace google::protobuf::io;

Client *netClient = NULL;

Client *createGlobalClient(string _username) {
  netClient = new Client(_username);
  netCommon = netClient;
  return netClient;
}

void deleteGlobalClient() {
  if (netClient) {
    netClient->shutdown();
  }
  netClient = NULL;
}

#define INITIAL_BUFFER_SIZE (1024 * 1024 * 32)
unsigned char *compressedBuffer = (unsigned char *)malloc(INITIAL_BUFFER_SIZE);
int compressedBufferSize = INITIAL_BUFFER_SIZE;

bool hasCompleteResync = false;

Client::Client(string _username) : Common(_username, 50) {
  initialSyncBuffer.reserve(1024 * 1024);

  rakInterface = RakNet::RakPeerInterface::GetInstance();
  // rakInterface->AllowConnectionResponseIPMigration(false);

  initComplete = false;
  syncing = false;
  firstResync = true;

  syncPtr = compressedBuffer;
  selfPeerID = 0;
  player = 1;
}

void Client::shutdown() {
  // We're done with the network
  RakNet::RakPeerInterface::DestroyInstance(rakInterface);
}

vector<std::shared_ptr<MemoryBlock> >
Client::createMemoryBlock(const std::string &name, unsigned char *ptr,
                          int size) {
  if (initComplete) {
    cout << "ERROR: CREATED MEMORY BLOCK TOO LATE\n";
    exit(1);
  }
  vector<std::shared_ptr<MemoryBlock> > retval;
  const int BYTES_IN_MB = 1024 * 1024;
  if (size > BYTES_IN_MB) {
    for (int a = 0;; a += BYTES_IN_MB) {
      if (a + BYTES_IN_MB >= size) {
        vector<std::shared_ptr<MemoryBlock> > tmp =
            createMemoryBlock(name, ptr + a, size - a);
        retval.insert(retval.end(), tmp.begin(), tmp.end());
        break;
      } else {
        vector<std::shared_ptr<MemoryBlock> > tmp =
            createMemoryBlock(name, ptr + a, BYTES_IN_MB);
        retval.insert(retval.end(), tmp.begin(), tmp.end());
      }
    }
    return retval;
  }
  // printf("Creating memory block at %X with size %d\n",ptr,size);
  blocks.push_back(
      std::shared_ptr<MemoryBlock>(new MemoryBlock(name, ptr, size)));
  staleBlocks.push_back(
      std::shared_ptr<MemoryBlock>(new MemoryBlock(name, size)));
  syncCheckBlocks.push_back(
      std::shared_ptr<MemoryBlock>(new MemoryBlock(name, size)));
  retval.push_back(blocks.back());
  return retval;
}

// Copied from Multiplayer.cpp
// If the first byte is ID_MAMEHUB_TIMESTAMP, then we want the
// 5th byte Otherwise we want the 1st byte
unsigned char GetPacketIdentifier(RakNet::Packet *p) {
  if (p == 0)
    return 255;

  if ((unsigned char)p->data[0] == ID_MAMEHUB_TIMESTAMP) {
    assert(p->length > sizeof(unsigned char) + sizeof(RakNet::Time));
    return (unsigned char)p->data[sizeof(unsigned char) + sizeof(RakNet::Time)];
  } else {
    return (unsigned char)p->data[0];
  }
}

unsigned char *GetPacketData(RakNet::Packet *p) {
  if (p == 0)
    return 0;

  if ((unsigned char)p->data[0] == ID_MAMEHUB_TIMESTAMP) {
    assert(p->length > (2 * sizeof(unsigned char)) + sizeof(RakNet::Time));
    return (unsigned char *)&(
        p->data[(2 * sizeof(unsigned char)) + sizeof(RakNet::Time)]);
  } else {
    return (unsigned char *)&(p->data[sizeof(unsigned char)]);
  }
}
int GetPacketSize(RakNet::Packet *p) {
  if (p == 0)
    return 0;

  if ((unsigned char)p->data[0] == ID_MAMEHUB_TIMESTAMP) {
    assert(p->length > (2 * sizeof(unsigned char)) + sizeof(RakNet::Time));
    return int(p->length) -
           int((2 * sizeof(unsigned char)) + sizeof(RakNet::Time));
  } else {
    return int(p->length) - int(sizeof(unsigned char));
  }
}

int initialSyncPercentComplete = 0;
extern bool waitingForClientCatchup;
extern int baseDelayFromPing;
extern attotime mostRecentSentReport;
RakNet::Time largestPacketTime = 0;
extern bool catchingUp;

bool Client::initializeConnection(running_machine *machine) {
  char buf[1];
  buf[0] = ID_CLIENT_HANDSHAKE;
  rakInterface->Send(buf, 1);
  machine->osd().pauseAudio(true);
  syncing = true;
  catchingUp = true;
  return true;

  // peerIDs[guid] = 1;

   /*while (initComplete == false) {
    RakNet::Packet *p = rakInterface->Receive();
    if (!p) {
      // printf("WAITING FOR SERVER TO SEND GAME WORLD...\n");
      machine->video().frame_update();
      RakSleep(10);
      continue; // We need the first few packets, so stall until we get them
    }
    unsigned char packetID = GetPacketIdentifier(p);
    printf("GOT PACKET: %d\n", int(packetID));

    switch (packetID) {
    case ID_HOST_ACCEPTED: {
      unsigned char *dataPtr = p->data + 1;
      int peerID;
      memcpy(&peerID, dataPtr, sizeof(int));
      dataPtr += sizeof(int);
      RakNet::RakNetGUID guid;
      memcpy(&(guid.g), dataPtr, sizeof(uint64_t));
      dataPtr += sizeof(uint64_t);
      int secs;
      long long attosecs;
      memcpy(&secs, dataPtr, sizeof(secs));
      dataPtr += sizeof(secs);
      memcpy(&attosecs, dataPtr, sizeof(attosecs));
      dataPtr += sizeof(attosecs);
      nsm::Attotime startTime = newAttotime(secs, attosecs);

      memcpy(&rollback, dataPtr, sizeof(bool));
      dataPtr += sizeof(bool);
      cout << "ROLLBACK " << (rollback ? "ENABLED" : "DISABLED") << endl;

      memcpy(&largestPacketTime, dataPtr, sizeof(RakNet::Time));
      dataPtr += sizeof(RakNet::Time);

      char buf[4096];
      strcpy(buf, (char *)(dataPtr));
      cout << "HOSTNAME " << sa.ToString() << " ACCEPTED (MY HOSTNAME IS "
           << rakInterface->GetExternalID(p->systemAddress).ToString() << ")\n";
      if (rakInterface->GetMyGUID() == guid) {
        // This is me, set my own ID and name
        selfPeerID = peerID;
        // Default player index is in order of join.
        player = selfPeerID - 1;
        mostRecentSentReport.seconds = startTime.seconds();
        mostRecentSentReport.attoseconds = startTime.attoseconds();
        cout << "CLIENT STARTED AT TIME: " << startTime.seconds() << "."
             << startTime.attoseconds() << endl;
      } else {
        // This is someone else, set their ID and name and wait for them to
        // catch up
        waitingForClientCatchup = true;
        machine->osd().pauseAudio(true);
      }
      upsertPeer(guid, peerID, buf, startTime);
    } break;
    case ID_INPUTS: {
      if (initComplete)
        throw emu_fatalerror("GOT INPUT BEFORE INIT COMPLETE");

      // if (p->guid == masterGuid)
      {
        // Inputs from server.  Record time if it's newer
        RakNet::BitStream timeBS((unsigned char *)&(p->data[1]),
                                 sizeof(RakNet::Time), false);
        RakNet::Time packetTime;
        timeBS.Read(packetTime);
        cout << "Got server time: " << packetTime << endl;
        if (packetTime > largestPacketTime) {
          largestPacketTime = packetTime;
        }
      }

      string s = doInflate(GetPacketData(p), GetPacketSize(p));
      PeerInputDataList inputDataList;
      inputDataList.ParseFromString(s);
      receiveInputs(&inputDataList);
      break;
    }
    case ID_BASE_DELAY: {
      cout << "Changing base delay from " << baseDelayFromPing;
      memcpy(&baseDelayFromPing, GetPacketData(p), sizeof(int));
      cout << " to " << baseDelayFromPing << endl;
    } break;
    case ID_SETTINGS: {
      if (p->data[1]) {
        doCatchup = 1;
        cout << "DOING CATCHUP\n";
      } else {
        doCatchup = 0;
        cout << "NOT DOING CATCHUP\n";
      }
      memcpy(&secondsBetweenSync, p->data + 2, sizeof(int));
      memcpy(&unmeasuredNoise, p->data + 2 + sizeof(int), sizeof(int));
     
      // Create peerdata for server
      upsertPeer(p->guid, 1, newAttotime(0, 0));
    } break;
    default:
      // printf("GOT AN INVALID PACKET TYPE: %d\n",int(packetID));
      // throw std::runtime_error("OOPS");
      break;
    }

    rakInterface->DeallocatePacket(p);
  }

  return true;*/
}

nsm::InitialSync initial_sync;

void Client::loadInitialData(unsigned char* data, int size, running_machine *machine) {
  {
    ArrayInputStream ais(data, size);
    GzipInputStream lis(&ais);
    initial_sync.ParseFromZeroCopyStream(&lis);
  }

  waitingForClientCatchup = true;

  syncGeneration = initial_sync.generation();
  syncSeconds = initial_sync.global_time().seconds();
  syncAttoseconds = initial_sync.global_time().attoseconds();

  for (int a = 0; a < initial_sync.peer_data_size(); a++) {
    const PeerInputDataList &peer_data = initial_sync.peer_data(a);
    receiveInputs(&peer_data);
  }

  cout << "GOT NVRAM: " << initial_sync.nvram_size() << endl;

  if (initial_sync.nvram_size()) {
    int nvram_index = 0;

    nvram_interface_iterator iter(machine->root_device());
    for (device_nvram_interface *nvram = iter.first(); nvram != NULL;
         nvram = iter.next()) {
      astring filename;
      emu_file file(machine->options().nvram_directory(),
                    OPEN_FLAG_WRITE | OPEN_FLAG_CREATE |
                        OPEN_FLAG_CREATE_PATHS);
      if (file.open(machine->nvram_filename(filename, nvram->device())) ==
          FILERR_NONE) {
        cout << "SAVING NVRAM OF SIZE: "
             << initial_sync.nvram(nvram_index).length() << endl;
        file.write(initial_sync.nvram(nvram_index).c_str(),
                   initial_sync.nvram(nvram_index).length());
        file.close();
      } else {
        cout << "ERROR STORING NVRAM\n";
        exit(1);
      }
      nvram_index++;
    }
  }
}

void Client::createInitialBlocks(running_machine *machine) {
  unsigned char checksum = 0;

  machine->save().dispatch_presave();

  // Server and client must match on blocks
  if (blocks.size() != initial_sync.initial_block_size()) {
    cout << "ERROR: CLIENT AND SERVER BLOCK COUNTS DO NOT MATCH!\n";
    exit(1);
  }

  for (int blockIndex = 0; blockIndex < initial_sync.initial_block_size();
        blockIndex++) {
    if (initial_sync.initial_block(blockIndex).length() !=
        blocks[blockIndex]->size) {
      cout << "ERROR: CLIENT AND SERVER BLOCK SIZES AT INDEX " << blockIndex
            << " DO NOT MATCH!\n";
      cout << initial_sync.initial_block(blockIndex).length()
            << " != " << blocks[blockIndex]->size << endl;
      exit(1);
    }

    // cout << "BLOCK " << blockIndex << ":\n";
    for (int a = 0; a < blocks[blockIndex]->size; a++) {
      unsigned char xorValue = initial_sync.initial_block(blockIndex)[a];
      // cout << int(blocks[blockIndex]->data[a] ^ xorValue) << '\n';
      staleBlocks[blockIndex]->data[a] =
          blocks[blockIndex]->data[a] ^ xorValue;
      checksum = checksum ^ staleBlocks[blockIndex]->data[a];
    }
    if (checksum != initial_sync.checksum(blockIndex)) {
      cout << "CHECKSUM ERROR: " << int(checksum)
            << " != " << int(initial_sync.checksum(blockIndex)) << " "
            << blocks[blockIndex]->name << " (index: " << blockIndex << ") "
            << endl;
      exit(1);
    }
  }

  revert(machine);

  machine->save().dispatch_postload();
}

void Client::updateSyncCheck() {
  printf("UPDATING SYNC CHECK\n");
  unsigned char checksum = 0;
  for (int blockIndex = 0; blockIndex < int(blocks.size()); blockIndex++) {
    memcpy(syncCheckBlocks[blockIndex]->data, blocks[blockIndex]->data,
           blocks[blockIndex]->size);
    for (int b = 0; b < syncCheckBlocks[blockIndex]->size; b++) {
      checksum = checksum ^ syncCheckBlocks[blockIndex]->data[b];
    }
  }
  cout << "SYNC CHECK CHECKSUM: " << int(checksum) << endl;
}

bool printWhenCheck = false;

bool Client::sync(running_machine *machine) {
  if (!hasCompleteResync)
    return false;
  hasCompleteResync = false;

  bool hadToResync =
      resync(compressedBuffer, int(syncPtr - compressedBuffer), machine);

  // We have to return here because processing two syncs without a frame
  // in between can cause crashes
  syncPtr = compressedBuffer;
  if (firstResync || hadToResync) {
    printf("BEGINNING VIDEO SKIP\n");
    firstResync = false;
    return true;
  } else {
    return false;
  }
}

bool Client::update(running_machine *machine) {
  if (printWhenCheck) {
    printWhenCheck = false;
    // printf("Checking for packets\n");
  }

  while (true) {
    RakNet::Packet *p = rakInterface->Receive();
    if (!p) {
      break;
    }

    rakInterface->DeallocatePacket(p);
    break;


    int packetID = GetPacketIdentifier(p);
    // cout << "GOT PACKET WITH ID: " << packetID << endl;

    switch (packetID) {
    case ID_HOST_ACCEPTED: {
      // 1 bytes - id
      // 4 bytes - assigned id
      // 4 bytes - start time seconds
      // 8 bytes - start time attoseconds
      // 4 bytes - boolean
      // 4 bytes - elapsed time

      unsigned char* tmpbuf = p->data + 1;
      memcpy(&selfPeerID, tmpbuf, sizeof(int));
      tmpbuf += sizeof(int);

      int secs;
      long long attosecs;
      memcpy(&secs, tmpbuf, sizeof(secs));
      tmpbuf += sizeof(secs);
      memcpy(&attosecs, tmpbuf, sizeof(attosecs));
      tmpbuf += sizeof(attosecs);
      
      nsm::Attotime startTime = newAttotime(secs, attosecs);

      cout << "HOST ACCEPTED, SELF PEER ID = " << selfPeerID << endl;
      player = selfPeerID - 1;

      mostRecentSentReport.seconds = startTime.seconds();
      mostRecentSentReport.attoseconds = startTime.attoseconds();
      cout << "CLIENT STARTED AT TIME: " << startTime.seconds() << "."
            << startTime.attoseconds() << endl;

      memcpy(&rollback, tmpbuf, sizeof(bool));
      tmpbuf += sizeof(bool);
      cout << "ROLLBACK " << (rollback ? "ENABLED" : "DISABLED") << endl;

      memcpy(&largestPacketTime, tmpbuf, sizeof(RakNet::Time));
      tmpbuf += sizeof(RakNet::Time);

      upsertPeer(username, selfPeerID, startTime);
    } break;
    case ID_INITIAL_SYNC_PARTIAL: {
      // printf("GOT PARTIAL SYNC FROM SERVER\n");
      int curPos = (int)initialSyncBuffer.size();
      initialSyncBuffer.resize(initialSyncBuffer.size() + GetPacketSize(p));
      memcpy(&initialSyncBuffer[curPos], GetPacketData(p), GetPacketSize(p));

      int totalSize;
      memcpy(&totalSize, (&initialSyncBuffer[sizeof(int)]), sizeof(int));
      initialSyncPercentComplete = initialSyncBuffer.size() * 1000 / totalSize;
    } break;
    case ID_INITIAL_SYNC_COMPLETE: {
      printf("GOT INITIAL SYNC FROM SERVER!\n");
      int curPos = (int)initialSyncBuffer.size();
      initialSyncBuffer.resize(initialSyncBuffer.size() + GetPacketSize(p));
      memcpy(&initialSyncBuffer[curPos], GetPacketData(p), GetPacketSize(p));

      loadInitialData(&initialSyncBuffer[0], (int)initialSyncBuffer.size(),
                      machine);

      initComplete = true;
      syncing = false;
      machine->osd().pauseAudio(false);
    } break;
    case ID_RESYNC_PARTIAL: {
      printf("GOT PARTIAL RESYNC\n");
      if (hasCompleteResync) {
        // hasCompleteResync=false;
        // syncPtr = compressedBuffer;
        printf(
            "ERROR: GOT NEW RESYNC WHILE ANOTHER RESYNC WAS ON THE QUEUE!\n");
        return false;
      }
      int bytesUsed = syncPtr - compressedBuffer;
      while (bytesUsed + GetPacketSize(p) >= compressedBufferSize) {
        compressedBufferSize *= 1.5;
        free(compressedBuffer);
        compressedBuffer = (unsigned char *)malloc(compressedBufferSize);
        if (!compressedBuffer) {
          cout << __FILE__ << ":" << __LINE__ << " OUT OF MEMORY\n";
          exit(1);
        }
        syncPtr = compressedBuffer + bytesUsed;
      }
      memcpy(syncPtr, GetPacketData(p), GetPacketSize(p));
      syncPtr += GetPacketSize(p);
      printWhenCheck = true;
      break;
    }
    case ID_RESYNC_COMPLETE: {
      cout << "GOT COMPLETE RESYNC WITH SIZE " << GetPacketSize(p)
           << " (Without header)\n";
      int bytesUsed = syncPtr - compressedBuffer;
      while (bytesUsed + GetPacketSize(p) >= compressedBufferSize) {
        compressedBufferSize *= 1.5;
        free(compressedBuffer);
        compressedBuffer = (unsigned char *)malloc(compressedBufferSize);
        if (!compressedBuffer) {
          cout << __FILE__ << ":" << __LINE__ << " OUT OF MEMORY\n";
          exit(1);
        }
        syncPtr = compressedBuffer + bytesUsed;
      }
      memcpy(syncPtr, GetPacketData(p), GetPacketSize(p));
      syncPtr += GetPacketSize(p);
      hasCompleteResync = true;
      return true;
      break;
    }
    case ID_INPUTS: {
      if (!initComplete) {
        break; // TODO FIXME, not sure what to do with these
      }

      if (peerIDs.find(p->sender) == peerIDs.end()) {
        throw std::runtime_error("GOT INPUTS FROM UNKNOWN PEER");
      }

      //if (p->guid == masterGuid)
      {
        // Inputs from server.  Record time if it's newer
        RakNet::BitStream timeBS((unsigned char *)&(p->data[1]),
                                 sizeof(RakNet::Time), false);
        RakNet::Time packetTime;
        timeBS.Read(packetTime);
        if (packetTime > largestPacketTime) {
          // cout << "GOT NEW PACKET TIME: " << packetTime << endl;
          largestPacketTime = packetTime;
        }
      }

      string s = doInflate(GetPacketData(p), GetPacketSize(p));
      PeerInputDataList inputDataList;
      inputDataList.ParseFromString(s);
      receiveInputs(&inputDataList);
      break;
    }
    case ID_BASE_DELAY: {
      cout << "Changing base delay from " << baseDelayFromPing;
      memcpy(&baseDelayFromPing, GetPacketData(p), sizeof(int));
      cout << " to " << baseDelayFromPing << endl;
    } break;
    /*case ID_SETTINGS:
      memcpy(&secondsBetweenSync, p->data + 1, sizeof(int));
      break;*/
    default:
      printf("GOT AN INVALID PACKET TYPE: %d %d\n", packetID, GetPacketSize(p));
      break;
    }

    rakInterface->DeallocatePacket(p);
  }

  return true;
}

bool Client::resync(unsigned char *data, int size, running_machine *machine) {
  int compressedSize;
  memcpy(&compressedSize, data, sizeof(int));
  data += sizeof(int);
  cout << "COMPRESSED SIZE: " << compressedSize << endl;

  nsm::Sync syncProto;

  {
    ArrayInputStream ais(data, compressedSize);
    GzipInputStream lis(&ais);
    syncProto.ParseFromZeroCopyStream(&lis);
  }

  syncGeneration = syncProto.generation();
  syncSeconds = syncProto.global_time().seconds();
  syncAttoseconds = syncProto.global_time().attoseconds();

  // Check to see if the client is clean
  bool clientIsClean = true;

  int badByteCount = 0;
  int totalByteCount = 0;

  for (int i = 0; i < syncProto.block_size(); i++) {
    const nsm::SyncBlock &syncBlock = syncProto.block(i);
    int blockIndex = syncBlock.index();
    unsigned char *syncBlockData = (unsigned char *)(&syncBlock.data()[0]);

    if (blockIndex >= int(blocks.size()) || blockIndex < 0) {
      cout << "GOT AN INVALID BLOCK INDEX: " << blockIndex << endl;
      break;
    }

    MemoryBlock &block = *(blocks[blockIndex]);
    MemoryBlock &syncCheckBlock = *(syncCheckBlocks[blockIndex]);
    MemoryBlock &staleBlock = *(staleBlocks[blockIndex]);

    // cout << "CLIENT: GOT MESSAGE FOR INDEX: " << blockIndex << endl;

    // if(clientSizeOfNextMessage!=block.size)
    //{
    // cout << "ERROR!: SIZE MISMATCH " << clientSizeOfNextMessage
    //<< " != " << block.size << endl;
    //}

    // cout << "BYTES READ: " << (xorBlock.size-strm.avail_out) << endl;
    for (int a = 0; a < block.size; a++) {
      totalByteCount++;
      if (syncCheckBlock.data[a] != (syncBlockData[a] ^ staleBlock.data[a])) {
        badByteCount++;
        if (badByteCount < 50) {
          printf("BLOCK %d BYTE %d IS BAD: %d %d %d\n", blockIndex, a,
                 int(syncBlockData[a]), int(syncCheckBlock.data[a]),
                 int(staleBlock.data[a]));
        }
        if (badByteCount > 64)
          clientIsClean = false;
        syncCheckBlock.data[a] = (syncBlockData[a] ^ staleBlock.data[a]);
      }
      staleBlock.data[a] = (syncBlockData[a] ^ staleBlock.data[a]);
    }
  }

  if (badByteCount * 100 / totalByteCount >= 1)
    clientIsClean = false; // 1% or more of the bytes are bad

  if (clientIsClean) {
    printf("CLIENT IS CLEAN\n");
    for (int a = 0; a < int(blocks.size()); a++) {
      // memcpy(staleBlocks[a].data,syncCheckBlocks[a].data,syncCheckBlocks[a].size);
    }
    return false;
  }

  if (machine->scheduler().can_save() == false) {
    printf("CLIENT IS DIRTY BUT HAD ANONYMOUS TIMER SO CAN'T FIX!\n");
    return false;
  }

  printf("CLIENT IS DIRTY (%d bad blocks, %f%% of total)\n", badByteCount,
         float(badByteCount) * 100.0f / totalByteCount);
  machine->ui().popup_time(3,
                           "You are out of sync with the server, resyncing...");

  machine->save().dispatch_presave();

  revert(machine);

  machine->save().dispatch_postload();
  cout << "POST LOAD FINISHED\n";

  {
    unsigned char checksum = 0;
    // If the client has predicted anything, erase the prediction
    for (int a = 0; a < blocks.size(); a++) {
      // cout << "BLOCK " << a << ": ";
      for (int b = 0; b < blocks[a]->size; b++) {
        checksum = checksum ^ blocks[a]->data[b];
      }
      // cout << int(checksum) << endl;
    }
    cout << "(revert) BLOCK CHECKSUM POST LOAD: " << int(checksum) << endl;
  }

  return true;
}

void Client::revert(running_machine *machine) {
  generation = syncGeneration;
  machine->machine_time().seconds = syncSeconds;
  machine->machine_time().attoseconds = syncAttoseconds;

  {
    unsigned char checksum = 0;
    // If the client has predicted anything, erase the prediction
    for (int a = 0; a < blocks.size(); a++) {
      // cout << "BLOCK " << a << ": ";
      memcpy(blocks[a]->data, staleBlocks[a]->data, blocks[a]->size);
      for (int b = 0; b < staleBlocks[a]->size; b++) {
        checksum = checksum ^ staleBlocks[a]->data[b];
      }
      // cout << int(checksum) << endl;
    }
    cout << "(revert) STALE CHECKSUM: " << int(checksum) << endl;
  }
}

unsigned long long Client::getCurrentServerTime() {
  // cout << "LAST PING: " << largestPacketTime << " " <<
  // (rakInterface->GetLastPing(masterGuid)/2) << endl;
  return largestPacketTime + (rakInterface->GetLastPing(masterGuid) / 2);
}
