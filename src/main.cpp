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

#if NODE_TYPE != BLEToLoRa
GameManager gameManager(&eventBus, &hardware, &network);
#endif

void powerResetCallback(Event e)
{
    if (e.data1)
        network.broadcastReset(); // tell the network first
    hardware.reboot();            // then go down yourself
}
\
void testRequestTask(void *pvParameters);

void testCallback(Event e)
{
    int fromNode = e.data1;
    if (e.data2 == 1)
    {
        LOG_INFO("MAIN", "Received TEST event, broadcasting test message");
        String msg = Protocol::buildDebugTestMessage();
        network.broadcast(msg);
    }
    if (e.data2 == 0)
    {
        LOG_INFO("MAIN", "Received TEST event with data 0, not broadcasting");
        //create test task
        xTaskCreate(
            testRequestTask,   // Task function
            "TestRequestTask", // Name of the task (for debugging)
            4096,             // Stack size in bytes
            (void *)(intptr_t)fromNode,             // Parameter to pass to the task
            1,                // Task priority
            NULL              // Task handle (not used)
        );
    }
}

void testRequestTask(void *pvParameters)
{
    // Audio visual connection test
    // Tests if node have connection by beeping.
    int responseId = (int)(intptr_t)pvParameters;
    hardware.lcd.clearScreen();
    hardware.lcd.displayText("TESTING", 0);
    hardware.lcd.displayText("CONNECTION OK", 1);
    int waitBeforeBeep = 1000 * LORA_ADDRESS;
    LOG_INFO("HARDWARE_MANAGER", "beeping in %d ms", waitBeforeBeep);
    vTaskDelay(waitBeforeBeep);
    // String msg = Protocol::buildDebugResponseMessage();
    // network.sendTo(responseId,msg);
    hardware.buzzer.beep(200, 3, 200);
    hardware.ledBlueButton.on();
    vTaskDelay(200);
    hardware.ledBlueButton.off();
    vTaskDelay(200);
    hardware.ledYellowButton.on();
    vTaskDelay(200);
    hardware.ledYellowButton.off();

    LOG_INFO("HARDWARE_MANAGER", "beeped");
    vTaskDelete(NULL); // Delete the task when done
}

void setup()
{
    LOG.begin(LOG_DEBUG, LOG_OUTPUT_SERIAL);
    LOG.enableColors(false);
    LOG.setTimestamps(false);

    LOG_INFO("MAIN", "System starting...");

    vTaskDelay(200); // Wait for LOG to initialize
#if SCREEN_TYPE == LCD_CHONKY_SCREEN
    hardware.lcd.begin(0x27, 20, 4);
#elif SCREEN_TYPE == LCD_SMOLL_SCREEN
    hardware.lcd.begin(0x27, 16, 2);
#elif SCREEN_TYPE == NONE_SCREEN
    // Do nothing
#else
#error "Unknown SCREEN_TYPE, please define it as LCD_CHONKY_SCREEN or LCD_SMOLL_SCREEN"
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
    eventBus.processEvents();
    hardware.update();

#if NODE_TYPE == CAPTURE_POINT || NODE_TYPE == INFORMATION
    gameManager.update();
#endif

    vTaskDelay(10);
}