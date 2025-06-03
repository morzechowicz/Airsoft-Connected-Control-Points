#include <LoRaMsg.h>


LoRaMsg::LoRaMsg(Config &config, ControlPoint &controlPoint)
    : config(config), controlPoint(controlPoint) {}

// NEW STANDARD FOR MSG TO FOLLOW
// TYPE/
// FROM/
// TO/ <- can be zero if zero then broadcast
// seqNum/
// everything else/


String LoRaMsg::createConfig(const Config &config, int to, int seqNum)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_CONFIG)) +
    "/" + String(controlPoint.getNodeId()) +
    "/" + String(to) +
    "/" + String(seqNum) +
    "/" + String(config.getCountdown()) +
    "/" + String(config.getDurration()) +
    "/" + String(config.getPointsTarget()) +
    "/" + String(config.getCaptureTime());
}

String LoRaMsg::createNodeControlled(int nodeId, TeamId teamId, int to, int seqNum)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_NODE_CONTROLLED_BY)) +
           "/" + String(controlPoint.getNodeId()) +
           "/" + String(to) +
           "/" + String(seqNum) +
           "/" + String(nodeId) +
           "/" + String(static_cast<int>(teamId));
}

String LoRaMsg::createNodeInfo(int to, int seqNum)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_NODE_REPORT)) +
           "/" + String(controlPoint.getNodeId()) +
           "/" + String(to) +
           "/" + String(seqNum);
}

String LoRaMsg::createScoreUpdate(int teamBluePoints, int teamYellowPoints, int to, int seqNum)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_SCORE)) +
    "/" + String(controlPoint.getNodeId()) +
    "/" + String(to) +
    "/" + String(seqNum) +
    "/" + String(teamBluePoints) +
    "/" + String(teamYellowPoints);
}

String LoRaMsg::createGameFinished(TeamId winner, int teamBluePoints, int teamYellowPoints, int to, int seqNum)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_FINISHED)) +
    "/" + String(controlPoint.getNodeId()) +
    "/" + String(to) +
    "/" + String(seqNum) +
    "/" + String(static_cast<int>(winner)) +
    "/" + String(teamBluePoints) +
    "/" + String(teamYellowPoints);
}

String LoRaMsg::AckMsgRepsonse(int seqNum, int to)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_RSP)) +
           "/" + String(controlPoint.getNodeId()) +
           "/" + String(to) +
           "/" + String(seqNum);
}

String LoRaMsg::AckMsgRepsonse(int seqNum, int nodeId, int to)
{
    return String(static_cast<int>(LoRaMsgCodes::MSG_ACK)) +
           "/" + String(nodeId) +
           "/" + String(to) +
           "/" + String(seqNum);
}