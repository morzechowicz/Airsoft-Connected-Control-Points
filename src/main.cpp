#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "BLE/BleSetup.h"
#include "GameManager.h"
#include "Config.h"
#include "../lib/Logging/LogManager.h"

EventBus eventBus;
MessageHandler msgHandler(eventBus);
HardwareManager hardware(&eventBus);
NetworkManager network(eventBus, msgHandler);
BleSetup ble(eventBus, msgHandler);

GameManager gameManager(&eventBus, &hardware, &network);

void powerResetCallback(Event e)
{
    if (e.data1)
        network.broadcastReset(); // tell the network first
    hardware.reboot();            // then go down yourself
}

void testCallback(Event e)
{
    if(e.data1 == 1)
    {
        LOG_INFO("MAIN", "Received TEST event, broadcasting test message");
        String msg = Protocol::buildDebugTestMessage();
        network.broadcast(msg);
    }
    if (e.data1 == 0)
    {
        LOG_INFO("MAIN", "Received TEST event with data 0, not broadcasting");
        hardware.handleTestRequest(e);
    }
    
}

void setup()
{
    LOG.begin(LOG_DEBUG, LOG_OUTPUT_SERIAL);
    LOG.enableColors(false);
    LOG.setTimestamps(false);

    LOG_INFO("MAIN", "System starting...");

    vTaskDelay(200); // Wait for LOG to initialize
#ifdef BIG_SCREEN
    hardware.lcd.begin(0x27, 20, 4);
#else
    hardware.lcd.begin(0x27, 16, 2);
#endif

    hardware.lcd.displayText("      SPAS", 0);
    hardware.lcd.displayText("INITIALAZING", 1);
    vTaskDelay(500);
    ble.BleStart();
    vTaskDelay(500);
    LOG.setBLECallback([](const char *msg)
                       { ble.sendMessage(msg); });
    network.begin();
    //callbacks that i dont know what to do with
    eventBus.subscribe(POWER_RESET, powerResetCallback);
    eventBus.subscribe(TEST, testCallback);

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