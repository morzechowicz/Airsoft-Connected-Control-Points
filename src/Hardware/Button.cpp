#include "Button.h"

Button::Button(EventBus *eb, int buttonPin)
    : pin(buttonPin),
      lastState(HIGH),
      debounceDelay(50),
      lastDebounceTime(0),
      holdThreshold(1000),
      holdTime(0),
      eventBus(eb)
{
    pinMode(pin, INPUT_PULLUP);
}

void Button::update()
{
    // Safety check
    if (eventBus == nullptr)
        return;

    // Read current state
    bool currentState = digitalRead(pin) == LOW; // Assuming active LOW

    // Did state change? (Debounce start)
    if (currentState != lastState)
    {
        lastDebounceTime = millis(); // Start debounce timer
    }

    // Has state been stable long enough?
    if ((millis() - lastDebounceTime) > debounceDelay)
    {

        // Is this a NEW stable state we haven't processed yet?
        if (currentState != stableState)
        {
            stableState = currentState;

            if (stableState == HIGH)
            {
                // Button just pressed!
                holdTime = millis();
                heldEventSent = false;
                eventBus->publish(BUTTON_PRESSED, pin);
            }
            else
            {
                // Button just released!
                eventBus->publish(BUTTON_RELEASED, pin);
                heldEventSent = false;
                holdTime = 0;
            }
        }

        // While button is held down, check for hold threshold
        if (stableState == HIGH && !heldEventSent)
        {
            if ((millis() - holdTime) >= holdThreshold)
            {
                eventBus->publish(BUTTON_HELD, pin);
                heldEventSent = true; // Only send once
            }
        }
    }

    lastState = currentState;
}

bool Button::getState()
{
    return stableState;
}
