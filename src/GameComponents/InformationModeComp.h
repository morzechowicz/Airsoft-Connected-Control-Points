#ifndef INFORMATION_MODE_COMP_H
#define INFORMATION_MODE_COMP_H

#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"

class InformationModeComp
{
public:
    InformationModeComp(EventBus* eventBus, HardwareManager* hardware, NetworkManager* network);
    
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
    
    KOTHGameScore lastKnownScore;
    int nodeCount = 0;
    NodeState lastKnownNodeStates[10];
    int gameTime = 0;

    void updateDisplay();
    void handleGameOver(Team winner);
    void onButtonPressed(Event e);
    void onButtonReleased(Event e);
    void onNetworkMessage(Event e);

};
#endif // INFORMATION_MODE_COMP_H