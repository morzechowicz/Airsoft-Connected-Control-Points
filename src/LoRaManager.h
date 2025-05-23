#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H

#include <RadioLib.h>
#include <Team.h>
#include <Config.h>
#include <ControlPoint.h>
#include <Clocker.h>
#include <LoRaMsgCodes.h>
#include <GameState.h>

// note to self
//  communication standard
//  side node MUST ACK RESPONSO FROM MAIN NODE
//  main node does not need any for total score
//  but IT ABSOLUTLY NEEDS ACK FOR GAME STATUS

class LoRaManager
{
public:
    LoRaManager();

    void begin();
    void recivingLoop();
    void sendNewConfig(Config config);
    void sendAllTotalTeamsScore(const Team *teams, size_t teamCount);
    void sendGameFinished(const Team *teams, size_t teamCount);
    void sendNodeControlledBy(Team team);
    void sendLeaderNodeId();
    void connectThisNode();

private:
    SX1278 radio = new Module(18, 26, 14, 33);
    volatile bool static recFlag;
    int transmissionState = RADIOLIB_ERR_NONE;
    volatile bool static transDoneFlag;
    int nodeId;
    int leaderId;
    int recivedId;
    String parts[10];
    int partCount = 0;
    int lastIndex = 0;

    void sendMsg(String msg);
    void static setRecFlag(void)
    {
        recFlag = true;
        Serial.println("recFlag set");
    };
    void static setTransFlag(void)
    {
        transDoneFlag = true;
        Serial.println("transDoneFlag set");
    };
    void recivedMessageHandler(String recived);
    bool sendMsgAck(String Msg);
};

#endif