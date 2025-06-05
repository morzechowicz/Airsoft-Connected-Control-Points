#include "DisplayLCD.h"

DisplayLCD::DisplayLCD()
{
    // Constructor implementation
}

void DisplayLCD::begin()
{
    Wire.begin(OLED_SDA, OLED_SCL);
    display.init();
    display.backlight();
    lcdCk.start();
}

void DisplayLCD::lcdLoop()
{
    if (lcdCk.getElapsedTime() > 100)
    {
        display.clear();
        lcdCk.reset();
    }
}

void DisplayLCD::displaySettings(Config config, int configState)
{

    display.setCursor(0, 0);
    display.print("Settings:");
    display.setCursor(0, 1);
    if (configState == 0)
    {
        display.print("Cntdwn(sec):");
        display.print(config.getCountdown());
    }
    if (configState == 1)
    {
        display.print("Dur(min):");
        display.print(config.getDurration());
    }
    if (configState == 2)
    {
        display.print("TrgtPt:");
        display.print(config.getPointsTarget());
    }
    if (configState == 3)
    {
        display.print("CapTime(sec):");
        display.print(config.getCaptureTime());
    }
}

void DisplayLCD::displayCountdown(int countdown)
{
    display.setCursor(0, 0);
    display.print("Countdown:");
    display.setCursor(0, 1);
    display.print(countdown);
}

void DisplayLCD::displayGame(ControlPoint controlPoint, int timeLeft)
{
    display.setCursor(0, 0);
    display.print("T:");
    display.print(timeLeft);
    display.print(" min");
    display.setCursor(0, 1);
    display.print("B:");
    display.print(controlPoint.getTeamPoints(TeamId::Blufor));
    display.print(" | Y:");
    display.print(controlPoint.getTeamPoints(TeamId::YellowFor));
}

void DisplayLCD::displayCapturing(TeamId capturingTeam, float progres)
{

    display.setCursor(0, 0);
    display.print("Team ");
    display.print((capturingTeam == TeamId::Blufor) ? "Blue" : "Yellow");
    display.setCursor(0, 1);
    int numOfHashes = (progres * 16);
    if (progres == 1)
    {
        display.print("Captured!");
    }
    else
    {
        for (int i = 0; i < numOfHashes; i++)
        {
            display.print("#");
        }
    }
}

void DisplayLCD::displayFinished(TeamId winner, ControlPoint controlPoint)
{
    display.setCursor(0, 0);
    display.print(winner == TeamId::Blufor ? "Blue WON" : winner == TeamId::YellowFor ? "Yellow WON"
                                                                                      : "Draw");
    display.setCursor(0, 1);
    display.print("B: ");
    display.print(controlPoint.getTeamPoints(TeamId::Blufor));
    display.print(" | Y: ");
    display.print(controlPoint.getTeamPoints(TeamId::YellowFor));
}

void DisplayLCD::displayNetworkStatus(int nodesCount, bool leaderStatus, bool connecting, String &lastLoraMsg)
{
    display.setCursor(0, 0);
    display.print("Nodes: ");
    display.print(nodesCount);
    display.print(" ");
    if (leaderStatus)
    {
        display.print("GM");
    }
    else
    {
        display.print(" ");
    }
}

void DisplayLCD::displayInitLogo()
{
    display.clear();
    display.setCursor(4, 0);
    display.print("SPAS");
    display.setCursor(0, 1);
    display.print("GAME SYSTEM");
}