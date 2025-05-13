#ifndef GAME_MANAGER_H  // Header guard to prevent multiple inclusions
#define GAME_MANAGER_H

#include <cstdint>
#include "Arduino.h"
#include <ezButton.h>

// Game States
#define GAME_STATE_CONFIG 0
#define GAME_STATE_COUNTDOWN 1
#define GAME_STATE_PLAYING 2
#define GAME_STATE_END 3

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

// default game settings
#define DEFAULT_MAX_SCORE 60 // in points
#define DEFAULT_MAX_TIME 60 // in minutes
#define DEFAULT_TIME_TO_START 300 //in seconds
#define DEFAULT_TIME_TO_CAPTURE 15 //in seconds



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

    GameManager();
    void initializeLoop(ezButton &teamYellowButton, ezButton &teamBlueButton, ezButton &startGameButton, void (*sendGameStatus)(int));
    void gameLoop(ezButton &teamBlueButton, ezButton &teamYellowButton, ezButton &startGameButton, uint16_t NodeId, uint16_t LeaderId);
    void countdownLoop();
    void endGameLoop();

    void updateTeamScore(int teamId, int score);
    void changeLedColor(int teamId);
    void updateTotalTeamScore(uint16_t NodeId,Team *singleScore);
    int getTotalScore(int teamId);
    bool startGameAction(ezButton &startGameButton, void (*sendGameStatus)(int));
    void endConfigAction(ezButton &startGameButton);
    void yellowButtonConfigAction(ezButton &teamBlueButton);
    void blueButtonConfigAction(ezButton &teamYellowButton);
    int endGameAction();

    //getters
    Team *getLocalTeamsScore() { return localTeamsScore; }
    int getMaxScore() const { return maxScore; }
    int getMaxTime() const { return maxTime; }
    int getTimeToStart() const { return timeToStart; }
    int getTimeToCapture() const { return timeToCapture; }
    int getCurrentTime() const { return currentTime; }
    int getCurrentSettingId() const { return currentSettingId; }
    int getPointControlledByTeam() const { return pointControlledByTeam; }
    int getCurrentGameState() const { return currentGameState; }

    //setters
    int setCurrentGameState(int gameState) { currentGameState = gameState; return currentGameState; }
private:
    Team localTeamsScore[2];
    TotalTeamScore totalTeamScore[10];
    
    int currentGameState;
    int pointControlledByTeam;
    
    int maxScore;      // in minutes 1 point every minute id 0
    int maxTime;       // max time in minutes id 1
    int timeToStart;    // coutdown time in minuts id 2
    int timeToCapture; // time to capture in seconds id 3
    int currentTime;    // id 4

    int currentSettingId;
};

#endif // GAME_MANAGER_H