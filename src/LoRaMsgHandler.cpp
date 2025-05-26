#include "LoRaMsgHandler.h"

LoRaMsgHandler::LoRaMsgHandler(Config &config, ControlPoint &controlPoint, GameState &gameState, String &lastLoraMsg, TeamId &winner)
    : config(config), controlPoint(controlPoint), gameState(gameState), lastLoraMsg(lastLoraMsg), winner(winner)
{
    splitter = StringSplitter('/');
}

void LoRaMsgHandler::handleMessage(const String &msg)
{
    String recived = msg;
    lastLoraMsg = recived;
    splitter.split(recived);
    int teamBluePoints;
    int teamYellowPoints;
    TeamId winner;
    switch (splitter.getItem(0)[0])
    {
    case 'C': // Configuration update
        config.fromString(recived);
        gameState = GameState::CountDown;
        break;
    case 'N': // Node controlled by a team
        nodeId = splitter.getItem(1).toInt();
        teamId = static_cast<TeamId>(splitter.getItem(2).toInt());
        controlPoint.setControllingTeam(teamId, nodeId);
        break;
    case 'L': // Node ping
        receivedId = splitter.getItem(1).toInt();
        controlPoint.addNode(receivedId);
        break;
    case 'S': // Score update
        teamBluePoints = splitter.getItem(1).toInt();
        teamYellowPoints = splitter.getItem(2).toInt();
        controlPoint.setTeamsScore(teamBluePoints, teamYellowPoints);
        break;
    case 'F': // Game finished
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