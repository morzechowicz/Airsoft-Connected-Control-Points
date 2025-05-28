#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include <Clocker.h>

class Buzzer {
public:
    Buzzer(int pin);
    void beep(unsigned int duration = 100);

    bool isBeeping() {return isOn;};
private:
    int pin;
    bool isOn;
    Clocker buzzClock;

};

#endif // BUZZER_H