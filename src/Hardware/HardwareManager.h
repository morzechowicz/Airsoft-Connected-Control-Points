#ifndef HARDWAREMANAGER_H
#define HARDWAREMANAGER_H

#include <Arduino.h>
#include "Button.h"
#include "LEDLight.h"
#include "Buzzer.h"
#include "EventBus.h"
#include "../Config.h"
#include "LCDScreen.h"

class HardwareManager
{
public:
    Button buttonBlue;
    Button buttonYellow;
    Button buttonSelect;
    Button buttonEnter;

    LEDLight ledBlueButton;
    LEDLight ledYellowButton;
    
    Buzzer buzzer;
    LCDScreen lcd;

    HardwareManager(EventBus *eventBus);
    void update();
    void reboot();
};

#endif // HARDWAREMANAGER_H