#include "Buzzer.h"

Buzzer::Buzzer(int pin) : pin(pin), isOn(false) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); 
}

void Buzzer::beep(unsigned int duration) {
    if (!isOn) {
        isOn = true;
        durration = duration;
        digitalWrite(pin, HIGH);
        buzzClock.reset();  
        buzzClock.start();       
    }
}

void Buzzer::beepXtimes(unsigned int pause,unsigned int reapet, unsigned int dur)
{
    if(!isOn)
    {
        reapets = reapet;
        setDuration = dur;
        inBetweenPause = pause+dur;
        beep(setDuration);   
    }
}

// loop checks if buzzer should stop without blocking
void Buzzer::beepLoop()
{
    if (buzzClock.getElapsedTime() >= durration && isOn) {
        isOn = false;
        Serial.println("beep of 1");
        digitalWrite(pin, LOW);  
    }
    
    // Check if there are remaining repetitions for the beep sequence
    if (reapets > 0)
    {
        if((buzzClock.getElapsedTime() >= inBetweenPause) && !isOn)
        {
            beep(setDuration);
            reapets--;
        }
    }else if(reapets > 0 && isOn){
        isOn = false;
        Serial.println("beep of 2");
        digitalWrite(pin, LOW);  
        durration = 0;
        buzzClock.stop();  
    }
}