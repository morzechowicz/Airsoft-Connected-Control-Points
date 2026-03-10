#ifndef BLE_CALLBACK_H
#define BLE_CALLBACK_H

#include <NimBLEDevice.h>
#include <config.h>
#include <EventBus.h>
#include <Protocol.h>
#include "ConfigurationHandler.h"


// Example BLE Server Callback class
class BleCallback : public BLECharacteristicCallbacks
{
public:
    BleCallback(EventBus &ev,ConfigurationHandler &handler);
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
    void onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
private:
    EventBus& eventBus;
    ConfigurationHandler& handler;
};

#endif // BLE_CALLBACK_H