#include <memory>
#include <stdexcept>

#include "raknet_shim.h"

#include "nsm_server.h"

#include <algorithm>
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

#define NO_MEM_TRACKING
#include "emu.h"

#include "emuopts.h"
#include "osdcore.h"
#include "osdepend.h"
#include "ui/ui.h"
#include "unicode.h"

using std::shared_ptr;

using namespace std;
using namespace nsm;
using namespace google::protobuf::io;

Server *netServer = NULL;

Server* createGlobalServer(string _username, int _unmeasuredNoise, bool _rollback) {
  cout << "Creating server (username = " << _username << ", noise = " << _unmeasuredNoise << ", rollback = " << _rollback << ")" << endl;
  netCommon = netServer = new Server(_username, _unmeasuredNoise, _rollback);
  return netServer;
}

void deleteGlobalServer() {
  if (netServer) {
    netServer->shutdown();
  }
  netServer = NULL;
}

extern unsigned char GetPacketIdentifier(RakNet::Packet *p);
extern unsigned char *GetPacketData(RakNet::Packet *p);
extern int GetPacketSize(RakNet::Packet *p);

#define INITIAL_BUFFER_SIZE (1024 * 1024 * 32)
unsigned char *syncBuffer = (unsigned char *)malloc(INITIAL_BUFFER_SIZE);
int syncBufferSize = INITIAL_BUFFER_SIZE;
// unsigned char *uncompressedBuffer = (unsigned
// char*)malloc(INITIAL_BUFFER_SIZE); int uncompressedBufferSize =
// INITIAL_BUFFER_SIZE;

class SyncProcessor {
public:
  nsm::Sync *sync;
  std::list<std::pair<unsigned char *, int> > *syncPacketQueue;
  int syncTransferSeconds;
  bool *syncReadyPtr;
  string compressedSync;

  SyncProcessor(nsm::Sync *_sync,
                std::list<std::pair<unsigned char *, int> > *_syncPacketQueue,
                int _syncTransferSeconds, bool *_syncReadyPtr)
      : sync(_sync), syncPacketQueue(_syncPacketQueue),
        syncTransferSeconds(_syncTransferSeconds), syncReadyPtr(_syncReadyPtr) {
    *syncReadyPtr = false;
  }

  void process() {
    {
      StringOutputStream sos(&compressedSync);
      {
        GzipOutputStream los(&sos);
        sync->SerializeToZeroCopyStream(&los);
        los.Flush();
      }
    }

    unsigned char *compressedBuffer = (unsigned char *)&compressedSync[0];
    int compressedSize = (int)compressedSync.length();
    printf("SYNC SIZE: %d\n", compressedSize);
    if (compressedSize >
        16 * 1024 * 1024) { // If bigger than 16MB, don't even bother.
      netServer->syncOverride = true;
      return;
    }

    int SYNC_PACKET_SIZE = 1024 * 1024;

    // If the sync is less than 2KB or syncTransferSeconds is 0, send
    // it all at once.
    if (compressedSize > 2 * 1024 && syncTransferSeconds) {
      int actualSyncTransferSeconds = max(1, syncTransferSeconds);
      while (true) {
        SYNC_PACKET_SIZE = compressedSize / 10 / actualSyncTransferSeconds;

        if (actualSyncTransferSeconds == 1) {
          // Make sure that we send SOMETHING each frame
          if (SYNC_PACKET_SIZE == 0) {
            SYNC_PACKET_SIZE = compressedSize;
          }
          break;
        }

        // This sends the data at 2 KB/sec minimum
        if (SYNC_PACKET_SIZE >= 210)
          break;

        actualSyncTransferSeconds--;
      }
    }

    int sendMessageSize =
        1 + sizeof(int) + min(SYNC_PACKET_SIZE, compressedSize);
    int totalSendSizeEstimate =
        sendMessageSize * (compressedSize / SYNC_PACKET_SIZE + 2);
    if (syncBufferSize <= totalSendSizeEstimate) {
      syncBufferSize = totalSendSizeEstimate * 1.5;
      free(syncBuffer);
      syncBuffer = (unsigned char *)malloc(totalSendSizeEstimate);
      if (!syncBuffer) {
        cout << __FILE__ << ":" << __LINE__ << " OUT OF MEMORY\n";
        exit(1);
      }
    }
    unsigned char *sendMessage = syncBuffer;
    sendMessage[0] = ID_RESYNC_PARTIAL;
    if (compressedSize <= SYNC_PACKET_SIZE)
      sendMessage[0] = ID_RESYNC_COMPLETE;
    memcpy(sendMessage + 1, &compressedSize, sizeof(int));
    memcpy(sendMessage + 1 + sizeof(int), compressedBuffer,
           min(SYNC_PACKET_SIZE, compressedSize));

    syncPacketQueue->push_back(make_pair(sendMessage, sendMessageSize));
    sendMessage += sendMessageSize;
    compressedSize -= SYNC_PACKET_SIZE;
    int atIndex = SYNC_PACKET_SIZE;

    while (compressedSize > 0) {
      int thisMessageSize = 1 + min(SYNC_PACKET_SIZE, compressedSize);
      sendMessage[0] = ID_RESYNC_PARTIAL;
      if (compressedSize <= SYNC_PACKET_SIZE)
        sendMessage[0] = ID_RESYNC_COMPLETE;
      memcpy(sendMessage + 1, compressedBuffer + atIndex,
             min(SYNC_PACKET_SIZE, compressedSize));
      compressedSize -= SYNC_PACKET_SIZE;
      atIndex += SYNC_PACKET_SIZE;

      syncPacketQueue->push_back(make_pair(sendMessage, thisMessageSize));
      sendMessage += thisMessageSize;
    }

    if (int(sendMessage - syncBuffer) >= totalSendSizeEstimate) {
      cout << "INVALID SEND SIZE ESTIMATE!\n";
      exit(1);
    }
    cout << "FINISHED QUEUEING SYNC\n";

    // HACK: The server could send sync info to the client before the
    // client has done his own sync, to preventthis, jsut sleep for a
    // second.
    RakSleep(1000);

    *syncReadyPtr = true;
  }
};

Server::Server(string guid, int _unmeasuredNoise, bool _rollback)
    : Common(guid, _unmeasuredNoise), syncOverride(false), blockNewClients(false), maxPeerID(10) {

  rakInterface = RakNet::RakPeerInterface::GetInstance();
  rollback = _rollback;

  syncReady = false;
  syncCount = 0;
  selfPeerID = 1;
  nextPeerID = 2;
  
  upsertPeer(guid, 1, newAttotime(1, 0));
}

Server::~Server() {}

void Server::shutdown() {
  // We're done with the network
  RakNet::RakPeerInterface::DestroyInstance(rakInterface);
}

extern RakNet::Time emulationStartTime;

void Server::acceptPeer(const std::string& guid,
                        running_machine *machine) {
  cout << "ACCEPTED PEER " << guid << endl;
  
  int peerID = -1;
  for (std::unordered_map<std::string, int>::iterator it = deadPeerIDs.begin();
       it != deadPeerIDs.end(); it++) {
    if (it->first == guid) {
      peerID = it->second;
      deadPeerIDs.erase(it);
      break;
    }
  }

  if (peerID == -1) {
    peerID = nextPeerID++;
  }

  printf("ASSIGNING ID %d TO NEW CLIENT\n", peerID);

  nsm::Attotime at = newAttotime(machine->machine_time().seconds, machine->machine_time().attoseconds);
  upsertPeer(guid, peerID, at);

  {
    char buf[32];

    buf[0] = ID_HOST_ACCEPTED;
    // 1 bytes - id
    // 4 bytes - assigned id
    // 4 bytes - start time seconds
    // 8 bytes - start time attoseconds
    // 4 bytes - boolean
    // 4 bytes - elapsed time
    
    char* tmpbuf = buf + 1;
    memcpy(tmpbuf, &peerID, sizeof(int));
    tmpbuf += sizeof(int);

    int secs = at.seconds();
    long long attosecs = at.attoseconds();
    memcpy(tmpbuf, &secs, sizeof(secs));
    tmpbuf += sizeof(secs);
    memcpy(tmpbuf, &attosecs, sizeof(attosecs));
    tmpbuf += sizeof(attosecs);

    memcpy(tmpbuf, &rollback, sizeof(bool));
    tmpbuf += sizeof(bool);

    RakNet::Time t = RakNet::GetTimeMS() - emulationStartTime;
    memcpy(tmpbuf, &t, sizeof(RakNet::Time));
    tmpbuf += sizeof(RakNet::Time);

    rakInterface->Send(buf, int(tmpbuf - buf), guid);
  }

  // TODO FIX ME

  /*{
    char buf[4096];
    buf[0] = ID_SETTINGS;
    buf[1] = ((syncCount <= 1) ? 0 : 1); // Should the client catch up?
    memcpy(buf + 2, &secondsBetweenSync, sizeof(int));
    memcpy(buf + 2 + sizeof(int), &unmeasuredNoise, sizeof(int));
    strcpy(buf + 2 + (2 * sizeof(int)), username.c_str());
    rakInterface->Send(buf, 2 + (2 * sizeof(int)) + username.length() + 1, p->guid, false);
  }*/

  // Perform initial sync with player
  // initialSync(guid, machine);
}

void Server::removePeer(const std::string& guid, running_machine *machine) {
  if (peerIDs.find(guid) != peerIDs.end())
    if (peerData.find(peerIDs[guid]) != peerData.end())
      peerData.erase(peerIDs[guid]);

  cout << "REMOVING PEER\n";
}

vector<std::shared_ptr<MemoryBlock> >
Server::createMemoryBlock(const std::string &name, unsigned char *ptr,
                          int size) {
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
      std::shared_ptr<MemoryBlock>(new MemoryBlock(name, size))); // TODO FIXME, empty?
  initialBlocks.push_back(
      std::shared_ptr<MemoryBlock>(new MemoryBlock(name, size))); // TODO FIXME, empty?
  retval.push_back(blocks.back());
  return retval;
}

extern bool waitingForClientCatchup;
attotime oldInputTime;
extern int nvram_size(running_machine &machine);

void Server::initialSync(const std::string& guid,
                         running_machine *machine) {
  cout << "INITIAL SYNC WITH " << guid << " AT TIME "
       << staleTime.seconds() << "." << staleTime.attoseconds() << endl;
  unsigned char checksum = 0;

  nsm::InitialSync initial_sync;
  initial_sync.set_generation(staleGeneration);
  nsm::Attotime *global_time = initial_sync.mutable_global_time();
  global_time->set_seconds(staleTime.seconds());
  global_time->set_attoseconds(staleTime.attoseconds());

  /*if (getSecondsBetweenSync()) {
    cout << "IN CRITICAL SECTION\n";
    cout << "SERVER: Sending initial snapshot\n";

    // NOTE: The server must send stale data to the client for the first time
    // So that future syncs will be accurate
    for (int blockIndex = 0; blockIndex < int(initialBlocks.size());
         blockIndex++) {
      vector<unsigned char> deltaBlock;
      // cout << "BLOCK " << blockIndex << ":\n";

      for (int a = 0; a < staleBlocks[blockIndex]->size; a++) {
        checksum = checksum ^ staleBlocks[blockIndex]->data[a];
        // cout << int(staleBlocks[blockIndex]->data[a]) << '\n';
        unsigned char value = initialBlocks[blockIndex]->data[a] ^
                              staleBlocks[blockIndex]->data[a];
        deltaBlock.push_back(value);
      }
      // cout << int(checksum) << endl;
      initial_sync.add_initial_block(&deltaBlock[0], deltaBlock.size());
      initial_sync.add_checksum(checksum);
    }
    cout << "CHECKSUM: " << int(checksum) << endl;
  }*/

  // waitingForClientCatchup = true;
  // machine->osd().pauseAudio(true);

  
  /*for (unordered_map<int, PeerData>::iterator it = peerData.begin(); it != peerData.end();
       it++) {
    nsm::PeerInputDataList *peer_data = initial_sync.add_peer_data();
    peer_data->set_peer_id(it->first);

    for (int a = 0; a < int(it->second.oldInputs.size()); a++) {
      nsm::PeerInputData *input_data = peer_data->add_input_data();
      input_data->CopyFrom(it->second.oldInputs[a]);
    }
    for (list<PeerInputData>::iterator it2 = it->second.availableInputs.begin();
         it2 != it->second.availableInputs.end(); it2++) {
      nsm::PeerInputData *input_data = peer_data->add_input_data();
      input_data->CopyFrom(*it2);
    }
    for (map<int, PeerInputData>::iterator it2 =
             it->second.delayedInputs.begin();
         it2 != it->second.delayedInputs.end(); it2++) {
      nsm::PeerInputData *input_data = peer_data->add_input_data();
      input_data->CopyFrom(it2->second);
    }
  }*/

  /*bool writenvram = (nvram_size(*machine) < 1024 * 1024 * 32);
  if (writenvram) {
    nvram_interface_iterator iter(machine->root_device());
    for (device_nvram_interface *nvram = iter.first(); nvram != NULL;
         nvram = iter.next()) {
      astring filename;
      emu_file file(machine->options().nvram_directory(), OPEN_FLAG_READ);
      machine->nvram_filename(filename, nvram->device());
      std::cout << "WRITING NVRAM FOR DEVICE " << filename.cstr() << std::endl;
      std::cout.flush();

      if (file.open(filename) == FILERR_NONE) {
        vector<unsigned char> fileContents(file.size());
        file.read(&fileContents[0], file.size());
        initial_sync.add_nvram(&fileContents[0], file.size());
        cout << "ADDING NVRAM OF SIZE: " << file.size() << " "
             << file.filename() << endl;
        file.close();
      }
      else {
        cout << "FAILED TO WRITE NVRAM" << endl;
      }
    }
  }*/

  string s;
  {
    StringOutputStream sos(&s);
    {
      GzipOutputStream los(&sos);
      initial_sync.SerializeToZeroCopyStream(&los);
      los.Flush();
    }
  }

  int sizeRemaining = s.length();
  int packetSize = min(MAX_PACKET_SIZE, sizeRemaining);
  int offset = 0;

  oldInputTime.seconds = oldInputTime.attoseconds = 0;

  while (sizeRemaining > packetSize) {
    RakNet::BitStream bitStreamPart(packetSize + 32);
    unsigned char header = ID_INITIAL_SYNC_PARTIAL;
    bitStreamPart.WriteBytes((const unsigned char *)&header, sizeof(unsigned char));
    bitStreamPart.WriteBytes((const unsigned char *)(s.c_str() + offset), packetSize);
    rakInterface->Send(bitStreamPart.data, bitStreamPart.dataPtr, guid);

    sizeRemaining -= packetSize;
    offset += packetSize;
  }

  {
    RakNet::BitStream bitStreamPart(packetSize + 32);
    unsigned char header = ID_INITIAL_SYNC_COMPLETE;
    bitStreamPart.WriteBytes((const unsigned char *)&header, sizeof(unsigned char));
    bitStreamPart.WriteBytes((const unsigned char *)(s.c_str() + offset), sizeRemaining);
    rakInterface->Send(bitStreamPart.data, bitStreamPart.dataPtr, guid);
  }

  cout << "FINISHED SENDING BLOCKS TO CLIENT\n";
  cout << "SERVER: Done with initial snapshot\n";
  cout << "OUT OF CRITICAL AREA\n";
  cout.flush();
}

nsm::PeerInputData Server::popInput(int peerID) {
  nsm::PeerInputData inputToPop = Common::popInput(peerID);
  if (peerData[peerID].oldInputs.size() > 10000) {
    if (!blockNewClients) {
      // TODO: put a warning here.
    }
    blockNewClients = true;
  }
  return inputToPop;
}

bool Server::update(running_machine *machine) {
  // cout << "SERVER TIME: " << RakNet::GetTimeMS()/1000.0f/60.0f << endl;
  // printf("Updating server\n");

  RakNet::Packet *p;
  while (true) {
    p = rakInterface->Receive();
    if (p == NULL) {
      break;
    }

    // We got a packet, get the identifier with our handy function
    int packetIdentifier = GetPacketIdentifier(p);
    // printf("GOT PACKET %d\n", int(packetIdentifier));

    // Check if this is a network message packet
    switch (packetIdentifier) {
    case ID_CLIENT_HANDSHAKE:
      cout << "GOT ID_CLIENT_HANDSHAKE\n";
      
      if (blockNewClients) {
        cout << "NOT ACCEPTING NEW CLIENTS\n";
        // We aren't allowing new clients
        rakInterface->CloseConnection(p->sender);
        break;
      }

      acceptPeer(p->sender, machine);
      break;

    case ID_INPUTS: {
      string s = doInflate(GetPacketData(p), GetPacketSize(p));
      PeerInputDataList inputDataList;
      inputDataList.ParseFromString(s);
      receiveInputs(&inputDataList);
      break;
    }
    default:
      printf("UNEXPECTED PACKET ID: %d\n", packetIdentifier);
      break;
    }

    rakInterface->DeallocatePacket(p);
  }

  return true;
}

void Server::sync(running_machine *machine)
{
  cout << "SYNCING (count): " << syncCount << endl;

  machine->save().dispatch_presave();

  if (syncOverride)
  {
    return;
  }

  syncProto.set_generation(generation);
  nsm::Attotime *global_time = syncProto.mutable_global_time();
  global_time->set_seconds(machine->machine_time().seconds);
  global_time->set_attoseconds(machine->machine_time().attoseconds);
  syncProto.clear_block();

  staleGeneration = generation;
  staleTime = newAttotime(machine->machine_time().seconds,
                          machine->machine_time().attoseconds);

  cout << "IN CRITICAL SECTION\n";

  int bytesSynched = 0;
  // cout << "IN CRITICAL SECTION\n";
  // cout << "SERVER: Syncing with clients\n";
  bool anyDirty = false;
  unsigned char blockChecksum = 0;
  unsigned char xorChecksum = 0;
  unsigned char staleChecksum = 0;
  unsigned char allStaleChecksum = 0;
  for (int blockIndex = 0; blockIndex < int(blocks.size()); blockIndex++)
  {
    MemoryBlock &block = *(blocks[blockIndex]);
    MemoryBlock &staleBlock = *(staleBlocks[blockIndex]);
    MemoryBlock &initialBlock = *(initialBlocks[blockIndex]);

    if (block.size != staleBlock.size || block.size != initialBlock.size)
    {
      cout << "BLOCK SIZE MISMATCH: " << blockIndex << ": " << block.size << " "
           << staleBlock.size << " " << initialBlock.size << endl;
    }

    bool dirty = false;
    if (syncCount == 0 || memcmp(block.data, staleBlock.data, block.size))
    {
      dirty = true;
    }

    if (dirty)
    {
      for (int a = 0; a < block.size; a++)
      {
        blockChecksum = blockChecksum ^ block.data[a];
        staleChecksum = staleChecksum ^ staleBlock.data[a];
      }
    }
    
    // dirty=true;
    if (syncCount == 0)
    {
      memcpy(initialBlock.data, block.data, block.size);
    }

    if (dirty && !anyDirty)
    {
      // First dirty block
      anyDirty = true;
    }

    if (dirty)
    {
      bytesSynched += block.size;
      nsm::SyncBlock *syncBlock = syncProto.add_block();
      syncBlock->set_index(blockIndex);
      string *s = syncBlock->mutable_data();
      for (int a = 0; a < block.size; a++)
      {
        s->append(1, block.data[a] ^ staleBlock.data[a]);
      }

      // Put the current block into stale blocks
      memcpy(staleBlock.data, block.data, block.size);
    }

    // cout << "BLOCK " << blockIndex << ": ";
    for (int a = 0; a < block.size; a++)
    {
      allStaleChecksum = allStaleChecksum ^ staleBlock.data[a];
    }
    // cout << int(allStaleChecksum) << endl;
  }

  printf("BLOCK CHECKSUM: %d\n", int(blockChecksum));
  printf("XOR CHECKSUM: %d\n", int(xorChecksum));
  printf("STALE CHECKSUM (dirty): %d\n", int(staleChecksum));
  printf("STALE CHECKSUM (all): %d\n", int(allStaleChecksum));
  
  // The first sync is not sent to clients
  //if (syncCount > 0)
  {
    syncProcessor = std::shared_ptr<SyncProcessor>(new SyncProcessor(
        &syncProto, &syncPacketQueue, syncTransferSeconds, &syncReady));
  }

  cout << "OUT OF CRITICAL AREA\n";
  cout.flush();

  machine->save().dispatch_postload();

  {
    unsigned char blockChecksum = 0;
    for (int blockIndex = 0; blockIndex < int(blocks.size()); blockIndex++) {
      MemoryBlock &block = *(blocks[blockIndex]);

      for (int a = 0; a < block.size; a++) {
        blockChecksum = blockChecksum ^ block.data[a];
      }
    }

    printf("BLOCK CHECKSUM POST LOAD: %d\n", int(blockChecksum));
  }

  syncCount++;
}

long long lastSyncQueueMs = -1;

void Server::popSyncQueue() {
  if (!syncReady)
    return;

  if (syncProcessor)
  {
    syncProcessor->process();
  }

  long long curRealTime = RakNet::GetTimeMS() - emulationStartTime;

  // cout << "SYNC TIMES: " << (curRealTime/100) << " " << (lastSyncQueueMs/100)
  // << endl;
  if (lastSyncQueueMs / 100 == curRealTime / 100) {
    return;
  }

  // cout << "sending packet (if it exists)" << endl;
  lastSyncQueueMs = curRealTime;
  if (syncPacketQueue.size()) {
    pair<unsigned char *, int> syncPacket = syncPacketQueue.front();
    printf("Sending sync message of size %d (%lu packets left)\n",
           syncPacket.second, syncPacketQueue.size());
    syncPacketQueue.pop_front();
    cout << "PACKET HEADER: " << int(syncPacket.first[0]) << " SIZE "
         << syncPacket.second << endl;

    // TODO: Found memory corruption when waiting to send, either
    // change all of these to immediate or find the corruption.
    rakInterface->Send((const char *)syncPacket.first, syncPacket.second);
  }
}

void Server::sendBaseDelay(int baseDelay) {
  char dataToSend[5];
  dataToSend[0] = ID_BASE_DELAY;
  memcpy(dataToSend + 1, &baseDelay, sizeof(int));
  // cout << "SENDING MESSAGE WITH LENGTH: " << intSize << endl;
  rakInterface->Send(dataToSend, 5);
}
