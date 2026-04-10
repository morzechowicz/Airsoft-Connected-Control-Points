#ifndef BLE_TYPES_H
#define BLE_TYPES_H

#include <BLECharacteristic.h>
#include <BLEServer.h>
#include <NimBLEDevice.h>
#include <Config.h>
#include <EventBus.h>
#include <MessageHandler.h>
#include "BleServer.h"
#include "BleCallback.h"
#include "LogManager.h"

class BleSetup
{
    private:
    EventBus &eventBus;
    MessageHandler &configHandler;
    
    BLEServer *pServer;
    BLECharacteristic *pCharacteristic;
    BleServer *bleServer;
public:
    BleSetup(EventBus &eb, MessageHandler &ch):
        eventBus(eb), configHandler(ch), bleServer(nullptr) {};
    void BleStart();
    BleServer* getBleServer() const { return bleServer; }
    ~BleSetup();

    void sendMessage(const String& message);
};
#endif // BLE_TYPES_H