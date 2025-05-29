#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <LoRaCom.h>
#include <LoRaMsgHandler.h>

class LoRaHandler {
public:
    LoRaHandler(LoRaCom &commManager, LoRaMsgHandler &msgHandler);
    void begin();
    void loop();

private:
    LoRaCom &commManager;
    LoRaMsgHandler &msgHandler;
};

#endif