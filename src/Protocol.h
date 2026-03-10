// Protocol.h
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include "EventType.h"

// message structure
//  TYPE;PARAM;PARAM;...;
struct Message
{
    EventType type;
    String params[10];
    int paramCount;
};

class Protocol
{
public:
    //universal message types
    static String buildGameStart();
    static String buildGameOver(uint8_t winner);
    static String buildDiscoverRequest();
    static String buildDiscoverResponse(uint8_t nodeID);
    static String buildPowerResetMsg();
    
    
    // KOTH Message builders
    static String buildKothConfig(uint8_t maxPoints,uint8_t countdown, uint8_t captureTime, uint8_t maxTime);
    static String buildCaptureMessage(uint8_t nodeId, uint8_t teamId);
    static String buildScoreUpdateMessage(uint16_t teamYPoints, uint16_t teamBPoints);
    static String buildKothConfigUpdated(uint8_t maxPoints, uint8_t countdown, uint8_t captureTime, uint8_t maxTime);

    // Message parser
    static Message parse(const char *data, size_t length);

    // Helper to get int from params
    static int parseIntParam(const String &param, int defaultVal = 0);

private:
};

#endif