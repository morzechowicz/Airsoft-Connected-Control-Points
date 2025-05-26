#include <LoRaMsg.h>

LoRaMsg::LoRaMsg(LoRaCom &commManager, Config &config, ControlPoint &controlPoint)
    : loracom(commManager), config(config), controlPoint(controlPoint) {}

String LoRaMsg::createConfig(const Config &config) {
    return "C/" + String(config.getCountdown()) +
           "/" + String(config.getDurration()) +
           "/" + String(config.getPointsTarget()) +
           "/" + String(config.getCaptureTime());
}

String LoRaMsg::createNodeControlled(int nodeId, TeamId teamId) {
    return "N/" + String(nodeId) + "/" + String(static_cast<int>(teamId));
}

String LoRaMsg::createNodeInfo() {
    return "L/" + String(controlPoint.getNodeId());
}

String LoRaMsg::createScoreUpdate(int teamBluePoints, int teamYellowPoints)
{
    return "S/" + String(teamBluePoints) + "/" + String(teamYellowPoints);
}

String LoRaMsg::createGameFinished(TeamId winner, int tesmBluePoints, int teamYellowPoints)
{
    return "F/" + String(static_cast<int>(winner)) + "/" +
           String(tesmBluePoints) + "/" + String(teamYellowPoints);
}
