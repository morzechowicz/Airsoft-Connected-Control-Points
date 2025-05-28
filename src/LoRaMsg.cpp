#include <LoRaMsg.h>

LoRaMsg::LoRaMsg(LoRaCom &commManager, Config &config, ControlPoint &controlPoint)
    : loracom(commManager), config(config), controlPoint(controlPoint) {}

String LoRaMsg::createConfig(const Config &config)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_CONFIG)) +
           "/" + String(config.getCountdown()) +
           "/" + String(config.getDurration()) +
           "/" + String(config.getPointsTarget()) +
           "/" + String(config.getCaptureTime());
}

String LoRaMsg::createNodeControlled(int nodeId, TeamId teamId)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_NODE_CONTROLLED_BY)) +
           "/" + String(nodeId) +
           "/" + String(static_cast<int>(teamId));
}

String LoRaMsg::createNodeInfo()
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_NODE_REPORT)) +
           "/" + String(controlPoint.getNodeId());
}

String LoRaMsg::createScoreUpdate(int teamBluePoints, int teamYellowPoints)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_SCORE)) +
           "/" + String(teamBluePoints) +
           "/" + String(teamYellowPoints);
}

String LoRaMsg::createGameFinished(TeamId winner, int tesmBluePoints, int teamYellowPoints)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_FINISHED)) +
           "/" + String(static_cast<int>(winner)) +
           "/" + String(tesmBluePoints) +
           "/" + String(teamYellowPoints);
}

String LoRaMsg::AckMsgRepsonse(int seqNum)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_ACK)) +
           "/" + String(seqNum);
}
