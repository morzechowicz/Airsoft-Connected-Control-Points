#ifndef DISPLAY_LCD_H  // Header guard to prevent multiple inclusions
#define DISPLAY_LCD_H

#include <Display.h>
#include "ControlPoint.h"
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Clocker.h>

#define OLED_SDA 21
#define OLED_SCL 22

class DisplayLCD : public Display
{
public:
    DisplayLCD();
    void begin();
    void lcdLoop();
    void displaySettings(Config config, int configState) override;
    void displayCountdown(int countdown) override;
    void displayGame(ControlPoint controlPoint, int timeLeft) override;
    void displayCapturing(TeamId capturingTeam, float progres) override;
    void displayFinished(TeamId winner, ControlPoint controlPoint) override;
    void displayNetworkStatus(int nodesCount, bool leaderStatus, bool connecting, String &lastLoraMsg) override;
    void displayInitLogo() override;
private:
    LiquidCrystal_I2C display = LiquidCrystal_I2C(0x27,16,2);
    Clocker lcdCk;
};

#endif