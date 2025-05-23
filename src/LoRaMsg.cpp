#include <LoRaMsg.h>

LoRaMsg::LoRaMsg(LoRaCom &commManager, Config &config, ControlPoint &controlPoint)
    : loracom(commManager), config(config), controlPoint(controlPoint) {}

String LoRaMsg::createConfigMessage(const Config &config) {
    return "C/" + String(config.getCountdown()) +
           "/" + String(config.getDurration()) +
           "/" + String(config.getPointsTarget()) +
           "/" + String(config.getCaptureTime());
}

String LoRaMsg::createNodeControlledMessage(int nodeId, TeamId teamId) {
    return "N/" + String(nodeId) + "/" + String(static_cast<int>(teamId));
}