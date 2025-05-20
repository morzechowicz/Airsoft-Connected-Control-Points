#ifndef DISPLAY_OLED_H // Header guard to prevent multiple inclusions
#define DISPLAY_OLED_H

#include <Display.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "ControlPoint.h"

// OLED Settings
#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_ADDR 0x3C

class DisplayOled : public Display
{
public:
    DisplayOled();
    void begin();
    void displaySettings(Config config,int configState) override;
    void displayCountdown(int countdown) override;
    void displayGame(ControlPoint controlPoint, int timeLeft) override;
    void displayCapturing(TeamId capturingTeam, float progres) override;
    void displayFinished(TeamId winner, ControlPoint controlPoint) override;

private:
    Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
};

#endif