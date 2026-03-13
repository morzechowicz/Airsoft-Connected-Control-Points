#ifndef BLE_TYPES_H
#define BLE_TYPES_H

#include <BLECharacteristic.h>
#include <BLEServer.h>
#include <NimBLEDevice.h>
#include <Config.h>
#include <EventBus.h>
#include <ConfigurationHandler.h>
#include "BleServer.h"
#include "BleCallback.h"


class BleSetup
{
    private:
    EventBus &eventBus;
    ConfigurationHandler &configHandler;
    
    BLEServer *pServer;
    BLECharacteristic *pCharacteristic;
public:
    BleSetup(EventBus &eb, ConfigurationHandler &ch):
        eventBus(eb), configHandler(ch) {};
    void BleStart();
    ~BleSetup();
};




#endif // BLE_TYPES_H