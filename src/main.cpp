#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "BLE/BleSetup.h"
#include "GameManager.h"
#include "Config.h"
#include "../lib/Logging/LogManager.h"

EventBus eventBus;
ConfigurationHandler confHandler(eventBus);
HardwareManager hardware(&eventBus);
NetworkManager network(eventBus, confHandler);
BleSetup ble(eventBus, confHandler);

GameManager gameManager(&eventBus, &hardware, &network);


void powerResetCallback(Event e) {
    if (e.data1) network.broadcastReset();  // tell the network first
    hardware.reboot();                       // then go down yourself
}

void setup()
{
    LOG.begin(LOG_DEBUG, LOG_OUTPUT_SERIAL | LOG_OUTPUT_BLE);
    LOG.enableColors(false);
    LOG.setTimestamps(false);
    
    LOG_INFO("MAIN", "System starting...");
    
    vTaskDelay(200); // Wait for LOG to initialize
    #ifdef BIG_SCREEN
    hardware.lcd.begin(0x27, 16, 4);
    #else
    hardware.lcd.begin(0x27, 16, 2);
    #endif
    
    hardware.lcd.displayText("      SPAS", 0);
    hardware.lcd.displayText("INITIALAZING", 1);
    vTaskDelay(500);
    ble.BleStart();
    vTaskDelay(500);
    
    LOG.setBLECallback([](const char* msg) { ble.sendMessage(msg); });
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

    #ifdef INFORMATION_NODE
    hardware.lcd.displayText("INF MODE", 1);
    #endif
}

void loop()
{
    hardware.update();
    eventBus.processEvents();
    gameManager.update();

    vTaskDelay(10);
}