#ifndef CLOCKER_H
#define CLOCKER_H

#include <stdint.h>
#include <Arduino.h>

class Clocker
{
private:
    uint64_t startTime;
    uint64_t elapsedTime;
    bool running;

public:
    Clocker();

    void start();
    void stop();
    void reset();
    void setTimeFromMinutes(int minutes);
    uint64_t getElapsedTime();
    bool isRunning() {return running;};
    int getElapsedTimeInMinutes();
    int getElapsedTimeInSeconds();
};

#endif