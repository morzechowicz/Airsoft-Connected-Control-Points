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
}

void HardwareManager::update()
{
    buttonBlue.update();
    buttonYellow.update();
    buttonSelect.update();
    buttonEnter.update();
}

void HardwareManager::reboot()
{
    vTaskDelay(2000);
    ESP.restart();
}

void HardwareManager::handleTestRequest(Event e)
{
    // Audio visual connection test
    // Tests if node have connection by beeping.

    lcd.clearScreen();
    lcd.displayText("TESTING", 0);
    lcd.displayText("CONNECTION OK", 1);
    int waitBeforeBeep = 1000 * myNodeId;
    LOG_INFO("HARDWARE_MANAGER", "beeping in %d ms", waitBeforeBeep);
    vTaskDelay(waitBeforeBeep);
    buzzer.beep(200, 3, 200);
    LOG_INFO("HARDWARE_MANAGER", "beeped");
    vTaskDelay(2000);
}