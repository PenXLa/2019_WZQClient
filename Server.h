#ifndef _SERVER_
#define _SERVER_

#include <winsock2.h>
#include "./JSONLib/CJsonObject.hpp"


bool connect2Server();
void startReceiving();
void sendPack(neb::CJsonObject&);
void onReceive(neb::CJsonObject& json);
void onDisconnected();
extern SOCKET server;
#endif