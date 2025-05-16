#pragma once

class Config
{
public:
    Config(long countdown, long durration, int pointsTarget, long captureTime)
        : countdown(countdown), durration(durration), pointsTarget(pointsTarget), captureTime(captureTime) {}

private:
    long countdown;
    long durration;
    int pointsTarget;
    long captureTime;

};