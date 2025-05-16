#ifndef CLOCKER_H
#define CLOCKER_H

#include <stdint.h>

class Clocker
{
private:
    uint32_t startTime;
    uint32_t elapsedTime;
    bool running;

public:
    Clocker();

    void start();
    void stop();
    void reset();
    uint32_t getElapsedTime() const;
    bool isRunning() const;
};

#endif