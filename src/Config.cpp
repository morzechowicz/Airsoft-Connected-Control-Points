#include "Config.h"
#include <Arduino.h> // For Serial (for error messages)

void Config::handleButtonPresses(ButtonManager& buttonManager, int& configState) {
    if (buttonManager.changeButton.isPressed()) {
        configState++;
        if (configState > 3) {
            configState = 0;
        }
    }
    if (buttonManager.blueButton.isPressed()) {
        switch (configState) {
            case 0:
                countdown += 15;
                break;
            case 1:
                duration += 15;
                break;
            case 2:
                pointsTarget += 10;
                break;
            case 3:
                captureTime += 1;
                break;
            default:
                break;
        }
    }
    if (buttonManager.yellowButton.isPressed()) {
        switch (configState) {
            case 0:
                countdown -= 15;
                break;
            case 1:
                duration -= 15;
                break;
            case 2:
                pointsTarget -= 10;
                break;
            case 3:
                captureTime -= 1;
                break;
            default:
                break;
        }
        //min
        if(countdown < 5){countdown = 5;}
        if(duration < 10){duration = 10;}
        if(pointsTarget < 10){pointsTarget = 10;}
        if(captureTime < 6){captureTime = 6;}
        //max
        if(countdown > 600){countdown = 600;}
        if(duration > 500){duration = 500;}
        if(pointsTarget > 999){pointsTarget = 999;}
        if(captureTime > 30){captureTime = 30;}
    }
}

void Config::fromString(String &data) {
    // Example: Parse "C/From/to/seq/10/20/30/40" into countdown, duration, pointsTarget, captureTime
    // refactor to use stringsplitter
    splitter.split(data);

    countdown = splitter.getItem(4).toInt();
    duration = splitter.getItem(5).toInt();
    pointsTarget = splitter.getItem(6).toInt();
    captureTime = splitter.getItem(7).toInt();


    Serial.println("Config updated:");
    Serial.print("Countdown: "); Serial.println(countdown);
    Serial.print("Duration: "); Serial.println(duration);
    Serial.print("Points Target: "); Serial.println(pointsTarget);
    Serial.print("Capture Time: "); Serial.println(captureTime);
}

void Config::setCountdown(int seconds) {
    countdown = seconds;
}

void Config::setDurration(int minutes) {
    duration = minutes;
}

void Config::setPointsTarget(int points) {
    pointsTarget = points;
}

void Config::setCaptureTime(int seconds) {
    captureTime = seconds;
}