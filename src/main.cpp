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

extern uint8_t myNodeId;

#if NODE_TYPE != FORWARDER
GameManager gameManager(&eventBus, &hardware, &network);
#endif

void powerResetCallback(Event e)
{
    if (e.data1)
        network.broadcastReset(); // tell the network first
    hardware.reboot();            // then go down yourself
}

void testCallback(Event e)
{
    if (e.data1 == 1)
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
#if SCREEN_TYPE == CHONKY_SCREEN
    hardware.lcd.begin(0x27, 20, 4);
#elif SCREEN_TYPE == SMOLL_SCREEN
    hardware.lcd.begin(0x27, 16, 2);
#elif SCREEN_TYPE == NONE_SCREEN
    // Do nothing
#else
#error "Unknown SCREEN_TYPE, please define it as CHONKY_SCREEN or SMOLL_SCREEN"
#endif

    hardware.lcd.displayText("      SPAS", 0);
    hardware.lcd.displayText("INITIALAZING", 1);
    vTaskDelay(500);
    ble.BleStart();
    vTaskDelay(500);
    LOG.setBLECallback([](const char *msg)
                       { ble.sendMessage(msg); });
    network.begin();
    // callbacks that i dont know what to do with
    eventBus.subscribe(POWER_RESET, powerResetCallback);
    eventBus.subscribe(TEST, testCallback);

    hardware.buzzer.createBeepTask();
    hardware.lcd.clearScreen();
    hardware.lcd.displayText("WAITING", 0);

#if NODE_TYPE == INFORMATION
    hardware.lcd.displayText("INF MODE", 1);
#endif
}

void loop()
{
    hardware.update();
    eventBus.processEvents();

#if NODE_TYPE != FORWARDER
    gameManager.update();
#endif

    vTaskDelay(10);
}