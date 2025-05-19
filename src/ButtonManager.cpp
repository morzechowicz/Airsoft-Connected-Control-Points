#include <ButtonManager.h>

ButtonManager::ButtonManager()
    : blueButton(2), yellowButton(3),
      changeButton(4), startButton(5)
{
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