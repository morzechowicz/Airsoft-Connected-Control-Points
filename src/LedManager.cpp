#include <LedManager.h>

// i have an idea for this const just not yet
LedManager::LedManager() {}
LedManager::LedManager(int blueLedPin, int yellowLedPin, int blinkingInterval)
    : blueLedPin(blueLedPin), yellowLedPin(yellowLedPin), blinkingInterval(blinkingInterval)
{
    pinMode(blueLedPin, OUTPUT);
    pinMode(yellowLedPin, OUTPUT);
}

void LedManager::start()
{
    ledClock.start();
    blueTime = blinkingInterval;
    yellowTime = blinkingInterval * 2;
}

void LedManager::stop()
{
    ledClock.stop();
    ledClock.reset();
}

void LedManager::blinkingBoth(bool alternate)
{
    if (!alternate)
    {
        if (ledClock.getElapsedTime() < blinkingInterval)
        {
            digitalWrite(blueLedPin, HIGH);
            digitalWrite(yellowLedPin, HIGH);
        }
        else if (ledClock.getElapsedTime() > blinkingInterval && ledClock.getElapsedTime() < blinkingInterval * 2)
        {
            digitalWrite(blueLedPin, LOW);
            digitalWrite(yellowLedPin, LOW);
        }
        else
        {
            ledClock.reset();
        }
    }
    else
    {

        if (ledClock.getElapsedTime() < blinkingInterval)
        {
            digitalWrite(blueLedPin, HIGH);
            digitalWrite(yellowLedPin, LOW);
        }
        else if (ledClock.getElapsedTime() > blinkingInterval && ledClock.getElapsedTime() < blinkingInterval * 2)
        {
            digitalWrite(blueLedPin, LOW);
            digitalWrite(yellowLedPin, HIGH);
        }
        else
        {
            ledClock.reset();
        }
    }
}

void LedManager::singleOn(int led)
{
    if (led == blueLedPin)
    {
        digitalWrite(blueLedPin, HIGH);
        digitalWrite(yellowLedPin, LOW);
    }
    else if (led == yellowLedPin)
    {
        digitalWrite(blueLedPin, LOW);
        digitalWrite(yellowLedPin, HIGH);
    }
}

void LedManager::bothOff()
{
    digitalWrite(blueLedPin, LOW);
    digitalWrite(yellowLedPin, LOW);
}

void LedManager::singleBlink(int led)
{
    if (led == blueLedPin)
    {
        if (ledClock.getElapsedTime() < blinkingInterval)
        {
            digitalWrite(blueLedPin, HIGH);
        }
        else if (ledClock.getElapsedTime() > blinkingInterval && ledClock.getElapsedTime() < blinkingInterval * 2)
        {
            digitalWrite(blueLedPin, LOW);
        }
        else
        {
            ledClock.reset();
        }
    }
    else if (led == yellowLedPin)
    {
        if (ledClock.getElapsedTime() < blinkingInterval)
        {
            digitalWrite(yellowLedPin, HIGH);
        }
        else if (ledClock.getElapsedTime() > blinkingInterval && ledClock.getElapsedTime() < blinkingInterval * 2)
        {
            digitalWrite(yellowLedPin, LOW);
        }
        else
        {
            ledClock.reset();
        }
    }
}

void LedManager::singleOn(int led)
{
    if (led == blueLedPin)
    {
        digitalWrite(blueLedPin, HIGH);
        digitalWrite(yellowLedPin, LOW);
    }
    else if (led == yellowLedPin)
    {
        digitalWrite(blueLedPin, LOW);
        digitalWrite(yellowLedPin, HIGH);
    }
}

void LedManager::singleOff(int led)
{
    if (led == blueLedPin)
    {
        digitalWrite(blueLedPin, LOW);
    }
    else if (led == yellowLedPin)
    {
        digitalWrite(yellowLedPin, LOW);
    }
}