#include <LoRaCom.h>

volatile bool LoRaCom::resumeReciving = false;

LoRaCom::LoRaCom(SX1278 &radio, ControlPoint &cp, LoRaMsg &LRM)
    : radio(radio), cp(cp), LRM(LRM) {}

volatile bool LoRaCom::operationRxTx = false;
volatile bool LoRaCom::transDoneFlag = false;
volatile LoRaCom::RadioMode LoRaCom::currentMode = LoRaCom::MODE_IDLE;

void LoRaCom::begin()
{
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

    radio.setDio0Action(setRecFlag, RISING); // METAL DIO0 RISING REVENGENCE
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

bool LoRaCom::sendMsg(const String &msg)
{
    // right now if radio is busy it will just skip sending
    // make it into loop or flag or what ever that waits for radio to be awailable
    if (currentMode == MODE_IDLE)
    {
        String send = msg;

        //Serial.println("sending msg");
        Serial.println(send);
        operationRxTx = false;
        currentMode = MODE_TRANSMIT;

        int state = radio.startTransmit(send);
        if (state == RADIOLIB_ERR_NONE)
        {
           // Serial.println(F("transmiting!"));
        }
        else
        {
            //Serial.print(F("failed, code transmission "));
            Serial.println(state);
            currentMode = MODE_IDLE;
            return false;
        }
    }
    return false;
}

bool LoRaCom::sendMsgAckTo(const String &msg, int targetId)
{
    String send = msg;

    LoRaAckMessage ackMsg(send, seqNum, targetId);
    msgAckList.add(ackMsg);

    return true;
}

void LoRaCom::sendMsgAckToAll(const String &msg)
{
    for (size_t i = 0; i < cp.nodeCount; i++)
    {
        if (cp.nodes[i].nodeId != cp.getNodeId())
        {
            String send = msg;
            sendMsgAckTo(send, cp.nodes[i].nodeId);
        }
    }
}

String LoRaCom::reciveMsg()
{
    if (resumeReciving)
    {
       // Serial.println("Resuming reciving");
        resumeReciving = false;
        radio.startReceive();
    }
    if (currentMode == MODE_IDLE)
    {
        if (operationRxTx)
        {
            currentMode = MODE_RECEIVE;
            operationRxTx = false;
            String msg = "";
            String response = "";
            int state = radio.readData(msg);
            if (state == RADIOLIB_ERR_NONE)
            {
                Serial.println(F("received data!"));
                currentMode = MODE_IDLE;
                splitter.split(msg);
                LoRaMsgCodes code = static_cast<LoRaMsgCodes>(splitter.getItem(0).toInt());
                int from = splitter.getItem(1).toInt();
                int addresedTo = splitter.getItem(2).toInt();
                int seqNumR = splitter.getItem(3).toInt();
                if (addresedTo != cp.getNodeId() && addresedTo != 0)
                {
                   //Serial.println("not addresed towards me ingoring");
                    Serial.println(addresedTo);
                    return "";
                }
                Serial.println(msg);

                if (code != LoRaMsgCodes::MSG_RSP)
                {
                    //Serial.println("Ack this");
                    response = LRM.AckMsgRepsonse(seqNumR, from);
                    sendMsg(response);
                }
                if (code == LoRaMsgCodes::MSG_RSP)
                {
                    //Serial.println("ack reported");
                    Serial.println(msg);
                    Serial.println(seqNumR);
                    LoRaAckMessage *resp = msgAckList.getBySeqNum(seqNumR);
                    if (resp != nullptr)
                    {
                        msgAckList.remove(resp);
                    }
                    else
                    {
                        //Serial.println("Warning: Ack message not found in list.");
                    }
                }
                return msg;
            }
            else
            {
                //Serial.print(F("failed, code reception "));
                Serial.println(state);
                currentMode = MODE_IDLE;
                return "";
            }
        }
    }
    return "";
}

void LoRaCom::sendMsgFromAckList()
{
    unsigned long now = millis();

    // Only run this function every 10-50ms
    if (now - lastCheck < 10)
    {
        return; // Skip this call
    }
    lastCheck = now;

    if (msgAckList.size() == 0)
    {
        return;
    }
    LoRaAckMessage *ackMesg = msgAckList.get(0);
    if (!ackClock.isRunning())
    {
        ackClock.start();
    }
    if (ackClock.getElapsedTime() > ackInterval)
    {
        ackInterval = random(1000, 2000);
        if (currentMode == MODE_IDLE)
        {
            sendMsg(ackMesg->getMessage());
            ackMesg->increamentRetry();

            if (ackMesg->getRetryCount() > 5)
            {
                //Serial.println("No response abording");
                msgAckList.remove(ackMesg);
            }
            //Serial.println("Sending msg from ack list");
            ackClock.reset();
        }
    }
    if (ackMesg->getRecived())
    {
        //Serial.println("Message Acknowlage removing from stack");
        msgAckList.remove(ackMesg);
        ackClock.stop();
        ackClock.reset();
    }
}

void LoRaCom::setRecFlag(void)
{
    if (currentMode == MODE_TRANSMIT)
    {
        //Serial.println("Recieve flag set in transmit mode");
        //Serial.println("Transmitting done, now reciving");
        resumeReciving = true;
        currentMode = MODE_IDLE;
    }
    else if (currentMode == MODE_IDLE)
    {
        operationRxTx = true;
        //Serial.println("Recieve flag set");
    }
    else if (currentMode == MODE_RECEIVE)
    {
        //Serial.println("Recieve flag set in receive mode");
    }
}

void LoRaCom::setTransFlag(void)
{
    transDoneFlag = true;
    //Serial.println("Transmit flag set");
}