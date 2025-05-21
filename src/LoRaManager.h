#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H

#include <RadioLib.h>
#include <Team.h>
#include <Config.h>
#include <ControlPoint.h>

//note to self
// communication standard
// side node MUST ACK RESPONSO FROM MAIN NODE
// main node does not need any for total score
// but IT ABSOLUTLY NEEDS ACK FOR GAME STATUS

class LoRaManager
{
public:
    LoRaManager();
    LoRaManager(SX1278 radioModule);
    
    void sendNewConfig(Config config);
    void sendAllTotalTeamsScore(const Team* teams, size_t teamCount);
    void sendGameFinished(const Team* teams, size_t teamCount);
    void sendNodeControlledBy(Team team);
    void incomingMessageHandler();

private:
    SX1278 radio;

    void ackMsg();
    void sendMsg(String msg);
};

#endif