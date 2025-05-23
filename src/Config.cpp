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
                pointsTarget += 15;
                break;
            case 3:
                captureTime += 15;
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
                pointsTarget -= 15;
                break;
            case 3:
                captureTime -= 15;
                break;
            default:
                break;
        }
    }
}

void Config::fromString(String &data) {
    // Example: Parse "C/10/20/30/40" into countdown, duration, pointsTarget, captureTime
    if (data[0] == 'C') {
        int firstSlash = data.indexOf('/');
        int secondSlash = data.indexOf('/', firstSlash + 1);
        int thirdSlash = data.indexOf('/', secondSlash + 1);
        int fourthSlash = data.indexOf('/', thirdSlash + 1);

        countdown = data.substring(firstSlash + 1, secondSlash).toInt();
        duration = data.substring(secondSlash + 1, thirdSlash).toInt();
        pointsTarget = data.substring(thirdSlash + 1, fourthSlash).toInt();
        captureTime = data.substring(fourthSlash + 1).toInt();

        Serial.println("Config updated:");
        Serial.print("Countdown: "); Serial.println(countdown);
        Serial.print("Duration: "); Serial.println(duration);
        Serial.print("Points Target: "); Serial.println(pointsTarget);
        Serial.print("Capture Time: "); Serial.println(captureTime);
    }
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