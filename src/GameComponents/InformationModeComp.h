#ifndef INFORMATION_MODE_COMP_H
#define INFORMATION_MODE_COMP_H

#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"

class InformationModeComp
{
public:
    InformationModeComp(EventBus* eventBus, HardwareManager* hardware, NetworkManager* network, KOTHConfig config);
    
    void start();
    void stop();
    void update();

    bool getDeleteThis() { return deleteThis; };
private:
    EventBus* eventBus;
    HardwareManager* hardware;
    NetworkManager* network;

    bool deleteThis = false;
    bool gameActive;
    bool gamePaused = false;
    
    const char teamChar(Team t) {
    switch(t) {
        case Team::YELLOW: return 'Y';
        case Team::BLUE:   return 'B';
        default:           return 'N';
    }
}

    KOTHConfig config;
    KOTHGameScore lastKnownScore;
    int nodeCount = 0;
    NodeState lastKnownNodeStates[10];
    int gameTime = 0;
    int gameMaxTime = 0;

    void updateDisplay();
    void handleGameOver(Team winner);
    void onButtonPressed(Event e);
    void onButtonReleased(Event e);
    void onNetworkMessage(Event e);
    void onScoreUpdate(Event e);

    void pauseGame(Event e);
    void resumeGame(Event e);
    String buildRow(int startIdx, int count, int totalNodes);

    
    // respawn Task
    TaskHandle_t respawnTaskHandle;
    static volatile bool respawnTaskBit;
    static void respawnTask(void* pvParameters);
    void startRespawnTask();
    void respawnHelper();
    void killRespawnTask();
};
#endif // INFORMATION_MODE_COMP_H