#include "LoRaMsgHandler.h"

LoRaMsgHandler::LoRaMsgHandler(Config &config, ControlPoint &controlPoint, GameState &gameState, String &lastLoraMsg, TeamId &winner, LoRaMsg &loramsg, Clocker &gameClock, LoRaCom &loraCom, StatusLog &statusLog)
    : config(config), controlPoint(controlPoint), gameState(gameState), lastLoraMsg(lastLoraMsg), winner(winner), loramsg(loramsg), gameClock(gameClock), loraCom(loraCom), statusLog(statusLog)
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
    Serial.println(recived);
    switch (static_cast<LoRaMsgCodes>(splitter.getItem(0).toInt()))
    {
    case LoRaMsgCodes::MSG_CONFIG: // Configuration update
        config.fromString(recived);
        if (!controlPoint.getGameMaster())
        {
            controlPoint.setGameMaster(false);
            controlPoint.setGameMasterNode(splitter.getItem(1).toInt());
            gameState = GameState::CountDownSetup;
        }
        break;
    case LoRaMsgCodes::MSG_NODE_CONTROLLED_BY: // Node controlled by a team
        nodeId = splitter.getItem(4).toInt();
        teamId = static_cast<TeamId>(splitter.getItem(5).toInt());
        controlPoint.setControllingTeam(teamId, nodeId);
        break;
    case LoRaMsgCodes::PING: // Ping message
        // not sure what to do with it yet
        break;
    case LoRaMsgCodes::MSG_NODE_REPORT: // Report from node
        receivedId = splitter.getItem(1).toInt();
        if (controlPoint.addNode(receivedId) && controlPoint.getGameMaster() && gameState == GameState::Ongoing)
        {
            // send current config and other needed data to restore point
            String configMsg = loramsg.createRestoreMsg(config, gameClock.getElapsedTimeInMinutes(), receivedId, loraCom.seqNum++);
            loraCom.sendMsgAckTo(configMsg, receivedId);
        }
        break;
    case LoRaMsgCodes::MSG_SCORE: // Score update
        teamBluePoints = splitter.getItem(4).toInt();
        teamYellowPoints = splitter.getItem(5).toInt();
        controlPoint.setTeamsScore(teamBluePoints, teamYellowPoints);
        break;
    case LoRaMsgCodes::MSG_FINISHED: // Game finished
        winner = static_cast<TeamId>(splitter.getItem(4).toInt());
        teamBluePoints = splitter.getItem(5).toInt();
        teamYellowPoints = splitter.getItem(6).toInt();
        controlPoint.setTeamsScore(teamBluePoints, teamYellowPoints);
        gameState = GameState::Finished;
        Serial.print("Game finished. Winner: ");
        Serial.println(static_cast<int>(winner));
        break;
    case LoRaMsgCodes::MSG_RESTORE: // restore after power lose
        config.fromString(recived);
        gameClock.setTimeFromMinutes(splitter.getItem(8).toInt());
        gameState = GameState::Ongoing;
        controlPoint.setGameMaster(false);
        break;
    case LoRaMsgCodes::MSG_ACK: // Acknowledgment message
        Serial.println("Acknowledgment received from node ID: ");
        Serial.println(splitter.getItem(1).toInt());

        break;
    case LoRaMsgCodes::MSG_RSP: // Response message
        Serial.println("Response received from node ID: ");
        Serial.println(splitter.getItem(1).toInt());
        if(controlPoint.addNode(splitter.getItem(1).toInt()))
        {
            statusLog.addLogEntry(EVENT_SYSTEM_RECEIVED_NODE_INFO, splitter.getItem(1).toInt());
        }
        break;
    default:
        Serial.println("Unknown message type");
        break;
    }
}