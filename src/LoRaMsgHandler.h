#ifndef LORA_MASG_HANDLER_H
#define LORA_MASG_HANDLER_H

#include <Config.h>
#include <ControlPoint.h>
#include <GameState.h>
#include <StringSplitter.h>

class LoRaMsgHandler {
public:
    LoRaMsgHandler(Config &config, ControlPoint &controlPoint, GameState &gameState);
    void handleMessage(const String &msg);

private:
    int nodeId;
    TeamId teamId;
    int receivedId;
    Config &config;
    ControlPoint &controlPoint;
    GameState &gameState;
    StringSplitter splitter;
};

#endif