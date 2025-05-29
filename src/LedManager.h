#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Clocker.h>
#include <Led.h>

class LedManager
{
public:
    LedManager();
    
    void addLed(Led led);
    void removeLed(Led led);
    Led getLedByPin(int pin);

    void ledOff(int led);
    void ledOn(int led);
    void ledBlinking(int led,int speed);



private:
    Led leds[20];

};

#endif