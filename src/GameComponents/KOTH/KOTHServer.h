// KOTHServer.h
#ifndef KOTH_SERVER_H
#define KOTH_SERVER_H

#include "GameComponents/BaseComponent.h"
#include "KOTHTypes.h"
#include "Protocol.h"

class KOTHServer : public BaseComponent {
public:
    KOTHServer(EventBus *eventBus, HardwareManager *hardware, NetworkManager *network, const KOTHConfig &config);
    void addNodeFromConfig(NodeInit node);
    ~KOTHServer();
    
    // BaseComponent overrides
    void enterMode() override;
    void exitMode() override;
    void run() override;
    
    // Getters
    const KOTHGameScore& getScore() const { return score; }

private:
    KOTHConfig config;
    KOTHGameScore score;
    int scoringInterval = 0;

    unsigned long gameStartTime;
    unsigned long lastScoreUpdate;
    bool gameRunning;
    
    // Event handlers
    void onCaptureRequest(Event e);
    
    // Game logic
    void processCaptureRequest(uint8_t nodeId, Team team);
    void updateScore();
    void broadcastScoreUpdate();
    void endGame(Team winner);
    void pauseGame(Event e);
    void resumeGame(Event e);
    void addingNodeAfterStart(Event e);
    void addNewNode(uint8_t nodeId, bool isInfo);
    void gameScoreRequest(Event e);

    //timer
    static void reconnectCallback(TimerHandle_t xtimer);
    void reconnectCallbackHelper();
    static void startCallback(TimerHandle_t xtimer);
    void startCallbackHelper();
    

    // Helper functions
    void gameOverInterup();
    bool isGameOver();
    Team determineWinner();
    String buildScoreWithoutInfNodes(uint16_t time, uint16_t teamYPoints, uint16_t teamBPoints, uint16_t pairs, NodeState teamPoints[10]);
};

#endif