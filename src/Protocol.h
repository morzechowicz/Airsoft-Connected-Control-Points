// Protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include "EventType.h"
#include "GameComponents/KOTH/KOTHTypes.h"
#include "Config.h"

// message structure
//  TYPE;PARAM;PARAM;...;
struct Message
{
    EventType type;
    String params[MAX_MESSAGE_PARAMS];
    int paramCount;
};

class Protocol
{
public:
    //universal message types
    static String buildGameStart();
    static String buildGameOver(uint8_t winner);
    static String buildPause();
    static String buildResume();
    static String buildDiscoverRequest();
    static String buildDiscoverResponse(uint8_t nodeID,uint8_t nodeType);
    static String buildPowerResetMsg();
    static String buildReqeustScoreUpdate(uint8_t nodeID);
    static String buildConfRequest(uint8_t nodeID);
    static String buildDebugTestMessage();
    static String buildDebugResponseMessage();;

    // KOTH Message builders
    static String buildKothConfig(uint8_t maxPoints,uint8_t countdown, uint8_t captureTime, uint8_t maxTime);
    static String buildCaptureMessage(uint8_t nodeId, uint8_t teamId);
    static String buildScoreUpdateMessage(uint16_t time,uint16_t teamYPoints, uint16_t teamBPoints,uint16_t pairs,NodeState teamPoints[10]);
    static String buildKothConfigClient(uint8_t maxPoints, uint8_t countdown, uint8_t captureTime, uint8_t maxTime);

    // Test message builders
    static String buildTestBrMsg(uint8_t sourceId);
    static String buildTestBrResponseMsg(uint8_t responderId);
    static String buildTestDrMsg(uint8_t targetNodeId, uint8_t packetId);
    static String buildTestDrResponseMsg(uint8_t responderId, uint8_t packetsReceived, uint8_t retryCount, int16_t rssi, float snr);

    // Message parser
    static Message parse(const char *data, size_t length);

    // Helper to get int from params
    static int parseIntParam(const String &param, int defaultVal = 0);

private:
};

#endif