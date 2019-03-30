#include <unistd.h>
#include <emscripten/emscripten.h>
#include "raknet_shim.h"

using namespace RakNet;

RakPeerInterface* RakPeerInterface::instance = NULL;

void RakPeerInterface::Send(const char* data, int length, int, int, char, SystemAddress address, bool broadcast, int)
{
  /*EM_ASM(
    console.log('RakPeerInterface::Send')
  );*/
}

Packet* RakPeerInterface::Receive()
{
  /*EM_ASM(
    console.log('RakPeerInterface::Receive')
  );*/

  return NULL;
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
