#ifndef LCDSCREEN_H
#define LCDSCREEN_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "../Config.h"
#include <GameComponents/KOTH/KOTHTypes.h>
#include <GameComponents/FLAG/FLAGTypes.h>

class LCDScreen
{
private:
    LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2); 
    
public:
    LCDScreen();

    // Initialize I2C and LCD hardware. Call from `setup()` (after Serial.begin()).
    void begin(int id,int width,int height);

    //koth specific methods
    void kothDisplayScore(int yellowScore, int blueScore);
    void kothDisplayCapturing(Team capturingTeam, float progress);
    void kothDisplayController(Team controller);
    void kothDisplayEnd(Team winner,int yellowScore, int blueScore, bool isDraw);
    
    //flag specifiv
    void flagDisplayController(FlagTeam controller);
    void flagDisplayScore(uint16_t score);
    void flagDisplayCapturin(FlagTeam capturingTeam,float progress);
    void flagDisplayEnd(FlagTeam winner);

    //universal
    void displayPause();

    void displayText(const char* text,int line);
    void clearScreen();
    void setCursor(int col, int row);
};

#endif // LCDSCREEN_H