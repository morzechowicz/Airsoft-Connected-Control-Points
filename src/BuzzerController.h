#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

#include <Arduino.h>

class BuzzerController {
private:
    int buzzerPin;

public:
    void playSignal(int reapet); //plays signal x times
};

#endif