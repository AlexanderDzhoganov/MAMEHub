#include <unistd.h>
#include <iostream>

#include <emscripten/emscripten.h>
#include "raknet_shim.h"

using namespace RakNet;

RakPeerInterface* RakPeerInterface::instance = NULL;

struct JSPacket
{
  std::vector<char> data;
  unsigned int address;
};

std::vector<JSPacket> sendQueue;
std::vector<JSPacket> recvQueue;

#define MAX_PACKET_SIZE 65535

extern "C" {
  void jsArrayTest(char* array)
  {
    array[0] = 3;
    array[1] = 1;
    array[2] = 4;
    array[3] = 5;
  }

  int jsGetNextPacket(char* data, int* metadata)
  {
    if (sendQueue.empty())
    {
      return 0;
    }

    JSPacket& packet = sendQueue.back();

    memcpy(data, packet.data.data(), packet.data.size() * sizeof(char));
    metadata[0] = (int)packet.data.size();
    metadata[1] = packet.address;

    sendQueue.pop_back();

    return 1;
  }

  void jsEnqueuePacket(char* data, int length, int address)
  {
    JSPacket packet;
    packet.data.resize(length);
    memcpy(packet.data.data(), data, length * sizeof(char));
    packet.address = address;
    recvQueue.push_back(packet);
  }
}

void RakPeerInterface::Send(const char* data, int length, int, int, char, SystemAddress address, bool broadcast, int)
{
  if (length >= MAX_PACKET_SIZE)
  {
    std::cout << "PACKET LENGTH >= MAX_PACKET SIZE" << std::endl;
    return;
  }

  sendQueue.push_back(JSPacket());
  JSPacket& packet = sendQueue.back();

  packet.data.resize(length);
  memcpy(packet.data.data(), data, length * sizeof(char));

  if (broadcast)
  {
    packet.address = 0;
  }
  else
  {
    packet.address = address.g;
  }

  std::cout << "PACKET SENT (" << length << " BYTES)" << std::endl;
}

Packet* RakPeerInterface::Receive()
{
  if (recvQueue.empty())
  {
    return NULL;
  }

  JSPacket js_packet = recvQueue.back();
  recvQueue.pop_back();

  Packet* packet = new Packet();
  packet->guid = js_packet.address;
  packet->systemAddress = js_packet.address;
  packet->length = js_packet.data.size();
  packet->data = new unsigned char[js_packet.data.size()];
  memcpy(packet->data, js_packet.data.data(), js_packet.data.size() * sizeof(char));
  std::cout << "PACKET RECEIVED" << std::endl;

  return packet;
}

void RakPeerInterface::Send(const char* data, int length, int, int, char, RakNetGUID guid, bool broadcast, int)
{
  SystemAddress addess = GetSystemAddressFromGuid(guid);
  return Send(data, length, 0, 0, 0, address, broadcast);
}

void RakPeerInterface::Send(const BitStream* stream, int, int, char, RakNetGUID guid, bool broadcast, int)
{
  SystemAddress addess = GetSystemAddressFromGuid(guid);
  return Send(stream->data, stream->dataPtr, 0, 0, 0, addess, broadcast);
}

void RakSleep(int ms) {
  usleep(ms);
}

Time RakNet::GetTimeMS()
{
  return (Time)emscripten_get_now();
}
