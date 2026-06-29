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
    const NodeState* getNodes() const { return nodes; }
    uint8_t getNodeCount() const { return nodeCount; }

private:
    KOTHConfig config;
    NodeState nodes[MAX_CAPTURE_NODES];  // Max 10 nodes
    uint8_t nodeCount = 0;
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
    NodeState* findNode(uint8_t nodeId);
    uint8_t countNodesControlledBy(Team team);
    void gameOverInterup();
    bool isGameOver();
    Team determineWinner();
};

#endif