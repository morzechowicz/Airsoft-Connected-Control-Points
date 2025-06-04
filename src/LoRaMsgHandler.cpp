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
    switch (static_cast<LoRaMsgCodes>(splitter.getItem(0).toInt()))
    {
    case LoRaMsgCodes::MSG_CONFIG : // Configuration update
        config.fromString(recived);
        controlPoint.setGameMaster(false);
        controlPoint.setGameMasterNode(splitter.getItem(1).toInt());
        gameState = GameState::CountDownSetup;
        break;
    case LoRaMsgCodes::MSG_NODE_CONTROLLED_BY : // Node controlled by a team
        nodeId = splitter.getItem(4).toInt();
        teamId = static_cast<TeamId>(splitter.getItem(5).toInt());
        controlPoint.setControllingTeam(teamId, nodeId);
        break;
    case LoRaMsgCodes::MSG_NODE_REPORT : // Node ping
        receivedId = splitter.getItem(1).toInt();
        controlPoint.addNode(receivedId);
        break;
    case LoRaMsgCodes::MSG_SCORE : // Score update
        teamBluePoints = splitter.getItem(4).toInt();
        teamYellowPoints = splitter.getItem(5).toInt();
        controlPoint.setTeamsScore(teamBluePoints, teamYellowPoints);
        break;
    case LoRaMsgCodes::MSG_FINISHED : // Game finished
        winner = static_cast<TeamId>(splitter.getItem(4).toInt());
        teamBluePoints = splitter.getItem(5).toInt();
        teamYellowPoints = splitter.getItem(6).toInt();
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