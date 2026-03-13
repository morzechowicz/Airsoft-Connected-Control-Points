#include "GameManager.h"

GameManager* GameManager::instance = nullptr;

void GameManager::countdownTask(int time)
{
    int countdown = (int)time;
    while (countdown > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        countdown--;
        Serial.println(countdown);
        hardwareManager->lcd.clearScreen();
        hardwareManager->lcd.displayText("COUNTDOWN", 0);
        hardwareManager->lcd.displayText(String(countdown).c_str(), 1);
    }
    Serial.println("Countdown ended");
    if (isMaster)
    {
        switch (selectedConfig)
        {
        case KOTH_CONFIG:
            Serial.println("Starting KOTH as MASTER");
            kothServer = new KOTHServer(eventBus, hardwareManager, networkManager, kothConfig);
            kothServer->startModeTask("KOTH-Server", 1, 8192);
            break;
        case FLAG_CONFIG:
            Serial.println("Starting FLAG as MASTER");
            flagServer = new FLAGServer(eventBus, hardwareManager, networkManager, flagConfig);
            flagServer->startModeTask("FLAG-Server", 1, 8192);
        default:
            Serial.println("Nothing was selected aborting");
            break;
        }
    }
}

void GameManager::startCountdownTask(int countdown)
{
    Serial.println("Starting countdown task");
    if (kothClient != nullptr || flagClient != nullptr)
    {
        Serial.println("already running aborting");
        return;
    }
    // Kill existing task if running
    if (countdownHandler != nullptr)
    {
        TaskHandle_t oldHandle = countdownHandler;
        countdownHandler = nullptr; // Clear first!

        if (eTaskGetState(oldHandle) != eDeleted)
        {
            Serial.print("Deleting old task: ");
            Serial.println((uint32_t)oldHandle, HEX);
            vTaskDelete(oldHandle);
            vTaskDelay(pdMS_TO_TICKS(100)); // Give scheduler time to clean up
        }
    }
    struct TaskParams { GameManager* mgr; int countdown; };

    auto* params = new TaskParams{ this, countdown };
    // Create new task
    xTaskCreate(
        [](void* param) {
            auto* p = static_cast<TaskParams*>(param);
            p->mgr->countdownTask(p->countdown);
            delete p;
            vTaskDelete(NULL);
        },
        "CountdownTask", 2048, params, 1, &countdownHandler);

    Serial.print("Created countdown task: ");
    Serial.println((uint32_t)countdownHandler, HEX);
}

GameManager::~GameManager()
{
}

void GameManager::onNewNode(Event e)
{
    if (!kothConfig.hasNode(e.data1))
    {
        kothConfig.addNode(e.data1);
    }
    else
    {
        Serial.println("Node already exists");
    }
}

void GameManager::onGameStarted(Event e)
{

    switch (selectedConfig)
    {
    case KOTH_CONFIG:
        Serial.println("Starting as CLIENT");
        kothClient = new KOTHClient(eventBus, hardwareManager, networkManager, myNodeId, kothConfig);
        kothClient->start();
        break;
    case FLAG_CONFIG:
        Serial.println("Starting as CLIENT");
        flagClient = new FLAGClient(eventBus, hardwareManager, networkManager, myNodeId, flagConfig);
        flagClient->start();
    default:
        Serial.println("Nothing was selected aborting");
        break;
    }
}

void GameManager::onConfigKothFromMaster(Event e)
{
    kothConfig.maxPoints = e.data3;
    kothConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    kothConfig.captureTime = e.data4;
    int countdown = e.data1;
    selectedConfig = KOTH_CONFIG;
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
    isMaster = true;
    int countdown = e.data1;
    selectedConfig = KOTH_CONFIG;

    Serial.print("maxPoints");
    Serial.println(kothConfig.maxPoints);
    Serial.print("gameDurationMinutes");
    Serial.println(kothConfig.gameDurationMinutes);
    Serial.print("scoreIntervalMs");
    Serial.println(kothConfig.scoreIntervalMs);
    Serial.print("captureTimeMs");
    Serial.println(kothConfig.captureTime);
    Serial.println("Countdown started");

    if (!kothConfig.singleNodeMode)
    {
        String configBroadcast = Protocol::buildKothConfigUpdated(kothConfig.maxPoints, countdown, kothConfig.captureTime, kothConfig.gameDurationMinutes);
        networkManager->broadcast(configBroadcast);
    }

    startCountdownTask(countdown);
}

void GameManager::onDiscover(Event e)
{
    String msg = Protocol::buildDiscoverRequest();
    networkManager->broadcast(msg);
}

void GameManager::onConfigureFlag(Event e)
{

    flagConfig.maxPoints = e.data3;
    flagConfig.maxTime = e.data2;
    flagConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    flagConfig.captureTime = e.data4;
    flagConfig.initTeamCount = e.data5;

    Event newNode;
    newNode.data1 = LORA_ADDRESS;

    isMaster = true;
    int countdown = e.data1;
    selectedConfig = FLAG_CONFIG;

    Serial.print("maxPoints");
    Serial.println(flagConfig.maxPoints);
    Serial.print("gameDurationMinutes");
    Serial.println(flagConfig.maxTime);
    Serial.print("scoreIntervalMs");
    Serial.println(flagConfig.scoreIntervalMs);
    Serial.print("captureTimeMs");
    Serial.println(flagConfig.captureTime);
    Serial.println("Countdown started");

    startCountdownTask(countdown);
}


void GameManager::update() {
    if (kothClient) {
        kothClient->update();
        if (kothClient->getDeleteThis()) {
            delete kothClient;        
            kothClient = nullptr;
            hardwareManager->ledBlueButton.off();
            hardwareManager->ledYellowButton.off();
        }
    }
    if (flagClient) {
        flagClient->update();
        if (flagClient->getDeleteThis()) {
            delete flagClient;
            flagClient = nullptr;
            hardwareManager->ledBlueButton.off();
            hardwareManager->ledYellowButton.off();
        }
    }
}