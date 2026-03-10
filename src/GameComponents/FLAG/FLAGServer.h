#ifndef FLAG_SERVER_H
#define FLAG_SERVER_H

#include "GameComponents/BaseComponent.h"
#include "Protocol.h"
#include "FLAGTypes.h"


class FLAGServer : public BaseComponent {
public:
    FLAGServer(EventBus *eb, HardwareManager *hw, NetworkManager *net, const FLAGConfig &cfg);
    ~FLAGServer();
    
    // BaseComponent overrides
    void enterMode() override;
    void exitMode() override;
    void run() override;
    
    // Getters
    const FLAGGameScore& getScore() const { return score; }

private:
    FLAGConfig config;
    FLAGGameScore score;
    int scoringInterval = 0;

    unsigned long gameStartTime;
    unsigned long lastScoreUpdate;
    bool gameRunning;
    
    // Event handlers
    void onCaptureRequest(Event e);
    
    // Game logic
    void processCaptureRequest(FlagTeam team);
    void updateScore();
    void broadcastScoreUpdate();
    void checkWinConditions();
    void endGame(FlagTeam winner);
    void pauseGame();
    
    // Helper functions
    bool isGameOver();
    FlagTeam determineWinner();
};

#endif //FLAG_SERVER_H