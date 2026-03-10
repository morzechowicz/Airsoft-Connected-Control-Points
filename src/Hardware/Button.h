#ifndef BUTTON_H
#define BUTTON_H

#include "EventBus.h"
#include <Arduino.h>

class Button
{
private:
    int pin;
    bool lastState = HIGH;
    int debounceDelay;
    unsigned long lastDebounceTime;
    unsigned long holdThreshold;
    int holdTime;
    bool heldEventSent;
    bool stableState = HIGH;
    EventBus *eventBus;

public:
    Button(EventBus *eb, int buttonPin);

    bool getState();

    void update();
};

#endif // BUTTON_H