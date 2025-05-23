#include <LoRaManager.h>

volatile bool LoRaManager::recFlag = false;
volatile bool LoRaManager::transDoneFlag = false;

LoRaManager::LoRaManager() {}

void LoRaManager::begin()
{
    // Generate a random nodeId between 1 and 255
    randomSeed(analogRead(0));
    nodeId = random(1, 2147483647);
    int state = radio.begin(433.0F, 125.0F, 9U, 5U, 18U, 12, 8U, 0U);

    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println("LoRa initializetiion successful");
    }
    else
    {
        Serial.println("LoRa Initialization failed Err: ");
        Serial.println(state);
    }

    radio.setPacketSentAction(setTransFlag);
    radio.setPacketReceivedAction(setRecFlag);
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
        {
            delay(10);
        }
    }
}

void LoRaManager::recivingLoop()
{
    if (recFlag)
    {
        // reset flag
        recFlag = false;

        String str;
        int state = radio.readData(str);

        /*
          byte byteArr[8];
          int numBytes = radio.getPacketLength();
          int state = radio.readData(byteArr, numBytes);
        */

        if (state == RADIOLIB_ERR_NONE)
        {
            Serial.println(F("[SX1280] Received packet!"));
            Serial.print(F("[SX1280] Data:\t\t"));
            Serial.println(str);
            recivedMessageHandler(str);
        }
        else if (state == RADIOLIB_ERR_CRC_MISMATCH)
        {
            // packet was received, but is malformed
            Serial.println(F("CRC error!"));
        }
        else
        {
            // some other error occurred
            Serial.print(F("failed, code reciving "));
            Serial.println(state);
        }
        radio.startReceive();
        Serial.println("Reciving again");
    }
}

void LoRaManager::recivedMessageHandler(String recived)
{
    extern Config config;
    extern GameState gameState;
    extern ControlPoint controlPoint;
    switch (recived[0])
    {
    case 'A':
        // ack should never raech here but just in case
        Serial.println("Recived ACK that should not happend");
        break;
    case 'N':
        Serial.println("Recived node controlled by");
        partCount = 0;
        lastIndex = 0;

        for (int i = 0; i <= recived.length(); i++)
        {
            if (i == recived.length() || recived[i] == '/')
            {
                if (partCount < 10)
                {
                    parts[partCount++] = recived.substring(lastIndex, i);
                }
                lastIndex = i + 1;
            }
        }
        controlPoint.setNodeControllingTeam(parts[2].toInt(), static_cast<TeamId>(parts[1].toInt()));
        sendMsg("ACK");
        break;
    case 'S':
        Serial.println("Recived score");
        break;
    case 'C':
        Serial.println("Recived new config");
        config.fromString(recived);
        gameState = GameState::CountDown;
        Serial.println("Config updated from received message");
        // sendMsg("ACK");
        break;
    case 'L':

            recivedId = recived.substring(2).toInt();
            Serial.println("Ping from node: ");
            Serial.println(recivedId);
            controlPoint.addNode(recivedId);
            sendMsg("ACK");

        break;
    default:
        break;
    }
}

void LoRaManager::sendNewConfig(Config config)
{
    Serial.println("Sending new config");
    String msg = "C/" + String(config.getCountdown()) +
                 "/" + String(config.getDurration()) +
                 "/" + String(config.getPointsTarget()) +
                 "/" + String(config.getCaptureTime());
    sendMsg(msg);
}

void LoRaManager::sendAllTotalTeamsScore(const Team *teams, size_t teamCount)
{
    Serial.println("Sending score to all");
    String msg = "S/";
    for (size_t i = 0; i < teamCount; i++)
    {
        msg += String(static_cast<int>(teams[i].getTeamId())) + ":" + String(teams[i].getTeamPoints()) + "/";
    }
    sendMsg(msg);
}

void LoRaManager::sendGameFinished(const Team *teams, size_t teamCount)
{
    Serial.println("Sending game has ended");
    String msg = "F/";
}

void LoRaManager::sendNodeControlledBy(Team team)
{
    Serial.println("Sending node controlled by");
    String msg = "N/" + String(static_cast<int>(team.getTeamId())) + "/" + String(nodeId);
    bool result = sendMsgAck(msg);
    if (result)
    {
        Serial.println("N ACK recived");
    }
    else
    {
        Serial.println("N ACK not recived");
    }
}

void LoRaManager::connectThisNode()
{
    Serial.println("Sending nodeId to leader");
    String msg = "L/" + String(nodeId);
    bool state = sendMsgAck(msg);

    if (state)
    {
        Serial.println("Leader ACK this node");
    }
    else
    {
        Serial.println("Leader not responding");
    }
}

void LoRaManager::sendMsg(String msg)
{
    if (!transDoneFlag)
    {
        transDoneFlag = true;
        String str = msg;
        Serial.print("sneeding");
        Serial.println(millis());
        transmissionState = radio.startTransmit(str);

        if (transmissionState == RADIOLIB_ERR_NONE)
        {
            Serial.println(F("transmission finished!"));
            Serial.print("done");
            Serial.println(millis());
        }
        else
        {
            Serial.print(F("failed, code transmission "));
            Serial.println(transmissionState);
            Serial.print("error at: ");
            Serial.println(millis());
        }
    }
}

bool LoRaManager::sendMsgAck(String msg)
{
    String str = msg;
    Clocker ackClock;
    int timeout = 5;
    int wait = 10;
    ackClock.start();
    while (timeout < ackClock.getElapsedTimeInSeconds())
    {
        if (!transDoneFlag)
        {
            transDoneFlag = true;
            sendMsg(str);
        }
        else
        {
            radio.startReceive();
        }
        if (recFlag)
        {
            recFlag = false;
            String str;
            int state = radio.readData(str);
            if (state == RADIOLIB_ERR_NONE)
            {
                if (str == "ACK")
                {
                    Serial.println("ACK recived");
                    return true;
                    break;
                }
            }
        }
        // if not ack then wait for a while hehehe
        delay(wait);
        wait *= 2;
    }
    return false;
}
