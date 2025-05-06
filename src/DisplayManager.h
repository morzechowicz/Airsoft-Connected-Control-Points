#ifndef DISPLAY_MANAGER_H  // Header guard to prevent multiple inclusions
#define DISPLAY_MANAGER_H

#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <GameManager.h>

// OLED Settings
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

class DisplayManager
{
public:
    Adafruit_SSD1306 display;
    DisplayManager();
    void initialize();
    void updateDisplay(bool isGameInProgress, uint16_t NodeId, uint16_t LeaderId, GameManager::Team *totalTeamsScore);
};

#endif 