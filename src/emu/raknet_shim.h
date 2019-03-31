#ifndef RAKNET_SHIM_H
#define RAKNET_SHIM_H

#include <cstdlib>
#include <vector>
#include <string>

/*
 * RakNet library shim, calls into JavaScript
 */

#define MAX_PACKET_SIZE 65535

void RakSleep(int time);

namespace RakNet
{
  typedef unsigned int Time;

  Time GetTimeMS();

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

    void WriteBytes(const unsigned char* _data, int length)
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

  struct Packet
  {
    std::string sender;
    std::string recipient;
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
    
    void CloseConnection(const std::string& peer)
    {
      // TODO FIXME
    }
    
    void Send(const char* data, int length, const std::string& peer = "");
    Packet* Receive();
    void DeallocatePacket(Packet* packet) { delete packet; }

    int GetLastPing(const std::string& peer) const { return 0; }
    int GetAveragePing(const std::string& peer) const { return 0; }
    
    RakNetStatistics* GetStatistics(const std::string& peer)
    {
      return NULL;
    }
  };
}

#endif
