// KOTHClient.h
#ifndef KOTH_CLIENT_H
#define KOTH_CLIENT_H

#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "KOTHTypes.h"
#include "Protocol.h"

class KOTHClient {
public:
    KOTHClient(EventBus* eb, HardwareManager* hw, NetworkManager* net, 
               uint8_t nodeId, KOTHConfig config);
    ~KOTHClient();
    
    void start();
    void stop();
    void update();
    
    // Getters
    Team getControllingTeam() const { return currentController; }
    bool isCapturing() const { return capturing; }
    float getCaptureProgress() const;
    bool getDeleteThis() {return deleteThis;};

private:
    EventBus* eventBus;
    HardwareManager* hardware;
    NetworkManager* network;
    
    uint8_t myNodeId;
    Team currentController;
    Team capturingTeam;
    
    bool deleteThis = false;
    bool gameActive;
    bool gamePaused = false;
    bool capturing;
    unsigned long captureStartTime;
    uint16_t captureTimeMs;
    
    bool gracePeriod;
    unsigned long graceStartTime;
    uint16_t gracePeriodMs;
    unsigned long lastDisplayUpdate = 0;
    unsigned long locatingBeepSpacingUpdate = 0;

    KOTHGameScore lastKnownScore;
    
    // Event handlers
    void onButtonPressed(Event e);
    void onButtonReleased(Event e);
    void onNetworkMessage(Event e);
    
    // Capture logic
    void startCapture(Team team);
    void updateCapture();
    void completeCapture();
    void cancelCapture();
    
    // Message handlers
    void handleGameOver(Team winner);
    
    // Display helpers
    void updateDisplay();
    void updateLEDs();
    void pauseGame(Event e);
    void resumeGame(Event e);
};

#endif