#ifndef MESSAGE_BUILDER_H
#define MESSAGE_BUILDER_H

#include <Arduino.h>
//wtf is this?
// is this even used any where?
//TO DO: remove if unsused
class MessageBuilder {
public:
    static String buildCapture(uint8_t nodeId, uint8_t teamId) {
        return "CAP;" + String(nodeId) + ";" + String(teamId);
    }
    
    static String buildScoreUpdate(uint16_t teamY, uint16_t teamB) {
        return "UPD;" + String(teamY) + ";" + String(teamB);
    }
    
    static String buildGameStarted() {
        return "START";
    }
    
    static String buildGameOver(uint8_t winner) {
        return "END;" + String(winner);
    }
};

#endif // MESSAGE_BUILDER_H