#ifndef LORA_COM_H
#define LORA_COM_H

#include <AckList.h>
#include <RadioLib.h>
#include <Clocker.h>
#include <StringSplitter.h>
#include <LoRaMsgCodes.h>
#include <ControlPoint.h>
#include <LoRaMsg.h>

class LoRaCom
{
public:
    LoRaCom(SX1278 &radio,ControlPoint &cp,LoRaMsg &LRM);
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
    bool sendMsgAckTo(const String &msg, int targetId);
    void sendMsgAckToAll(const String &msg);
    String reciveMsg();
    void sendMsgFromAckList();
    
    private:
    SX1278 &radio;
    LoRaMsg &LRM;
    static volatile RadioMode currentMode;
    static volatile bool operationRxTx;
    static volatile bool transDoneFlag;
    static volatile bool resumeReciving;
    StringSplitter splitter;
    Clocker ackClock;
    ControlPoint &cp;
    unsigned long lastCheck = 0;
    long ackInterval = random(1000, 2000);
    static void setRecFlag();
    static void setTransFlag();
};
#endif