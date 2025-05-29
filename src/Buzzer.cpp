#include "Buzzer.h"

Buzzer::Buzzer(int pin) : pin(pin), isOn(false) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Ensure the buzzer is off initially
}

void Buzzer::beep(unsigned int duration) {
    if (!isOn) {
        isOn = true;
        digitalWrite(pin, HIGH); // Turn the buzzer on
        buzzClock.reset();       // Reset the clock
        buzzClock.start();       // Start the clock
    }

    // Check if the duration has passed
    if (buzzClock.getElapsedTime() >= duration) {
        isOn = false;
        digitalWrite(pin, LOW);  // Turn the buzzer off
        buzzClock.stop();        // Stop the clock
    }
}