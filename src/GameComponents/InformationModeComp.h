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

private:
    EventBus* eventBus;
    HardwareManager* hardware;
    NetworkManager* network;

    bool deleteThis = false;
    bool gameActive;
    bool gamePaused = false;
    
    KOTHGameScore lastKnownScore;

    void updateDisplay();

};
#endif // INFORMATION_MODE_COMP_H