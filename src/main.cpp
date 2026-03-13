#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "BLE/BleServer.h"
#include <NimBLEDevice.h>
#include "GameManager.h"
#include "Config.h"

EventBus eventBus;
ConfigurationHandler confHandler(eventBus);
HardwareManager hardware(&eventBus);
NetworkManager network(eventBus, confHandler);

GameManager gameManager(&eventBus, &hardware, &network);

BLEServer *pServer;
BLECharacteristic *pCharacteristic;

void powerResetCallback(Event e)
{
    if (e.data1)
    {
        Serial.println("Sending Restart order");
        String msg = Protocol::buildPowerResetMsg();
        Serial.println(msg);
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

    GameManager::instance = &gameManager;

    eventBus.subscribe(SEARCH, [](Event e)
                       { GameManager::instance->onDiscover(e); });
    eventBus.subscribe(NETWROK_REPORT, [](Event e)
                       { GameManager::instance->onNewNode(e); });
    eventBus.subscribe(GAME_STARTED, [](Event e)
                       { GameManager::instance->onGameStarted(e); });
    eventBus.subscribe(KOTH_CONF_UPDATED, [](Event e)
                       { GameManager::instance->onConfigKothFromMaster(e); });
    eventBus.subscribe(KOTH_CONFIG, [](Event e)
                       { GameManager::instance->onConfKoth(e); });
    eventBus.subscribe(FLAG_CONFIG, [](Event e)
                       { GameManager::instance->onConfigureFlag(e); });
    eventBus.subscribe(POWER_RESET, powerResetCallback);

    hardware.buzzer.createBeepTask();
    hardware.lcd.clearScreen();
    hardware.lcd.displayText("WAITING", 0);
}

void loop()
{
    hardware.update();
    eventBus.processEvents();
    gameManager.update();

    vTaskDelay(10);
}