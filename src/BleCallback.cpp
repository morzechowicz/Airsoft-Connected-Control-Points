#include <BleCallback.h>

BleCallback::BleCallback(Config &config, GameState &state, ControlPoint &cp)
 : config(config), state(state),cp(cp)
{
}

void BleCallback::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)
{
    std::string value = pCharacteristic->getValue();
    if (!value.empty())
    {
        Serial.print("BLE Received: ");
        Serial.println(value.c_str());
        String configString = value.c_str();
        // Example: Parse "C/From/to/seq/10/20/30/40" into countdown, duration, pointsTarget, captureTime
        config.fromString(configString);
        if(!cp.getGameMaster())
        {
            cp.setGameMaster(true);
            state = GameState::CountDownSetup;
        }
    }
}