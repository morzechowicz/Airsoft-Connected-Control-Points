#include "LEDLight.h"

LEDLight::LEDLight(int ledPin)
{
    pin = ledPin;
    state = false;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

bool LEDLight::getState()
{
    return state;
}

void LEDLight::on()
{
    state = true;
    digitalWrite(pin, HIGH);
}

void LEDLight::off()
{
    state = false;
    digitalWrite(pin, LOW);
}

void LEDLight::toggle()
{
    state = !state;
    digitalWrite(pin, state ? HIGH : LOW);
}
