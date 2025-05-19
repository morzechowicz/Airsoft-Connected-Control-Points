#include "Config.h"
#include <Arduino.h> // For Serial (for error messages)

void Config::handleButtonPresses(ButtonManager& buttonManager, int& configState) {
    if (buttonManager.configButton1.isPressed()) {
        configState++;
        if (configState > 3) {
            configState = 0;
        }
    }
    if (buttonManager.playerButton1.isPressed()) {
        switch (configState) {
            case 0:
                countdown += 15;
                break;
            case 1:
                durration += 15;
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
    if (buttonManager.playerButton2.isPressed()) {
        switch (configState) {
            case 0:
                countdown -= 15;
                break;
            case 1:
                durration -= 15;
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

void Config::setCountdown(int seconds) {
    countdown = seconds;
}

void Config::setDurration(int minutes) {
    durration = minutes;
}

void Config::setPointsTarget(int points) {
    pointsTarget = points;
}

void Config::setCaptureTime(int seconds) {
    captureTime = seconds;
}