#ifndef BLE_CALLBACK_H
#define BLE_CALLBACK_H

#include <NimBLEDevice.h>
#include <config.h>
#include "GameState.h"
#include <ControlPoint.h>
#include <StatusLog.h>
#include "LoRaCom.h"
#include "LoRaMsg.h"

// Example BLE Server Callback class
class BleCallback : public BLECharacteristicCallbacks
{
public:
    BleCallback(Config &config, GameState &state, ControlPoint &cp, StatusLog &log, LoRaCom &loraCom, LoRaMsg &loramsg);
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo) override;

private:
    Config &config;
    GameState &state;
    ControlPoint &cp;
    StatusLog &statusLog;
    String msg;
    LoRaCom &loraCom;
    LoRaMsg &loramsg;
    BLECharacteristic *pCharacteristic;
};

#endif // BLE_CALLBACK_H