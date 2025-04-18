#include <RadioLib.h>
#include <map>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ezButton.h>
#include "Arduino.h"

// OLED Settings
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Team variables
#define TEAM_BLUE 0
#define TEAM_YELLOW 1

// Node variables
//how to prevent nodes from rolling the same id? 65536 is enough to avoid collisions i think
uint16_t NodeId; // 16 characters
void setNodeId()
{
  NodeId = random(5, 65536); 
  Serial.print("Generated Node ID: ");
  Serial.println(NodeId);
}

uint16_t LeaderId = 0; // Id of leader node
bool isLeader = false; // Is this node a leader?
// Mesh Settings
SX1278 radio = new Module(18, 26, 14, 33); // CS, DIO0, RST, DIO1

// Message format
#define WHO_IS_OUT_THERE 0x01
#define I_AM_HERE 0x02
#define SCORE_UPDATE 0x03

uint8_t seqNumber = 0;

#pragma pack(push, 1)
typedef struct
{
  uint8_t type;          // message type
  uint16_t nodeId;       // node id
  uint8_t seqNumber;    // sequence number
  uint16_t neighbors[5]; // neighbor node ids
  uint8_t neighborCount; // number of neighbors
  uint8_t isLeader;      // is this node a leader
} DiscoveryMessage;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
  uint8_t type;          // message type
  uint16_t nodeIdSender;       // node id from
  uint16_t nodeIdReciver;     // node id to
  uint8_t seqNumber;    // sequence number
  uint16_t BlueScore;    // blue team score
  uint16_t YellowScore;  // yellow team score
} StandardMessage;
#pragma pack(pop)

struct NodesTable
{
  uint16_t nodeId;
  uint16_t nextHop;
  uint64_t lastSeen;
  bool isLeader;
  int HopCount;
};

NodesTable nodesTable[10]; // Array to store node information

// Buttons
#define debounce 250          // 250ms debounce time
ezButton teamYellowButton(0); // GPIO 0 (IO0)
ezButton teamBlueButton(4);   // GPIO 4 (IO4)
ezButton startGameButton(2);  // GPIO 2 (IO2)

// LEDs
#define LED_YELLOW 13 // GPIO 13 (IO15)
#define LED_BLUE 12   // GPIO 12 (IO13)

// System variables
Ticker teamScoreTicker;
Ticker screenUpdateTicker;
Ticker nodeScoreUpdateTicker;
Ticker totalScoreTicker;

// Game variables
bool isGameInProgress = false;
long gameDuration = 14400; // 4 hours in seconds  
int pointControlledByTeam = 99; // 99 = none, 1 = blue, 2 = yellow
String msg = "";

// Team variables
struct Team
{
  int id;
  uint16_t score;
};

Team totalTeamsScore[] = {
    {TEAM_BLUE, 0},
    {TEAM_YELLOW, 0}};

Team localTeamsScore[] = {
    {TEAM_BLUE, 0},
    {TEAM_YELLOW, 0}};

struct NodeTeamScores
{
  uint32_t nodeId;
  Team teamScores[2]; // Index 1 for TEAM_BLUE, Index 2 for TEAM_YELLOW
};

#define NODE_SCORES_SIZE 10
NodeTeamScores nodeScores[NODE_SCORES_SIZE];

// LoRa Settings
#define LORA_BAND 433.0         // LoRa frequency band (433 MHz)
#define LORA_SPREADING_FACTOR 7 // Spreading factor (7-12)
#define LORA_BANDWIDTH 125      // Bandwidth (125 kHz)
#define LORA_CODING_RATE 5      // Coding rate (4/5)
#define LORA_SYNC_WORD 0x12     // Sync word for LoRa communication
#define LORA_PREAMBLE_LENGTH 8  // Preamble length (in symbols)
#define LORA_TX_POWER 10        // Transmission power (in dBm)
#define LORA_RX_TIMEOUT 1000    // Receive timeout (in milliseconds)
// Function declarations
void resetNodeScores();
void onRecive();
bool isChannelClear();
void sendLoRaMsg(uint8_t *msg, size_t length);
void sendDiscoveryPacket(uint16_t nodeId, uint16_t seqNumber, uint16_t *neighbors, uint8_t neighborCount);
void sendTeamScoreUpdate(uint16_t reciverNodeId);
void teamScoreUpdateLoop();
void discoveryLoop();
void updateNodeScores(uint32_t nodeId, int teamId, int score);
void updateTotalTeamScores();
void updateDisplay();
void updateTeamScore(int teamId, int score);
void updateLocalTeamScore();
void changeLedColor(int teamId);

// Function definitions
void resetNodeScores()
{
  // Reset all scores stored in this node to 0
  for (int i = 0; i < NODE_SCORES_SIZE; i++)
  {
    nodeScores[i].nodeId = 0;
    nodeScores[i].teamScores[TEAM_BLUE].score = 0;
    nodeScores[i].teamScores[TEAM_YELLOW].score = 0;
  }
}

void onRecive()
{
  uint8_t receivedBuffer[256];             // Buffer to store received bytes
  size_t length = radio.getPacketLength(); // Get the length of the received packet
  int state = radio.readData(receivedBuffer, length);
  uint8_t type = receivedBuffer[0];
  
  if (type == WHO_IS_OUT_THERE)
  {
    DiscoveryMessage *packet = (DiscoveryMessage *)receivedBuffer;
    for (int i = 0; i < sizeof(nodesTable) / sizeof(nodesTable[0]); i++)
    {
      if (nodesTable[i].nodeId == 0)
      {
        nodesTable[i].nodeId = packet->nodeId;
        nodesTable[i].nextHop = packet->nodeId;
        nodesTable[i].lastSeen = millis();
        nodesTable[i].isLeader = packet->isLeader;
        nodesTable[i].HopCount = 1;
        break;
      }
    }
    //this is mostly for debugging purposes
    Serial.println("Node Table:");
    for (int i = 0; i < sizeof(nodesTable) / sizeof(nodesTable[0]); i++)
    {
      if (nodesTable[i].nodeId != 0)
      {
        Serial.print("Node ID: ");
        Serial.print(nodesTable[i].nodeId);
        Serial.print(", Next Hop: ");
        Serial.print(nodesTable[i].nextHop);
        Serial.print(", Last Seen: ");
        Serial.print(nodesTable[i].lastSeen);
        Serial.print(", Is Leader: ");
        Serial.print(nodesTable[i].isLeader);
        Serial.print(", Hop Count: ");
        Serial.println(nodesTable[i].HopCount);
      }
    }
  }
  if(type == SCORE_UPDATE)
  {
    StandardMessage *packet = (StandardMessage *)receivedBuffer;
    updateNodeScores(packet->nodeIdSender, TEAM_BLUE, packet->BlueScore);
    updateNodeScores(packet->nodeIdSender, TEAM_YELLOW, packet->YellowScore);
    Serial.print("Received score update from node: ");
    Serial.println(packet->nodeIdSender);
  }
}
// Listen for a very short time to check if channel is clear
bool isChannelClear() {

  radio.startReceive();

  delay(10);

  if (radio.available()) {
    return false; // Channel is busy
  }
  
  return true; // Channel is clear
}

void sendLoRaMsg(uint8_t *msg, size_t length)
{
 const int maxAttempts = 5;
 const int initialBackoff = 100; // ms
 int attempt = 0;
 bool sent = false;
 
 while (attempt < maxAttempts && !sent) {
   if (isChannelClear()) {
     // Channel is clear, attempt to send
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
     int backoffTime = initialBackoff * (1 << attempt); // 100, 200, 400, 800, 1600 ms
     delay(backoffTime);
   }
   
   attempt++;
 }
 
 if (!sent) {
   Serial.println("Failed to send message after maximum attempts");
 }
 
 radio.startReceive();  // Start listening again its important to start receiving again after sending a message
}

void sendDiscoveryPacket(uint16_t nodeId, uint16_t seqNumber, uint16_t *neighbors, uint8_t neighborCount)
{
  DiscoveryMessage msg;
  msg.type = WHO_IS_OUT_THERE;
  msg.nodeId = nodeId;
  msg.seqNumber = seqNumber;
  msg.neighborCount = neighborCount;
  if(isLeader)
  {
    msg.isLeader = 0x01;
  }
  else
  {
    msg.isLeader = 0x00;
  }
  memcpy(msg.neighbors, neighbors, sizeof(uint16_t) * neighborCount);
  size_t msgSize = sizeof(msg) - sizeof(msg.neighbors) + sizeof(uint16_t) * neighborCount;
  Serial.print("Sending discovery packet of size: ");
  Serial.println(msgSize);
  uint8_t *msgBuffer = (uint8_t *)&msg;
  sendLoRaMsg(msgBuffer, msgSize);
}

//sends teams score update to  node
void sendTeamScoreUpdate(uint16_t reciverNodeId)
{
  StandardMessage msg;
  msg.type = SCORE_UPDATE;
  msg.nodeIdSender = NodeId;
  msg.nodeIdReciver = reciverNodeId;
  msg.seqNumber = seqNumber++;
  msg.BlueScore = localTeamsScore[TEAM_BLUE].score;
  msg.YellowScore = localTeamsScore[TEAM_YELLOW].score;
  size_t msgSize = sizeof(msg);
  uint8_t *msgBuffer = (uint8_t *)&msg;
  sendLoRaMsg(msgBuffer, msgSize);
  Serial.print("Sending team score update to node: " + LeaderId);
}

//team score update loop
void teamScoreUpdateLoop()
{
  //check if the node is a leader if not send the score to the leader node
  if (isLeader)
  {
    for (int i = 0; i < sizeof(nodesTable) / sizeof(nodesTable[0]); i++)
    {
      if (nodesTable[i].nodeId != 0 && nodesTable[i].isLeader == true)
      {
        sendTeamScoreUpdate(nodesTable[i].nodeId);
        break;
      }
    }
  }
  else
  {
    sendTeamScoreUpdate(LeaderId);
  }
}

void discoveryLoop()
{
    uint16_t neighbors[5] = {};
    sendDiscoveryPacket(NodeId, seqNumber++, neighbors, sizeof(neighbors) / sizeof(neighbors[0]));
    Serial.println("Discovery packet sent.");
}

// Updates the scores of a specific node for a given team
void updateNodeScores(uint32_t nodeId, int teamId, int score)
{
  // Check if the node already exists in the list
  for (auto &node : nodeScores)
  {
    if (node.nodeId == nodeId)
    {
      if (teamId == TEAM_BLUE)
      {
        node.teamScores[TEAM_BLUE].score = score;
      }
      else if (teamId == TEAM_YELLOW)
      {
        node.teamScores[TEAM_YELLOW].score = score;
      }
      return;
    }
  }
  // If the node is not found, add it to the list
  for (auto &node : nodeScores)
  {
    if (node.nodeId == 0)
    {
      NodeTeamScores newNode;
      newNode.nodeId = nodeId;
      newNode.teamScores[TEAM_BLUE].score = 0;
      newNode.teamScores[TEAM_YELLOW].score = 0;
      node = newNode;
      return;
    }
  }
}

//count total points for each team
void updateTotalTeamScores()
{
  totalTeamsScore[TEAM_BLUE].score = 0;
  totalTeamsScore[TEAM_YELLOW].score = 0;

  for (auto &node : nodeScores)
  {
    if (node.nodeId != 0)
    {
      totalTeamsScore[TEAM_BLUE].score += node.teamScores[TEAM_BLUE].score;
      totalTeamsScore[TEAM_YELLOW].score += node.teamScores[TEAM_YELLOW].score;
    }
  }
}

void updateDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("Connected nodes: ");

  display.print("Game in progress: ");
  display.println(isGameInProgress ? "Yes" : "No");

  display.print("Game duration: ");
  display.println(gameDuration);

  display.println("Teams:");
  for (int i = 0; i < sizeof(totalTeamsScore) / sizeof(totalTeamsScore[0]); i++)
  {
    display.print("Team ");
    display.print(totalTeamsScore[i].id == TEAM_BLUE ? "Blue" : "Yellow");
    display.print(": ");
    display.println(totalTeamsScore[i].score);
  }

  display.display();
}

void updateTeamScore(int teamId, int score)
{
  for (int i = 0; i < sizeof(localTeamsScore) / sizeof(localTeamsScore[0]); i++)
  {
    if (localTeamsScore[i].id == teamId)
    {
      localTeamsScore[i].score += score;
      break;
    }
  }
}
//update the team score
void updateLocalTeamScore()
{
  if (pointControlledByTeam == TEAM_BLUE)
  {
    updateTeamScore(TEAM_BLUE, 1);
  }
  else if (pointControlledByTeam == TEAM_YELLOW)
  {
    updateTeamScore(TEAM_YELLOW, 1);
  }
}

void changeLedColor(int teamId)
{
  if (teamId == TEAM_BLUE)
  {
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_BLUE, HIGH);
  }
  else if (teamId == TEAM_YELLOW)
  {
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_YELLOW, HIGH);
  }
  else
  {
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_YELLOW, LOW);
  }
}

// Setup and loop
void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  setNodeId();

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.display();
  delay(1000);

  screenUpdateTicker.attach(1, updateDisplay);
  screenUpdateTicker.active();
  // teamScoreTicker.attach(5, updateTeamScoreTest);
  // teamScoreTicker.active();
  nodeScoreUpdateTicker.attach(30, discoveryLoop); // 30 seconds is completly arbitrary. I'll worry about it later
  nodeScoreUpdateTicker.active();
  totalScoreTicker.attach(3, updateTotalTeamScores);
  totalScoreTicker.active();

  teamBlueButton.setDebounceTime(debounce);
  teamYellowButton.setDebounceTime(debounce);
  startGameButton.setDebounceTime(debounce);

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  Serial.println("Mesh initialized.");
  Serial.print("Node ID: ");
  Serial.begin(115200);

  int state = radio.begin(433.0);
  radio.setSyncWord(LORA_SYNC_WORD);
  radio.setPreambleLength(LORA_PREAMBLE_LENGTH);
  radio.setOutputPower(LORA_TX_POWER);
  radio.setFrequency(LORA_BAND);
  radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
  radio.setBandwidth(LORA_BANDWIDTH);
  radio.setCodingRate(LORA_CODING_RATE);

  radio.setDio0Action(onRecive, RISING);
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

void loop()
{
  teamBlueButton.loop();
  teamYellowButton.loop();
  startGameButton.loop();

  if (startGameButton.isPressed())
  {
    isGameInProgress = !isGameInProgress;
    Serial.println(isGameInProgress ? "Game started." : "Game stopped.");
  }

  if (teamYellowButton.isPressed())
  {
    pointControlledByTeam = TEAM_YELLOW;
    Serial.println("Team Yellow button pressed. Score updated.");
    changeLedColor(TEAM_YELLOW);
  }

  if (teamBlueButton.isPressed())
  {
    pointControlledByTeam = TEAM_BLUE;
    Serial.println("Team Blue button pressed. Score updated.");
    changeLedColor(TEAM_BLUE);
  }
}
