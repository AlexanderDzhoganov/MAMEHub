#include <unistd.h>
#include <iostream>
#include <deque>

#include <emscripten/emscripten.h>
#include "raknet_shim.h"

using namespace RakNet;

RakPeerInterface* RakPeerInterface::instance = NULL;

struct JSPacket
{
  std::vector<char> data;
  std::string sender;
  std::string recipient;
};

std::deque<JSPacket> sendQueue;
std::deque<JSPacket> recvQueue;

#define MAX_PACKET_SIZE 65535

extern "C" {
  void jsArrayTest(char* array)
  {
    array[0] = 3;
    array[1] = 1;
    array[2] = 4;
    array[3] = 5;
  }

  int jsGetNextPacket(char* data, char* recipient)
  {
    if (sendQueue.empty())
    {
      return 0;
    }

    JSPacket& packet = sendQueue.front();

    memcpy(data, packet.data.data(), packet.data.size() * sizeof(char));

    memcpy(recipient, packet.recipient.c_str(), packet.recipient.length());
    recipient[packet.recipient.length()] = 0;

    int length = (int)packet.data.size();
    sendQueue.pop_front();

    return length;
  }

  void jsEnqueuePacket(char* data, int length, char* sender)
  {
    JSPacket packet;

    packet.data.resize(length);
    memcpy(packet.data.data(), data, length * sizeof(char));

    packet.sender = std::string(sender);

    recvQueue.push_back(packet);
  }
}

Packet* RakPeerInterface::Receive()
{
  if (recvQueue.empty())
  {
    return NULL;
  }

  JSPacket js_packet = recvQueue.front();
  recvQueue.pop_front();

  Packet* packet = new Packet();
  packet->sender = js_packet.sender;
  packet->recipient = js_packet.recipient;
  packet->length = js_packet.data.size();
  packet->data = new unsigned char[js_packet.data.size()];
  memcpy(packet->data, js_packet.data.data(), js_packet.data.size() * sizeof(char));

  return packet;
}

void RakPeerInterface::Send(const char* data, int length, const std::string& peer)
{
  if (length >= MAX_PACKET_SIZE)
  {
    std::cout << "PACKET LENGTH >= MAX_PACKET SIZE" << std::endl;
    return;
  }

  sendQueue.push_back(JSPacket());
  JSPacket& packet = sendQueue.back();

  packet.recipient = peer;
  packet.data.resize(length);
  memcpy(packet.data.data(), data, length * sizeof(char));
}

void RakSleep(int ms)
{
  usleep(ms);
}

Time RakNet::GetTimeMS()
{
  return (Time)emscripten_get_now();
}
