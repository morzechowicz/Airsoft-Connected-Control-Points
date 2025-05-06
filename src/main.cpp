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
#include "LoraCommunication.h"

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

// Global instances
LoRaCommunication lora;
GameManager gameManager;
DisplayManager displayManager;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  displayManager.initialize();
  lora.initialize();
}

void loop() {
  gameManager.updateTeamScore(TEAM_BLUE, 1);
  gameManager.changeLedColor(TEAM_BLUE);
  displayManager.updateDisplay(isGameInProgress, NodeId, LeaderId, gameManager.localTeamsScore);
}
