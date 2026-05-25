#ifndef LCDSCREEN_H
#define LCDSCREEN_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "../Config.h"
#include <GameComponents/KOTH/KOTHTypes.h>
#include <GameComponents/FLAG/FLAGTypes.h>
#include "../lib/Logging/LogManager.h"

enum class DisplayPriority : uint8_t
{
    LOW_PR = 0,
    MEDIUM_PR = 1,
    HIGH_PR = 2,
};

struct LcdDisplayMessage
{
    DisplayPriority priority = DisplayPriority::LOW_PR; // default should be like this
    int durationMs = 0;                                 // 0 = persistent, >0 = transient
    char lines[4][21];

    void setLine(int line, const char *text)
    {
        if (line < 0 || line > 3)
            return;
        strncpy(lines[line], text, 20);
        lines[line][20] = '\0';
    }
    void clearLine(int line) { setLine(line, "                    "); }
};

class LCDScreen
{
private:
    LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
    xQueueHandle displayQueue;
    xTaskHandle displayTask;

    const char teamChar(Team t)
    {
        switch (t)
        {
        case Team::YELLOW:
            return 'Y';
        case Team::BLUE:
            return 'B';
        default:
            return 'N';
        }
    }

public:
    LCDScreen();

    // Initialize I2C and LCD hardware. Call from `setup()` (after Serial.begin()).
    void begin(int id, int width, int height);
    void runDisplayTask();
    void startDisplayTask();

    // koth specific methods
    void kothDisplayScore(int yellowScore, int blueScore);
    void kothDisplayCapturing(Team capturingTeam);
    void kothDisplayCapturingProgress(float progress);
    void kothDisplayController(Team controller);
    void kothDisplayEnd(Team winner, int yellowScore, int blueScore, bool isDraw);
    void kothDisplayInformation(NodeState lastKnownNodeStates[], int gameTime, int durration, KOTHGameScore lastKnownScore, int nodeCount);

    // flag specifiv
    void flagDisplayController(FlagTeam controller);
    void flagDisplayScore(uint16_t score);
    void flagDisplayCapturin(FlagTeam capturingTeam, float progress);
    void flagDisplayEnd(FlagTeam winner);

    // universal
    void displayLogo();
    void displayPause();
    void displayRespawn();
    void displayCountdown(int count);
    void displayText(LcdDisplayMessage msg);
    void render(LcdDisplayMessage content);
    void addToQueue(LcdDisplayMessage msg);

    String buildRow(int startIdx, int count, int totalNodes, NodeState lastKnownNodeStates[]);
};

#endif // LCDSCREEN_H