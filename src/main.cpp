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

// LoRa Settings
#define LORA_BAND 433.0
#define LORA_BANDWIDTH 125E3
#define LORA_SYNC_WORD 0x12
#define LORA_PREAMBLE_LENGTH 8
#define LORA_TX_POWER 12
#define LORA_RX_TIMEOUT 1000
#define LORA_SF 9
#define LORA_CODING_RATE 5

// Buttons
#define debounce 500
ezButton teamYellowButton(0);
ezButton teamBlueButton(4);
ezButton startGameButton(2);
ezButton changeConfig(15);

// Node variables
uint16_t NodeId;
uint16_t LeaderId = 0;
bool isLeader = false;

// Game variables

GameManager gameManager;
DisplayManager displayManager;

bool isReciving = false;
bool isTransmiting = false;
SX1278 radio = new Module(18, 26, 14, 33);

// LoraMsg structure
// Flag 0x00
struct LocalScore
{
  uint16_t NodeId;
  int score[2];
};
// Flag 0x01
struct LoraTotalScore
{
  int blueScore;
  int yellowScore;
};
// Flag 0x02
struct LoraGameStatus
{
  int currentGameState;
};
// Flag 0x03
struct LoraConfig
{
  int maxScore;
  int maxTime;
  int timeToStart;
  int timeToCapture;
};

// Function prototypes
void setReciving();
void receiveLoRaLoop();
void initialize();
bool isChannelClear();
void sendLoRaMsg(uint8_t *msg, size_t length);
void sendLoRaData(uint8_t flag, const void *data, size_t dataSize);
void sendGameStatus(int gameStatus);
void sendTotalScore(int blueScore, int yellowScore);
void sendLocalScore(uint16_t nodeId, int blueScore, int yellowScore);
void sendConfig(int maxScore, int maxTime, int timeToStart, int timeToCapture);

void sendLoRaData(uint8_t flag, const void *data, size_t dataSize)
{
  uint8_t buffer[dataSize + 3]; // +3 for Flag, seq number, and checksum
  static uint8_t seqNumber = 0; // Sequence number
  uint8_t checksum = 0;

  buffer[0] = flag;
  memcpy(buffer + 1, data, dataSize);
  buffer[dataSize + 1] = seqNumber;

  // Calculate checksum by XORing all bytes in the buffer, including the flag and sequence number
  for (size_t i = 0; i < dataSize + 2; i++)
  {
    checksum ^= buffer[i];
  }
  buffer[dataSize + 2] = checksum;

  sendLoRaMsg(buffer, sizeof(buffer));

  seqNumber++; // Increment sequence number for next message
}

void sendGameStatus(int gameStatus)
{
  LoraGameStatus status;
  status.currentGameState = gameStatus;
  sendLoRaData(0x02, &status, sizeof(LoraGameStatus));
  Serial.print("Game status sent: ");
  Serial.println(status.currentGameState);
}

void sendTotalScore(int blueScore, int yellowScore)
{
  LoraTotalScore totalScore;
  totalScore.blueScore = blueScore;
  totalScore.yellowScore = yellowScore;
  sendLoRaData(0x01, &totalScore, sizeof(LoraTotalScore));
}

void sendConfig(int maxScore, int maxTime, int timeToStart, int timeToCapture)
{
  LoraConfig config;
  config.maxScore = maxScore;
  config.maxTime = maxTime;
  config.timeToStart = timeToStart;
  config.timeToCapture = timeToCapture;
  sendLoRaData(0x03, &config, sizeof(LoraConfig));
  Serial.print("Config sent: ");
  Serial.print("Max Score: ");
  Serial.print(config.maxScore); 
  Serial.print(", Max Time: ");
  Serial.print(config.maxTime);
  Serial.print(", Time to Start: ");
  Serial.print(config.timeToStart);
  Serial.print(", Time to Capture: ");
  Serial.println(config.timeToCapture);
}

void sendLocalScore(uint16_t nodeId, int blueScore, int yellowScore)
{
  LocalScore localScore;
  localScore.NodeId = nodeId;
  localScore.score[0] = blueScore;
  localScore.score[1] = yellowScore;
  sendLoRaData(0x00, &localScore, sizeof(LocalScore));
}

void setReciving()
{
  // idk why but dio0 gets activated when transmitting
  // this is a band aid solution but i hope it works
  Serial.println("Received data");
  if(!isTransmiting)
  {
    isReciving = true;
  }
}

void receiveLoRaLoop()
{
  if (isReciving && !isTransmiting)
  {
    uint8_t buffer[256];
    size_t length = sizeof(buffer);
    int state = radio.readData(buffer, length);
    if (state == RADIOLIB_ERR_NONE)
    {

      uint8_t flag = buffer[0];
      uint8_t checksum = buffer[length - 1];
      uint8_t calculatedChecksum = 0;

      for (size_t i = 0; i < length - 1; i++)
      {
        calculatedChecksum ^= buffer[i];
      }

      if (calculatedChecksum == checksum)
      {
        switch (flag)
        {
        case 0x00: // LocalScore
        {
          LocalScore receivedScore;
          memcpy(&receivedScore, buffer + 1, sizeof(LocalScore));
          Serial.print("Received LocalScore from NodeId: ");
          Serial.println(receivedScore.NodeId);
          break;
        }
        case 0x01: // LoraTotalScore
        {
          LoraTotalScore totalScore;
          memcpy(&totalScore, buffer + 1, sizeof(LoraTotalScore));
          Serial.print("Received TotalScore - Blue: ");
          Serial.print(totalScore.blueScore);
          Serial.print(", Yellow: ");
          Serial.println(totalScore.yellowScore);
          break;
        }
        case 0x02: // LoraGameStatus
        {
          LoraGameStatus gameStatus;
          memcpy(&gameStatus, buffer + 1, sizeof(LoraGameStatus));
          gameManager.setCurrentGameState(gameStatus.currentGameState);
          Serial.print("Received GameStatus : ");
          Serial.println(gameManager.getCurrentGameState());
          break;
        }
        default:
          Serial.println("Unknown flag received");
          break;
        }
      }
      else
      {
        Serial.println("Checksum mismatch, discarding packet");
      }
    }
    else
    {
      Serial.print("Failed to read data, error: ");
      Serial.println(state);
    }

    isReciving = false;
  }
  radio.startReceive();
}

void initializeLoRa()
{
  int state = radio.begin(LORA_BAND);
  radio.setSyncWord(LORA_SYNC_WORD);
  radio.setPreambleLength(LORA_PREAMBLE_LENGTH);
  radio.setBandwidth(LORA_BANDWIDTH);
  radio.setOutputPower(LORA_TX_POWER);
  radio.setSpreadingFactor(LORA_SF);
  radio.setCodingRate(LORA_CODING_RATE);


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
  isTransmiting = true;
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

  isTransmiting = false;

  radio.startReceive();
}

void setup()
{
  teamYellowButton.setDebounceTime(debounce);
  teamBlueButton.setDebounceTime(debounce);
  startGameButton.setDebounceTime(debounce);
  changeConfig.setDebounceTime(debounce);

  Serial.begin(115200);
  Serial.println("Starting...");

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  displayManager.initialize();
  initializeLoRa();
}

void loop()
{
  teamYellowButton.loop();
  teamBlueButton.loop();
  startGameButton.loop();
  changeConfig.loop();

  switch (gameManager.getCurrentGameState())
  {
  case 0: // Config mode
    gameManager.initializeLoop(teamYellowButton, teamBlueButton,startGameButton, changeConfig, sendGameStatus);
    displayManager.settingDisplayOLED(gameManager.getCurrentSettingId(), gameManager.getMaxScore(), gameManager.getMaxTime(), gameManager.getTimeToStart(), gameManager.getTimeToCapture());
    break;
  case 1: // countdown mode
    gameManager.countdownLoop(sendGameStatus);
    displayManager.countdownDisplayOled(gameManager.getTimeToStart());
    break;
  case 2: // Game mode
    gameManager.gameLoop(teamBlueButton, teamYellowButton, startGameButton, NodeId, LeaderId);
    displayManager.gameDisplayOLED(gameManager.getCurrentGameState(), NodeId, LeaderId, gameManager.getLocalTeamsScore()[0].score, gameManager.getLocalTeamsScore()[1].score,gameManager.getMaxTime());
    break;
  case 3: // Game ended
    gameManager.endGameLoop();
    displayManager.endDisplayOLED(gameManager.getLocalTeamsScore()[0].score, gameManager.getLocalTeamsScore()[1].score,gameManager.getWinner());
    break;
  default:
    break;
  }
  receiveLoRaLoop();
}
