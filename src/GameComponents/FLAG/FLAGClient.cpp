#include "FLAGClient.h"

FLAGClient::FLAGClient(EventBus *eb, HardwareManager *hw, NetworkManager *net,
                       uint8_t nodeId, FLAGConfig config)
    : eventBus(eb),
      hardware(hw),
      network(net),
      myNodeId(nodeId),
      captureTimeMs(config.captureTime),
      maxTeams(config.initTeamCount),
      currentController(FlagTeam::NONE),
      capturingTeam(FlagTeam::NONE),
      capturing(false),
      captureStartTime(0)
{
}

FLAGClient::~FLAGClient()
{
    stop();
}

void FLAGClient::start()
{
    LOG_INFO("FLAG_CLIENT", "Starting FLAG Client with Node ID: %d", myNodeId);
    
    LOG_INFO("FLAG_CLIENT", "Game started!");
    LOG_INFO("FLAG_CLIENT", "Node ID: %d", myNodeId);
    LOG_INFO("FLAG_CLIENT", "Capture Time: %d", captureTimeMs);
    captureTimeMs = captureTimeMs * 1000;
    currentController = FlagTeam::NONE;
    gameActive = true;
    locatingBeepSpacingUpdate = millis() + 5000;
    hardware->buzzer.beepOnce(4000);
    updateDisplay();
    
    LOG_DEBUG("FLAG_CLIENT", "Initial Controller: %s", getFlagTeamName(currentController));
    // Subscribe to game events
    eventBus->subscribe(GAME_OVER, [this](Event e)
                        {
        handleGameOver(lastKnownScore.getWinner());
        gameActive = false;
        updateDisplay();
        this->~FLAGClient();
        return; });
    // Subscribe to button events
    eventBus->subscribe(BUTTON_PRESSED, [this](Event e)
                        { this->onButtonPressed(e); });

    // Subscribe to network messages
    // eventBus->subscribe(NETWORK_MESSAGE_RECEIVED, [this](Event e)
    //                     { this->onNetworkMessage(e); });

    // Subscribe to score updates
    eventBus->subscribe(FLAG_SCORE_UPDATE, [this](Event e)
                        {
        currentController = (FlagTeam)e.data1;
        lastKnownScore.teams[currentController].score = e.data2;
        updateDisplay(); });

    updateDisplay();
}

void FLAGClient::stop()
{
    eventBus->unsubscribe(GAME_OVER);
    eventBus->unsubscribe(BUTTON_PRESSED);
    // eventBus->unsubscribe(NETWORK_MESSAGE_RECEIVED);
    eventBus->unsubscribe(KOTH_SCORE_UPDATE);
    capturing = false;
    LOG_INFO("FLAG_CLIENT", "FLAG Client stopped");
}

void FLAGClient::update()
{
    if (capturing)
    {
        updateCapture();
    }

    // beep when working
    if (gameActive)
    {
        if (millis() > locatingBeepSpacingUpdate)
        {
            hardware->buzzer.beepOnce(100);
            locatingBeepSpacingUpdate = locatingBeepSpacingUpdate + LOCALIZER_BEEP;
        }
    }
}

void FLAGClient::onButtonPressed(Event e)
{
    // Determine which team button was pressed
    FlagTeam buttonTeam = FlagTeam::NONE;

    // Map button pin to team (adjust based on your wiring)
    if (e.data1 == BUTTON_BLUE_PIN)
    { // Blue button pin
        if (capturingTeam != FlagTeam::NONE)
        {
            buttonTeam = lastKnownScore.nextTeam(currentController,maxTeams);
        }else{
            buttonTeam = lastKnownScore.nextTeam(capturingTeam,maxTeams);
        }
    }
    else if (e.data1 == BUTTON_YELLOW_PIN)
    { // Yellow button pin
        if (capturingTeam != FlagTeam::NONE)
        {
            buttonTeam = lastKnownScore.previousTeam(currentController,maxTeams);
        }else{
            buttonTeam = lastKnownScore.previousTeam(capturingTeam,maxTeams);
        }
    }

    if (buttonTeam == FlagTeam::NONE)
    {
        return;
    }

    // Already controlled by this team?
    if (currentController == buttonTeam && currentController != FlagTeam::NONE)
    {
        LOG_DEBUG("FLAG_CLIENT", "Already controlled by %s, ignoring button press", getFlagTeamName(buttonTeam));
        return;
    }

    // Start or continue capture
    startCapture(buttonTeam);
}

void FLAGClient::startCapture(FlagTeam team)
{
    capturing = true;
    capturingTeam = team;
    captureStartTime = millis();
    LOG_INFO("FLAG_CLIENT", "Started capturing for %s", getFlagTeamName(team));
}

void FLAGClient::updateCapture()
{
    unsigned long elapsed = millis() - captureStartTime;
    float progress = (float)elapsed / (float)captureTimeMs;

    // Update display with progress every second
    if (millis() - lastDisplayUpdate >= 1000)
    {
        updateDisplay();
        lastDisplayUpdate = millis();
    }

    // Check for completion
    if (elapsed >= captureTimeMs)
    {
        completeCapture();
    }
}

void FLAGClient::completeCapture()
{
    currentController = capturingTeam;
    LOG_INFO("FLAG_CLIENT", "Point captured by %s", getFlagTeamName(currentController));

    // Send capture message to server
    String msg = Protocol::buildCaptureMessage(myNodeId, (uint8_t)capturingTeam);
    if (network)
    {
        LOG_INFO("FLAG_CLIENT", "Sending capture message to server");
        network->sendToMaster(msg);
    }

    // Local event
    eventBus->publish(KOTH_POINT_CAPTURED, currentController);

    // Victory feedback
    if (hardware)
    {
        hardware->buzzer.beep(300, 3, 300);
    }

    // Reset capture state
    capturing = false;
    capturingTeam = FlagTeam::NONE;

    updateLEDs();
    updateDisplay();
}

float FLAGClient::getCaptureProgress() const
{
    if (!capturing)
        return 0.0f;

    unsigned long elapsed = millis() - captureStartTime;
    float progress = (float)elapsed / (float)captureTimeMs;

    return (progress > 1.0f) ? 1.0f : progress;
}

void FLAGClient::onNetworkMessage(Event e)
{
    // Parse message (assuming e.data is pointer to message string)
    // This depends on how you implement NETWORK_MESSAGE_RECEIVED
    // For now, placeholder
}

void FLAGClient::handleGameOver(FlagTeam winner)
{
    LOG_INFO("FLAG_CLIENT", "========== GAME OVER ==========");
    LOG_INFO("FLAG_CLIENT", "Winner: %s", getFlagTeamName(winner));
    // Victory animation
    if (hardware)
    {
        hardware->buzzer.beep(2000, 3, 1000);
    }

    deleteThis = true;
}

void FLAGClient::updateDisplay()
{
    if (!hardware)
    {
        return;
    }

    if (capturing)
    {
        hardware->lcd.flagDisplayCapturin(capturingTeam, getCaptureProgress());
    }
    else
    {
        hardware->lcd.flagDisplayScore(lastKnownScore.teams[currentController].score);
        hardware->lcd.flagDisplayController(currentController);
    }
    if (!gameActive)
    {
        hardware->lcd.flagDisplayEnd(lastKnownScore.getWinner());
    }
}

void FLAGClient::updateLEDs()
{
    if (!hardware)
        return;

    if (capturing)
    {
        hardware->ledBlueButton.on();
        hardware->ledBlueButton.on();
    }
    else
    {
        hardware->ledBlueButton.off();
        hardware->ledBlueButton.off();
    }
}