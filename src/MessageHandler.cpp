#include "MessageHandler.h"

MessageHandler::MessageHandler(EventBus &eb)
    : eventBus(eb), lastSuccess(false)
{
}

bool MessageHandler::handleCommand(const Message &command, String rasMsg, uint8_t rssi, float snr)
{
    if (command.type == FORWARD)
    {
        LOG_DEBUG("HANDLER", "Handling forwarding message");
        handleForwardingMsg(command.params[0], command.params, command.paramCount, rasMsg);
        return true;
    }
    if (command.type == SYS)
    {
        LOG_DEBUG("HANDLER", "Handling system message");
        handleSystemMessage(command.params[0], command.params, command.paramCount);
        return true;
    };
    if (command.type == GAME)
    {
        LOG_DEBUG("HANDLER", "Handling game message");
        handleGameMessage(command.params[0], command.params, command.paramCount);
        return true;
    }
    if (command.type == CONF)
    {
        LOG_DEBUG("HANDLER", "Handling configuration message");
        handleConfigurationMessage(command.params[0], command.params, command.paramCount);
        return true;
    }
    if (command.type == SEARCH)
    {
        LOG_DEBUG("HANDLER", "Received BLE_SEARCH_REQUEST message");
        eventBus.publish(SEARCH);
        return true;
    }
    if (command.type == END_RESTART)
    {
        LOG_DEBUG("HANDLER", "Received BLE_END_RESTART message");
        return true;
    }
    if (command.type == REQUEST_LOG)
    {
        LOG_DEBUG("HANDLER", "Received BLE_REQUEST_LOG message");
        return true;
    }
    if (command.type == UNKNOWN)
    {
        LOG_DEBUG("HANDLER", "Received UNKNOWN message");
    }
    if (command.type == DEBUG)
    {
        handleDebugMessage(command.params[0], command.params, command.paramCount);
        LOG_DEBUG("HANDLER", "Received DEBUG message");
        return true;
    }
    if (command.type == TEST)
    {
        handleTestMessage(command.params[0], command.params, command.paramCount, rssi, snr);
        LOG_DEBUG("HANDLER", "Received TEST message");
        return true;
    }
    return false;
}

void MessageHandler::handleSystemMessage(const String &cmd, const String params[], int paramCount)
{
    if (cmd.toInt() == NETWORK_DISCOVER)
    {
        LOG_INFO("HANDLER", "Received discovery request from master at address %s", params[1].c_str());
        uint8_t masterAddress = params[1].toInt();
// If this is an information node, we do not want to respond to discovery messages
#if NODE_TYPE == INFORMATION || NODE_TYPE == BLEToLoRa || NODE_TYPE == HEADLESS
        LOG_DEBUG("HANDLER", "Not sending response because this is an information node or BLEToLoRa node or Headless node");
        return;
#endif
        eventBus.publish(NETWORK_DISCOVER, masterAddress);
    }
    if (cmd.toInt() == NETWROK_REPORT)
    {
        eventBus.publish(NETWROK_REPORT, params[1].toInt());
    }
    if (cmd.toInt() == POWER_RESET)
    {
        eventBus.publish(POWER_RESET, params[1].toInt());
    }
}

void MessageHandler::handleConfigurationMessage(const String &cmd, const String params[], int paramCount)
{
    uint16_t countdown = Protocol::parseIntParam(params[1], 0);
    uint16_t gameDurationMinutes = Protocol::parseIntParam(params[2], 0);
    uint16_t maxPoints = Protocol::parseIntParam(params[3], 0);
    uint16_t captureTime = Protocol::parseIntParam(params[4], 0);
    uint16_t respawnTime = Protocol::parseIntParam(params[5], 0);
    switch (cmd.toInt())
    {
    case KOTH_CONFIG:
        eventBus.publish(KOTH_CONFIG, countdown, gameDurationMinutes, maxPoints, captureTime, respawnTime);
        break;
    case KOTH_CONF_UPDATED:
        eventBus.publish(KOTH_CONF_UPDATED, countdown, gameDurationMinutes, maxPoints, captureTime, respawnTime);
        break;
    case FLAG_CONFIG:
        // eventBus.publish(FLAG_CONFIG, countdown, gameDurationMinutes, maxPoints, captureTime, teamsCount);
    default:
        break;
    }
}

void MessageHandler::handleGameMessage(const String &cmd, const String params[], int paramCount)
{
    switch (cmd.toInt())
    {
    case GAME_STARTED:
        eventBus.publish(GAME_STARTED, params[1].toInt(), params[2].toInt(), params[3].toInt());
        break;
    case GAME_OVER:
    {
        uint8_t winner = Protocol::parseIntParam(params[1], 0);
        eventBus.publish(GAME_OVER, (int)winner);
        break;
    }
    case PAUSE:
    {
        LOG_DEBUG("HANDLER", "Received PAUSE message");
        eventBus.publish(PAUSE, Protocol::parseIntParam(params[1], 0));
        break;
    }
    case RESUME:
    {
        LOG_DEBUG("HANDLER", "Received RESUME message");
        eventBus.publish(RESUME, Protocol::parseIntParam(params[1], 0));
        break;
    }
    case KOTH_POINT_CAPTURED:
    {
        uint8_t nodeId = Protocol::parseIntParam(params[1], 0);
        uint8_t teamId = Protocol::parseIntParam(params[2], 0);
        eventBus.publish(KOTH_POINT_CAPTURED, (int)nodeId, (int)teamId);
        break;
    }
    case KOTH_SCORE_UPDATE:
    {
        int time = Protocol::parseIntParam(params[1], 0);
        int teamYPoints = Protocol::parseIntParam(params[2], 0);
        int teamBPoints = Protocol::parseIntParam(params[3], 0);
        int pairs = Protocol::parseIntParam(params[4], 0);

        for (int i = 0; i < pairs; i++)
        {
            nodeState[i].nodeId = Protocol::parseIntParam(params[5 + i * 2], 0);
            nodeState[i].controllingTeam = (Team)Protocol::parseIntParam(params[5 + i * 2 + 1], 0);
            LOG_DEBUG("HANDLER", "Updating node state: nodeId=%d, controllingTeam=%d", nodeState[i].nodeId, nodeState[i].controllingTeam);
        }

        eventBus.publish(KOTH_SCORE_UPDATE, time, teamYPoints, teamBPoints, pairs, nodeState);
        break;
    }
    case GAME_OVER_INTERUPT:
    {
        eventBus.publish(GAME_OVER_INTERUPT, (uint16_t)Protocol::parseIntParam(params[1], 0));
        break;
    }
    case GAME_REQUEST_START_CONF:
    {
        int nodeID = Protocol::parseIntParam(params[1], 0);
        eventBus.publish(GAME_REQUEST_START_CONF, nodeID);
        break;
    }
    case GAME_FORCE_RESPAWN:
    {
        eventBus.publish(GAME_FORCE_RESPAWN);
        break;
    }
    default:
        break;
    }
}

void MessageHandler::handleForwardingMsg(const String &cmd, const String params[], int paramCount, String rawMsg)
{
    uint8_t targetNode = Protocol::parseIntParam(params[0], 0);

    if (targetNode == 0)
    {
        LOG_ERROR("MESSAGE_HANDLER", "node id 0 is not correct, id must be positive integer");
        return;
    }
    // remove first 2 parameters from rawMsg(Forward id and target node id)
    for (size_t i = 0; i < 2; i++)
    {
        int firstSemi = rawMsg.indexOf(';');
        if (firstSemi != -1)
        {
            rawMsg = rawMsg.substring(firstSemi + 1);
        }
    }

    LOG_DEBUG("HANDLER", "Forwarding message to node %d: %s", targetNode, rawMsg.c_str());
    eventBus.publish(FORWARD, targetNode, rawMsg);
}

void MessageHandler::handleDebugMessage(const String &cmd, const String params[], int paramCount)
{
    uint16_t nodeCount = Protocol::parseIntParam(params[1], 0);
    if (cmd.toInt() == TEST)
    {
        LOG_INFO("HANDLER", "Testing connection with audio response");
        eventBus.publish(TEST, nodeCount);
    }
    if (cmd.toInt() == SEARCH)
    {
        LOG_INFO("HANDLER", "Received SEARCH message from controller, publishing SEARCH event");
        // eventBus.publish(TEST, fromNode, fromController);
    }
}

void MessageHandler::handleTestMessage(const String &cmd, const String params[], int paramCount, uint8_t rssi, float snr)
{
    uint16_t fromNode = Protocol::parseIntParam(params[1], 0);
    uint8_t packetId = Protocol::parseIntParam(params[2], 0);
    if (cmd.toInt() == TEST_BROADCAST)
    {
        LOG_INFO("HANDLER", "Received TEST_BROADCAST message");
        eventBus.publish(TEST_BROADCAST, fromNode, rssi, snr);
    }
    if (cmd.toInt() == TEST_BR_RESPONSE)
    {
        LOG_INFO("HANDLER", "Received TEST_BR_RESPONSE message");
        eventBus.publish(TEST_BR_RESPONSE, fromNode, rssi, snr);
    }
    if (cmd.toInt() == TEST_DIRECT)
    {
        LOG_INFO("HANDLER", "Received TEST_DIRECT message");
        eventBus.publish(TEST_DIRECT, fromNode, rssi, snr);
    }
    if (cmd.toInt() == TEST_DR_RESPONSE)
    {
        uint8_t packetsReceived = Protocol::parseIntParam(params[2], 0);
        uint8_t retryCount = Protocol::parseIntParam(params[3], 0);
        LOG_INFO("HANDLER", "Received TEST_DR_RESPONSE message");
        eventBus.publish(TEST_DR_RESPONSE, fromNode, rssi, snr, packetId);
    }
}
