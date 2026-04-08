#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LEDLight
{
private:
    int pin;
    bool state;

public:
    LEDLight(int ledPin);

    void on();
    void off();
    void toggle();
    bool getState();
};

#endif // LED_H