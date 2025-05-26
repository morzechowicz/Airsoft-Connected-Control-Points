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
    if (currentMode == MODE_IDLE)
    {

        Serial.println("ALL YOU HAD TO DO WAS FOLLOW THE DAMN TRAIN, CJ");
        operationRxTx = false;
        currentMode = MODE_TRANSMIT;

        String send = msg;
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

String LoRaCom::reciveMsg()
{
    if(resumeReciving)
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
bool LoRaCom::sendMsgAck(const String &msg)
{
    return false;
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