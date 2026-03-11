#include "BleCallback.h"

BleCallback::BleCallback(EventBus &ev, ConfigurationHandler &handler) : eventBus(ev), handler(handler)
{
}

void BleCallback::onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
{
    std::string value = pCharacteristic->getValue();
    Message msg;
    msg = Protocol::parse(value.c_str(), value.length());
    bool result = handler.handleCommand(msg);
    if (!result)
    {
        Serial.println("Something went wrong at Configuration handler");
    }
}

void BleCallback::onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
{
}
