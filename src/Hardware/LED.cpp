#include "LED.h"

LED::LED(int ledPin)
{
    pin = ledPin;
    state = false;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

bool LED::getState()
{
    return state;
}

void LED::on()
{
    state = true;
    digitalWrite(pin, HIGH);
}

void LED::off()
{
    state = false;
    digitalWrite(pin, LOW);
}

void LED::toggle()
{
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
}
