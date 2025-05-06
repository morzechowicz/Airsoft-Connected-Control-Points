#ifndef LORA_COMMUNICATION_H // Header guard to prevent multiple inclusions
#define LORA_COMMUNICATION_H

#include <cstdint>
#include <cstddef>
#include <RadioLib.h>


// LoRa Settings
#define LORA_BAND 433.0
#define LORA_SYNC_WORD 0x12
#define LORA_PREAMBLE_LENGTH 8
#define LORA_TX_POWER 10
#define LORA_RX_TIMEOUT 1000

class LoRaCommunication
{
public:
    void initialize();
    void sendLoRaMsg(uint8_t *msg, size_t length);
    bool isChannelClear();
};

#endif