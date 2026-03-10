#include "LCDScreen.h"
#include <Wire.h>

LCDScreen::LCDScreen()
{
    // Do not initialize hardware in the constructor. Call begin() from setup().
}

void LCDScreen::begin(int id,int width,int height)
{
    lcd = LiquidCrystal_I2C(id,width,height);
    Wire.begin(SDA_PIN, SCL_PIN);
    lcd.init();
    lcd.backlight();
}

void LCDScreen::kothDisplayScore(int yellowScore, int blueScore)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Y: ");
    lcd.print(yellowScore);
    lcd.print("|");
    lcd.print(" B: ");
    lcd.print(blueScore);
}

void LCDScreen::kothDisplayCapturing(Team capturingTeam, float progress)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CAP: ");
    lcd.print(capturingTeam == Team::YELLOW ? "YELLOW" : "BLUE");
    lcd.setCursor(0, 1);
    lcd.print("PROGRESS: ");
    lcd.print(progress * 100, 0);
    lcd.print("%");
}

void LCDScreen::kothDisplayController(Team controller)
{
    lcd.setCursor(0, 1);
    lcd.print("CTRL: ");
    if (controller == Team::NONE)
    {
        lcd.print("NONE");
    }
    else
    {
        lcd.print(controller == Team::YELLOW ? "YELLOW" : "BLUE");
    }
}

void LCDScreen::kothDisplayEnd(Team winner, int yellowScore, int blueScore, bool isDraw)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    if (isDraw)
    {
        lcd.print("GAME OVER: DRAW");
    }
    else
    {
        lcd.print("WINNER: ");
        lcd.print(winner == Team::YELLOW ? "YELLOW" : "BLUE");
    }
    lcd.setCursor(0, 1);
    lcd.print("Y: ");
    lcd.print(yellowScore);
    lcd.print("|");
    lcd.print(" B: ");
    lcd.print(blueScore);
}

void LCDScreen::flagDisplayController(FlagTeam controller)
{
    lcd.setCursor(0, 1);
    lcd.print("CTRL: ");
    if (controller == FlagTeam::NONE)
    {
        lcd.print("NONE");
    }
    else
    {
        lcd.print(getFlagTeamName(controller));
    }
}

void LCDScreen::flagDisplayScore(uint16_t score)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("points ");
    lcd.print(score);
}

void LCDScreen::flagDisplayCapturin(FlagTeam capturingTeam, float progress)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CAP: ");
    lcd.print(getFlagTeamName(capturingTeam));
    lcd.setCursor(0, 1);
    lcd.print("PROGRESS: ");
    lcd.print(progress * 100, 0);
    lcd.print("%");
}

void LCDScreen::flagDisplayEnd(FlagTeam winner)
{
    lcd.setCursor(0, 1);
    lcd.print("WINNER: ");
    lcd.print(getFlagTeamName(winner));
}

void LCDScreen::displayText(const char *text, int line)
{
    lcd.setCursor(0, line);
    lcd.print(text);
}

void LCDScreen::clearScreen()
{
    lcd.clear();
}
