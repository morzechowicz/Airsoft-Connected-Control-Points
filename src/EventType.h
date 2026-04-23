#ifndef EVENT_TYPE_H
#define EVENT_TYPE_H

#include <Arduino.h>
#include "GameComponents/KOTH/KOTHTypes.h"

enum EventType
{
    // Hardware
    BUTTON_PRESSED,
    BUTTON_HELD,
    BUTTON_RELEASED,

    // CONF MASTER EVENT
    KOTH_CONF_UPDATED,
    SEARCH,
    END_RESTART,
    POWER_RESET,
    REQUEST_LOG,
    PAUSE,
    RESUME,

    // Network
    NETWORK_CONNECTED,
    NETWORK_DISCONNECTED,
    NETWORK_MESSAGE_RECEIVED,
    NETWORK_DISCOVER,
    NETWROK_REPORT,
    NETWORK_MAIN_LOOKUP,

    // Game universal
    GAME_STARTED,
    GAME_OVER,
    GAME_COUNTDOWN,
    GAME_PAUSE,
    GAME_OVER_INTERUPT,
    GAME_REQUEST_START_CONF,
    GAME_REQUEST_SCORE_UPDATE,

    // KOTH specific
    KOTH_CONFIG,
    KOTH_CAPTURE_REQUEST,
    KOTH_POINT_CAPTURED,
    KOTH_POINT_NEUTRALIZED,
    KOTH_SCORE_UPDATE,

    // CTF specific (for future)
    CTF_FLAG_PICKED,
    CTF_FLAG_CAPTURED,

    // System
    SYS,
    GAME,
    CONF,
    DEBUG,
    UNKNOWN,
    FORWARD,
    TEST,

    // FLAG specific
    FLAG_CAPTURED,
    FLAG_CONFIG,
    FLAG_SCORE_UPDATE,
    
    EVENT_MAX
};

struct Event
{
    EventType type;
    int data1;
    int data2;
    int data3;
    int data4;
    int data5;
    NodeState teamPoints[10];
    String message;
};

#endif // EVENT_TYPE_H