#include <Clocker.h>


Clocker::Clocker() : startTime(0), elapsedTime(0), running(false) {}

void Clocker::start() {
    if(!running)
    {
        startTime = millis();
        running = true;
    }
}

void Clocker::stop()
{
    if(running)
    {
        running = false;
    }
}

void Clocker::reset()
{
    elapsedTime = 0;
    startTime = 0;
    running = false;
}

uint64_t Clocker::getElapsedTime()
{
    if(running)
    {
        elapsedTime = millis() - startTime;
    }
    return elapsedTime;
}

int Clocker::getElapsedTimeInMinutes()
{
    return elapsedTime / 60000;
}