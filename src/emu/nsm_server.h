#include "nsm_common.h"

#include "zlib.h"

Server *createGlobalServer(std::string _username, int _unmeasuredNoise, bool _rollback);

void deleteGlobalServer();

class running_machine;
class SyncProcessor;

class Server : public Common {
  public:
  bool syncOverride;

  protected:
  std::vector<std::shared_ptr<MemoryBlock> > initialBlocks;
  nsm::Attotime staleTime;
  int staleGeneration;

  int syncTransferSeconds;
  int syncCount;
  bool syncReady;
  bool blockNewClients;
  int maxPeerID;
  int nextPeerID;
  nsm::Sync syncProto;

  std::list<std::pair<unsigned char *, int> > syncPacketQueue;
  std::unordered_map<std::string, int> deadPeerIDs;
  std::shared_ptr<SyncProcessor> syncProcessor;

  public:
  Server(std::string _username, int _unmeasuredNoise, bool _rollback);

  virtual ~Server();

  void shutdown();

  void acceptPeer(const std::string& guid, running_machine *machine);

  void removePeer(const std::string& guid, running_machine *machine);

  std::vector<std::shared_ptr<MemoryBlock> >
  createMemoryBlock(const std::string &name, unsigned char *ptr, int size);

  void initialSync(const std::string& guid, running_machine *machine);

  virtual nsm::PeerInputData popInput(int peerID);

  bool update(running_machine *machine);

  void sync(running_machine *machine);

  void popSyncQueue();

  void setSyncTransferTime(int _syncTransferSeconds) {
    syncTransferSeconds = _syncTransferSeconds;
  }

  void sendBaseDelay(int baseDelay);

  inline void setBlockNewClients(bool blockNewClients) {
    this->blockNewClients = blockNewClients;
  }

  inline bool isBlockNewClients() { return blockNewClients; }
};
