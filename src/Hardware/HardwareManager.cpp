#include "HardwareManager.h"

HardwareManager::HardwareManager(EventBus *eventBus) :
    buttonBlue(eventBus, BUTTON_BLUE_PIN),
    buttonYellow(eventBus, BUTTON_YELLOW_PIN),
    buttonSelect(eventBus, BUTTON_SELECT_PIN),
    buttonEnter(eventBus, BUTTON_ENTER_PIN),
    ledBlueButton(LED_BLUE_PIN),
    ledYellowButton(LED_YELLOW_PIN),
    buzzer(BUZZER_PIN, BUZZER_GENERATOR),
    lcd()
{

}

void HardwareManager::update()
{
    buttonBlue.update();
    buttonYellow.update();
    buttonSelect.update();
    buttonEnter.update();

}

void HardwareManager::reboot() {
    vTaskDelay(2000);
    ESP.restart();
}