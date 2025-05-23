#include "LoRaMsgHandler.h"

LoRaMsgHandler::LoRaMsgHandler(Config &config, ControlPoint &controlPoint, GameState &gameState)
    : config(config), controlPoint(controlPoint), gameState(gameState)
    {
        splitter = StringSplitter('/');
    }

void LoRaMsgHandler::handleMessage(const String &msg) {
    String recived = msg;
    splitter.split(recived);
    switch (splitter.getItem(0)[0]) {
    case 'C': // Configuration update
        config.fromString(recived);
        gameState = GameState::CountDown;
        break;
    case 'N': // Node controlled by a team
        nodeId = splitter.getItem(1).toInt();
        teamId = static_cast<TeamId>(splitter.getItem(2).toInt());
        controlPoint.setNodeControllingTeam(nodeId, teamId);
        break;
    case 'L': // Node ping
        receivedId = splitter.getItem(1).toInt();
        controlPoint.addNode(receivedId);
        break;
    default:
        Serial.println("Unknown message type");
        break;
    }
}