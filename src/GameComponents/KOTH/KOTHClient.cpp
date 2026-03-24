// KOTHClient.cpp
#include "KOTHClient.h"

KOTHClient::KOTHClient(EventBus *eb, HardwareManager *hw, NetworkManager *net,
                       uint8_t nodeId, KOTHConfig config)
    : eventBus(eb),
      hardware(hw),
      network(net),
      myNodeId(nodeId),
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
    Serial.println("=== KOTH Client Started ===");
    Serial.print("Node ID: ");
    Serial.println(myNodeId);
    Serial.print("Team: ");
    Serial.print("[CLIENT] ");
    Serial.println("Game started!");
    Serial.print("myNodeId");
    Serial.println(myNodeId);
    Serial.print("captureTimeMs");
    Serial.println(captureTimeMs);
    captureTimeMs = captureTimeMs * 1000;
    currentController = Team::NONE;
    gameActive = true;
    locatingBeepSpacingUpdate = millis() + 5000;
    hardware->buzzer.beepOnce(4000);
    updateDisplay();
    updateLEDs();
    if (currentController == Team::NONE)
    {
        Serial.println("NONE");
    }
    else
    {
        Serial.println(currentController == Team::YELLOW ? "YELLOW" : "BLUE");
    }
    //Subscribe to pause/resume
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
        lastKnownScore.yellowPoints = e.data2;
        lastKnownScore.bluePoints = e.data3;
        updateDisplay(); });

    updateDisplay();
}

void KOTHClient::stop()
{
    eventBus->unsubscribe(GAME_OVER);
    eventBus->unsubscribe(BUTTON_PRESSED);
    eventBus->unsubscribe(BUTTON_RELEASED);
    eventBus->unsubscribe(NETWORK_MESSAGE_RECEIVED);
    eventBus->unsubscribe(KOTH_SCORE_UPDATE);
    capturing = false;
    Serial.println("KOTH Client stopped");
}

void KOTHClient::update()
{
    if (capturing)
    {
        updateCapture();
    }

    // Handle grace period
    if (gracePeriod)
    {
        if (millis() - graceStartTime >= gracePeriodMs)
        {
            cancelCapture();
            gracePeriod = false;
        }
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

void KOTHClient::onButtonPressed(Event e)
{
    if(gamePaused)
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
        Serial.print("[CLIENT] ");
        Serial.println("Already controlled by your team");
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
        Serial.print("[CLIENT] ");
        Serial.print("Started capturing for ");
        Serial.println(team == Team::YELLOW ? "YELLOW" : "BLUE");
    }
}

void KOTHClient::updateCapture()
{
    unsigned long elapsed = millis() - captureStartTime;
    float progress = (float)elapsed / (float)captureTimeMs;

    // Update display with progress every second
    if (millis() - lastDisplayUpdate >= 1000)
    {
        updateDisplay();
        lastDisplayUpdate = millis();
    }

    // Check for neutralization (halfway point)
    if (progress >= 0.5f && currentController != Team::NONE &&
        currentController != capturingTeam && captureTimeMs > 5000)
    {
        currentController = Team::NONE;
        Serial.print("[CLIENT] ");
        Serial.println("Point neutralized!");
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
    Serial.print("[CLIENT] ");
    Serial.print("Point captured by ");
    Serial.println(capturingTeam == Team::YELLOW ? "YELLOW" : "BLUE");

    // Send capture message to server
    String msg = Protocol::buildCaptureMessage(myNodeId, (uint8_t)capturingTeam);
    if (network)
    {
        Serial.print("[CLIENT] ");
        Serial.println("Sending capture message to server");
        network->sendToMaster(msg);
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
    Serial.print("[CLIENT] ");
    Serial.println("Capture cancelled");
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
    Serial.print("[CLIENT] ");
    Serial.println("========== GAME OVER ==========");
    Serial.print("Winner: ");
    Serial.println(winner == Team::YELLOW ? "YELLOW" : winner == Team::BLUE ? "BLUE"
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
        hardware->lcd.kothDisplayCapturing(capturingTeam, getCaptureProgress());
    }
    else
    {
        hardware->lcd.kothDisplayScore(lastKnownScore.yellowPoints, lastKnownScore.bluePoints);
        hardware->lcd.kothDisplayController(currentController);
    }

    if(gamePaused)
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
        hardware->ledYellowButton.on();
        hardware->ledBlueButton.off();
    }
    else if (currentController == Team::BLUE)
    {
        hardware->ledBlueButton.on();
        hardware->ledYellowButton.off();
    }
    else
    {
        hardware->ledYellowButton.off();
        hardware->ledBlueButton.off();
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