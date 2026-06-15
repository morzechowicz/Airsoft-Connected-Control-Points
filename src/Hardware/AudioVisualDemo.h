#ifndef AUDIO_VISUAL_DEMO_H
#define AUDIO_VISUAL_DEMO_H

#include <Arduino.h>
#include "HardwareManager.h"
#include "EventBus.h"
#include "../lib/Logging/LogManager.h"

enum AudioSignalLenght
{
    SHORT,
    MEDIUM,
    LONG
};


class AudioVisualDemo
{
private:
    HardwareManager *hardwareManager;
    EventBus *eventBus;

public:
    AudioVisualDemo(HardwareManager *hardwareManager, EventBus *eventBus);

    void init();
    void demoEventListener(Event e);
    void audioDemo(AudioSignalLenght signalLength, int numberOfSignals = 1);
    void captureDemo(int captureTime = 5000);
    void respawnDemo();
};

#endif // AUDIO_VISUAL_DEMO_H