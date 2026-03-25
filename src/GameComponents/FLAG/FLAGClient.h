// KOTHClient.h
#ifndef FLAG_CLIENT_H
#define FLAG_CLIENT_H

#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "FLAGTypes.h"
#include "Protocol.h"
#include "../lib/Logging/LogManager.h"

class FLAGClient
{
public:
    FLAGClient(EventBus *eb, HardwareManager *hw, NetworkManager *net,
               uint8_t nodeId, FLAGConfig config);
    ~FLAGClient();

    void start();
    void stop();
    void update();

    // Getters
    FlagTeam getControllingTeam() const { return currentController; }
    bool isCapturing() const { return capturing; }
    float getCaptureProgress() const;
    bool getDeleteThis() { return deleteThis; };

private:
    EventBus *eventBus;
    HardwareManager *hardware;
    NetworkManager *network;

    uint8_t myNodeId;
    FlagTeam currentController;
    FlagTeam capturingTeam;

    bool deleteThis = false;
    bool gameActive;
    bool capturing;
    unsigned long captureStartTime;
    uint16_t captureTimeMs;
    uint8_t maxTeams; 

    unsigned long lastDisplayUpdate = 0;
    unsigned long locatingBeepSpacingUpdate = 0;

    FLAGGameScore lastKnownScore;

    // Event handlers
    void onButtonPressed(Event e);
    void onButtonReleased(Event e);
    void onNetworkMessage(Event e);

    // Capture logic
    void startCapture(FlagTeam team);
    void updateCapture();
    void completeCapture();
    void cancelCapture();

    // Message handlers
    void handleGameOver(FlagTeam winner);

    // Display helpers
    void updateDisplay();
    void updateLEDs();
};

#endif