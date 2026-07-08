#include "HardwareManager.h"

HardwareManager::HardwareManager(EventBus *eventBus) : buttonBlue(eventBus, BUTTON_BLUE_PIN),
                                                       buttonYellow(eventBus, BUTTON_YELLOW_PIN),
                                                       buttonSelect(eventBus, BUTTON_SELECT_PIN),
                                                       buttonEnter(eventBus, BUTTON_ENTER_PIN),
                                                       ledBlueButton(LED_BLUE_PIN),
                                                       ledYellowButton(LED_YELLOW_PIN),
                                                       buzzer(BUZZER_PIN, BUZZER_GENERATOR),
                                                       lcd()
{
#if SCREEN_TYPE == OLED_128x36_SCREEN
    pinMode(PWR_OLED, OUTPUT);
    digitalWrite(PWR_OLED, LOW);
    vTaskDelay(1000);

    pinMode(RST_OLED, OUTPUT);
    digitalWrite(RST_OLED, LOW);
    vTaskDelay(100);
    digitalWrite(RST_OLED, HIGH);
    vTaskDelay(100);

#endif
    Wire.begin(SDA_PIN, SCL_PIN);
}

void HardwareManager::update()
{
    buttonBlue.update();
    buttonYellow.update();
    buttonSelect.update();
    buttonEnter.update();
    ledBlueButton.update();
    ledYellowButton.update();
}

void HardwareManager::reboot()
{
    vTaskDelay(2000);
    ESP.restart();
}
