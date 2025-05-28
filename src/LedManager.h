#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Clocker.h>

class LedManager
{
public:
    LedManager();
    LedManager(int blueLedPin, int yellowLedPin,int blinkingInterval);

    //start clock before using any other method
    void start();
    //stop clock after 
    void stop();
    
    void blinkingBoth(bool alternate);
    void singleOn(int led);
    void bothOff();
    void singleBlink(int led);
    void singleOff(int led);

private:
    Clocker ledClock;
    //this is in ms
    int blinkingInterval;
    int blueLedPin;
    int yellowLedPin;
    int blueTime;
    int yellowTime;

};

#endif