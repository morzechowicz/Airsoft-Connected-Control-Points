#include "BleServer.h"

BleServer::BleServer()
{
}

void BleServer::onConnect(BLEServer *pServer, NimBLEConnInfo &connInfo)
{
        deviceConnected = true;
        LOG_INFO("BLE_SERVER", "Device connected");
}
void BleServer::onDisconnect(BLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
{
        deviceConnected = false;
        LOG_INFO("BLE_SERVER", "Device disconnected");
        pServer->startAdvertising();  // Restart advertising
}

void BleServer::sendMessage(const String& message) {
    if (deviceConnected && pCharacteristic) {
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
    }
}