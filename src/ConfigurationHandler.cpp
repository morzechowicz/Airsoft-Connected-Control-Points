#include "ConfigurationHandler.h"

ConfigurationHandler::ConfigurationHandler(EventBus &eb)
    : eventBus(eb), lastSuccess(false)
{
}

bool ConfigurationHandler::handleCommand(const Message &command)
{

    if (command.type == SYS)
    {
        handleSystemMessage(command.params[0], command.params, command.paramCount);
        return true;
    };
    if (command.type == GAME)
    {
        handleGameMessage(command.params[0], command.params, command.paramCount);
        return true;
    }
    if (command.type == CONF)
    {
        handleConfigurationMessage(command.params[0], command.params, command.paramCount);
        return true;
    }
    if (command.type == SEARCH)
    {
        Serial.println("Received BLE_SEARCH_REQUEST message");
        eventBus.publish(SEARCH);
        return true;
    }
    if (command.type == END_RESTART)
    {
        Serial.println("Received BLE_END_RESTART message");
        return true;
    }
    if (command.type == POWER_RESET)
    {
        handlePowerReset();
        Serial.println("Received BLE_POWER_RESET message");
        return true;
    }
    if (command.type == REQUEST_LOG)
    {
        Serial.println("Received BLE_REQUEST_LOG message");
        return true;
    }
    if (command.type == PAUSE)
    {
        Serial.println("Received BLE_PAUSE message");
        eventBus.publish(PAUSE);
        return true;
    }
    if (command.type == UNKNOWN)
    {
        Serial.println("Received UNKNOWN message");
    }
    return false;
}

void ConfigurationHandler::handlePowerReset()
{
    eventBus.publish(POWER_RESET);
}

void ConfigurationHandler::handleSystemMessage(const String &cmd, const String params[], int paramCount)
{
    if (cmd.toInt() == NETWORK_DISCOVER)
    {
        Serial.print("response to");
        Serial.print(params[1]);
        Serial.println("");
        uint8_t masterAddress = params[1].toInt();
        eventBus.publish(NETWORK_DISCOVER, masterAddress);
    }
    if (cmd.toInt() == NETWROK_REPORT)
    {
        eventBus.publish(NETWROK_REPORT, params[1].toInt());
    }
    if (cmd.toInt() == POWER_RESET)
    {
        eventBus.publish(POWER_RESET,params[1].toInt());
    }
    
}

void ConfigurationHandler::handleConfigurationMessage(const String &cmd, const String params[], int paramCount)
{
    uint16_t countdown = Protocol::parseIntParam(params[1], 0);
    uint16_t gameDurationMinutes = Protocol::parseIntParam(params[2], 0);
    uint16_t maxPoints = Protocol::parseIntParam(params[3], 0);
    uint16_t captureTime = Protocol::parseIntParam(params[4], 0);
    uint16_t teamsCount = Protocol::parseIntParam(params[5], 0);
    switch (cmd.toInt())
    {
    case KOTH_CONFIG:
        eventBus.publish(KOTH_CONFIG, countdown, gameDurationMinutes, maxPoints, captureTime);
        break;
    case KOTH_CONF_UPDATED:
        eventBus.publish(KOTH_CONF_UPDATED, countdown, gameDurationMinutes, maxPoints, captureTime);
        break;
    case FLAG_CONFIG:
        eventBus.publish(FLAG_CONFIG, countdown, gameDurationMinutes, maxPoints, captureTime, teamsCount);
    default:
        break;
    }
}

void ConfigurationHandler::handleGameMessage(const String &cmd, const String params[], int paramCount)
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
    case GAME_PAUSE:
    {
        eventBus.publish(GAME_PAUSE);
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
        uint16_t teamYPoints = (uint16_t)Protocol::parseIntParam(params[1], 0);
        uint16_t teamBPoints = (uint16_t)Protocol::parseIntParam(params[2], 0);
        eventBus.publish(KOTH_SCORE_UPDATE, teamYPoints, teamBPoints);
        break;
    }
    default:
        break;
    }
}
