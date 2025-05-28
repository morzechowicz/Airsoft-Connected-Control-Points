#ifndef LORA_MSG_H
#define LORA_MSG_H

#include <Arduino.h>
#include <Config.h>
#include <ControlPoint.h>
#include <Team.h>
#include <LoRaCom.h>
#include <LoRaMsgCodes.h>

class LoRaMsg {
public:
    
    LoRaMsg(LoRaCom &commManager, Config &config, ControlPoint &controlPoint);

    String createConfig(const Config &config);
    String createNodeControlled(int nodeId, TeamId teamId);
    String createNodeInfo();
    String createScoreUpdate(int teamBluePoints, int teamYellowPoints);
    String createGameFinished(TeamId winner,int tesmBluePoints, int teamYellowPoints);
    String AckMsgRepsonse(int seqNum);

private:
    LoRaCom &loracom;
    Config &config;
    ControlPoint &controlPoint;
};

#endif