#include <LoRaCom.h>

LoRaCom::LoRaCom(SX1278 &radio) : radio(radio) {}

volatile bool LoRaCom::recFlag = false;
volatile bool LoRaCom::transDoneFlag = false;
void LoRaCom::begin() {
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

bool LoRaCom::sendMsg(const String &msg)
{
    if(transDoneFlag)
    {
        transDoneFlag = false;
        String send = msg;
        int state = radio.startTransmit(send);
        if (state == RADIOLIB_ERR_NONE)
        {
            Serial.println(F("transmission finished!"));
        }
        else
        {
            Serial.print(F("failed, code transmission "));
            Serial.println(state);
            return false;
        }

    }
}

String LoRaCom::reciveMsg()
{
    if(recFlag)
    {
        recFlag = false;
        String msg = "";
        int state = radio.readData(msg);
        if (state == RADIOLIB_ERR_NONE)
        {
            Serial.println(F("received data!"));
            return msg;
        }
        else
        {
            Serial.print(F("failed, code reception "));
            Serial.println(state);
            return "";
        }
    }
}
bool LoRaCom::sendMsgAck(const String &msg)
{

}

void LoRaCom::setRecFlag() {
    recFlag = true;
}

void LoRaCom::setTransFlag() {
    transDoneFlag = true;
}