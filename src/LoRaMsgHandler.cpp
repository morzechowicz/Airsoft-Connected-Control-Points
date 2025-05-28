#include "LoRaMsgHandler.h"

LoRaMsgHandler::LoRaMsgHandler(Config &config, ControlPoint &controlPoint, GameState &gameState, String &lastLoraMsg, TeamId &winner, LoRaMsg &loramsg)
    : config(config), controlPoint(controlPoint), gameState(gameState), lastLoraMsg(lastLoraMsg), winner(winner),loramsg(loramsg)
{
    splitter = StringSplitter('/');
}

void LoRaMsgHandler::handleMessage(const String &msg)
{
    String response = "";
    String recived = msg;
    lastLoraMsg = recived;
    splitter.split(recived);
    int teamBluePoints;
    int teamYellowPoints;
    TeamId winner;
    switch (static_cast<LoRaMsgCodes>(splitter.getItem(0).toInt()))
    {
    case LoRaMsgCodes::MSG_CONFIG : // Configuration update
        config.fromString(recived);
        gameState = GameState::CountDownSetup;
        break;
    case LoRaMsgCodes::MSG_NODE_CONTROLLED_BY : // Node controlled by a team
        nodeId = splitter.getItem(1).toInt();
        teamId = static_cast<TeamId>(splitter.getItem(2).toInt());
        controlPoint.setControllingTeam(teamId, nodeId);
        break;
    case LoRaMsgCodes::PING : // Node ping
        receivedId = splitter.getItem(1).toInt();
        controlPoint.addNode(receivedId);
        break;
    case LoRaMsgCodes::MSG_SCORE : // Score update
        teamBluePoints = splitter.getItem(1).toInt();
        teamYellowPoints = splitter.getItem(2).toInt();
        controlPoint.setTeamsScore(teamBluePoints, teamYellowPoints);
        break;
    case LoRaMsgCodes::MSG_FINISHED : // Game finished
        winner = static_cast<TeamId>(splitter.getItem(1).toInt());
        teamBluePoints = splitter.getItem(2).toInt();
        teamYellowPoints = splitter.getItem(3).toInt();
        controlPoint.setTeamsScore(teamBluePoints, teamYellowPoints);
        gameState = GameState::Finished;
        Serial.print("Game finished. Winner: ");
        Serial.println(static_cast<int>(winner));
        break;
    default:
        Serial.println("Unknown message type");
        break;
    }
}