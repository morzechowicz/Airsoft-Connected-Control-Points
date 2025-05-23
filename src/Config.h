#ifndef CONFIG_H
#define CONFIG_H

#include "ButtonManager.h" // Include ButtonManager

class Config {
public:
    Config(long countdown, long duration, int pointsTarget, long captureTime)
        : countdown(countdown), duration(duration), pointsTarget(pointsTarget), captureTime(captureTime) {}

    void configLoop();

    void handleButtonPresses(ButtonManager& buttonManager, int& configState);
    void fromString(String &data);

    long getCountdown() const { return countdown; }
    long getDurration() const { return duration; }
    int getPointsTarget() const { return pointsTarget; }
    long getCaptureTime() const { return captureTime; }

    void setCountdown(int seconds);
    void setDurration(int minutes);
    void setPointsTarget(int minutes);
    void setCaptureTime(int seconds);

private:
    long countdown;
    long duration;
    int pointsTarget;
    long captureTime;
};

#endif