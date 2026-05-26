#include "LCDScreen.h"
#include <Wire.h>

// TODO: rework direct displayLine calls to use LcdDisplayMessage queue
// see LCDScreen::showScore, showPause, showGameOver for the pattern
// kill all tasks on game over, reset LcdDisplay priority on new game

LCDScreen::LCDScreen()
{
    // Do not initialize hardware in the constructor. Call begin() from setup().
}

void LCDScreen::runDisplayTask()
{
    LcdDisplayMessage current;        // what's showing now
    LcdDisplayMessage lastPersistent; // fallback after transients expire
    bool hasContent = false;

    LcdDisplayMessage incoming;
    for (;;)
    {
        TickType_t waitTime = hasContent && current.durationMs > 0
                                  ? pdMS_TO_TICKS(current.durationMs)
                                  : portMAX_DELAY;

        if (xQueueReceive(displayQueue, &incoming, waitTime) == pdTRUE)
        {
            if (incoming.priority >= current.priority || !hasContent)
            {
                current = incoming;
                hasContent = true;
                if (incoming.durationMs == 0)
                {
                    lastPersistent = incoming; // remember for fallback
                }
                render(current);
            }
            // else discard — lower priority than what's showing
        }
        else
        {
            // queue timed out — transient expired, restore persistent
            if (current.durationMs > 0)
            {
                current = lastPersistent;
                current.durationMs = 0;
                render(current);
            }
        }
    }
}

void LCDScreen::startDisplayTask()
{
    xTaskCreate(
        [](void *param)
        { static_cast<LCDScreen *>(param)->runDisplayTask(); },
        "LCDDisplay", 4096, this, 1, &displayTask);
}

void LCDScreen::begin(int id, int width, int height)
{
    lcd = LiquidCrystal_I2C(id, width, height);
    lcd.init();
    lcd.backlight();
    displayQueue = xQueueCreate(8, sizeof(LcdDisplayMessage));
    startDisplayTask();
}

void LCDScreen::kothDisplayScore(int yellowScore, int blueScore)
{
    LcdDisplayMessage msg{};
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "Y: %d : B: %d", yellowScore, blueScore);
    addToQueue(msg);
}

void LCDScreen::kothDisplayCapturing(Team capturingTeam)
{
    LcdDisplayMessage msg{};
    msg.priority = DisplayPriority::MEDIUM_PR;
    snprintf(msg.lines[0], sizeof(msg.lines[0]), "CAP: %s", (capturingTeam == Team::YELLOW ? "YELLOW" : "BLUE"));
    addToQueue(msg);
}

void LCDScreen::kothDisplayCapturingProgress(float progress)
{
    LcdDisplayMessage msg{};
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "PROGRESS: %3.2f%", progress * 100);
    addToQueue(msg);
}

void LCDScreen::kothDisplayController(Team controller)
{
    LcdDisplayMessage msg{};
    char ctrl[8];
    if (controller == Team::NONE)
    {
        strcpy(ctrl, "NONE");
    }
    if (controller == Team::YELLOW)
    {
        strcpy(ctrl, "YELLOW");
    }
    if (controller == Team::BLUE)
    {
        strcpy(ctrl, "BLUE");
    }
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "CTRL : %s", ctrl);
    addToQueue(msg);
}

void LCDScreen::displayLogo()
{
    LcdDisplayMessage msg{};
    msg.setLine(0, "      SPAS");
    msg.setLine(1, "INITIALAZING");
    addToQueue(msg);
}

void LCDScreen::displayPause()
{
    LcdDisplayMessage msg{};
    for (int i = 0; i < 4; i++)
    {
        msg.setLine(i, "     GAME PAUSED    ");
    }
    addToQueue(msg);
}

void LCDScreen::displayRespawn()
{
    LcdDisplayMessage msg{};
    msg.priority = DisplayPriority::MEDIUM_PR;
    msg.durationMs = 5000;
    for (int i = 0; i < 4; i++)
    {
        msg.setLine(i, "     RESPAWN    ");
    }
    addToQueue(msg);
}

void LCDScreen::displayCountdown(int count)
{
    LcdDisplayMessage msg{};
    msg.setLine(0, "Countdown:");
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "%d S", count);
    addToQueue(msg);
}

void LCDScreen::displayText(LcdDisplayMessage msg)
{
    addToQueue(msg);
}

void LCDScreen::render(const LcdDisplayMessage msg)
{
    for (int i = 0; i < 4; i++)
    {
        if (msg.lines[i][0] != '\0')
        { // only touch lines that are set
            lcd.setCursor(0, i);
            lcd.print("                   "); // i have no better idea how to clear line first
            lcd.setCursor(0, i);
            lcd.print(msg.lines[i]);
        }
    }
}

void LCDScreen::addToQueue(LcdDisplayMessage msg)
{
#if (SCREEN_TYPE == LCD_CHONKY_SCREEN || SCREEN_TYPE == LCD_SMOLL_SCREEN)
    xQueueSend(displayQueue, &msg, pdMS_TO_TICKS(20));
#endif
}

void LCDScreen::kothDisplayEnd(Team winner, int yellowScore, int blueScore, bool isDraw)
{
    LcdDisplayMessage msg{};
    if (isDraw)
    {
        msg.setLine(0, "GAME OVER: DRAW");
    }
    else
    {
        snprintf(msg.lines[0], sizeof(msg.lines[0]), "WINNER: %s", (winner == Team::YELLOW ? "YELLOW" : "BLUE"));
    }
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "Y: %d : B: %d", yellowScore, blueScore);
    addToQueue(msg);
}

void LCDScreen::kothDisplayInformation(NodeState lastKnownNodeStates[], int gameTime, int durration, KOTHGameScore lastKnownScore, int nodeCount)
{
    String timer = "T: " + String(gameTime) + "/" + String(durration);
    String score = ("Y: " + String(lastKnownScore.yellowPoints) + " B: " + String(lastKnownScore.bluePoints));
    String line1 = buildRow(0, 4, nodeCount, lastKnownNodeStates);
    String line2 = buildRow(4, 4, nodeCount, lastKnownNodeStates);
    LcdDisplayMessage msg{};
    //clear before render
    msg.clearLine(0);
    msg.clearLine(1);
    msg.clearLine(2);
    msg.clearLine(3);
    // display on LCD
    msg.setLine(0, timer.c_str());
    msg.setLine(1, score.c_str());
    msg.setLine(2, line1.c_str());
    msg.setLine(3, line2.c_str());
    addToQueue(msg);
}

String LCDScreen::buildRow(int startIdx, int count, int totalNodes, NodeState lastKnownNodeStates[])
{
    LOG_DEBUG("INFO_MODE", "buildRow: startIdx=%d, count=%d, totalNodes=%d", startIdx, count, totalNodes);
    String row = "";
    for (int i = startIdx; i < startIdx + count && i < totalNodes; i++)
    {
        if (i > startIdx)
            row += " ";
        row += "P" + String(lastKnownNodeStates[i].nodeId) + ":" + teamChar(lastKnownNodeStates[i].controllingTeam);
    }
    LOG_DEBUG("INFO_MODE", "buildRow result: %s", row.c_str());
    return row;
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
