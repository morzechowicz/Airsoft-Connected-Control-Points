#ifndef LORA_MASG_HANDLER_H
#define LORA_MASG_HANDLER_H

#include <Config.h>
#include <ControlPoint.h>
#include <GameState.h>
#include <StringSplitter.h>
#include <LoRaMsgCodes.h>
#include <LoRaMsg.h>
#include <Clocker.h>
#include <LoRaCom.h>

class LoRaMsgHandler {
public:
    LoRaMsgHandler(Config &config, ControlPoint &controlPoint, GameState &gameState, String &lastLoraMsg, TeamId &winner, LoRaMsg &LoRaMsg,Clocker &gameClock,LoRaCom &loraCom);
    void handleMessage(const String &msg);

private:
    int nodeId;
    TeamId teamId;
    LoRaMsg loramsg;
    int receivedId;
    TeamId winner;
    Config &config;
    String &lastLoraMsg;
    ControlPoint &controlPoint;
    GameState &gameState;
    Clocker &gameClock;
    LoRaCom &loraCom;
    StringSplitter splitter;
};

#endif