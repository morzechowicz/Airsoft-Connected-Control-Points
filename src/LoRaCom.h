#ifndef LORA_COM_H
#define LORA_COM_H

#include <AckList.h>
#include <RadioLib.h>
#include <Clocker.h>
#include <StringSplitter.h>
#include <LoRaMsgCodes.h>

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
    int seqNum = 0;
    AckList msgAckList;
    
    void begin();
    bool sendMsg(const String &msg);
    bool sendMsgAck(const String &msg);
    String reciveMsg();
    void sendMsgFromAckList();
    
    private:
    SX1278 &radio;
    static volatile RadioMode currentMode;
    static volatile bool operationRxTx;
    static volatile bool transDoneFlag;
    static volatile bool resumeReciving;
    StringSplitter splitter;
    Clocker ackClock;

    static void setRecFlag();
    static void setTransFlag();
};
#endif