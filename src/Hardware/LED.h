#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED
{
private:
    int pin;
    bool state;

public:
    LED(int ledPin);

    void on();
    void off();
    void toggle();
    bool getState();
};

#endif // LED_H