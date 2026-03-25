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
        LOG_ERROR("BLE_CALLBACK", "Failed to handle command: %s", value.c_str());
    }
}

void BleCallback::onRead(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
{
}
