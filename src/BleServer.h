#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <NimBLEDevice.h>
#include <BleCallback.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BleServer : public BLEServerCallbacks
{
public:
    BleServer();
    void onConnect(BLEServer *pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(BLEServer *pServer, NimBLEConnInfo& connInfo, int reason) override;

private:
    BLEServer *pServer;
    BLECharacteristic *pCharacteristic;
    bool deviceConnected;
};

#endif // BLE_SERVER_H