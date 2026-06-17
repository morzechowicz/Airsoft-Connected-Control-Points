#include <Arduino.h>
#include "EventBus.h"
#include "Hardware/HardwareManager.h"
#include "Network/NetworkManager.h"
#include "BLE/BleSetup.h"
#include "GameManager.h"
#include "Config.h"
#include "../lib/Logging/LogManager.h"
#include "Network/ConnectionTester.h"
#include "Hardware/AudioVisualDemo.h"

EventBus eventBus;
MessageHandler msgHandler(eventBus);
HardwareManager hardware(&eventBus);
NetworkManager network(eventBus, msgHandler);
ConnectionTester connectionTester(&network, &hardware, &eventBus);
BleSetup ble(eventBus, msgHandler);
AudioVisualDemo demo(&hardware, &eventBus);

extern uint8_t myNodeId;

#if NODE_TYPE == CAPTURE_POINT || NODE_TYPE == INFORMATION || NODE_TYPE == HEADLESS
GameManager gameManager(&eventBus, &hardware, &network);
#endif

void powerResetCallback(Event e)
{
    if (e.data1)
        network.broadcastReset(); // tell the network first
    hardware.reboot();            // then go down yourself
}

void testRequestTask(void *pvParameters);


void batVoltageTask(void *pvParameters);

void batVoltStart()
{
    LOG_DEBUG("MAIN", "Starting battery voltage task for ");
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
    #ifdef LOG_LEVEL
    LOG.begin(LOG_LEVEL, LOG_OUTPUT_SERIAL | LOG_OUTPUT_BLE);
    
    #endif
    LOG.enableColors(true);
    LOG.setTimestamps(true);

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

    hardware.lcd.displayLogo();
    
    vTaskDelay(500);
    ble.BleStart();
    vTaskDelay(500);
    LOG.setBLECallback([](const char *msg)
                       { ble.sendMessage(msg); });
    LOG.createBleLogTask();
    network.begin();
    // callbacks that i dont know what to do with
    eventBus.subscribe(POWER_RESET, powerResetCallback);
    
#if SCREEN_TYPE == OLED_128x36_SCREEN
    hardware.oled.writeln("STATUS");
    hardware.oled.writeln("READY");
#endif
    LcdDisplayMessage dsp {};
    
#if SCREEN_TYPE == LCD_CHONKY_SCREEN || SCREEN_TYPE == LCD_SMOLL_SCREEN
    dsp.setLine(0,"WAITING");
    dsp.clearLine(1);
#if NODE_TYPE == INFORMATION
    dsp.setLine(1,"INF MODE");
    dsp.clearLine(2);
    dsp.clearLine(3);
#endif
#endif

    hardware.lcd.displayText(dsp);
    demo.init();
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