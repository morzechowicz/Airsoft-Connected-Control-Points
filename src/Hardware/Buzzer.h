#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer
{
private:
    int buzzerPin;
    bool generator = false;
    int beepCount = 0;
    unsigned long pause = 500;
    unsigned long duration = 0;

    TaskHandle_t BeepTaskHandle = nullptr;

    void beepTask();

public:
    Buzzer(int pin, bool generator = false);

    void createBeepTask();
    void beepOnce(unsigned long duration = 200);
    void beep(unsigned long duration = 200, int count = 1, unsigned long pause = 100);
    void abortBeep();

    void on();
    void off();
};

#endif // BUZZER_H