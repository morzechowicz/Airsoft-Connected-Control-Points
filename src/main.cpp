#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "BLE/BleServer.h"
#include <NimBLEDevice.h>
#include "GameComponents/KOTH/KOTHServer.h"
#include "GameComponents/KOTH/KOTHClient.h"
#include "GameComponents/KOTH/KOTHTypes.h"
#include "GameComponents/FLAG/FLAGServer.h"
#include "GameComponents/FLAG/FLAGClient.h"

EventBus eventBus;
ConfigurationHandler confHandler(eventBus);
HardwareManager hardware(&eventBus);
NetworkManager network(eventBus, confHandler);

KOTHConfig kothConfig;

KOTHServer *kothServer = nullptr;
KOTHClient *kothClient = nullptr;

FLAGConfig flagConfig;

FLAGServer *flagServer = nullptr;
FLAGClient *flagClient = nullptr;

#ifdef LORA_ADDRESS
uint8_t myNodeId = LORA_ADDRESS;
#else
#define LORA_ADDRESS 0x09
#pragma message "Compiling for uknown"
#endif

bool isMaster = false;
EventType selectedConfig = CONF;

BLEServer *pServer;
BLECharacteristic *pCharacteristic;

TaskHandle_t countdownHandler = nullptr;

void countdownTask(int time)
{
    int countdown = (int)time;
    while (countdown > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        countdown--;
        Serial.println(countdown);
        hardware.lcd.clearScreen();
        hardware.lcd.displayText("COUNTDOWN", 0);
        hardware.lcd.displayText(String(countdown).c_str(), 1);
    }
    Serial.println("Countdown ended");
    if (isMaster)
    {
        switch (selectedConfig)
        {
        case KOTH_CONFIG:
            Serial.println("Starting KOTH as MASTER");
            kothServer = new KOTHServer(&eventBus, &hardware, &network, kothConfig);
            kothServer->startModeTask("KOTH-Server", 1, 8192);
            break;
        case FLAG_CONFIG:
            Serial.println("Starting FLAG as MASTER");
            flagServer = new FLAGServer(&eventBus, &hardware, &network, flagConfig);
            flagServer->startModeTask("FLAG-Server", 1, 8192);
        default:
            Serial.println("Nothing was selected aborting");
            break;
        }
    }
}

void startCountdownTask(int countdown)
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

    // Create new task
    xTaskCreate(
        [](void *param)
        {
            Serial.println((int)param);
            int c = (int)param;
            countdownTask(c);

            countdownHandler = nullptr; // Clear handle BEFORE deleting
            vTaskDelete(NULL);          // Delete current task (self)
        },
        "CountdownTask",
        2048,
        (void *)countdown,
        1,
        &countdownHandler);

    Serial.print("Created countdown task: ");
    Serial.println((uint32_t)countdownHandler, HEX);
}

void newNodeCallback(Event e)
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

void gameStartedCallback(Event e)
{

    switch (selectedConfig)
    {
    case KOTH_CONFIG:
        Serial.println("Starting as CLIENT");
        kothClient = new KOTHClient(&eventBus, &hardware, &network, myNodeId, kothConfig);
        kothClient->start();
        break;
    case FLAG_CONFIG:
        Serial.println("Starting as CLIENT");
        flagClient = new FLAGClient(&eventBus, &hardware, &network, myNodeId, flagConfig);
        flagClient->start();
    default:
        Serial.println("Nothing was selected aborting");
        break;
    }
}

void configKothFromMasterCallback(Event e)
{
    kothConfig.maxPoints = e.data3;
    kothConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    kothConfig.captureTime = e.data4;
    int countdown = e.data1;
    selectedConfig = KOTH_CONFIG;
    startCountdownTask(countdown);
}

void confKothCallback(Event e)
{
    kothConfig.maxPoints = e.data3;
    kothConfig.gameDurationMinutes = e.data2;
    kothConfig.scoreIntervalMs = SCORING_INTERVAL_MS;
    kothConfig.captureTime = e.data4;
    Event newNode;
    newNode.data1 = LORA_ADDRESS;
    newNodeCallback(newNode);
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
        network.broadcast(configBroadcast);
    }

    startCountdownTask(countdown);
}

void discoverCallback(Event e)
{
    String msg = Protocol::buildDiscoverRequest();
    network.broadcast(msg);
}

void configureFlagCallback(Event e)
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

void powerResetCallback(Event e)
{
    if(e.data1)
    {
        Serial.println("Sending Restart order");
        String msg = Protocol::buildPowerResetMsg();
        network.broadcast(msg);
    }
    Serial.println("Restarting this device");
    vTaskDelay(2000);
    ESP.restart();
}

void setup()
{
    Serial.begin(115200);
    vTaskDelay(200); // Wait for Serial to initialize
    #ifdef BIG_SCREEN
    hardware.lcd.begin(0x27, 16, 4);
    #else
    hardware.lcd.begin(0x27, 16, 2);
    #endif
        
    hardware.lcd.displayText("      SPAS", 0);
    hardware.lcd.displayText("INITIALAZING", 1);
    vTaskDelay(200);
    // Initialize BLE
    Serial.println("Initializing BLE...");
    String deviceName = "LoRaCP_" + String(myNodeId);
    NimBLEDevice::init(deviceName.c_str());
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new BleServer());

    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristic (read/write)
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    pCharacteristic->setValue("Hello from ESP32 (NimBLE)");
    pCharacteristic->setCallbacks(new BleCallback(eventBus, confHandler));

    // Start the service
    pService->start();
    Serial.println("BLE Service started");
    vTaskDelay(200);
    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setName(deviceName.c_str());
    pAdvertising->start();
    Serial.println("BLE Ready! Waiting for connections...");

    vTaskDelay(200);
    network.begin();

    eventBus.subscribe(SEARCH, discoverCallback);
    eventBus.subscribe(NETWROK_REPORT, newNodeCallback);
    eventBus.subscribe(GAME_STARTED, gameStartedCallback);
    eventBus.subscribe(KOTH_CONF_UPDATED, configKothFromMasterCallback);
    eventBus.subscribe(KOTH_CONFIG, confKothCallback);
    eventBus.subscribe(FLAG_CONFIG, configureFlagCallback);
    eventBus.subscribe(POWER_RESET, powerResetCallback);

    hardware.buzzer.createBeepTask();
    hardware.lcd.clearScreen();
    hardware.lcd.displayText("WAITING", 0);
}

void loop()
{
    hardware.update();
    eventBus.processEvents();

    if (kothClient)
    {
        kothClient->update();
        if (kothClient->getDeleteThis())
        {
            kothClient->~KOTHClient();
            kothClient = nullptr;
            hardware.ledBlueButton.off();
            hardware.ledYellowButton.off();
        }
    }
    if (flagClient)
    {
        flagClient->update();
        if (flagClient->getDeleteThis())
        {
            flagClient->~FLAGClient();
            flagClient = nullptr;
            hardware.ledBlueButton.off();
            hardware.ledYellowButton.off();
        }
    }

    vTaskDelay(10);
}