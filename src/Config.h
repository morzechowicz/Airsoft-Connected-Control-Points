#ifndef CONFIG_H
#define CONFIG_H

#include "ButtonManager.h" // Include ButtonManager

class Config {
public:
    Config(long countdown, long durration, int pointsTarget, long captureTime)
        : countdown(countdown), durration(durration), pointsTarget(pointsTarget), captureTime(captureTime) {}

    void configLoop();

    // New method to handle button presses and update config values
    void handleButtonPresses(ButtonManager& buttonManager, int& configState);

    long getCountdown() const { return countdown; }
    long getDurration() const { return durration; }
    int getPointsTarget() const { return pointsTarget; }
    long getCaptureTime() const { return captureTime; }

    void setCountdown(int seconds);
    void setDurration(int minutes);
    void setPointsTarget(int minutes);
    void setCaptureTime(int seconds);

private:
    long countdown;
    long durration;
    int pointsTarget;
    long captureTime;
};

#endif