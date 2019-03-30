#include <unistd.h>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include "raknet_shim.h"

using namespace RakNet;

RakPeerInterface* RakPeerInterface::instance = NULL;

struct JSPacket
{
  std::vector<char> data;
  unsigned int length;
  unsigned int address;

  JSPacket()
  {
    length = 0;
    address = 0;
  }
};

JSPacket emptyPacket;

std::vector<JSPacket> sendQueue;
std::vector<JSPacket> recvQueue;

bool JS_HasEnqueuedPackets()
{
  return !sendQueue.empty();
}

JSPacket JS_FetchNextEnqueuedPacket()
{
  if (sendQueue.empty())
  {
    return emptyPacket;
  }

  JSPacket packet = sendQueue.back();
  sendQueue.pop_back();
  return packet;
}

void JS_EnqueueIncomingPacket(JSPacket packet)
{
  recvQueue.push_back(packet);
}

EMSCRIPTEN_BINDINGS(js_packet) {
  emscripten::value_object<JSPacket>("JSPacket")
    .field("data", &JSPacket::data)
    .field("length", &JSPacket::length)
    .field("address", &JSPacket::address)
    ;

  emscripten::function("JS_HasEnqueuedPackets", &JS_HasEnqueuedPackets);
  emscripten::function("JS_FetchNextEnqueuedPacket", &JS_HasEnqueuedPackets);
  emscripten::function("JS_EnqueueIncomingPacket", &JS_HasEnqueuedPackets);
}

void RakPeerInterface::Send(const char* data, int length, int, int, char, SystemAddress address, bool broadcast, int)
{
  JSPacket packet;

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

  sendQueue.push_back(std::move(packet));
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
  packet->length = js_packet.length;
  packet->data = new unsigned char[js_packet.length];
  memcpy(packet->data, js_packet.data.data(), js_packet.length * sizeof(char));

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
