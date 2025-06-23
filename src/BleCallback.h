#ifndef BLE_CALLBACK_H
#define BLE_CALLBACK_H

#include <NimBLEDevice.h>
#include <config.h>
#include "GameState.h"
#include <ControlPoint.h>

// Example BLE Server Callback class
class BleCallback : public BLECharacteristicCallbacks
{
public:
    BleCallback(Config &config,GameState &state,ControlPoint &cp);
    // void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
private:
    Config &config;
    GameState &state;
    ControlPoint &cp;
    BLECharacteristic *pCharacteristic;
};

#endif // BLE_CALLBACK_H