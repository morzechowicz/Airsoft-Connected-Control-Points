#include "InformationModeComp.h"

InformationModeComp::InformationModeComp(EventBus *eventBus, HardwareManager *hardware, NetworkManager *network, KOTHConfig config) : eventBus(eventBus),
                                                                                                                                      hardware(hardware),
                                                                                                                                      network(network),
                                                                                                                                      config(config)
{
}

void InformationModeComp::start()
{
    LOG_INFO("INFO_MODE", "Starting Information Mode Component");
    gameActive = true;

    // Subscribe to game pause/resume events
    eventBus->subscribe(PAUSE, [this](Event e)
                        { pauseGame(e); });
    eventBus->subscribe(RESUME, [this](Event e)
                        { resumeGame(e); });

    // Subscribe to game events
    eventBus->subscribe(GAME_OVER, [this](Event e)
                        { handleGameOver(lastKnownScore.getWinner()); });
    // Subscribe to button events
    eventBus->subscribe(BUTTON_PRESSED, [this](Event e)
                        { this->onButtonPressed(e); });

    eventBus->subscribe(BUTTON_RELEASED, [this](Event e)
                        { this->onButtonReleased(e); });

    // Subscribe to network messages
    eventBus->subscribe(NETWORK_MESSAGE_RECEIVED, [this](Event e)
                        { this->onNetworkMessage(e); });

    // Subscribe to score updates
    eventBus->subscribe(KOTH_SCORE_UPDATE, [this](Event e)
                        {
        gameTime = e.data1;
        lastKnownScore.yellowPoints = e.data2;
        lastKnownScore.bluePoints = e.data3;
        nodeCount = e.data4;
        for (int i = 0; i < nodeCount; i++) {
            lastKnownNodeStates[i].nodeId = e.teamPoints[i].nodeId;
            lastKnownNodeStates[i].controllingTeam = e.teamPoints[i].controllingTeam;
        }
        updateDisplay();
        hardware->buzzer.beepOnce(4000); });
    updateDisplay();
}

void InformationModeComp::stop()
{
}

void InformationModeComp::update()
{
}

void InformationModeComp::updateDisplay()
{
    if (!hardware)
    {
        return;
    }
    hardware->lcd.clearScreen();

    if (gameActive)
    {
        String timer = "T: " + String(gameTime) + "/" + String(config.gameDurationMinutes);
        String score = ("Y: " + String(lastKnownScore.yellowPoints) + " B: " + String(lastKnownScore.bluePoints));
        String line1 = buildRow(0, 4, nodeCount);
        String line2 = buildRow(4, 4, nodeCount);

        // display on LCD
        hardware->lcd.displayText(timer.c_str(), 0);
        hardware->lcd.displayText(score.c_str(), 1);
        hardware->lcd.displayText(line1.c_str(), 2);
        hardware->lcd.displayText(line2.c_str(), 3);
    }

    if (gamePaused)
    {
        hardware->lcd.displayPause();
    }

    if (!gameActive)
    {
        hardware->lcd.kothDisplayEnd(lastKnownScore.getWinner(),
                                     lastKnownScore.yellowPoints,
                                     lastKnownScore.bluePoints,
                                     lastKnownScore.getWinner() == Team::NONE);
    }
}

void InformationModeComp::handleGameOver(Team winner)
{
    LOG_INFO("INFO_MODE", "Game Over! Winner: %d", (int)winner);
    hardware->buzzer.beep(2000, 3, 1000);
    gameActive = false;
    updateDisplay();
    return;
}

void InformationModeComp::onButtonPressed(Event e)
{
    LOG_INFO("INFO_MODE", "Button Pressed: %d", e.data1);
    // Here you can add code to handle button presses, e.g., switch display modes
}

void InformationModeComp::onButtonReleased(Event e)
{
    LOG_INFO("INFO_MODE", "Button Released: %d", e.data1);
    // Here you can add code to handle button releases if needed
}

void InformationModeComp::onNetworkMessage(Event e)
{
}

void InformationModeComp::pauseGame(Event e)
{
    gamePaused = true;
    hardware->buzzer.beep(2000, 2, 1000);
    updateDisplay();
}

void InformationModeComp::resumeGame(Event e)
{
    gamePaused = false;
    hardware->buzzer.beepOnce(4000);
    updateDisplay();
}

String InformationModeComp::buildRow(int startIdx, int count, int totalNodes)
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