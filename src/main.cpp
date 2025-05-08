#include <RadioLib.h>
#include <map>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ezButton.h>
#include "Arduino.h"
#include "GameManager.h"
#include "DisplayManager.h"
#include <cstdint>
#include <cstddef>
#include <RadioLib.h>


// LoRa Settings
#define LORA_BAND 433.0
#define LORA_SYNC_WORD 0x12
#define LORA_PREAMBLE_LENGTH 8
#define LORA_TX_POWER 10
#define LORA_RX_TIMEOUT 1000

// Buttons
#define debounce 250
ezButton teamYellowButton(0);
ezButton teamBlueButton(4);
ezButton startGameButton(2);

// Node variables
uint16_t NodeId;
uint16_t LeaderId = 0;
bool isLeader = false;

// Game variables
bool isGameInProgress = false;
int pointControlledByTeam = 99;

GameManager gameManager;
DisplayManager displayManager;

bool isReciving = false;
SX1278 radio = new Module(18, 26, 14, 33);

void setReciving()
{
  isReciving = true;
}

void receiveLoRaLoop()
{
  if(isReciving)
  {
    
  }
}

void initialize()
{
  int state = radio.begin(LORA_BAND);
  radio.setSyncWord(LORA_SYNC_WORD);
  radio.setPreambleLength(LORA_PREAMBLE_LENGTH);
  radio.setOutputPower(LORA_TX_POWER);
  radio.setDio0Action(setReciving, RISING);
  radio.startReceive();

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa initialized!");
  }
  else
  {
    Serial.print("Failed, code: ");
    Serial.println(state);
    while (true)
      ;
  }
}

bool isChannelClear()
{
  radio.startReceive();
  delay(10);
  return !radio.available();
}

void sendLoRaMsg(uint8_t *msg, size_t length)
{
  const int maxAttempts = 5;
  const int initialBackoff = 100;
  int attempt = 0;
  bool sent = false;

  while (attempt < maxAttempts && !sent)
  {
    if (isChannelClear())
    {
      int state = radio.transmit(msg, length);
      if (state == RADIOLIB_ERR_NONE)
      {
        Serial.println("Message sent successfully!");
        sent = true;
      }
      else
      {
        Serial.print("Transmission failed, error: ");
        Serial.println(state);
      }
    }
    else
    {
      Serial.println("Channel busy, waiting...");
      int backoffTime = initialBackoff * (1 << attempt);
      delay(backoffTime);
    }
    attempt++;
  }

  if (!sent)
  {
    Serial.println("Failed to send message after maximum attempts");
  }

  radio.startReceive();
}


void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  displayManager.initialize();
  initialize();
}

void loop() {

  displayManager.updateDisplay(isGameInProgress, NodeId, LeaderId, gameManager.localTeamsScore);
}
