#include <BleCallback.h>

BleCallback::BleCallback(Config &config, GameState &state, ControlPoint &cp, StatusLog &log, LoRaCom &loraCom, LoRaMsg &loramsg)
    : config(config), state(state), cp(cp), statusLog(log), loraCom(loraCom), loramsg(loramsg)
{
}

void BleCallback::onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo &connInfo)
{
    std::string value = pCharacteristic->getValue();
    if (!value.empty())
    {
        Serial.print("BLE Received: ");
        Serial.println(value.c_str());
        String configString = value.c_str();
        // Example: Parse "C/From/to/seq/10/20/30/40" into countdown, duration, pointsTarget, captureTime
        if (value[0] == 'C')
        {
            Serial.println("C");
            config.fromString(configString);

            cp.setGameMaster(true);
            state = GameState::CountDownSetup;
        }
        // restoring the game data in case of gm board failure
        if (value[0] == 'R')
        {
            Serial.println("R");
            state = GameState::Restore;
        }
        // Get log data
        if (value[0] == 'L')
        {
            Serial.println("L");
            // Convert the log entries to a string
            String logData = "";
            logData = statusLog.getLogAsString();
            // Call statusLog methods to get log data
            statusLog.printLog(); // This currently prints to Serial
            pCharacteristic->setValue(logData.c_str());
            pCharacteristic->notify();
        }
        // End and restart
        if (value[0] == 'E')
        {
            Serial.println("E");
            state = GameState::Finished;
        }
        // Send the report packet to find other nodes
        if (value[0] == 'N')
        {
            loraCom.seqNum = loraCom.seqNum + 1;
            msg = loramsg.createNodeInfo(0, loraCom.seqNum);
            loraCom.sendMsgAckTo(msg, 0);
            Serial.println("Sending node info");
            statusLog.addLogEntry(EVENT_SYSTEM_SEND_NODE_INFO);
        }

    }
}