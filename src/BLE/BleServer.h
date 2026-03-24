#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <NimBLEDevice.h>
#include <config.h>
#include "BleCallback.h"

class BleServer : public BLEServerCallbacks
{
public:
    BleServer();
    void onConnect(BLEServer *pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(BLEServer *pServer, NimBLEConnInfo& connInfo, int reason) override;

    bool isDeviceConnected() const { return deviceConnected; }
    void sendMessage(const String& message);
private:
    BLEServer *pServer;
    BLECharacteristic *pCharacteristic;
    bool deviceConnected;
};

#endif // BLE_SERVER_H