#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include <Clocker.h>

class Buzzer {
public:
    Buzzer(int pin);
    void beep(unsigned int dur = 100);
    void beepXtimes(unsigned int pause, unsigned int reapet, unsigned int dur);

    void beepLoop();
    void stopBeeping();
private:
    int pin;
    bool isOn;
    int durration;
    int setDuration;
    int inBetweenPause;
    int reapets;
    Clocker buzzClock;

};

#endif // BUZZER_H