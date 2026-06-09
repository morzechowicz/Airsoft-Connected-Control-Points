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

enum DemoType
{
    CAPTURE_DEMO = 1, // data 2 should be capture time in seconds
    RESPAWN_DEMO = 2,
    AUDIO_DEMO = 3 // data 2 should be signal length, data 3 should be number of signals
    // add more later maybe, maybe not
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
    void audioDemo(AudioSignalLenght signalLength, int numberOfSignals);
    void captureDemo(int captureTime = 5000);
    void respawnDemo();
};

#endif // AUDIO_VISUAL_DEMO_H