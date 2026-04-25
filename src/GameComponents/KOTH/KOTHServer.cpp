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
    LOG_INFO("KOTH_SERVER", "Initializing KOTH Server with %d nodes", nodeCount);

    // Initialize nodes from config
    for (uint8_t i = 0; i < nodeCount; i++)
    {
        nodes[i].nodeId = config.nodeIds[i];
        nodes[i].controllingTeam = Team::NONE;
        nodes[i].capturedAt = 0;
        LOG_INFO("KOTH_SERVER", "Configured node %d with ID %d", i, nodes[i].nodeId);
    }
}

KOTHServer::~KOTHServer()
{
    LOG_DEBUG("KOTH_SERVER", "Destroying KOTH Server");
    eventBus->unsubscribe(KOTH_POINT_CAPTURED);
    eventBus->unsubscribe(PAUSE);
    eventBus->unsubscribe(RESUME);
    eventBus->unsubscribe(GAME_OVER_INTERUPT);

    stopModeTask();
}

void KOTHServer::enterMode()
{
    LOG_INFO("KOTH_SERVER", "Entering KOTH Server mode");

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
    eventBus->subscribe(GAME_REQUEST_SCORE_UPDATE, [this](Event e)
                        { gameScoreRequest(e); });
    eventBus->subscribe(GAME_REQUEST_START_CONF, [this](Event e)
                        { gameConfRequest(e); });
    eventBus->subscribe(GAME_OVER_INTERUPT, [this](Event e)
                        { gameOverInterup(); });

    eventBus->publish(GAME_STARTED, 0);
    // Broadcast game start

    String startMsg = Protocol::buildGameStart();
    if (networkManager)
    {
        networkManager->broadcast(startMsg);
        networkManager->broadcast(Protocol::buildScoreUpdateMessage(scoringInterval, score.yellowPoints, score.bluePoints, nodeCount, nodes));
    }

    LOG_INFO("KOTH_SERVER", "KOTH Server ready!");
    eventBus->publish(DEBUG, KOTH_CONFIG, 0, 0, nodeCount, nodes); // change later
}

void KOTHServer::exitMode()
{
    gameRunning = false;
    LOG_INFO("KOTH_SERVER", "KOTH Server exiting...");

    LOG_DEBUG("KOTH_SERVER", "Destroying KOTH Server");
    eventBus->unsubscribe(KOTH_POINT_CAPTURED);
    eventBus->unsubscribe(PAUSE);
    eventBus->unsubscribe(RESUME);
    eventBus->unsubscribe(GAME_OVER_INTERUPT);

    stopModeTask();
}

void KOTHServer::run()
{
    LOG_INFO("KOTH_SERVER", "KOTHServer::run() starting main loop");

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

    LOG_INFO("KOTH_SERVER", "KOTHServer::run() exiting loop");
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
        LOG_ERROR("KOTH_SERVER", "Invalid team in capture request");
        return;
    }

    // Find node
    NodeState *node = findNode(nodeId);
    if (!node)
    {
        LOG_ERROR("KOTH_SERVER", "Unknown node: %d", nodeId);
        return;
    }

    // Check if already controlled by this team
    if (node->controllingTeam == team)
    {
        LOG_INFO("KOTH_SERVER", "Node %d already controlled by this team", nodeId);
        return;
    }

    // Capture the node!
    node->controllingTeam = team;
    node->capturedAt = millis();

    LOG_INFO("KOTH_SERVER", "Node %d captured by %s team", nodeId, team == Team::YELLOW ? "YELLOW" : "BLUE");

    if (networkManager)
    {
        networkManager->broadcast(Protocol::buildScoreUpdateMessage(scoringInterval, score.yellowPoints, score.bluePoints, nodeCount, nodes));
    }
}

void KOTHServer::updateScore()
{
    // Count nodes controlled by each team
    uint8_t yellowNodes = countNodesControlledBy(Team::YELLOW);
    uint8_t blueNodes = countNodesControlledBy(Team::BLUE);

    // Award points (1 point per node per interval)
    score.yellowPoints += yellowNodes;
    score.bluePoints += blueNodes;

    LOG_INFO("KOTH_SERVER", "Score: Yellow %d (%d nodes), Blue %d (%d nodes)", score.yellowPoints, yellowNodes, score.bluePoints, blueNodes);
    // Broadcast score update

    broadcastScoreUpdate();

    eventBus->publish(KOTH_SCORE_UPDATE, scoringInterval, score.yellowPoints, score.bluePoints, nodeCount, nodes);
}

void KOTHServer::broadcastScoreUpdate()
{
    String msg = Protocol::buildScoreUpdateMessage(scoringInterval, score.yellowPoints, score.bluePoints, nodeCount, nodes);

    if (networkManager)
    {
        networkManager->broadcast(msg);
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
    LOG_INFO("KOTH_SERVER", "========== GAME OVER ==========");
    LOG_INFO("KOTH_SERVER", "Winner: %s", winner == Team::YELLOW ? "YELLOW" : winner == Team::BLUE ? "BLUE"
                                                                                                   : "DRAW");
    LOG_INFO("KOTH_SERVER", "Final Score - Yellow: %d, Blue: %d", score.yellowPoints, score.bluePoints);

    // Broadcast game over
    String msg = Protocol::buildGameOver((uint8_t)winner);

    // Iterate through nodes and send them the game over message
    for (uint8_t i = 0; i < nodeCount; i++)
    {
        if (nodes[i].nodeId == LORA_ADDRESS) // skip this node
        {
            LOG_ERROR("KOTH_SERVER", "Skipping myself");
            continue;
        }
        EventGroupHandle_t ackEvents = xEventGroupCreate();
        EventBits_t expectedBits = (1 << nodes[i].nodeId);
        networkManager->sendToAndWait(nodes[i].nodeId, msg, ackEvents, expectedBits);
        EventBits_t result = xEventGroupWaitBits(ackEvents, expectedBits, pdTRUE, pdTRUE, pdMS_TO_TICKS(10000));
        if (result & expectedBits)
        {
            LOG_DEBUG("KOTH_SERVER", "Node %d acknowledged game over", nodes[i].nodeId);
        }
        vEventGroupDelete(ackEvents);
        // networkManager->sendTo(nodes[i].nodeId, msg);
        vTaskDelay(500);
    }
    vTaskDelay(1000); // assume thats enough and then broadcast fin to everyone else
    networkManager->broadcast(msg);
    // spamming netwrok a little arent we?

    // Publish local event
    eventBus->publish(GAME_OVER, (int)winner);

    gameRunning = false;
}

void KOTHServer::pauseGame(Event e)
{
    gameRunning = false;
    if (e.data1)
    {
        LOG_INFO("KOTH_SERVER", "Sending Pause msg");
        String msg = Protocol::buildPause();
        networkManager->broadcast(msg);
    }
}

void KOTHServer::resumeGame(Event e)
{
    gameRunning = true;
    if (e.data1)
    {
        LOG_INFO("KOTH_SERVER", "Sending Resume msg");
        String msg = Protocol::buildResume();
        networkManager->broadcast(msg);
    }
}

void KOTHServer::gameConfRequest(Event e)
{
    uint8_t nodeId = e.data1;
    bool alreadyIn = false;
    // add new node if it doesn't exist
    LOG_DEBUG("KOTH_SERVER", "Received game config request from node %d", nodeId);
    if (findNode(nodeId) == nullptr && nodeCount < 10)
    {
        nodes[nodeCount].nodeId = nodeId;
        nodes[nodeCount].controllingTeam = Team::NONE;
        nodes[nodeCount].capturedAt = 0;
        nodeCount++;
    }
    else
    {
        alreadyIn = true;
    }
    if (alreadyIn)
    {

        LOG_INFO("KOTH_SERVER", "Node %d already in game, reviving that node", nodeId);
        addingNodeAfterStart(nodeId, e);
    }
    else
    {
        LOG_INFO("KOTH_SERVER", "Adding new node %d to game after start", nodeId);
        addingNodeAfterStart(nodeId, e);
    }
}

void KOTHServer::addingNodeAfterStart(uint8_t nodeId, Event e)
{
    EventGroupHandle_t ackforDiscover = xEventGroupCreate();
    EventBits_t expectedBits = (1 << 2);

    // send discover message to new node
    String discoverMsg = Protocol::buildDiscoverRequest();
    networkManager->sendToAndWait(nodeId, discoverMsg, ackforDiscover, expectedBits);
    EventBits_t resultDsc = xEventGroupWaitBits(ackforDiscover, expectedBits, pdTRUE, pdTRUE, pdMS_TO_TICKS(10000));
    if (resultDsc & expectedBits)
    {
        LOG_INFO("KOTH_SERVER", "Node %d responded to discover, sending game config", nodeId);
    }
    else
    {
        LOG_ERROR("KOTH_SERVER", "Node %d did not respond to discover, cannot add to game", nodeId);
        vEventGroupDelete(ackforDiscover);
        return;
    }
    vEventGroupDelete(ackforDiscover);

    EventGroupHandle_t ackforConfig = xEventGroupCreate();
    LOG_INFO("KOTH_SERVER", "Received game config request, sending current config");
    String configMsg = Protocol::buildKothConfigClient(config.maxPoints, 10, config.captureTime, config.gameDurationMinutes);
    networkManager->sendToAndWait(nodeId, configMsg, ackforConfig, expectedBits);

    LOG_DEBUG("KOTH_SERVER", "Waiting for ACK from node %d", nodeId);
    EventBits_t resultConfig = xEventGroupWaitBits(ackforConfig, expectedBits, pdTRUE, pdTRUE, pdMS_TO_TICKS(10000));
    if (resultConfig & expectedBits)
    {
        LOG_INFO("KOTH_SERVER", "Node %d acknowledged config, sending game start", nodeId);
    }
    else
    {
        LOG_ERROR("KOTH_SERVER", "Node %d did not acknowledge config, cannot add to game", nodeId);
        vEventGroupDelete(ackforConfig);
        return;
    }
    vEventGroupDelete(ackforConfig);

    String startMsg = Protocol::buildGameStart();
    networkManager->sendTo(nodeId, startMsg);
    vTaskDelay(2000); // update can be sent even minutes later not gonna affect much
    networkManager->sendTo(nodeId, Protocol::buildScoreUpdateMessage(scoringInterval, score.yellowPoints, score.bluePoints, nodeCount, nodes));
}

void KOTHServer::gameScoreRequest(Event e)
{
    uint8_t nodeId = e.data1;
    LOG_DEBUG("KOTH_SERVER", "Received game score request from node %d", nodeId);
    String msg = Protocol::buildScoreUpdateMessage(scoringInterval, score.yellowPoints, score.bluePoints, nodeCount, nodes);
    networkManager->sendTo(nodeId, msg);
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