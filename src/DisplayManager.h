#ifndef DISPLAY_MANAGER_H  // Header guard to prevent multiple inclusions
#define DISPLAY_MANAGER_H

#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// OLED Settings
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// LCD Settings
#define LCD_ADDR 0x27

class DisplayManager
{
public:
    Adafruit_SSD1306 displayOLED;
    LiquidCrystal_I2C displayLCD;
    DisplayManager();
    void initialize();
    void settingDisplayOLED(int currentSetting,int maxScore, int maxTime, int timeToStart, int timeToCapture);

    void countdownDisplayOled(int timeToStrart);
    void countdownDisplayLCD(int timeToStrart);

    void gameDisplayOLED(int isGameInProgress, uint16_t NodeId, uint16_t LeaderId, int blueScore, int yellowScore,int time);
    void gameDisplayLCD(int blueScore,int yellowScore); 

    void EndDisplayOLED(int blueScore, int yellowScore);
    void EndDisplayLCD(int blueScore, int yellowScore);

private:

};

#endif 