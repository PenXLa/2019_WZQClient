#include "./JSONLib/CJsonObject.hpp"
bool connect2Server();
void startReceiving();
void sendPack(neb::CJsonObject&);
void onReceive(const neb::CJsonObject& json);
void onDisconnected();