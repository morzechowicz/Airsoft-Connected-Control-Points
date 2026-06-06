#include "InformationModeComp.h"
static volatile bool respawnTaskBit;

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

    if (gameActive)
    {
        hardware->lcd.kothDisplayInformation(lastKnownNodeStates, gameTime, config.gameDurationMinutes, lastKnownScore, nodeCount);
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
    hardware->lcd.clearScreen();
    updateDisplay();
}

void InformationModeComp::respawnHelper()
{
    hardware->buzzer.beep(100, 5, 300);
    hardware->lcd.displayRespawn();
    updateDisplay();
}

void InformationModeComp::killRespawnTask()
{
    respawnTaskBit = false;
}

void InformationModeComp::respawnTask(void *pvParameters)
{
    int respawnTime = static_cast<InformationModeComp *>(pvParameters)->config.respawnTime;
    respawnTaskBit = true;
    long lastRespawnTime = xTaskGetTickCount();

    if (respawnTime <= 0)
    {
        LOG_ERROR("INFO_MODE", "Respawn time is zero or less, exiting");
        vTaskDelete(NULL);
        return;
    }

    LOG_DEBUG("INFO_MODE", "Respawn task started %d", respawnTime);

    while (respawnTaskBit)
    {
        if (xTaskGetTickCount() - lastRespawnTime >= pdMS_TO_TICKS(respawnTime * 60 * 1000))
        {
            LOG_INFO("INFO_MODE", "Respawn time reached");
            static_cast<InformationModeComp *>(pvParameters)->respawnHelper();
            lastRespawnTime = xTaskGetTickCount();
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
    LOG_DEBUG("INFO_MODE", "Respawn task aborted");
    vTaskDelete(NULL);
}

void InformationModeComp::startRespawnTask()
{
    xTaskCreate(respawnTask, "RespawnTask", 4096, this, 1, &respawnTaskHandle);
}
