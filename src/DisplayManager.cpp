#include <DisplayManager.h>

DisplayManager::DisplayManager() : displayOLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1), displayLCD(LCD_ADDR, 16, 2) {}

void DisplayManager::initialize()
{
    Wire.begin(OLED_SDA, OLED_SCL);
    displayOLED.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    displayOLED.display();

    Wire.begin(OLED_SDA, OLED_SCL);
    displayLCD.begin(16, 2);
    displayLCD.clear();
    displayLCD.home();
    displayLCD.print("Initializing...");
    delay(2000);
    displayLCD.clear();
    displayLCD.home();
    displayLCD.print("GET READY");
    displayLCD.backlight();
}

void DisplayManager::idleDisplayLCD()
{
    displayLCD.clear();
    displayLCD.setCursor(0, 0);
    displayLCD.print("GET READY");
    displayLCD.setCursor(0, 1);

    static unsigned long lastUpdate = 0;
    static int dotCount = 1;
    unsigned long now = millis();
    if (now - lastUpdate > 500) {
        lastUpdate = now;
        dotCount = dotCount % 3 + 1; // cycles 1,2,3
    }
    String msg = "Waiting for init";
    for (int i = 0; i < dotCount; ++i) msg += ".";
    displayLCD.print(msg);
}

void DisplayManager::settingDisplayOLED(int currentSetting, int maxScore, int maxTime, int timeToStart, int timeToCapture)
{
    displayOLED.clearDisplay();
    displayOLED.setTextSize(1);
    displayOLED.setTextColor(SSD1306_WHITE);
    displayOLED.setCursor(0, 0);

    displayOLED.print("Current Setting: ");
    displayOLED.println(currentSetting);
    displayOLED.print("Max Score: ");
    if ((currentSetting == 1 && ((millis() / 500) % 2 == 0)) || currentSetting != 1)
        displayOLED.println(maxScore);
    else
        displayOLED.println("");
    displayOLED.print("Max Time: ");
    if ((currentSetting == 2 && ((millis() / 500) % 2 == 0)) || currentSetting != 2)
        displayOLED.println(maxTime);
    else
        displayOLED.println("");
    displayOLED.print("Time to Start: ");
    if ((currentSetting == 3 && ((millis() / 500) % 2 == 0)) || currentSetting != 3)
        displayOLED.println(timeToStart);
    else
        displayOLED.println("");
    displayOLED.print("Time to Capture: ");
    if ((currentSetting == 4 && ((millis() / 500) % 2 == 0)) || currentSetting != 4)
        displayOLED.println(timeToCapture);
    else
        displayOLED.println("");
    displayOLED.display();
}

void DisplayManager::gameDisplayOLED(int isGameInProgress, uint16_t NodeId, uint16_t LeaderId, int blueScore, int yellowScore,int time)
{
    displayOLED.clearDisplay();
    displayOLED.setTextSize(1);
    displayOLED.setTextColor(SSD1306_WHITE);
    displayOLED.setCursor(0, 0);

    displayOLED.print("Game in progress: ");
    displayOLED.println(isGameInProgress == 2 ? "Yes" : "No");

    displayOLED.println("Teams:");
    displayOLED.print("Blue: ");
    displayOLED.println(blueScore);
    displayOLED.print("Yellow: ");
    displayOLED.println(yellowScore);
    displayOLED.print("Node Id: ");
    displayOLED.println(NodeId);
    displayOLED.print("Leader Id: ");
    displayOLED.println(LeaderId);
    displayOLED.print("Time: ");
    displayOLED.println(time);
    displayOLED.display();
};

void DisplayManager::gameDisplayLCD(int blueScore, int yellowScore)
{
    displayLCD.clear();
    displayLCD.setCursor(0, 0);
    displayLCD.print("Blue: ");
    displayLCD.print(blueScore);
    displayLCD.setCursor(0, 1);
    displayLCD.print("Yellow: ");
    displayLCD.print(yellowScore);
}

void DisplayManager::countdownDisplayOled(int timeToStart)
{
    displayOLED.clearDisplay();
    displayOLED.setTextSize(1);
    displayOLED.setTextColor(SSD1306_WHITE);
    displayOLED.setCursor(0, 0);
    displayOLED.print("Starting in: ");
    displayOLED.print(timeToStart);
    displayOLED.display();
}

void DisplayManager::countdownDisplayLCD(int timeToStart)
{
    displayLCD.clear();
    displayLCD.setCursor(0, 0);
    displayLCD.print("Starting in: ");
    displayLCD.print(timeToStart);
}

void DisplayManager::endDisplayOLED(int blueScore, int yellowScore,int winner)
{
    displayOLED.clearDisplay();
    displayOLED.setTextSize(1);
    displayOLED.setTextColor(SSD1306_WHITE);
    displayOLED.setCursor(0, 0);
    displayOLED.print("Game Over");
    displayOLED.setCursor(0, 10);
    displayOLED.print("Blue: ");
    displayOLED.println(blueScore);
    displayOLED.setCursor(0, 20);
    displayOLED.print("Yellow: ");
    displayOLED.println(yellowScore);
    displayOLED.display();
}

void DisplayManager::endDisplayLCD(int blueScore, int yellowScore,int winner)
{
    displayLCD.clear();
    displayLCD.setCursor(0, 0);
    if(winner == 0)
    {
        displayLCD.print("BLUE TEAM WON");
    }else{
        displayLCD.print("YELLOW TEAM WON");
    }
    displayLCD.setCursor(0, 1);
    displayLCD.print("B: ");
    displayLCD.print(blueScore);
    displayLCD.print("Y: ");
    displayLCD.print(yellowScore);
}