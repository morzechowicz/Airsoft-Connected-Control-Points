#ifndef GAME_LOGIC_H  // Header guard to prevent multiple inclusions
#define GAME_LOGIN_H

#include "Team.h"
#include "GameState.h"


class GameLogic
{
private:
    Team teams[2];
    GameState gameState;

public:
    void configureGame();
    void startCountDown();
    void startGame();
    void endGame();
};

#endif