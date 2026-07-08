#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LEDLight
{
private:
    int pin;
    bool state;
    bool blinking = false;
    int blinkHz = 0;
    long lastBlink = 0;

public:
    LEDLight(int ledPin);

    void on();
    void off();
    void toggle();
    bool getState();
    void blink(int herz = 200);

    void blinkOff();

    void update();
};

#endif // LED_H