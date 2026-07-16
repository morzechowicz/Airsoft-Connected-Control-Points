#include "GameManager.h"

GameManager *GameManager::instance = nullptr;

void GameManager::onConfigureFlag(Event e)
{

    flagConfig.maxPoints = e.data3;
    flagConfig.maxTime = e.data2;
    flagConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    flagConfig.captureTime = e.data4;
    flagConfig.initTeamCount = e.data5;

    Event newNode;
    newNode.data1 = LORA_ADDRESS;

    isMain = true;
    int countdown = e.data1;
    selectedConfig = FLAG_CONFIG;
    LOG_INFO("GAME_MANAGER", "Received FLAG configuration: maxPoints=%d, maxTime=%d, captureTime=%d, scoreIntervalMs=%d, initTeamCount=%d",
             flagConfig.maxPoints, flagConfig.maxTime, flagConfig.captureTime, flagConfig.scoreIntervalMs, flagConfig.initTeamCount);
    LOG_INFO("GAME_MANAGER", "Countdown started");

    startCountdownTask(countdown);
}

void GameManager::onConfKoth(Event e)
{
    kothConfig.maxPoints = e.data3;
    kothConfig.gameDurationMinutes = e.data2;
    kothConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    kothConfig.captureTime = e.data4;
    kothConfig.respawnTime = e.data5;
#if NODE_TYPE != HEADLESS
    Event newNode;
    newNode.data1 = LORA_ADDRESS;
    newNode.data2 = NODE_TYPE;
    // add itself to the table
    onNewNode(newNode);
#endif
    isMain = true;
    int countdown = e.data1;
    selectedConfig = KOTH_CONFIG;

    LOG_INFO("GAME_MANAGER", "Received KOTH configuration: maxPoints=%d, gameDurationMinutes=%d, captureTime=%d, scoreIntervalMs=%d, respawnTime=%d",
             kothConfig.maxPoints, kothConfig.gameDurationMinutes, kothConfig.captureTime, kothConfig.scoreIntervalMs, kothConfig.respawnTime);
    
    String configMsg = Protocol::buildKothConfigClient(kothConfig.maxPoints, countdown, kothConfig.captureTime, kothConfig.gameDurationMinutes, kothConfig.respawnTime);
    
    LOG_INFO("GAME_MANAGER", "Countdown started");
    startCountdownTask(countdown);
    
    for (uint8_t i = 0; i < kothConfig.nodeCount; i++)
    {
        uint8_t nodeId = kothConfig.NodeStates[i].nodeId;
        if(nodeId == LORA_ADDRESS)
        {
            continue;
        }
        EventGroupHandle_t ackEvents = xEventGroupCreate();
        EventBits_t expectedBits = (1 << nodeId);
        networkManager->sendToAndWait(nodeId, configMsg, ackEvents, expectedBits);
        EventBits_t result = xEventGroupWaitBits(ackEvents, expectedBits, pdTRUE, pdTRUE, pdMS_TO_TICKS(10000));
        if (result & expectedBits)
        {
            LOG_DEBUG("KOTH_SERVER", "Node %d acknowledged game config", kothConfig.NodeStates[i].nodeId);
        }
        vEventGroupDelete(ackEvents);
        // networkManager->sendTo(config.NodeStates[i].Id, msg);
        vTaskDelay(500);
    }

}

int GameManager::calCulateCoutdown(int remoteTime)
{
    int localTime = millis() / 1000;
    int countdown = remoteTime - (localTime + syncTimeDelta);
    LOG_DEBUG("GAME MANAGER", "Calculated local %d remotestarting %d delta %d countdown %d", localTime, remoteTime, syncTimeDelta, countdown);
    if (countdown < 0)
    {
        countdown = 0;
    }
    return countdown;
}

void GameManager::onConfigKothFromMaster(Event e)
{
#if NODE_TYPE == HEADLESS
    LOG_INFO("GAME_MANAGER", "Received KOTH configuration from master but this is a headless node, ignoring");
    return;
#endif
    // dont start if main node is not set
#if NODE_TYPE == CAPTURE_POINT
    if (networkManager->isMainNodeSet())
    {
        LOG_INFO("GAME_MANAGER", "Main node is set, starting client");
    }
    else
    {
        LOG_ERROR("GAME_MANAGER", "Main node is not set, cannot start client");
        return;
    }
#endif
    kothConfig.maxPoints = e.data3;
    kothConfig.gameDurationMinutes = e.data2;
    kothConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    kothConfig.captureTime = e.data4;
    kothConfig.respawnTime = e.data5;
    int countdown = calCulateCoutdown(e.data1);
    selectedConfig = KOTH_CONFIG;
    startCountdownTask(countdown);
}

void GameManager::startCountdownTask(int countdown)
{
    LOG_DEBUG("GAME_MANAGER", "Starting countdown task");
    if (kothClient != nullptr || flagClient != nullptr || infoNode != nullptr)
    {
        LOG_WARN("GAME_MANAGER", "A client is already running, aborting");
        return;
    }

    // Delete previous task if still somehow alive
    if (countdownHandler != nullptr)
    {
        vTaskDelete(countdownHandler);
        countdownHandler = nullptr;
    }

    // Heap-allocate so each invocation gets its own params
    struct TaskParams
    {
        GameManager *mgr;
        int countdown;
    };
    auto *params = new TaskParams{this, countdown};

    xTaskCreate(
        [](void *param)
        {
            auto *p = static_cast<TaskParams *>(param);
            GameManager *mgr = p->mgr;
            int cd = p->countdown;

            delete p; // free immediately, before doing any work

            mgr->countdownTask(cd);

            // Clear the handle on the manager before self-deleting
            mgr->countdownHandler = nullptr;
            vTaskDelete(NULL);
        },
        "CountdownTask", 4096, params, 1, &countdownHandler);

    LOG_DEBUG("GAME_MANAGER", "Created countdown task: %p", (void *)countdownHandler);

    hardwareManager->buzzer.beep(500, 2, 300); // beep 2 times so everyone know is about to begin
}

void GameManager::startAfterCountdownTask(int waitTime)
{
    LOG_DEBUG("GAME_MANAGER", "Starting after countdown task");
    xTaskCreate(
        [](void *param)
        {
            auto *p = static_cast<std::pair<GameManager *, int> *>(param);
            GameManager *mgr = p->first;
            int wt = p->second;

            delete p; // free immediately, before doing any work

            mgr->afterCountdownTask(wt);

            // Clear the handle on the manager before self-deleting
            mgr->afterCountdownHandler = nullptr;
            vTaskDelete(NULL);
        },
        "AfterCountdownTask", 2048, new std::pair<GameManager *, int>{this, waitTime}, 1, &afterCountdownHandler);
}

void GameManager::afterCountdownTask(int time)
{
    int responseWait = time;
    while (time > 0)
    {
        if (kothClient)
        {
            LOG_DEBUG("GAME_MANAGER", "AfterCountdownTask: KOTH client already running, aborting");
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        time--;
    }
    LOG_INFO("GAME_MANAGER", "After countdown ended, client didnt start");

    String msg = Protocol::buildReqeustScoreUpdate(LORA_ADDRESS);
    networkManager->sendToMain(msg);
    while (responseWait > 0)
    {
        if (kothClient)
        {
            LOG_DEBUG("GAME_MANAGER", "AfterCountdownTask: KOTH client started after request");
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        responseWait--;
    }
    LOG_INFO("GAME_MANAGER", "No response received PANIC MODE");
    LcdDisplayMessage dsp{};
    dsp.setLine(0, "NET ERROR");
    dsp.setLine(1, "CALL GAME ORG");
    dsp.durationMs = 10;
    hardwareManager->lcd.displayText(dsp);
}

void GameManager::countdownTask(int time)
{
    int countdown = (int)time;
    while (countdown > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        countdown--;
        LOG_DEBUG("GAME_MANAGER", "Countdown: %d", countdown);
        // make it update time only and set screen countdown once
        hardwareManager->lcd.displayCountdown(countdown);
    }
    LOG_INFO("GAME_MANAGER", "Countdown ended");
    if (isMain)
    {
        switch (selectedConfig)
        {
        case KOTH_CONFIG:
            LOG_INFO("GAME_MANAGER", "Starting KOTH as MASTER");
            kothServer = new KOTHServer(eventBus, hardwareManager, networkManager, kothConfig);
            kothServer->startModeTask("KOTH-Server", 1, 16384);
            break;
        case FLAG_CONFIG:
            LOG_INFO("GAME_MANAGER", "Starting FLAG as MASTER");
            flagServer = new FLAGServer(eventBus, hardwareManager, networkManager, flagConfig);
            flagServer->startModeTask("FLAG-Server", 1, 16384);
        default:
            LOG_INFO("GAME_MANAGER", "Nothing was selected aborting");
            break;
        }
    }
    if (!isMain)
    {
#if NODE_TYPE == CAPTURE_POINT
        LOG_INFO("GAME_MANAGER", "Starting as CAPTURE POINT");
        // startAfterCountdownTask(10); //i dont think it wil be needed any more
#else
        LOG_INFO("GAME_MANAGER", "Not starting client because this is not a capture point node and not main");
#endif
    }
    onGameStarted();
}

void GameManager::onGameStarted()
{
#if NODE_TYPE == HEADLESS
    LOG_INFO("GAME_MANAGER", "Received game start event but this is a headless node, ignoring");
    return;
#endif
#if NODE_TYPE == INFORMATION
    LOG_INFO("GAME_MANAGER", "Starting as INFORMATION NODE");
    // check if info node exists before creating new one
    if (infoNode)
    {
        LOG_WARN("GAME_MANAGER", "Information node already exists, not creating another one");
        return;
    }
    infoNode = new InformationModeComp(eventBus, hardwareManager, networkManager, kothConfig);
    infoNode->start();
    return;
#endif
    switch (selectedConfig)
    {
    case KOTH_CONFIG:
        LOG_INFO("GAME_MANAGER", "Starting as CLIENT");
        kothClient = new KOTHClient(eventBus, hardwareManager, networkManager, LORA_ADDRESS, kothConfig);
        kothClient->start();
        break;
    case FLAG_CONFIG:
        LOG_INFO("GAME_MANAGER", "Starting as CLIENT");
        flagClient = new FLAGClient(eventBus, hardwareManager, networkManager, LORA_ADDRESS, flagConfig);
        flagClient->start();
    default:
        LOG_INFO("GAME_MANAGER", "Nothing was selected aborting");
        break;
    }
}

GameManager::GameManager(EventBus *eb, HardwareManager *hw, NetworkManager *net)
    : eventBus(eb), hardwareManager(hw), networkManager(net)
{
    eventBus->subscribe(NETWORK_DISCOVER, [this](Event e)
                        { onDiscovered(e); }); // show who we are connected to
    eventBus->subscribe(SEARCH, [this](Event e)
                        { onDiscoverRequest(e); }); // broadcast discover request
    eventBus->subscribe(NETWROK_REPORT, [this](Event e)
                        { onNewNode(e); }); // add new node to the list when discovered
    // this also wont be missed
    // eventBus->subscribe(GAME_STARTED, [this](Event e)
    //                     { onGameStarted(); });
    eventBus->subscribe(KOTH_CONF_UPDATED, [this](Event e)
                        { onConfigKothFromMaster(e); });
    eventBus->subscribe(KOTH_CONFIG, [this](Event e)
                        { onConfKoth(e); });
    eventBus->subscribe(FLAG_CONFIG, [this](Event e)
                        { onConfigureFlag(e); });
    eventBus->subscribe(GAME_REQUEST_START_CONF, [this](Event e)
                        { onGameStartconfRequest(e); });
}

GameManager::~GameManager()
{
}

void GameManager::onNewNode(Event e)
{
    // why i added this and what it does?
    //  if (e.data2 == INFORMATION || e.data2 == CAPTURE_POINT)
    //  {
    //      return;
    //  }
    if (!kothConfig.hasNode(e.data1))
    {
        kothConfig.addNode(e.data1, e.data2);
        LOG_INFO("GAME_MANAGER", "New node added: %d, type: %d", e.data1, e.data2);
        String nodes = "";
        for (int i = 0; i < kothConfig.nodeCount; i++)
        {
            nodes += "N" + String(kothConfig.NodeStates[i].nodeId);
        }
        LcdDisplayMessage dsp{};
        dsp.setLine(0, "REMOTE NODES :");
        dsp.setLine(1, nodes.c_str());
        hardwareManager->lcd.displayText(dsp);
    }
    else
    {
        LOG_WARN("GAME_MANAGER", "Node %d already exists", e.data1);
    }
}

void GameManager::onDiscoverRequest(Event e)
{
    String msg = Protocol::buildDiscoverRequest();
    networkManager->broadcast(msg);
}

void GameManager::onDiscovered(Event e)
{
    String msg = "CONNECTED: " + String(e.data1);
    hardwareManager->buzzer.beep(200, 2, 200);
    LcdDisplayMessage dsp{};
    dsp.setLine(0, msg.c_str());
    dsp.clearLine(1);

    // synchronize time
    int mainTime = e.data2;
    int localTime = millis() / 1000;
    syncTimeDelta = mainTime - localTime;
    LOG_DEBUG("GAME MANAGER", "Calculated mainTime %d localTime %d syncTimeDelta %d", mainTime, localTime, syncTimeDelta);

    hardwareManager->lcd.displayText(dsp);
    if (networkManager)
    {
#if NODE_TYPE == FORWARDER
        LOG_ERROR("GAME_MANAGER", "Received discovery event but this is a forwarder node");
        return;
#endif
        networkManager->setAsClient(e.data1);
        String response = Protocol::buildDiscoverResponse(LORA_ADDRESS, NODE_TYPE);
        LOG_INFO("GAME_MANAGER", "Responding with: %s", response.c_str());
        vTaskDelay(pdMS_TO_TICKS(200) * LORA_ADDRESS); // small delay times node id to avoid collisons
        networkManager->sendToMain(response);
    }
}

void GameManager::onGameStartconfRequest(Event e)
{
#if NODE_TYPE == HEADLESS
    return;
#endif
    if (!isMain)
    {
#if NODE_TYPE == CAPTURE_POINT || NODE_TYPE == INFORMATION
        LOG_INFO("GAME_MANAGER", "Trying to connect to existing game");
        if (e.data1 == LORA_ADDRESS)
        {
            LOG_INFO("GAME_MANAGER", "Connecting to existing game");
            String msg = Protocol::buildConfRequest(LORA_ADDRESS, (NODE_TYPE == INFORMATION) );
            networkManager->broadcast(msg);
        }

#else
        LOG_INFO("GAME_MANAGER", "Received game start request but this isnt capture node, ignoring");
        return;
#endif
    }
}

void GameManager::update()
{
    if (kothClient)
    {
        kothClient->update();
        if (kothClient->getDeleteThis())
        {
            delete kothClient;
            kothClient = nullptr;
            hardwareManager->ledBlueButton.off();
            hardwareManager->ledYellowButton.off();
        }
    }
    if (flagClient)
    {
        flagClient->update();
        if (flagClient->getDeleteThis())
        {
            delete flagClient;
            flagClient = nullptr;
            hardwareManager->ledBlueButton.off();
            hardwareManager->ledYellowButton.off();
        }
    }
    if (infoNode)
    {
        infoNode->update();
        if (infoNode->getDeleteThis())
        {
            delete infoNode;
            infoNode = nullptr;
        }
    }
}