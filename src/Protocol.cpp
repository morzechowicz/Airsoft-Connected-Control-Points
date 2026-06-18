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

String Protocol::buildPause()
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(PAUSE) + ";";
    buffer += String(0);
    return buffer;
}

String Protocol::buildResume()
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(RESUME) + ";";
    buffer += String(0);
    return buffer;
}

String Protocol::buildDiscoverRequest()
{
    String buffer;
    buffer = String(SYS) + ";";
    buffer += String(NETWORK_DISCOVER) + ";";
    buffer += String(LORA_ADDRESS) + ";";
    buffer += String(millis()/1000) + ";"; //send seconds since boot
    return buffer;
}

String Protocol::buildDiscoverResponse(uint8_t nodeID,uint8_t nodeType)
{
    String buffer;
    buffer = String(SYS) + ";";
    buffer += String(NETWROK_REPORT) + ";";
    buffer += String(nodeID) + ";";
    buffer += String(nodeType) + ";";
    return buffer;
}

String Protocol::buildPowerResetMsg()
{
    String buffer;
    buffer = String(SYS) + ";";
    buffer += String(POWER_RESET) + ";";
    buffer += String(0) + ";";
    return buffer;
}

String Protocol::buildReqeustScoreUpdate(uint8_t nodeID)
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(GAME_REQUEST_SCORE_UPDATE) + ";";
    buffer += String(nodeID);
    return buffer;
}

String Protocol::buildConfRequest(uint8_t nodeID)
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(GAME_REQUEST_START_CONF) + ";";
    buffer += String(nodeID);
    return buffer;
}

String Protocol::buildDebugTestMessage()
{
    String buffer;
    buffer = String(DEBUG) + ";";
    buffer += String(TEST) + ";";
    buffer += String(LORA_ADDRESS) + ";";
    buffer += String(0);
    return buffer;
}

String Protocol::buildDebugResponseMessage()
{
    String buffer;
    buffer = String(DEBUG) + ";";
    buffer += String(SEARCH) + ";";
    buffer += String(LORA_ADDRESS) + ";";
    buffer += String(0);
    return buffer;
}

String Protocol::buildKothConfigClient(uint16_t maxPoints, uint16_t countdown, uint16_t captureTime, uint16_t maxTime, uint16_t respawnTime)
{
    String buffer;
    buffer = String(CONF) + ";";
    buffer += String(KOTH_CONF_UPDATED) + ";";
    buffer += String(countdown) + ";";
    buffer += String(maxTime) + ";";
    buffer += String(maxPoints) + ";";
    buffer += String(captureTime) + ";";
    buffer += String(respawnTime) + ";";
    return buffer;
}

String Protocol::buildKothConfig(uint16_t maxPoints, uint16_t countdown, uint16_t captureTime, uint16_t maxTime,uint16_t respawnTime)
{
    String buffer;
    buffer = String(CONF) + ";";
    buffer += String(KOTH_CONFIG) + ";";
    buffer += String(countdown) + ";";
    buffer += String(maxTime) + ";";
    buffer += String(maxPoints) + ";";
    buffer += String(captureTime) + ";";
    buffer += String(respawnTime) + ";";
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

String Protocol::buildScoreUpdateMessage(uint16_t time, uint16_t teamYPoints, uint16_t teamBPoints, uint16_t pairs, NodeState teamPoints[10])
{
    String buffer;
    buffer = String(GAME) + ";";
    buffer += String(KOTH_SCORE_UPDATE) + ";";
    buffer += String(time) + ";";
    buffer += String(teamYPoints) + ";";
    buffer += String(teamBPoints) + ";";
    buffer += String(pairs) + ";";

    for (uint16_t i = 0; i < pairs && i < 10; i++)
    {
        buffer += String(teamPoints[i].nodeId) + ";";
        buffer += String((int)teamPoints[i].controllingTeam) + ";";
    }

    return buffer;
}

String Protocol::buildTestDrMsg(uint8_t sourceNodeid, uint8_t packetId)
{
    String buffer;
    buffer = String(TEST) + ";";
    buffer += String(TEST_DIRECT) + ";";
    buffer += String(sourceNodeid) + ";";
    buffer += String(packetId) + ";";

    return buffer;
}

String Protocol::buildTestDrResponseMsg(uint8_t responderId, uint8_t packetsReceived, uint8_t retryCount, int16_t rssi, float snr)
{
    String buffer;
    buffer = String(TEST) + ";";
    buffer += String(TEST_DR_RESPONSE) + ";";
    buffer += String(responderId) + ";";
    buffer += String(packetsReceived) + ";";
    buffer += String(retryCount) + ";";
    buffer += String(rssi) + ";";
    buffer += String(snr);

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

    while (paramIndex < MAX_MESSAGE_PARAMS)
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