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
    Event newNode;
    newNode.data1 = LORA_ADDRESS;
    // add itself to the table
    onNewNode(newNode);
    isMain = true;
    int countdown = e.data1;
    selectedConfig = KOTH_CONFIG;

    LOG_INFO("GAME_MANAGER", "Received KOTH configuration: maxPoints=%d, gameDurationMinutes=%d, captureTime=%d, scoreIntervalMs=%d",
             kothConfig.maxPoints, kothConfig.gameDurationMinutes, kothConfig.captureTime, kothConfig.scoreIntervalMs);
    LOG_INFO("GAME_MANAGER", "Countdown started");

    String configBroadcast = Protocol::buildKothConfigClient(kothConfig.maxPoints, countdown, kothConfig.captureTime, kothConfig.gameDurationMinutes);
    networkManager->broadcast(configBroadcast);

    startCountdownTask(countdown);
}

void GameManager::onConfigKothFromMaster(Event e)
{
    // dont start if main node is not set
    if (networkManager->isMainNodeSet())
    {
        LOG_INFO("GAME_MANAGER", "Main node is set, starting client");
    }
    else
    {
        LOG_ERROR("GAME_MANAGER", "Main node is not set, cannot start client");
        return;
    }
    kothConfig.maxPoints = e.data3;
    kothConfig.gameDurationMinutes = e.data2;
    kothConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    kothConfig.captureTime = e.data4;
    int countdown = e.data1;
    selectedConfig = KOTH_CONFIG;
    startCountdownTask(countdown);
}

void GameManager::startCountdownTask(int countdown)
{
    LOG_DEBUG("GAME_MANAGER", "Starting countdown task");
    if (kothClient != nullptr || flagClient != nullptr)
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
        "CountdownTask", 2048, params, 1, &countdownHandler);

    LOG_DEBUG("GAME_MANAGER", "Created countdown task: %p", (void *)countdownHandler);
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
    hardwareManager->lcd.clearScreen();
    hardwareManager->lcd.displayText("NET ERROR", 0);
    hardwareManager->lcd.displayText("CALL GAME ORG", 1);
}

void GameManager::countdownTask(int time)
{
    int countdown = (int)time;
    while (countdown > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        countdown--;
        LOG_DEBUG("GAME_MANAGER", "Countdown: %d", countdown);
        hardwareManager->lcd.clearScreen();
        hardwareManager->lcd.displayText("COUNTDOWN", 0);
        hardwareManager->lcd.displayText(String(countdown).c_str(), 1);
    }
    LOG_INFO("GAME_MANAGER", "Countdown ended");
    if (isMain)
    {
        switch (selectedConfig)
        {
        case KOTH_CONFIG:
            LOG_INFO("GAME_MANAGER", "Starting KOTH as MASTER");
            kothServer = new KOTHServer(eventBus, hardwareManager, networkManager, kothConfig);
            kothServer->startModeTask("KOTH-Server", 1, 8192);
            break;
        case FLAG_CONFIG:
            LOG_INFO("GAME_MANAGER", "Starting FLAG as MASTER");
            flagServer = new FLAGServer(eventBus, hardwareManager, networkManager, flagConfig);
            flagServer->startModeTask("FLAG-Server", 1, 8192);
        default:
            LOG_INFO("GAME_MANAGER", "Nothing was selected aborting");
            break;
        }
    }
    if (!isMain)
    {
#if NODE_TYPE == CAPTURE_POINT
        LOG_INFO("GAME_MANAGER", "Starting as CAPTURE POINT");
        startAfterCountdownTask(10);
#else
        LOG_INFO("GAME_MANAGER", "Not starting client because this is an information node");
#endif
    }
}

void GameManager::onGameStarted(Event e)
{
#if NODE_TYPE == INFORMATION
    LOG_INFO("GAME_MANAGER", "Starting as INFORMATION NODE");
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
    eventBus->subscribe(GAME_STARTED, [this](Event e)
                        { onGameStarted(e); });
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
    if (!kothConfig.hasNode(e.data1))
    {
        kothConfig.addNode(e.data1);
        eventBus->publish(DEBUG, SEARCH, "Node" + String(e.data1) + " added \n");
        String nodes = "";
        for (int i = 0; i < kothConfig.nodeCount; i++)
        {
            nodes += "N" + String(kothConfig.nodeIds[i]);
        }

        hardwareManager->lcd.clearScreen();
        hardwareManager->lcd.displayText("REMOTE NODES :", 0);
        hardwareManager->lcd.displayText(nodes.c_str(), 1);
    }
    else
    {
        LOG_WARN("GAME_MANAGER", "Node already exists");
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
    hardwareManager->lcd.clearScreen();
    hardwareManager->lcd.displayText(msg.c_str(), 1);
}

void GameManager::onGameStartconfRequest(Event e)
{
    if (!isMain)
    {
#if NODE_TYPE == INFORMATION
        LOG_INFO("GAME_MANAGER", "Received game start request but this is an information node, ignoring");
        return;
#else
        LOG_INFO("GAME_MANAGER", "Trying to connect to existing game");
#endif
        if (e.data1 < 0x02)
        {
            LOG_ERROR("GAME_MANAGER", "Connecting to existing game");
            String msg = Protocol::buildConfRequest(LORA_ADDRESS);
            networkManager->broadcast(msg);
        }
        else
        {
            LOG_INFO("GAME_MANAGER", "Received game start request from node %d", e.data1);
            startAfterCountdownTask(10);
        }
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
            hardwareManager->ledBlueButton.off();
            hardwareManager->ledYellowButton.off();
        }
    }
}