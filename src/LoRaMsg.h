#ifndef LORA_MSG_H
#define LORA_MSG_H

#include <Arduino.h>
#include <Config.h>
#include <ControlPoint.h>
#include <Team.h>
#include <LoRaCom.h>

class LoRaMsg {
public:
    LoRaMsg(LoRaCom &commManager, Config &config, ControlPoint &controlPoint);

    String createConfigMessage(const Config &config);
    String createNodeControlledMessage(int nodeId, TeamId teamId);
private:
    LoRaCom &loracom;
    Config &config;
    ControlPoint &controlPoint;
};

#endif