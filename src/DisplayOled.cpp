#include <DisplayOled.h>

DisplayOled::DisplayOled()
{
}

void DisplayOled::begin()
{
    Wire.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.setTextColor(SSD1306_WHITE);
}

void DisplayOled::displaySettings(Config config, int configState)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Settings:");
    display.print("Countdown(sec):");
    display.print(config.getCountdown());
    display.println(configState == 0 ? "<--" : "");
    display.print("Duration(min):");
    display.print(config.getDurration());
    display.println(configState == 1 ? "<--" : "");
    display.print("Target Points:");
    display.print(config.getPointsTarget());
    display.println(configState == 2 ? "<--" : "");
    display.print("Cap Time(sec):");
    display.print(config.getCaptureTime());
    display.println(configState == 3 ? "<--" : "");
    display.display();
}

void DisplayOled::displayCountdown(int countdown)
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print("Countdown:");
    display.setTextSize(3);
    display.setCursor(0, 20);
    display.print(countdown);
    display.display();
}

void DisplayOled::displayGame(ControlPoint controlPoint, int timeLeft)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Controlled by:");
    switch (controlPoint.getControllingTeam())
    {
    case TeamId::Blufor:
        display.println("Blue");
        break;
    case TeamId::YellowFor:
        display.println("Yellow");
        break;
    default:
        display.println("None");
        break;
    }
    display.print("Time left:");
    display.print(timeLeft);
    display.println(" min");
    display.print("Blue:");
    display.print(controlPoint.getTeamPoints(TeamId::Blufor));
    display.print(" | Yellow:");
    display.print(controlPoint.getTeamPoints(TeamId::YellowFor));
    display.display();
}

void DisplayOled::displayCapturing(TeamId capturingTeam, float progres)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Team ");
    display.print((capturingTeam == TeamId::Blufor) ? "Blue" : "Yellow");
    display.println(" is capturing!");
    display.print("Captured:");
    display.println("");
    int numOfHashes = (progres * 21);
    for (int i = 0; i < numOfHashes; i++)
    {
        display.print("#");
    }
    if (progres == 1)
    {
        display.println("Captured!");
    }
    display.display();
}

void DisplayOled::displayFinished(TeamId winner, ControlPoint ControlPoint)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Game Over!");
    display.println(winner == TeamId::Blufor ? "Blue WON" : winner == TeamId::YellowFor ? "Yellow WON"
                                                                                        : "Draw");
    display.println("Final Score:");
    display.print("Blue: ");
    display.print(ControlPoint.getTeamPoints(TeamId::Blufor));
    display.print(" | Yellow: ");
    display.print(ControlPoint.getTeamPoints(TeamId::YellowFor));
    display.display();
}

void DisplayOled::displayNetworkStatus(int nodesCount, bool leaderStatus, bool connecting, String &lastLoraMsg)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Network Status:");
    display.print("Nodes: ");
    display.print(nodesCount);
    display.println("");
    if (leaderStatus)
    {
        display.println("GameMaster");
    }
    else
    {
        display.println("Not GameMaster");
    }
    if(connecting)
    {
        display.println("information has been sent");
    }
    display.print("Last Lora Msg: ");
    display.println(lastLoraMsg);
    display.display();
}

void DisplayOled::displayInitLogo()
{
    //yes is looks bad, i figure out something better one day
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("SPAS");
    display.println("ENGI");
    display.println("DIV");
    display.display();
}