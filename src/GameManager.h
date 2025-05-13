#ifndef GAME_MANAGER_H  // Header guard to prevent multiple inclusions
#define GAME_MANAGER_H

#include <cstdint>
#include "Arduino.h"
#include <ezButton.h>

// Team variables
#define TEAM_BLUE 0
#define TEAM_YELLOW 1
#define TEAM_NONE 99

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
        int score;
    };
    struct TotalTeamScore
    {
        uint16_t NodeId;
        Team score[2];
    };

    GameManager();  // Constructor declaration

    void updateTeamScore(int teamId, int score);
    void changeLedColor(int teamId);
    void updateTotalTeamScore(uint16_t NodeId,Team *singleScore);
    int getTotalScore(int teamId);
    bool startGameAction(ezButton &startGameButton, void (*sendGameStatus)(bool));
    void endConfigAction(ezButton &startGameButton);
    void yellowButtonConfigAction(ezButton &teamBlueButton);
    void blueButtonConfigAction(ezButton &teamYellowButton);

    //getters
    Team *getLocalTeamsScore() { return localTeamsScore; }
    int getMaxScore() const { return maxScore; }
    int getMaxTime() const { return maxTime; }
    bool getIsGameInProgress() const { return isGameInProgress; }
    bool getConfigMode() const { return configMode; }
    int getTimeToStart() const { return timeToStart; }
    int getTimeToCapture() const { return timeToCapture; }
    int getCurrentTime() const { return currentTime; }
    int getCurrentSettingId() const { return currentSettingId; }

    //setters
    bool setIsGameInProgress(bool isGameInProgress) { this->isGameInProgress = isGameInProgress; return this->isGameInProgress; }
private:
    Team localTeamsScore[2];
    TotalTeamScore totalTeamScore[10];
    
    bool isGameInProgress;
    int pointControlledByTeam;
    bool configMode;
    
    int maxScore;      // in minutes 1 point every minute id 0
    int maxTime;       // max time in minutes id 1
    int timeToStart;    // coutdown time in minuts id 2
    int timeToCapture; // time to capture in seconds id 3
    int currentTime;    // id 4

    int currentSettingId;
};

#endif // GAME_MANAGER_H