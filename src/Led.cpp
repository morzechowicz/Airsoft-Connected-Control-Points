#include "Led.h"
#include <Arduino.h>

Led::Led()
{
}

Led::Led(int pin) : pin(pin), state(false), blink(false), blinkInterval(500)
{
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void Led::on() {
    state = true;
    blink = false; // Stop blinking if the LED is turned on
    digitalWrite(pin, HIGH);
}

void Led::off() {
    state = false;
    blink = false; // Stop blinking if the LED is turned off
    digitalWrite(pin, LOW);
}

void Led::toggle() {
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
}

bool Led::isOn() const {
    return state;
}

void Led::blinking() {
    blink = true;
    blinkClock.reset(); // Reset the clock when blinking starts
    blinkClock.start();
}

int Led::getBlinkInterval() {
    return blinkInterval;
}

void Led::setBlinkInterval(int interval) {
    blinkInterval = interval;
}

// This method should be called periodically in the main loop to handle blinking
void Led::update() {
    if (blink) {
        if (blinkClock.getElapsedTime() >= blinkInterval) {
            toggle(); 
            blinkClock.reset(); 
        }
    }
}