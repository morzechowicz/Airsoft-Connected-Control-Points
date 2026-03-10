#include "BleServer.h"

BleServer::BleServer()
{
}

void BleServer::onConnect(BLEServer *pServer, NimBLEConnInfo &connInfo)
{
        deviceConnected = true;
        Serial.println("Device connected");
}
void BleServer::onDisconnect(BLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
{
        deviceConnected = false;
        Serial.println("Device disconnected");
        pServer->startAdvertising();  // Restart advertising
}