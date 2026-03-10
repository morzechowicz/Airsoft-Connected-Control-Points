#include "Protocol.h"

String Protocol::buildGameStart()
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(GAME_STARTED);
    return buffer;
}

String Protocol::buildGameOver(uint8_t winner)
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(GAME_OVER) + ";";
    buffer += String(winner);
    return buffer;
}

String Protocol::buildDiscoverRequest()
{
    String buffer;
    buffer = String(SYS) + ";";
    buffer += String(NETWORK_DISCOVER) + ";";
    buffer += String(LORA_ADDRESS) + ";";
    return buffer;
}

String Protocol::buildDiscoverResponse(uint8_t nodeID)
{
    String buffer;
    buffer = String(SYS) + ";";
    buffer += String(NETWROK_REPORT) + ";";
    buffer += String(nodeID) + ";";
    return buffer;
}

String Protocol::buildPowerResetMsg()
{
    String buffer;
    buffer = String(SYS) + ";";
    buffer += String(POWER_RESET) + ";";
    buffer += String(0) + ";";
    return String();
}

String Protocol::buildKothConfigUpdated(uint8_t maxPoints, uint8_t countdown, uint8_t captureTime, uint8_t maxTime)
{
    String buffer;
    buffer = String(CONF) + ";";
    buffer += String(KOTH_CONF_UPDATED) + ";";
    buffer += String(countdown) + ";";
    buffer += String(maxTime) + ";";
    buffer += String(maxPoints) + ";";
    buffer += String(captureTime) + ";";
    return buffer;
}

String Protocol::buildKothConfig(uint8_t maxPoints, uint8_t countdown, uint8_t captureTime, uint8_t maxTime)
{
    String buffer;
    buffer = String(CONF) + ";";
    buffer += String(KOTH_CONFIG) + ";";
    buffer += String(countdown) + ";";
    buffer += String(maxTime) + ";";
    buffer += String(maxPoints) + ";";
    buffer += String(captureTime) + ";";
    return buffer;
}

String Protocol::buildCaptureMessage(uint8_t nodeId, uint8_t teamId)
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(KOTH_POINT_CAPTURED) + ";";
    buffer += String(nodeId) + ";";
    buffer += String(teamId);
    return buffer;
}

String Protocol::buildScoreUpdateMessage(uint16_t teamYPoints, uint16_t teamBPoints)
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(KOTH_SCORE_UPDATE) + ";";
    buffer += String(teamYPoints) + ";";
    buffer += String(teamBPoints);
    return buffer;
}

Message Protocol::parse(const char *data, size_t length)
{
    Message msg = {UNKNOWN, {}, 0}; // Initialize with defaults

    if (data == nullptr || length == 0)
    {
        return msg; // Early exit for invalid input
    }

    String dataStr = String(data).substring(0, length);
    int firstSemicolon = dataStr.indexOf(';');

    // Parse message type (before first semicolon)
    if (firstSemicolon != -1)
    {
        String typeStr = dataStr.substring(0, firstSemicolon);
        msg.type = (EventType)typeStr.toInt();
    }
    else
    {
        return msg; // No semicolon = invalid format
    }

    // Parse parameters (after first semicolon)
    int paramStart = firstSemicolon + 1;
    int paramEnd;
    int paramIndex = 0;

    while (paramIndex < 10)
    {
        paramEnd = dataStr.indexOf(';', paramStart);
        if (paramEnd == -1)
        {
            // Last parameter (or only parameter)
            if (paramStart < dataStr.length())
            {
                msg.params[paramIndex] = dataStr.substring(paramStart);
                paramIndex++;
            }
            break;
        }

        // Extract parameter
        msg.params[paramIndex] = dataStr.substring(paramStart, paramEnd);
        paramIndex++;
        paramStart = paramEnd + 1;
    }

    msg.paramCount = paramIndex;
    return msg;
}

int Protocol::parseIntParam(const String &param, int defaultVal)
{
    if (param.length() == 0)
    {
        return defaultVal;
    }
    return param.toInt();
}