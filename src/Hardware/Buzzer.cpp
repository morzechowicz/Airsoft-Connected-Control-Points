#include "Buzzer.h"

Buzzer::Buzzer(int pin, bool generator)
{
    this->generator = generator;
    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
}

void Buzzer::beepTask(void)
{
    Serial.println("Buzzer task started");
    while (true)
    {
        if (beepCount)
        {
            on();
            vTaskDelay(duration / portTICK_PERIOD_MS);
            off();
            vTaskDelay(pause / portTICK_PERIOD_MS);
            beepCount--;
        }
        vTaskDelay(100);
    }
    BeepTaskHandle = nullptr;
    Serial.println("Buzzer task ended");
}

void Buzzer::createBeepTask()
{
    if (BeepTaskHandle == nullptr)
    {
        xTaskCreate(
            [](void *param)
            {
                Buzzer *buzzer = static_cast<Buzzer *>(param);
                buzzer->beepTask();
                vTaskDelete(nullptr); // Delete this task when done
            },
            "BuzzerBeepTask",
            2048,
            this,
            1,
            &BeepTaskHandle);
    }
}

void Buzzer::beepOnce(unsigned long duration)
{
    //this should be skipped if its already working
    if(!beepCount)
    {
        this->duration = duration;
        this->beepCount += 1;
        this->pause = 500;
        createBeepTask();
    }
}

void Buzzer::beep(unsigned long duration, int count, unsigned long pause)
{
    this->duration = duration;
    this->beepCount += count;
    this->pause = pause;
    createBeepTask();
}

void Buzzer::abortBeep()
{
    if (BeepTaskHandle != nullptr)
    {
        vTaskDelete(BeepTaskHandle);
        BeepTaskHandle = nullptr;
    }
    off();
}

void Buzzer::on()
{
    if (generator)
    {
        tone(buzzerPin, 3000); // Start generating a tone at 3000 Hz
        return;
    }
    else
    {
        digitalWrite(buzzerPin, HIGH);
    }
}
void Buzzer::off()
{
    if (generator)
    {
        noTone(buzzerPin); // Stop generating the tone
        return;
    }
    else
    {
        digitalWrite(buzzerPin, LOW);
    }
}
