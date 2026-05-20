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
                        { this->onScoreUpdate(e); });
    hardware->buzzer.beepOnce(4000);
    updateDisplay();
    startRespawnTask();
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

    eventBus->unsubscribe(PAUSE);
    eventBus->unsubscribe(RESUME);
    eventBus->unsubscribe(GAME_OVER);
    eventBus->unsubscribe(BUTTON_PRESSED);
    eventBus->unsubscribe(BUTTON_RELEASED);
    eventBus->unsubscribe(NETWORK_MESSAGE_RECEIVED);
    eventBus->unsubscribe(KOTH_SCORE_UPDATE);
    
    killRespawnTask();
    gameActive = false;
    deleteThis = true;
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

void InformationModeComp::onScoreUpdate(Event e)
{
    gameTime = e.data1;
    lastKnownScore.yellowPoints = e.data2;
    lastKnownScore.bluePoints = e.data3;
    nodeCount = e.data4;
    for (int i = 0; i < nodeCount; i++)
    {
        lastKnownNodeStates[i].nodeId = e.teamPoints[i].nodeId;
        lastKnownNodeStates[i].controllingTeam = e.teamPoints[i].controllingTeam;
    }
    updateDisplay();
}

void InformationModeComp::pauseGame(Event e)
{
    gamePaused = true;
    killRespawnTask(); // this is such a bandaid solution
    hardware->buzzer.beep(2000, 2, 1000);
    updateDisplay();
}

void InformationModeComp::resumeGame(Event e)
{
    gamePaused = false;
    startRespawnTask(); // its gonna be funny when it fails
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

void InformationModeComp::respawnHelper()
{
    hardware->buzzer.beep(100, 5, 300);
    hardware->lcd.clearScreen();
    for (size_t i = 0; i < 5; i++)
    {
        hardware->lcd.displayText("      RESPAWN", 0);
        hardware->lcd.displayText("      RESPAWN", 1);
        vTaskDelay(pdMS_TO_TICKS(300));
        hardware->lcd.clearScreen();
        vTaskDelay(pdMS_TO_TICKS(300));
    }

    updateDisplay();
}

void InformationModeComp::killRespawnTask()
{
    respawnTaskBit = false;
}

void InformationModeComp::respawnTask(void *pvParameters)
{
    LOG_DEBUG("INFO_MODE", "Respawn task started");
    respawnTaskBit = true;
    int respawnTime = static_cast<InformationModeComp *>(pvParameters)->config.respawnTime;
    while (respawnTaskBit)
    {
        vTaskDelay(pdMS_TO_TICKS(respawnTime * 60 * 1000));
        LOG_INFO("INFO_MODE", "Respawn time reached");
        static_cast<InformationModeComp *>(pvParameters)->respawnHelper();
    }

    vTaskDelete(NULL);
}

void InformationModeComp::startRespawnTask()
{
    xTaskCreate(respawnTask, "RespawnTask", 2048, this, 1, &respawnTaskHandle);
}
