#ifndef GAME_LOGIC_H  // Header guard to prevent multiple inclusions
#define GAME_LOGIC_H

#include "Team.h"
#include "GameState.h"
#include <LoRaManager.h>

class GameLogic
{
public:
    void startCountDown();
    void startGame();
    void gameLoop();
    void endGame();

private:
        Team teams[2];
        GameState gameState;
        LoRaHandler loRa;

};

#endif