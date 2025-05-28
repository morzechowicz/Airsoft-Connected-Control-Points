#include <LoRaCom.h>

volatile bool LoRaCom::resumeReciving = false;

LoRaCom::LoRaCom(SX1278 &radio) : radio(radio) {}

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

        Serial.println("sending msg");
        Serial.println(send);
        operationRxTx = false;
        currentMode = MODE_TRANSMIT;

        int state = radio.startTransmit(send);
        if (state == RADIOLIB_ERR_NONE)
        {
            Serial.println(F("transmiting!"));
        }
        else
        {
            Serial.print(F("failed, code transmission "));
            Serial.println(state);
            currentMode = MODE_IDLE;
            return false;
        }
    }
    return false;
}

bool LoRaCom::sendMsgAck(const String &msg)
{
    // old one sucked time for new one
    seqNum++;
    String send = msg;
    send += "/"+ String(static_cast<int>(LoRaMsgCodes::MSG_ACK))+"/" + String(seqNum);
    Serial.println("sending msg with ack");
    Serial.println(send);

    msgAckList.add(LoRaAckMessage(send, seqNum));

    return true;
}

String LoRaCom::reciveMsg()
{
    if (resumeReciving)
    {
        Serial.println("Resuming reciving");
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
            int state = radio.readData(msg);
            if (state == RADIOLIB_ERR_NONE)
            {
                Serial.println(F("received data!"));
                currentMode = MODE_IDLE;
                splitter.split(msg);
                if (splitter.getItem(splitter.getItemCount() - 1).toInt() == static_cast<int>(LoRaMsgCodes::MSG_ACK))
                {
                    msg = String(static_cast<int>(LoRaMsgCodes::MSG_RSP)) + "/" + String(splitter.getItem(splitter.getItemCount()));
                    sendMsg(msg);
                }
                if(splitter.getItem(0).toInt() == static_cast<int>(LoRaMsgCodes::MSG_RSP))
                {
                    LoRaAckMessage resp = msgAckList.getBySeqNum(splitter.getItem(1).toInt());
                    msgAckList.remove(resp);
                }
                return msg;
            }
            else
            {
                Serial.print(F("failed, code reception "));
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
    if (msgAckList.size() > 0)
    {
        LoRaAckMessage ackMesg = msgAckList.get(0);
        if (!ackClock.isRunning())
        {
            ackClock.start();
        }
        if (ackClock.getElapsedTime() > 100)
            if (currentMode == MODE_IDLE)
            {
                sendMsg(ackMesg.getMessage());
                ackMesg.increamentRetry();
                if(ackMesg.getRetryCount() > 10)
                {
                    Serial.println("No response abording");
                    msgAckList.remove(ackMesg);
                }
                Serial.println("Sending msg from ack list");
                ackClock.reset();
            }
        if (ackMesg.getRecived())
        {
            Serial.println("Message Acknowlage removing from stack");
            msgAckList.remove(ackMesg);
            ackClock.stop();
            ackClock.reset();
        }
    }
}

void LoRaCom::setRecFlag(void)
{
    if (currentMode == MODE_TRANSMIT)
    {
        Serial.println("Recieve flag set in transmit mode");
        Serial.println("Transmitting done, now reciving");
        resumeReciving = true;
        currentMode = MODE_IDLE;
    }
    else if (currentMode == MODE_IDLE)
    {
        operationRxTx = true;
        Serial.println("Recieve flag set");
    }
    else if (currentMode == MODE_RECEIVE)
    {
        Serial.println("Recieve flag set in receive mode");
    }
}

void LoRaCom::setTransFlag(void)
{
    transDoneFlag = true;
    Serial.println("Transmit flag set");
}