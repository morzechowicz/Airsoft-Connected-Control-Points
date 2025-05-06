#ifndef GAME_MANAGER_H  // Header guard to prevent multiple inclusions
#define GAME_MANAGER_H

#include <cstdint>
#include "Arduino.h"

// Team variables
#define TEAM_BLUE 0
#define TEAM_YELLOW 1

// Message format
#define WHO_IS_OUT_THERE 0x01
#define I_AM_HERE 0x02
#define SCORE_UPDATE 0x03

// LEDs
#define LED_YELLOW 13
#define LED_BLUE 12

class GameManager
{
public:
    struct Team
    {
        int id;
        uint16_t score;
    };
    Team localTeamsScore[2];
    
    GameManager();  // Constructor declaration
    void updateTeamScore(int teamId, int score);
    void changeLedColor(int teamId);
};

#endif // GAME_MANAGER_H