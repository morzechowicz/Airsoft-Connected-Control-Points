#include "LCDScreen.h"
#include <Wire.h>

// TODO: rework direct displayLine calls to use LcdDisplayMessage queue
// see LCDScreen::showScore, showPause, showGameOver for the pattern
// kill all tasks on game over, reset LcdDisplay priority on new game

LCDScreen::LCDScreen()
{
    // Do not initialize hardware in the constructor. Call begin() from setup().
}

void LCDScreen::runDisplayTask() {
    LcdDisplayMessage lastNormal{};
    LcdDisplayMessage incoming{};
    bool hasNormal = false;

    for (;;) {
        // check if overwrite queue has anything
        if (uxQueueMessagesWaiting(overwriteQueue) > 0) {
            LcdDisplayMessage overwrite{};
            xQueueReceive(overwriteQueue, &overwrite, 0);
            render(overwrite);

            if (overwrite.durationMs > 0) {
                // wait for duration but keep accepting normal msgs silently
                TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(overwrite.durationMs);
                while (xTaskGetTickCount() < deadline) {
                    if (xQueueReceive(normalSlot, &incoming, pdMS_TO_TICKS(100)) == pdTRUE) {
                        lastNormal = incoming;  // store silently, dont render
                        hasNormal = true;
                    }
                }
                // duration expired, fall through to check overwrite queue again
            } else {
                // permanent, just drain normal msgs forever
                for (;;) {
                    xQueueReceive(normalSlot, &incoming, portMAX_DELAY);
                    lastNormal = incoming;  // game over is showing, just keep updating lastNormal
                    hasNormal = true;
                }
            }
        } else {
            // no overwrites — display lastNormal if we have it
            if (xQueueReceive(normalSlot, &incoming, portMAX_DELAY) == pdTRUE) {
                lastNormal = incoming;
                hasNormal = true;
                render(lastNormal);
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
    overwriteQueue = xQueueCreate(4, sizeof(LcdDisplayMessage));
    normalSlot     = xQueueCreate(4, sizeof(LcdDisplayMessage));    
    startDisplayTask();
}

void LCDScreen::kothDisplayScore(int yellowScore, int blueScore)
{
    LcdDisplayMessage msg{};
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "Y: %d : B: %d", yellowScore, blueScore);
    postNormal(msg);
    LOG_DEBUG("LCD_SCREEN", "Displayed score update: Y: %d : B: %d", yellowScore, blueScore);
}

void LCDScreen::kothDisplayCapturing(Team capturingTeam)
{
    LcdDisplayMessage msg{};
    snprintf(msg.lines[0], sizeof(msg.lines[0]), "CAP: %s", (capturingTeam == Team::YELLOW ? "YELLOW" : "BLUE"));
    postNormal(msg);
    LOG_DEBUG("LCD_SCREEN", "Displayed capturing update: %s capturing", (capturingTeam == Team::YELLOW ? "YELLOW" : "BLUE"));
}

void LCDScreen::kothDisplayCapturingProgress(float progress)
{
    LcdDisplayMessage msg{};
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "PROGRESS: %3.0f%", progress * 100);
    postNormal(msg);
    LOG_DEBUG("LCD_SCREEN", "Displayed capturing progress: %3.0f%%", progress * 100);
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
    snprintf(msg.lines[0], sizeof(msg.lines[0]), "CTRL : %s", ctrl);
    postNormal(msg);
    LOG_DEBUG("LCD_SCREEN", "Displayed controller update: %s controlling", ctrl);
}

void LCDScreen::displayLogo()
{
    LcdDisplayMessage msg{};
    msg.setLine(0, "      SPAS");
    msg.setLine(1, "INITIALAZING");
    postNormal(msg);
}

void LCDScreen::displayPause()
{
    LcdDisplayMessage msg{};
    for (int i = 0; i < 4; i++)
    {
        msg.setLine(i, "     GAME PAUSED    ");
    }
    postNormal(msg);
}

void LCDScreen::displayRespawn()
{
    LcdDisplayMessage msg{};
    msg.durationMs = 5000;
    for (int i = 0; i < 4; i++)
    {
        msg.setLine(i, "     RESPAWN    ");
    }
    postNormal(msg);
}

void LCDScreen::displayCountdown(int count)
{
    LcdDisplayMessage msg{};
    msg.setLine(0, "Countdown:");
    snprintf(msg.lines[1], sizeof(msg.lines[1]), "%d S", count);
    postNormal(msg);
}

void LCDScreen::displayText(LcdDisplayMessage msg)
{
    postNormal(msg);
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

void LCDScreen::postNormal(const LcdDisplayMessage& msg) {
    // latest wins — reset queue and post fresh
    #if (SCREEN_TYPE == LCD_CHONKY_SCREEN || SCREEN_TYPE == LCD_SMOLL_SCREEN)
    xQueueSend(normalSlot, &msg, pdMS_TO_TICKS(20));
    #endif
}

void LCDScreen::postOverwrite(const LcdDisplayMessage& msg) {
    #if (SCREEN_TYPE == LCD_CHONKY_SCREEN || SCREEN_TYPE == LCD_SMOLL_SCREEN)
    xQueueSend(overwriteQueue, &msg, pdMS_TO_TICKS(20));
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
    postNormal(msg);
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
    postNormal(msg);
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
