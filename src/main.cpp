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

#if NODE_TYPE == CAPTURE_POINT || NODE_TYPE == INFORMATION
GameManager gameManager(&eventBus, &hardware, &network);
#endif

void powerResetCallback(Event e)
{
    if (e.data1)
        network.broadcastReset(); // tell the network first
    hardware.reboot();            // then go down yourself
}

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
        // create test task
        xTaskCreate(
            testRequestTask,            // Task function
            "TestRequestTask",          // Name of the task (for debugging)
            4096,                       // Stack size in bytes
            (void *)(intptr_t)fromNode, // Parameter to pass to the task
            1,                          // Task priority
            NULL                        // Task handle (not used)
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

void batVoltageTask(void *pvParameters);

void batVoltStart()
{
    LOG_INFO("MAIN", "Starting battery voltage task for ");
    // create test task
    xTaskCreate(
        batVoltageTask,   // Task function
        "batVoltageTask", // Name of the task (for debugging)
        4096,             // Stack size in bytes
        (void *)nullptr,  // Parameter to pass to the task
        1,                // Task priority
        NULL              // Task handle (not used)
    );
}
#if SX_CHIP_TYPE == HELTECSX1262
void batVoltageTask(void *pvParameters)
{
    while (true)
    {
        uint32_t raw = analogRead(VBAT_PIN);

        float voltage = (raw / 4095.0) * 3.3 * 5.19;
        LOG_DEBUG("MAIN", "Raw ADC: %d, Voltage: %.2f V", raw, voltage);
        vTaskDelay(pdMS_TO_TICKS(5000));
        hardware.oled.clear();
        hardware.oled.writeln("Battery:");
        hardware.oled.writeln((String(voltage, 2) + " V").c_str());
        hardware.oled.display();
        // Also pulse diode here why not
        if (digitalRead(STATUS_LED_PIN) == LOW)
        {
            digitalWrite(STATUS_LED_PIN, HIGH);
        }else{
            digitalWrite(STATUS_LED_PIN, LOW);
        }
    }
}
#endif
void setup()
{
    LOG.begin(LOG_DEBUG, LOG_OUTPUT_SERIAL);
    LOG.enableColors(false);
    LOG.setTimestamps(false);

    LOG_INFO("MAIN", "System starting...");

#if SX_CHIP_TYPE == HELTECSX1262
    pinMode(ADC_CTRL_PIN, OUTPUT);
    digitalWrite(ADC_CTRL_PIN, LOW);

    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);

    batVoltStart();
#endif

    vTaskDelay(200); // Wait for LOG to initialize
#if SCREEN_TYPE == LCD_CHONKY_SCREEN
    hardware.lcd.begin(0x27, 20, 4);
#elif SCREEN_TYPE == LCD_SMOLL_SCREEN
    hardware.lcd.begin(0x27, 16, 2);
#elif SCREEN_TYPE == LCD_SMOLL_SCREEN
    hardware.lcd.begin(0x27, 16, 2);
#elif SCREEN_TYPE == OLED_128x36_SCREEN
    hardware.oled.begin();
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