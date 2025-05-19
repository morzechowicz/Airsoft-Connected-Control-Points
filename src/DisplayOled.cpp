#include <DisplayOled.h>

void DisplayOled::displaySettings(Config config)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Settings:");
    display.println("Duration(min): ");
    display.print(config.getDurration());
    display.println("Countdown(sec): ");
    display.print(config.getCountdown());
    display.println("Capturing Time(sec):");
    display.print(config.getCaptureTime());
    display.println("Target Points:");
    display.print(config.getPointsTarget());
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

void DisplayOled::displayGame(ControlPoint controlPoint)
{
    display.clearDisplay();
    display.println("Controlled by:");
    switch (controlPoint.getControllingTeam())
    {
        case TeamId::Blufor:
            display.print("Blue");
            break;
        case TeamId::YellowFor:
            display.print("Yellow");
            break;
    default:
        display.print("None");
        break;
    }
    display.println("Time left:");
    display.print("Later"); //add later
    display.println("Blue:");
    display.print(controlPoint.getTeamPoints(TeamId::Blufor));
    display.print(" | Yellow:");
    display.print(controlPoint.getTeamPoints(TeamId::YellowFor));
    display.display();
}

void DisplayOled::displayCapturing(TeamId capturingTeam, int progres)
{
    display.clearDisplay();
    display.println("Team ");
    display.print((capturingTeam == TeamId::Blufor) ? "Blue" : "Yellow");
    display.println(" is capturing!");
    display.print("Captured:");
    display.println("");
    int numOfHashes = (progres / 21) * 100;
    for(int i = 0; i < numOfHashes; i++)
    {
        display.print("#");
    }
    display.display();
}

void DisplayOled::displayFinished(TeamId winner, ControlPoint ControlPoint)
{
    display.clearDisplay();
    display.println("Game Over!");
    display.println(winner == TeamId::Blufor ? "Blue WON" : winner == TeamId::YellowFor ? "Yellow WON" : "Draw");
    display.println("Final Score:");
    display.print("Blue: ");
    display.print(ControlPoint.getTeamPoints(TeamId::Blufor));
    display.print(" | Yellow: ");
    display.print(ControlPoint.getTeamPoints(TeamId::YellowFor));
    display.display();
}