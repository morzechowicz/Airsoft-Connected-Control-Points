#ifndef BLE_CALLBACK_H
#define BLE_CALLBACK_H

#include <NimBLEDevice.h>
#include "Config.h"
#include <EventBus.h>
#include <Protocol.h>
#include "MessageHandler.h"


// Example BLE Server Callback class
class BleCallback : public BLECharacteristicCallbacks
{
public:
    BleCallback(EventBus &ev,MessageHandler &handler);
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
    void onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;
private:
    EventBus& eventBus;
    MessageHandler& handler;
};

#endif // BLE_CALLBACK_H