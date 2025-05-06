#include <DisplayManager.h>

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

void DisplayManager::initialize()
{
    Wire.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.display();
    delay(1000);
}

void DisplayManager::updateDisplay(bool isGameInProgress, uint16_t NodeId, uint16_t LeaderId, GameManager::Team *totalTeamsScore)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.print("Game in progress: ");
    display.println(isGameInProgress ? "Yes" : "No");

    display.println("Teams:");
    for (int i = 0; i < 2; i++)
    {
        display.print("Team ");
        display.print(totalTeamsScore[i].id == TEAM_BLUE ? "Blue" : "Yellow");
        display.print(": ");
        display.println(totalTeamsScore[i].score);
    }

    display.print("Node Id: ");
    display.println(NodeId);
    display.print("Leader Id: ");
    display.println(LeaderId);
    display.display();
};
