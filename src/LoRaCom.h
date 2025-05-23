#ifndef LORA_COM_H
#define LORA_COM_H

#include <RadioLib.h>

class LoRaCom
{
public:
    LoRaCom(SX1278 &radio);

    void begin();
    bool sendMsg(const String &msg);
    bool sendMsgAck(const String &msg);
    String reciveMsg();

private:
    SX1278 &radio;
    static volatile bool recFlag;
    static volatile bool transDoneFlag;

    static void setRecFlag();
    static void setTransFlag();
};
#endif