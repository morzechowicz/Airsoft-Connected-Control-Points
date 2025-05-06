#include <LoraCommunication.h>

class LoRaCommunication {
public:
  SX1278 radio = new Module(18, 26, 14, 33);

  void initialize() {
    int state = radio.begin(LORA_BAND);
    radio.setSyncWord(LORA_SYNC_WORD);
    radio.setPreambleLength(LORA_PREAMBLE_LENGTH);
    radio.setOutputPower(LORA_TX_POWER);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("LoRa initialized!");
    } else {
      Serial.print("Failed, code: ");
      Serial.println(state);
      while (true);
    }
  }

  void sendLoRaMsg(uint8_t *msg, size_t length) {
    const int maxAttempts = 5;
    const int initialBackoff = 100;
    int attempt = 0;
    bool sent = false;

    while (attempt < maxAttempts && !sent) {
      if (isChannelClear()) {
        int state = radio.transmit(msg, length);
        if (state == RADIOLIB_ERR_NONE) {
          Serial.println("Message sent successfully!");
          sent = true;
        } else {
          Serial.print("Transmission failed, error: ");
          Serial.println(state);
        }
      } else {
        Serial.println("Channel busy, waiting...");
        int backoffTime = initialBackoff * (1 << attempt);
        delay(backoffTime);
      }
      attempt++;
    }

    if (!sent) {
      Serial.println("Failed to send message after maximum attempts");
    }

    radio.startReceive();
  }

  bool isChannelClear() {
    radio.startReceive();
    delay(10);
    return !radio.available();
  }
};