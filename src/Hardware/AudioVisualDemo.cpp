#include "AudioVisualDemo.h"

AudioVisualDemo::AudioVisualDemo(HardwareManager *hardwareManager, EventBus *eventBus)
: hardwareManager(hardwareManager), eventBus(eventBus)
{
}

void AudioVisualDemo::init()
{
    //Demo Trigger listener
    eventBus->subscribe(DEMO, [this](Event e) {
        this->demoEventListener(e);
    });
}

void AudioVisualDemo::demoEventListener(Event e)
{
    DemoType demoType = static_cast<DemoType>(e.data1);
    if(demoType == AUDIO_DEMO)
    {
        AudioSignalLenght signalLength = static_cast<AudioSignalLenght>(e.data2);
        int numberOfSignals = e.data3;
        audioDemo(signalLength, numberOfSignals);
    }
    else if(demoType == RESPAWN_DEMO)
    {
        respawnDemo();
    }
    else if(demoType == CAPTURE_DEMO)
    {
        int captureTime = e.data2; // in seconds
        captureDemo(captureTime);
    }
    else{
        LOG_DEBUG("DEMO","Unknown demo type: %d", demoType);
    }
}

void AudioVisualDemo::audioDemo(AudioSignalLenght signalLength, int numberOfSignals)
{
    int durration;
    if(signalLength == SHORT)
    {
        durration = 200;
    }
    else if(signalLength == MEDIUM)
    {
        durration = 500;// leave it like this i dont have use for it right now
    }
    else if(signalLength == LONG)
    {
        durration = 4000;
    }

    for(int i = 0; i < numberOfSignals; i++)
    {
        hardwareManager->buzzer.beepOnce(durration);
        vTaskDelay(pdMS_TO_TICKS(1000)); //TO DO: standarize delay for all audio signals
    }

}

void AudioVisualDemo::captureDemo(int captureTime)
{
    hardwareManager->lcd.kothDisplayCapturing(Team::YELLOW); // just for demo
    //progress loop
    unsigned long startTime = millis();
    unsigned long capTime = captureTime * 1000; // convert to ms
    float progress = 0.0f;
    while(millis() - startTime < capTime)
    {
        progress = (float)(millis() - startTime) / (float)capTime;
        hardwareManager->lcd.kothDisplayCapturingProgress(progress);
        vTaskDelay(pdMS_TO_TICKS(500)); // update every 0.5 seconds
    }
    hardwareManager->lcd.kothDisplayController(Team::YELLOW); // just for demo
    hardwareManager->lcd.kothDisplayScore(10, 5); // just for demo
    hardwareManager->buzzer.beep(300, 3, 300); // i swear i will standarize it at some point

    vTaskDelay(pdMS_TO_TICKS(5000)); // wait before clearing the screen
    hardwareManager->lcd.clearScreen();
}

void AudioVisualDemo::respawnDemo()
{
    hardwareManager->lcd.displayRespawn();
    hardwareManager->buzzer.beep(100, 5, 300);
}
