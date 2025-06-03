#ifndef LORA_MSG_H
#define LORA_MSG_H

#include <Arduino.h>
#include <Config.h>
#include <ControlPoint.h>
#include <Team.h>
#include <LoRaMsgCodes.h>

class LoRaMsg {
public:
    LoRaMsg( Config &config, ControlPoint &controlPoint);

    String createConfig(const Config &config, int to, int seqNum);
    String createNodeControlled(int nodeId, TeamId teamId, int to, int seqNum);
    String createNodeInfo(int to, int seqNum);
    String createScoreUpdate(int teamBluePoints, int teamYellowPoints, int to, int seqNum);
    String createGameFinished(TeamId winner, int teamBluePoints, int teamYellowPoints, int to, int seqNum);
    String AckMsgRepsonse(int seqNum, int to);
    String AckMsgRepsonse(int seqNum, int nodeId, int to);

private:
    Config &config;
    ControlPoint &controlPoint;
};

#endif