#include "LedManager.h"

LedManager::LedManager() {}

void LedManager::addLed(Led led) {
    for (int i = 0; i < 20; i++) {
        if (leds[i].getBlinkInterval() == -1) { // Find an unused slot
            leds[i] = led;
            return;
        }
    }
    Serial.println("LedManager: No space to add more LEDs!");
}

void LedManager::removeLed(Led led) {
    for (int i = 0; i < 20; i++) {
        if (leds[i].getBlinkInterval() != -1 && leds[i].isOn() == led.isOn()) {
            leds[i] = Led(-1); // Reset the slot to an unused state
            return;
        }
    }
    Serial.println("LedManager: LED not found!");
}

Led LedManager::getLedByPin(int pin) {
    for (int i = 0; i < 20; i++) {
        if (leds[i].getBlinkInterval() != -1 && leds[i].isOn() == pin) {
            return leds[i];
        }
    }
    Serial.println("LedManager: LED not found!");
    return Led(-1); // Return an invalid LED if not found
}

void LedManager::ledOff(int pin) {
    Led led = getLedByPin(pin);
    if (led.isOn()) {
        led.off();
    }
}

void LedManager::ledOn(int pin) {
    Led led = getLedByPin(pin);
    if (!led.isOn()) {
        led.on();
    }
}

void LedManager::ledBlinking(int pin, int speed) {
    Led led = getLedByPin(pin);
    if (led.isOn()) {
        led.setBlinkInterval(speed);
        led.blinking();
    }
}