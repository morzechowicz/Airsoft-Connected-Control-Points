#include "Buzzer.h"

Buzzer::Buzzer(int pin, bool generator) {
    this->generator = generator;
    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
    
    commandQueue = xQueueCreate(8, sizeof(BeepCommand));
    xTaskCreate(
        [](void* param) {
            static_cast<Buzzer*>(param)->beepTask();
            vTaskDelete(nullptr);
        },
        "BuzzerTask", 2048, this, 1, &BeepTaskHandle
    );
}

void Buzzer::beepTask() {
    LOG_INFO("Buzzer", "Buzzer task started");
    BeepCommand cmd;

    for (;;) {
        if (xQueueReceive(commandQueue, &cmd, portMAX_DELAY) != pdTRUE) continue;
        if (cmd.abort) { off(); continue; }

        for (int i = 0; i < cmd.count; i++) {
            on();
            vTaskDelay(pdMS_TO_TICKS(cmd.duration));
            off();

            if (i < cmd.count - 1) {
                vTaskDelay(pdMS_TO_TICKS(cmd.pause));
            }

            BeepCommand peek;
            if (xQueuePeek(commandQueue, &peek, 0) == pdTRUE && peek.abort) {
                xQueueReceive(commandQueue, &peek, 0);
                off();
                goto next;   // yes, a goto - not mine idea i just stole it from someone
            }
        }
        next:;
    }
}

// void Buzzer::createBeepTask()
// {
//     if (BeepTaskHandle == nullptr)
//     {
//         LOG_DEBUG("Buzzer", "Creating beep task");
//         xTaskCreate(
//             [](void *param)
//             {
//                 Buzzer *buzzer = static_cast<Buzzer *>(param);
//                 buzzer->beepTask();
//                 vTaskDelete(nullptr); // Delete this task when done
//                 LOG_DEBUG("Buzzer", "Beep task deleted");
//             },
//             "BuzzerBeepTask",
//             2048,
//             this,
//             1,
//             &BeepTaskHandle);
//     }
// }

void Buzzer::beep(unsigned long duration, int count, unsigned long pause) {
    BeepCommand cmd{ duration, pause, count, false };
    xQueueSend(commandQueue, &cmd, pdMS_TO_TICKS(20));
}

void Buzzer::beepOnce(unsigned long duration) {
    if (uxQueueMessagesWaiting(commandQueue) == 0) {
        beep(duration, 1, 500);
    }
}

void Buzzer::abortBeep() {
    xQueueReset(commandQueue);              // drop pending commands
    BeepCommand abort{ 0, 0, 0, true };
    xQueueSend(commandQueue, &abort, pdMS_TO_TICKS(20));  // signal task to stop
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
