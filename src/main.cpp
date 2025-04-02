#include <map>
#include <ArduinoJson.h>
#include <painlessMesh.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ezButton.h>

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

// Mesh Settings
#define MESH_PREFIX "SPASLORAMESH"
#define MESH_PASSWORD "SPASRATSSTAR"
#define MESH_PORT 2137
#define LORA_FREQ 433E6 // 433E6 (Asia/EU),  868E6 (EU), 915E6 (US)
#define LORA_SF 12      // Spreading Factor (7-12, higher = longer range but slower)
#define LORA_BW 125E3   // Bandwidth (125 kHz is common)

// buttons
#define debounce 250 // 250ms debounce time

ezButton teamYellowButton(0); // GPIO 0 (IO0)
ezButton teamBlueButton(4);   // GPIO 4 (IO4)
ezButton startGameButton(2);  // GPIO 2 (IO2)

// leds
#define LED_YELLOW 13 // GPIO 13 (IO15)
#define LED_BLUE 12   // GPIO 12 (IO13)

// system variables
painlessMesh mesh;
Ticker teamScoreTicker;
Ticker screenUpdateTicker;
Ticker nodeScoreUpdateTicker;
Ticker totalScoreTicker;

// game variables
bool isGameInProgress = false;
int gameDuration = 300;         // 5 minutes in seconds
int pointControlledByTeam = 99; // 99 = none, 1 = blue, 2 = yellow

// team variables
struct Team
{
  int id;
  int score;
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
  Team teamScores[2]; // Index 0 for TEAM_BLUE, Index 1 for TEAM_YELLOW
};

NodeTeamScores nodeScores[10];

// set all node scores and nodeid to 0
void resetNodeScores()
{
  for (int i = 0; i < sizeof(nodeScores) / sizeof(nodeScores[0]); i++)
  {
    nodeScores[i].nodeId = 0;
    nodeScores[i].teamScores[TEAM_BLUE].score = 0;   // TEAM_BLUE
    nodeScores[i].teamScores[TEAM_YELLOW].score = 0; // TEAM_YELLOW
  }
}

void updateNodeTeamScore(uint32_t nodeId, int teamId, int score)
{
  for (auto &node : nodeScores)
  { // Iterate through the nodeScores array
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
  // find empty slot in nodeScores array
  for (auto &node : nodeScores)
  {
    if (node.nodeId == 0)
    { // Assuming 0 means uninitialized
      NodeTeamScores newNode;
      newNode.nodeId = nodeId;
      newNode.teamScores[TEAM_BLUE].score = 0;
      newNode.teamScores[TEAM_YELLOW].score = 0;
      node = newNode; // Assign the new node to the empty slot
      return;
    }
  }
}

// sum all node scores and update total team scores
void updateTotalTeamScores()
{
  totalTeamsScore[TEAM_BLUE].score = 0;
  totalTeamsScore[TEAM_YELLOW].score = 0;

  for (auto &node : nodeScores)
  {
    if (node.nodeId != 0) // Check if the node is initialized
    {
      totalTeamsScore[TEAM_BLUE].score += node.teamScores[TEAM_BLUE].score;
      totalTeamsScore[TEAM_YELLOW].score += node.teamScores[TEAM_YELLOW].score;
    }
  }
}

// send node scores to all nodes
void broadcastCurrentNodeScore()
{
  if (isGameInProgress)
  {
    JsonDocument doc;
    doc["id"] = mesh.getNodeId();
    doc["tB"] = localTeamsScore[0].score;
    doc["tY"] = localTeamsScore[1].score;

    String jsonString;
    serializeJson(doc, jsonString);
    if (mesh.sendBroadcast(jsonString, true))
    {
      Serial.println("Broadcasting current node scores...");
    }
    else
    {
      Serial.println("Failed to broadcast current node scores.");
    }
  }
}

// receive node scores from all nodes
void receivedCallback(uint32_t from, String &msg)
{
  Serial.printf("Received from %u: %s\n", from, msg.c_str());

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, msg);

  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  uint32_t nodeId = doc["id"];
  int teamBlueScore = doc["tB"];
  int teamYellowScore = doc["tY"];

  updateNodeTeamScore(nodeId, TEAM_BLUE, teamBlueScore);
  updateNodeTeamScore(nodeId, TEAM_YELLOW, teamYellowScore);
}

void updateDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("Connected nodes: ");
  display.println(mesh.getNodeList().size());

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

// TESTING TEAM SCORE UPDATE
void updateTeamScoreTest()
{

  if (pointControlledByTeam == TEAM_BLUE)
  {
    updateTeamScore(TEAM_BLUE, 1); // Increment Blue team score by 1
  }
  else if (pointControlledByTeam == TEAM_YELLOW)
  {
    updateTeamScore(TEAM_YELLOW, 1); // Increment Yellow team score by 1
  }
}

// change led color based on team
void changeLedColor(int teamId)
{
  if (teamId == TEAM_BLUE)
  {
    digitalWrite(LED_YELLOW, LOW); // Turn off yellow LED
    digitalWrite(LED_BLUE, HIGH);  // Turn on blue LED
  }
  else if (teamId == TEAM_YELLOW)
  {
    digitalWrite(LED_BLUE, LOW);    // Turn off blue LED
    digitalWrite(LED_YELLOW, HIGH); // Turn on yellow LED
  }
  else
  {
    digitalWrite(LED_BLUE, LOW);   // Turn off blue LED
    digitalWrite(LED_YELLOW, LOW); // Turn off yellow LED
  }
}

void setup()
{
  // add serial monitor for debugging
  Serial.begin(115200);
  Serial.println("Starting...");

  // Initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL); // SDA=GPIO4, SCL=GPIO15 (change pins if needed)
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.display();
  delay(1000);

  // Initialize Mesh
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection([](uint32_t nodeId)
                       { Serial.printf("New Connection: %u\n", nodeId); });
  mesh.onDroppedConnection([](uint32_t nodeId)
                           { Serial.printf("Dropped Connection: %u\n", nodeId); });

  // Tickers setup
  screenUpdateTicker.attach(1, updateDisplay);                // Call updateDisplay every second
  screenUpdateTicker.active();                                // Start the ticker
  teamScoreTicker.attach(5, updateTeamScoreTest);             // Call updateTeamScoreTest every second
  teamScoreTicker.active();                                   // Activate the ticker
  nodeScoreUpdateTicker.attach(5, broadcastCurrentNodeScore); // Call broadcastCurrentNodeScore every second
  nodeScoreUpdateTicker.active();                             // Activate the ticker
  totalScoreTicker.attach(3, updateTotalTeamScores);          // Call updateTotalTeamScores every second
  totalScoreTicker.active();                                  // Activate the ticker

  // button setup
  teamBlueButton.setDebounceTime(debounce);   // Set debounce time to 50 milliseconds
  teamYellowButton.setDebounceTime(debounce); // Set debounce time to 50 milliseconds
  startGameButton.setDebounceTime(debounce);  // Set debounce time to 50 milliseconds

  // Initialize LEDs
  pinMode(LED_YELLOW, OUTPUT); // Set LED pin as output
  pinMode(LED_BLUE, OUTPUT);   // Set LED pin as output

  Serial.println("Mesh initialized.");
  Serial.print("Node ID: ");
  Serial.println(mesh.getNodeId());
}

// Main loop
void loop()
{
  mesh.update();
  teamBlueButton.loop();
  teamYellowButton.loop();
  startGameButton.loop();
  // Check if the start game button is pressed
  if (startGameButton.isPressed())
  {
    isGameInProgress = !isGameInProgress; // Toggle game state
    Serial.println(isGameInProgress ? "Game started." : "Game stopped.");
  }

  if (teamYellowButton.isPressed())
  {
    pointControlledByTeam = TEAM_YELLOW; // Set point controlled by Yellow team
    Serial.println("Team Yellow button pressed. Score updated.");
    changeLedColor(TEAM_YELLOW); // Change LED color to Yellow
  }

  if (teamBlueButton.isPressed())
  {
    pointControlledByTeam = TEAM_BLUE; // Set point controlled by Blue team
    Serial.println("Team Blue button pressed. Score updated.");
    changeLedColor(TEAM_BLUE); // Change LED color to Blue
  }
}