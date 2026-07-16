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

void LEDLight::blink(int herz)
{
    blinkHz = herz;
    blinking = true;
}

void LEDLight::blinkOff()
{
    blinking = false;
}

void LEDLight::update()
{
    if(blinking)
    {
        if(millis() > lastBlink + blinkHz)
        {
            toggle();
            lastBlink = millis();
        }
    }
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
