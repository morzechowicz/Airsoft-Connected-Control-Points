// KOTHClient.cpp
#include "KOTHClient.h"

KOTHClient::KOTHClient(EventBus *eb, HardwareManager *hw, NetworkManager *net,
                       uint8_t nodeId, KOTHConfig config)
    : eventBus(eb),
      hardware(hw),
      network(net),
      myNodeId(nodeId),
      maxGameTime(config.gameDurationMinutes),
      lastScoreUpdateTime(millis()),
      captureTimeMs(config.captureTime),
      currentController(Team::NONE),
      capturingTeam(Team::NONE),
      capturing(false),
      captureStartTime(0),
      gracePeriod(false),
      graceStartTime(0),
      gracePeriodMs(500) // 0.5 second grace period
{
}

KOTHClient::~KOTHClient()
{
    stop();
}

void KOTHClient::start()
{
    LOG_INFO("KOTH_CLIENT", "Starting KOTH Client on node %d", myNodeId);

    LOG_INFO("KOTH_CLIENT", "Game started!");
    LOG_INFO("KOTH_CLIENT", "Node ID: %d", myNodeId);
    LOG_INFO("KOTH_CLIENT", "Capture Time: %d ms", captureTimeMs);
    LOG_INFO("KOTH_CLIENT", "Max Game Time: %d minutes", maxGameTime);    captureTimeMs = captureTimeMs * 1000;
    currentController = Team::NONE;
    gameActive = true;
    locatingBeepSpacingUpdate = millis() + 5000;
    hardware->buzzer.beepOnce(4000);
    updateDisplay();
    updateLEDs();

    // Subscribe to pause/resume
    eventBus->subscribe(PAUSE, [this](Event e)
                        { pauseGame(e); });
    eventBus->subscribe(RESUME, [this](Event e)
                        { resumeGame(e); });

    // Subscribe to game events
    eventBus->subscribe(GAME_OVER, [this](Event e)
                        {
        handleGameOver(lastKnownScore.getWinner());
        gameActive = false;
        updateDisplay();
        return; });
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
        lastScoreUpdateTime = millis();
        timeElapsedSinceStart = e.data1;
        lastKnownScore.yellowPoints = e.data2;
        lastKnownScore.bluePoints = e.data3;
        updateDisplay(); });

    updateDisplay();
}

void KOTHClient::stop()
{
    eventBus->unsubscribe(PAUSE);
    eventBus->unsubscribe(RESUME);
    eventBus->unsubscribe(GAME_OVER);
    eventBus->unsubscribe(BUTTON_PRESSED);
    eventBus->unsubscribe(BUTTON_RELEASED);
    eventBus->unsubscribe(NETWORK_MESSAGE_RECEIVED);
    eventBus->unsubscribe(KOTH_SCORE_UPDATE);
    capturing = false;
    LOG_INFO("KOTH_CLIENT", "KOTH Client stopped");
}

void KOTHClient::update()
{
    uint32_t now = millis();
    if (capturing)
    {
        updateCapture();
    }

    // Handle grace period
    if (gracePeriod)
    {
        if (now - graceStartTime >= gracePeriodMs)
        {
            cancelCapture();
            gracePeriod = false;
        }
    }

    // beep when working
    if (gameActive)
    {
        if (now > locatingBeepSpacingUpdate)
        {
            hardware->buzzer.beepOnce(100);
            locatingBeepSpacingUpdate = locatingBeepSpacingUpdate + calculateGameQuater(maxGameTime, timeElapsedSinceStart);
        }
    }

    // Ask server for update if we haven't heard from it for a while
    // why it stopped calling us? is it my fualt? maybe they hate me?
    // maybe they prefer other better clients? maybe they are just busy? maybe they are dead? who knows
    // such is life in client-server architecture, you are at the mercy of the server, you can only hope it treats you well and doesn't forget about you
    // remember to not spam it just send it once
    if (gameActive && (now - lastScoreUpdateTime) > SCORING_INTERVAL_MS * 2)
    {
        LOG_WARN("KOTH_CLIENT", "Haven't received score update in some time, requesting update from server");
        String msg = Protocol::buildReqeustScoreUpdate(myNodeId);
        if (network)
        {
            network->sendToMain(msg);
        }
        lastScoreUpdateTime = now; // reset timer to avoid spamming
    }
}

u_int64_t KOTHClient::calculateGameQuater(int maxTime, int elapsedTime)
{
    float quarter = (float)elapsedTime / (float)maxTime;
    if (quarter < 0.25f)
    {
        return LOCALIZER_BEEP_FULL;
    }
    else if (quarter < 0.5f)
    {
        return LOCALIZER_BEEP_THREE_FOURTH;
    }
    else if (quarter < 0.75f)
    {
        return LOCALIZER_BEEP_ONE_HALF;
    }
    else if (quarter < 0.95f)
    {
        return LOCALIZER_BEEP_ONE_FOURTH;
    }
    else
    {
        return LOCALIZER_BEEP_LAST_MINUTE;
    }
    return 3000UL;
}

void KOTHClient::onButtonPressed(Event e)
{
    if (gamePaused)
    {
        return;
    }
    // Determine which team button was pressed
    Team buttonTeam = Team::NONE;

    // Map button pin to team (adjust based on your wiring)
    if (e.data1 == BUTTON_BLUE_PIN)
    { // Blue button pin
        buttonTeam = Team::BLUE;
    }
    else if (e.data1 == BUTTON_YELLOW_PIN)
    { // Yellow button pin
        buttonTeam = Team::YELLOW;
    }

    if (buttonTeam == Team::NONE)
    {
        return;
    }

    // Already controlled by this team?
    if (currentController == buttonTeam && currentController != Team::NONE)
    {
        LOG_INFO("KOTH_CLIENT", "Already controlled by your team");
        return;
    }

    // Start or continue capture
    startCapture(buttonTeam);
    gracePeriod = false;
}

void KOTHClient::onButtonReleased(Event e)
{
    if (capturing)
    {
        // Start grace period
        gracePeriod = true;
        graceStartTime = millis();
    }
}

void KOTHClient::startCapture(Team team)
{
    if (!capturing || capturingTeam != team)
    {
        capturing = true;
        capturingTeam = team;
        captureStartTime = millis();
        hardware->lcd.kothDisplayCapturing(capturingTeam);
        LOG_DEBUG("KOTH_CLIENT", "Started capturing for %s team", team == Team::YELLOW ? "YELLOW" : "BLUE");
    }
}

void KOTHClient::updateCapture()
{
    unsigned long elapsed = millis() - captureStartTime;
    float progress = (float)elapsed / (float)captureTimeMs;

    // Update display with progress every half second
    if (millis() - lastDisplayUpdate >= 500)
    {
        updateDisplay();
        lastDisplayUpdate = millis();
    }

    // Check for neutralization (halfway point)
    if (progress >= 0.5f && currentController != Team::NONE &&
        currentController != capturingTeam && captureTimeMs > 5000)
    {
        currentController = Team::NONE;
        LOG_INFO("KOTH_CLIENT", "Point neutralized!");
        updateLEDs();
        updateDisplay();
        if (hardware)
        {
            hardware->buzzer.beep(200, 3, 200);
        }
    }

    // Check for completion
    if (elapsed >= captureTimeMs)
    {
        completeCapture();
    }
}

void KOTHClient::completeCapture()
{
    currentController = capturingTeam;
    LOG_INFO("KOTH_CLIENT", "Point captured by %s team", capturingTeam == Team::YELLOW ? "YELLOW" : "BLUE");

    // Send capture message to server
    String msg = Protocol::buildCaptureMessage(myNodeId, (uint8_t)capturingTeam);
    if (network)
    {
        LOG_DEBUG("KOTH_CLIENT", "Sending capture message to server");
        network->sendToMain(msg);
    }

    // Local event
    eventBus->publish(KOTH_POINT_CAPTURED, myNodeId, (int)capturingTeam);

    // Victory feedback
    if (hardware)
    {
        hardware->buzzer.beep(300, 3, 300);
    }

    // Reset capture state
    capturing = false;
    capturingTeam = Team::NONE;

    updateLEDs();
    updateDisplay();
}

void KOTHClient::cancelCapture()
{
    capturing = false;
    capturingTeam = Team::NONE;
    LOG_DEBUG("KOTH_CLIENT", "Capture cancelled");
    updateLEDs();
    updateDisplay();
}

float KOTHClient::getCaptureProgress() const
{
    if (!capturing)
        return 0.0f;

    unsigned long elapsed = millis() - captureStartTime;
    float progress = (float)elapsed / (float)captureTimeMs;

    return (progress > 1.0f) ? 1.0f : progress;
}

void KOTHClient::onNetworkMessage(Event e)
{
    // Parse message (assuming e.data is pointer to message string)
    // This depends on how you implement NETWORK_MESSAGE_RECEIVED
    // For now, placeholder
}

void KOTHClient::handleGameOver(Team winner)
{
    LOG_INFO("KOTH_CLIENT", "Game over! Winner: %s", winner == Team::YELLOW ? "YELLOW" : winner == Team::BLUE ? "BLUE"
                                                                                                              : "DRAW");
    // Victory animation
    if (hardware)
    {
        hardware->buzzer.beep(2000, 3, 1000);
    }

    deleteThis = true;
}

void KOTHClient::updateDisplay()
{
    if (!hardware)
    {
        return;
    }

    if (capturing)
    {
        hardware->lcd.kothDisplayCapturingProgress(getCaptureProgress());
    }
    else
    {
        hardware->lcd.kothDisplayScore(lastKnownScore.yellowPoints, lastKnownScore.bluePoints);
        hardware->lcd.kothDisplayController(currentController);
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

void KOTHClient::updateLEDs()
{
    if (!hardware)
        return;

    if (currentController == Team::YELLOW)
    {
        hardware->ledYellowButton.blink();
        hardware->ledBlueButton.blinkOff();
    }
    else if (currentController == Team::BLUE)
    {
        hardware->ledBlueButton.blink();
        hardware->ledYellowButton.blinkOff();
    }
    else
    {
        hardware->ledYellowButton.off();
        hardware->ledBlueButton.off();
        hardware->ledYellowButton.blinkOff();
        hardware->ledBlueButton.blinkOff();
    }
}

void KOTHClient::pauseGame(Event e)
{
    gamePaused = true;
    hardware->buzzer.beep(2000, 2, 1000);
    updateDisplay();
}

void KOTHClient::resumeGame(Event e)
{
    gamePaused = false;
    hardware->buzzer.beepOnce(4000);
    updateDisplay();
}