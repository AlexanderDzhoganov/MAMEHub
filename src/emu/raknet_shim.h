#ifndef RAKNET_SHIM_H
#define RAKNET_SHIM_H

#include <cstdlib>
#include <vector>
#include <string>

/*
 * RakNet library shim, calls into JavaScript
 */

enum
{
  ID_TIMESTAMP = 0,
  ID_USER_PACKET_ENUM
};

enum
{
  IMMEDIATE_PRIORITY = 0,
  RELIABLE,
  RELIABLE_ORDERED
};

void RakSleep(int time);

namespace RakNet
{
  enum
  {
    CONNECTION_ATTEMPT_STARTED = 0,
    RAKNET_STARTED = 0
  };

  typedef unsigned int Time;
  typedef unsigned int StartupResult;

  Time GetTime();
  Time GetTimeMS();

  struct SystemAddress
  {
    public:
    unsigned int g;
    std::string s;

    SystemAddress() : g(0), s("0") {}
    SystemAddress(unsigned int _g) : g(_g), s(std::to_string(_g)) {}
    const char* ToString(bool = false) const { return s.c_str(); }
    void SetBinaryAddress(char* address) { g = atoi(address); s = address; }
    unsigned short GetPort() { return 0; }
    bool operator==(const SystemAddress& rhs) const { return g == rhs.g; }
    bool operator!=(const SystemAddress& rhs) const { return g != rhs.g; }
  };

  static SystemAddress UNASSIGNED_SYSTEM_ADDRESS;

  struct RakNetGUID
  {
    unsigned int g;
    std::string s;

    RakNetGUID()
    {
      // allocate random guid
      g = 1 + random();
      s = std::to_string(g);
    }

    RakNetGUID(unsigned int _g) : g(_g), s(std::to_string(_g)) {}
    const char* ToString(bool = false) const { return s.c_str(); }
    bool operator==(const RakNetGUID& rhs) const { return g == rhs.g; }
    bool operator!=(const RakNetGUID& rhs) const { return g != rhs.g; }
    bool operator<(const RakNetGUID& rhs) const { return g < rhs.g; }
  };

  class BitStream
  {
    public:
    char* data;
    int dataPtr;
    bool ownsData;

    BitStream(unsigned int length) : dataPtr(0), ownsData(true)
    {
      data = new char[length];
    }

    BitStream(unsigned char* _data, unsigned int length, bool copyData) : dataPtr(0)
    {
      if (copyData)
      {
        data = new char[length];
        ownsData = true;
        memcpy(data, _data, length * sizeof(char));
      }
      else
      {
        data = (char*)_data;
        ownsData = false;
      }
    }

    ~BitStream()
    {
      if (ownsData)
      {
        delete [] data;
      }
    }

    void SetWriteOffset(unsigned int offset)
    {
      dataPtr = offset;
    }

    void WriteBits(const unsigned char* _data, int length)
    {
      memcpy(data + dataPtr, _data, length * sizeof(char));
      dataPtr += length;
    }

    template <typename T> void Write(T value)
    {
      memcpy(data + dataPtr, &value, sizeof(T));
      dataPtr += sizeof(T);
    }

    template <typename T> void Read(T& _data)
    {
      _data = *((T*)(data + dataPtr));
      dataPtr += sizeof(T);
    }

    // stubbed
    void EndianSwapBytes(int byteOffset, int length) {}
  };

  enum
  {
    ACTUAL_BYTES_SENT = 0,
    ACTUAL_BYTES_RECEIVED = 1
  };

  struct RakNetStatistics
  {
    int valueOverLastSecond[4];
    double packetlossLastSecond;
  };

  struct SocketDescriptor
  {
    SocketDescriptor(int, int) {}
    unsigned short port;
  };

  struct RakPeer
  {
    RakNetGUID guid;
    SystemAddress systemAddress;
    RakNetStatistics stats;
  };

  struct Packet
  {
    RakNetGUID guid;
    SystemAddress systemAddress;
    unsigned char* data;
    unsigned int length;

    Packet()
    {
      data = NULL;
      length = 0;
    }

    ~Packet()
    {
      if (data != NULL)
      {
        delete [] data;
      }
    }
  };

  class RakPeerInterface
  {
    static RakPeerInterface* instance;
    std::vector<RakPeer> peers;
    RakNetGUID guid;
    SystemAddress address;

    public:
    static RakPeerInterface* GetInstance()
    {
      if (instance == NULL)
      {
        instance = new RakPeerInterface();
      }

      return instance;
    }

    static void DestroyInstance(RakPeerInterface*)
    {
      if (instance != NULL)
      {
        delete instance;
        instance = NULL;
      }
    }
    
    // implementation
    RakNetGUID GetMyGUID() const
    {
      return guid;
    }

    SystemAddress GetSystemAddressFromIndex(int index) const
    {
      if (index < 0 || index >= peers.size())
      {
        return UNASSIGNED_SYSTEM_ADDRESS;
      }

      return peers[index].systemAddress;
    }

    int NumberOfConnections() const
    {
      return peers.size();
    }

    RakNetGUID GetGuidFromSystemAddress(SystemAddress address) const
    {
      for (unsigned int i = 0; i < peers.size(); i++)
      {
        if (peers[i].systemAddress == address)
        {
          return peers[i].guid;
        }
      }

      return RakNetGUID(0);
    }

    SystemAddress GetSystemAddressFromGuid(RakNetGUID guid) const
    {
      for (unsigned int i = 0; i < peers.size(); i++)
      {
        if (peers[i].guid == guid)
        {
          return peers[i].systemAddress;
        }
      }

      return UNASSIGNED_SYSTEM_ADDRESS;
    }

    void CloseConnection(RakNetGUID guid, bool, unsigned char = 0, int = 0)
    {
      for (std::vector<RakPeer>::iterator it = peers.begin(); it != peers.end(); ++it)
      {
        if ((*it).guid == guid)
        {
          peers.erase(it);
          return;
        }
      }
    }
    
    void Send(const char* data, int length, SystemAddress address, bool broadcast = false);

    void Send(const char* data, int length, RakNetGUID guid, bool broadcast = false)
    {
      return Send(data, length, GetSystemAddressFromGuid(guid), broadcast);
    }

    void Send(const BitStream* stream, RakNetGUID guid, bool broadcast = false)
    {
      return Send(stream->data, stream->dataPtr, GetSystemAddressFromGuid(guid), broadcast);
    }

    Packet* Receive();
    void DeallocatePacket(Packet* packet) { delete packet; }

    SystemAddress GetExternalID(SystemAddress) const { return address; }

    int GetLastPing(RakNetGUID guid) const { return 0; }
    int GetLastPing(SystemAddress address) const { return 0; }
    int GetAveragePing(RakNetGUID address) const { return 0; }
    
    RakNetStatistics* GetStatistics(SystemAddress address)
    {
      for (unsigned int i = 0; i < peers.size(); i++)
      {
        if (peers[i].systemAddress == address)
        {
          return &peers[i].stats;
        }
      }

      return NULL;
    }
  };
}

#endif
