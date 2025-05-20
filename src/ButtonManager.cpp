#include <ButtonManager.h>

ButtonManager::ButtonManager()
    : blueButton(4), yellowButton(0),
      changeButton(2), startButton(15)
{
    Serial.println("ButtonManager initialized");
}

void ButtonManager::begin()
{
    blueButton.setDebounceTime(50);
    yellowButton.setDebounceTime(50);
    changeButton.setDebounceTime(50);
    startButton.setDebounceTime(50);
}

void ButtonManager::update()
{
    blueButton.loop();
    yellowButton.loop();
    changeButton.loop();
    startButton.loop();
}