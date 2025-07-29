#pragma once

enum class LoRaMsgCodes {
    MSG_CONFIG = 1,
    MSG_ACK = 2,
    MSG_SCORE = 3,
    MSG_FINISHED = 4,
    MSG_NODE_REPORT = 5,
    MSG_NODE_CONTROLLED_BY = 6,
    PING = 7,
    MSG_RSP = 8,
    MSG_RESTORE = 9
};