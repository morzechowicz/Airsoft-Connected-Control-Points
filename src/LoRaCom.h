#ifndef LORA_COM_H
#define LORA_COM_H

#include <RadioLib.h>

class LoRaCom
{
public:
    LoRaCom(SX1278 &radio);
    enum RadioMode
    {
        MODE_IDLE,
        MODE_TRANSMIT,
        MODE_RECEIVE
    };

    void begin();
    bool sendMsg(const String &msg);
    bool sendMsgAck(const String &msg);
    String reciveMsg();

private:
    SX1278 &radio;
    static volatile RadioMode currentMode;
    static volatile bool operationRxTx;
    static volatile bool transDoneFlag;
    static volatile bool resumeReciving;

    static void setRecFlag();
    static void setTransFlag();
};
#endif