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
    startTime = millis();
}

void Clocker::setTimeFromMinutes(int minutes)
{
    elapsedTime = minutes * 60000;
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
    elapsedTime = millis() - startTime;
    return elapsedTime / 60000;
}

int Clocker::getElapsedTimeInSeconds()
{
    elapsedTime = millis() - startTime;
    return elapsedTime / 1000;
}