// KOTHServer.cpp
#include "KOTHServer.h"

KOTHServer::KOTHServer(EventBus *eb, HardwareManager *hw, NetworkManager *net, const KOTHConfig &cfg)
    : BaseComponent(eb, hw, net),
      config(cfg),
      gameStartTime(0),
      lastScoreUpdate(0),
      gameRunning(false)
{
    LOG_INFO("KOTH_SERVER", "Initializing KOTH Server with %d nodes", config.nodeCount);
    LOG_DEBUG("KOTH_SERVER", "hwt: %d", uxTaskGetStackHighWaterMark(NULL));
}

KOTHServer::~KOTHServer()
{
    LOG_INFO("KOTH_SERVER", "Destroying KOTH Server");
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
        config.NodeStates[i].controllingTeam = Team::NONE;
        config.NodeStates[i].capturedAt = 0;
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
                        { addingNodeAfterStart(e); });
    eventBus->subscribe(GAME_OVER_INTERUPT, [this](Event e)
                        { gameOverInterup(); });

    eventBus->publish(GAME_STARTED, 0);
    // Broadcast game start

    // send score update to initialize statistics with a little delay
    //
    TimerHandle_t timer = xTimerCreate(
        "startTimer",
        pdMS_TO_TICKS(5000),
        pdFALSE,
        (void *)this,
        KOTHServer::startCallback);
    xTimerStart(timer, 0);

    LOG_INFO("KOTH_SERVER", "KOTH Server ready!");
    eventBus->publish(DEBUG, KOTH_CONFIG, 0, 0, config.nodeCount, config.NodeStates); // change later
}

void KOTHServer::startCallback(TimerHandle_t xtimer)
{
    KOTHServer *self = (KOTHServer *)pvTimerGetTimerID(xtimer);

    String startMsg = Protocol::buildGameStart();
    self->startCallbackHelper();
}

void KOTHServer::startCallbackHelper()
{
    if (networkManager)
    {
        networkManager->broadcast(buildScoreWithoutInfNodes(scoringInterval, score.yellowPoints, score.bluePoints, config.nodeCount, config.NodeStates));
        LOG_DEBUG("KOTH_SERVER", "Sent score update to clients");
    }
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
    LOG_INFO("KOTH_SERVER", "starting main loop");

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

    LOG_INFO("KOTH_SERVER", "exiting main loop");
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
    NodeState *node = config.findNode(nodeId);
    if (!node)
    {
        LOG_ERROR("KOTH_SERVER", "Unknown node: %d", nodeId);
        return;
    }

    // Check if already controlled by this team
    if (node->controllingTeam == team)
    {
        LOG_WARN("KOTH_SERVER", "Node %d already controlled by this team", nodeId);
        return;
    }

    // Capture the node!
    node->controllingTeam = team;
    node->capturedAt = millis();

    LOG_INFO("KOTH_SERVER", "Node %d captured by %s team", nodeId, team == Team::YELLOW ? "YELLOW" : "BLUE");
    String msg = buildScoreWithoutInfNodes(scoringInterval, score.yellowPoints, score.bluePoints, config.nodeCount, config.NodeStates);
    LOG_DEBUG("KOTH_SERVER", "Broadcasting capture update: %s", msg.c_str());
    if (networkManager)
    {
        networkManager->broadcast(msg);
    }

    // TESTING
    //  display nodes table
    LOG_DEBUG("KOTH_SERVER", "Current Node States:");
    for (uint8_t i = 0; i < config.nodeCount; i++)
    {
        LOG_DEBUG("KOTH_SERVER", "Node %d: Controlled by %s, Captured at %lu", config.NodeStates[i].nodeId,
                  config.NodeStates[i].controllingTeam == Team::YELLOW ? "YELLOW" : config.NodeStates[i].controllingTeam == Team::BLUE ? "BLUE"
                                                                                                                                       : "NONE",
                  config.NodeStates[i].capturedAt);
    }
}

void KOTHServer::updateScore()
{
    // Count nodes controlled by each team
    uint8_t yellowNodes = config.countNodesControlledBy(Team::YELLOW);
    uint8_t blueNodes = config.countNodesControlledBy(Team::BLUE);

    // Award points (1 point per node per interval)
    score.yellowPoints += yellowNodes;
    score.bluePoints += blueNodes;

    LOG_INFO("KOTH_SERVER", "Score: Yellow %d (%d nodes), Blue %d (%d nodes)", score.yellowPoints, yellowNodes, score.bluePoints, blueNodes);
    // Broadcast score update

    broadcastScoreUpdate();

    eventBus->publish(KOTH_SCORE_UPDATE, scoringInterval, score.yellowPoints, score.bluePoints, config.nodeCount, config.NodeStates);
}

void KOTHServer::broadcastScoreUpdate()
{
    String msg = buildScoreWithoutInfNodes(scoringInterval, score.yellowPoints, score.bluePoints, config.nodeCount, config.NodeStates);

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

//i swear i will optimize this when i have more time this is just a band-aid
String KOTHServer::buildScoreWithoutInfNodes(uint16_t time, uint16_t teamYPoints, uint16_t teamBPoints, uint16_t pairs, NodeState teamPoints[10])
{
    NodeState cap[10];
    uint16_t capSize = 0;
    for (size_t i = 0; i < pairs; i++)
    {
        LOG_DEBUG("KOTH_SERVER", "hwt: %d", uxTaskGetStackHighWaterMark(NULL));
        if (teamPoints[i].type == CAPTURE_POINT)
        {
            cap[capSize] = teamPoints[i];
            capSize++;
            LOG_DEBUG("KOTH_SERVER", "Node %d is a capture point, adding to score update", teamPoints[i].nodeId);
        }
    }

    String msg = Protocol::buildScoreUpdateMessage(time, teamYPoints, teamBPoints, capSize, cap);
    return msg;
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
    for (uint8_t i = 0; i < config.nodeCount; i++)
    {
        if (config.NodeStates[i].nodeId == LORA_ADDRESS) // skip this node
        {
            LOG_ERROR("KOTH_SERVER", "Skipping myself");
            continue;
        }
        EventGroupHandle_t ackEvents = xEventGroupCreate();
        EventBits_t expectedBits = (1 << config.NodeStates[i].nodeId);
        networkManager->sendToAndWait(config.NodeStates[i].nodeId, msg, ackEvents, expectedBits);
        EventBits_t result = xEventGroupWaitBits(ackEvents, expectedBits, pdTRUE, pdTRUE, pdMS_TO_TICKS(10000));
        if (result & expectedBits)
        {
            LOG_DEBUG("KOTH_SERVER", "Node %d acknowledged game over", config.NodeStates[i].nodeId);
        }
        vEventGroupDelete(ackEvents);
        // networkManager->sendTo(config.NodeStates[i].Id, msg);
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

void KOTHServer::addingNodeAfterStart(Event e)
{
    EventGroupHandle_t ackforDiscover = xEventGroupCreate();
    EventBits_t expectedBits = (1 << 2);
    uint8_t nodeId = e.data1;
    bool isInfo = e.data2;

    // send discover message to new node
    String discoverMsg = Protocol::buildDiscoverRequest();
    networkManager->sendToAndWait(nodeId, discoverMsg, ackforDiscover, expectedBits);
    EventBits_t resultDsc = xEventGroupWaitBits(ackforDiscover, expectedBits, pdTRUE, pdTRUE, pdMS_TO_TICKS(10000));
    if (resultDsc & expectedBits)
    {
        LOG_INFO("KOTH_SERVER", "Node %d responded to discover, sending game config", nodeId);
        addNewNode(nodeId, isInfo);
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
    String configMsg = Protocol::buildKothConfigClient(config.maxPoints, 0, config.captureTime, config.gameDurationMinutes, config.respawnTime);
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

    TimerHandle_t timer = xTimerCreate(
        "reconnectTimer",
        pdMS_TO_TICKS(3000),
        pdFALSE,
        (void *)this,
        KOTHServer::reconnectCallback);
    xTimerStart(timer, 0);
    networkManager->sendTo(nodeId, buildScoreWithoutInfNodes(scoringInterval, score.yellowPoints, score.bluePoints, config.nodeCount, config.NodeStates));
}

void KOTHServer::addNewNode(uint8_t nodeId, bool isInfo)
{
    bool alreadyIn = false;
    // add new node if it doesn't exist
    LOG_DEBUG("KOTH_SERVER", "Received game config request from node %d, type %s", nodeId, isInfo ? "INFO" : "CAPTURE");
    uint8_t idx = 0;
    if (!config.hasNode(nodeId))
    {
        config.addNode(nodeId, isInfo ? INFORMATION : CAPTURE_POINT);
        idx = config.nodeCount - 1;
        LOG_DEBUG("KOTH_SERVER", "added to array %d, type %d", config.NodeStates[idx].nodeId, config.NodeStates[idx].type);
    }
    else
    {
        alreadyIn = true;
    }

    if (alreadyIn)
    {
        LOG_INFO("KOTH_SERVER", "Node %d already in game, reviving that node", nodeId);
    }
    else
    {
        LOG_INFO("KOTH_SERVER", "Adding new node %d to game after start", nodeId);
    }
}

void KOTHServer::reconnectCallback(TimerHandle_t xtimer)
{
    KOTHServer *self = (KOTHServer *)pvTimerGetTimerID(xtimer);

    String startMsg = Protocol::buildGameStart();
    self->reconnectCallbackHelper();
}

void KOTHServer::reconnectCallbackHelper()
{
    if (networkManager)
    {
        networkManager->broadcast(buildScoreWithoutInfNodes(scoringInterval, score.yellowPoints, score.bluePoints, config.nodeCount, config.NodeStates));
        LOG_DEBUG("KOTH_SERVER", "Sent score update to clients");
    }
}

void KOTHServer::gameScoreRequest(Event e)
{
    uint8_t nodeId = e.data1;
    LOG_INFO("KOTH_SERVER", "Received game score request from node %d", nodeId);
    String msg = buildScoreWithoutInfNodes(scoringInterval, score.yellowPoints, score.bluePoints, config.nodeCount, config.NodeStates);
    networkManager->sendTo(nodeId, msg);
}

void KOTHServer::gameOverInterup()
{
    scoringInterval = config.gameDurationMinutes;
}