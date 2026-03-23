// KOTHServer.cpp
#include "KOTHServer.h"

KOTHServer::KOTHServer(EventBus *eb, HardwareManager *hw, NetworkManager *net, const KOTHConfig &cfg)
    : BaseComponent(eb, hw, net),
      config(cfg),
      nodeCount(cfg.nodeCount),
      gameStartTime(0),
      lastScoreUpdate(0),
      gameRunning(false)
{
    Serial.print("Initializing KOTH with ");
    Serial.print(nodeCount);
    Serial.println(" nodes");

    // Initialize nodes from config
    for (uint8_t i = 0; i < nodeCount; i++)
    {
        nodes[i].nodeId = config.nodeIds[i];
        nodes[i].controllingTeam = Team::NONE;
        nodes[i].capturedAt = 0;

        Serial.print("  Node ");
        Serial.print(i);
        Serial.print(": ID = ");
        Serial.println(nodes[i].nodeId);
    }

    if (config.singleNodeMode)
    {
        Serial.println("Single node mode - network messages disabled");
    }
}

KOTHServer::~KOTHServer()
{
    eventBus->unsubscribe(KOTH_POINT_CAPTURED);
    eventBus->unsubscribe(PAUSE);
    eventBus->unsubscribe(RESUME);
    eventBus->unsubscribe(GAME_STARTED);
    eventBus->unsubscribe(GAME_OVER_INTERUPT);
    eventBus->unsubscribe(GAME);

    stopModeTask();
}

void KOTHServer::enterMode()
{
    Serial.println("=== KOTH Server Mode ===");

    // Reset game state
    score.yellowPoints = 0;
    score.bluePoints = 0;
    gameStartTime = millis();
    lastScoreUpdate = millis();
    gameRunning = true;

    // Reset all nodes
    for (uint8_t i = 0; i < config.nodeCount; i++)
    {
        nodes[i].controllingTeam = Team::NONE;
        nodes[i].capturedAt = 0;
    }

    // Subscribe to events
    eventBus->subscribe(KOTH_POINT_CAPTURED, [this](Event e)
                        { onCaptureRequest(e); });
    eventBus->subscribe(PAUSE, [this](Event e)
                        { pauseGame(e); });
    eventBus->subscribe(RESUME, [this](Event e)
                        { resumeGame(e); });

    eventBus->publish(GAME_STARTED, 0);
    eventBus->subscribe(GAME_OVER_INTERUPT, [this](Event e)
                        { gameOverInterup(); });

    // Broadcast game start
    if (!config.singleNodeMode)
    {
        String startMsg = Protocol::buildGameStart();
        if (networkManager)
        {
            networkManager->broadcast(startMsg);
            networkManager->broadcast(Protocol::buildScoreUpdateMessage(scoringInterval,score.yellowPoints, score.bluePoints,nodeCount,nodes));
        }
    }
    Serial.println("KOTH Server ready!");
    eventBus->publish(DEBUG, KOTH_CONFIG, 0,0, nodeCount, nodes); //change later
}

void KOTHServer::exitMode()
{
    gameRunning = false;
    Serial.println("KOTH Server exiting...");
}

void KOTHServer::run()
{
    Serial.println("KOTHServer::run() starting main loop");

    unsigned long lastUpdate = millis();

    while (taskRunning)
    {
        if (gameRunning)
        {
            // Update score at intervals
            if (millis() - lastUpdate >= config.scoreIntervalMs)
            {
                lastUpdate = millis();
                scoringInterval++;
                updateScore();
            }

            // Check win conditions
            if (isGameOver())
            {
                Team winner = determineWinner();
                endGame(winner);
                break; // Exit loop
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    Serial.println("KOTHServer::run() exiting loop");
}

void KOTHServer::onCaptureRequest(Event e)
{
    // Extract packed data:
    uint8_t nodeId = e.data1;
    uint8_t teamId = e.data2;
    Team team = (Team)teamId;

    processCaptureRequest(nodeId, team);
}

void KOTHServer::processCaptureRequest(uint8_t nodeId, Team team)
{
    // Validate team
    if (team != Team::YELLOW && team != Team::BLUE)
    {
        Serial.print("[SERVER] ");
        Serial.println("Invalid team in capture request");
        return;
    }

    // Find node
    NodeState *node = findNode(nodeId);
    if (!node)
    {
        Serial.print("[SERVER] ");
        Serial.print("Unknown node: ");
        Serial.println(nodeId);
        return;
    }

    // Check if already controlled by this team
    if (node->controllingTeam == team)
    {
        Serial.print("[SERVER] ");
        Serial.print("Node ");
        Serial.print(nodeId);
        Serial.println(" already controlled by this team");
        return;
    }

    // Capture the node!
    node->controllingTeam = team;
    node->capturedAt = millis();

    Serial.print("[SERVER] ");
    Serial.print("Node ");
    Serial.print(nodeId);
    Serial.print(" captured by ");
    Serial.println(team == Team::YELLOW ? "YELLOW" : "BLUE");
}

void KOTHServer::updateScore()
{
    // Count nodes controlled by each team
    uint8_t yellowNodes = countNodesControlledBy(Team::YELLOW);
    uint8_t blueNodes = countNodesControlledBy(Team::BLUE);

    // Award points (1 point per node per interval)
    score.yellowPoints += yellowNodes;
    score.bluePoints += blueNodes;

    Serial.print("[SERVER] ");
    Serial.print("Score: Yellow ");
    Serial.print(score.yellowPoints);
    Serial.print(" (");
    Serial.print(yellowNodes);
    Serial.print(" nodes), Blue ");
    Serial.print(score.bluePoints);
    Serial.print(" (");
    Serial.print(blueNodes);
    Serial.println(" nodes)");

    // Broadcast score update
    if (!config.singleNodeMode)
    {
        broadcastScoreUpdate();
    }
    eventBus->publish(KOTH_SCORE_UPDATE, scoringInterval, score.yellowPoints, score.bluePoints, nodeCount, nodes);
}

void KOTHServer::broadcastScoreUpdate()
{
    String msg = Protocol::buildScoreUpdateMessage(scoringInterval,score.yellowPoints, score.bluePoints,nodeCount,nodes);

    if (networkManager)
    {
        networkManager->broadcast(msg);
    }
}

void KOTHServer::checkWinConditions()
{
    if (isGameOver())
    {
        Team winner = determineWinner();
        endGame(winner);
    }
}

bool KOTHServer::isGameOver()
{
    // Check point limit
    if (score.yellowPoints >= config.maxPoints || score.bluePoints >= config.maxPoints)
    {
        return true;
    }

    // Check time limit
    if (scoringInterval >= config.gameDurationMinutes)
    {
        return true;
    }

    return false;
}

Team KOTHServer::determineWinner()
{
    return score.getWinner();
}

void KOTHServer::endGame(Team winner)
{
    Serial.print("[SERVER] ");
    Serial.println("========== GAME OVER ==========");
    Serial.print("Winner: ");

    if (winner == Team::YELLOW)
    {
        Serial.println("YELLOW");
    }
    else if (winner == Team::BLUE)
    {
        Serial.println("BLUE");
    }
    else
    {
        Serial.println("DRAW");
    }

    Serial.print("Final Score - Yellow: ");
    Serial.print(score.yellowPoints);
    Serial.print(", Blue: ");
    Serial.println(score.bluePoints);
    Serial.println("===============================");

    // Broadcast game over
    String msg = Protocol::buildGameOver((uint8_t)winner);
    if (!config.singleNodeMode)
    {
        if (networkManager)
        {
            networkManager->broadcast(msg);
        }
    }

    // Publish local event
    eventBus->publish(GAME_OVER, (int)winner);

    gameRunning = false;
}

void KOTHServer::pauseGame(Event e)
{
    gameRunning = false;
    if (e.data1)
    {
        Serial.println("Sending Pause msg");
        String msg = Protocol::buildPause();
        networkManager->broadcast(msg);
    }
}

void KOTHServer::resumeGame(Event e)
{
    gameRunning = true;
    if (e.data1)
    {
        Serial.println("Sending Resume msg");
        String msg = Protocol::buildResume();
        networkManager->broadcast(msg);
    }
}

NodeState *KOTHServer::findNode(uint8_t nodeId)
{
    for (uint8_t i = 0; i < nodeCount; i++)
    {
        if (nodes[i].nodeId == nodeId)
        {
            return &nodes[i];
        }
    }
    return nullptr;
}

uint8_t KOTHServer::countNodesControlledBy(Team team)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < nodeCount; i++)
    {
        if (nodes[i].controllingTeam == team)
        {
            count++;
        }
    }
    return count;
}

void KOTHServer::gameOverInterup()
{
    scoringInterval = config.gameDurationMinutes;
}