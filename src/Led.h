#ifndef LED_H
#define LED_H

#include <Clocker.h>

class Led {
public:
    Led();
    Led(int pin);

    void on();
    void off();
    void toggle();
    void blinking();
    bool isOn() const;

    int getBlinkInterval();
    void setBlinkInterval(int interval);

    void update();
private:
    int pin;
    bool state;
    bool blink;
    int blinkInterval;
    Clocker blinkClock;
};

#endif // LED_H