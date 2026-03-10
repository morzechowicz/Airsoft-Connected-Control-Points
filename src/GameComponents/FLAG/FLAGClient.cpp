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
    Serial.println("=== FLAG Client Started ===");
    Serial.print("Node ID: ");
    Serial.println(myNodeId);
    Serial.print("[CLIENT] ");
    Serial.println("Game started!");
    Serial.print("myNodeId");
    Serial.println(myNodeId);
    Serial.print("captureTimeMs");
    Serial.println(captureTimeMs);
    Serial.print("FlagTeam: ");
    captureTimeMs = captureTimeMs * 1000;
    currentController = FlagTeam::NONE;
    gameActive = true;
    locatingBeepSpacingUpdate = millis() + 5000;
    hardware->buzzer.beepOnce(4000);
    updateDisplay();
    if (currentController == FlagTeam::NONE)
    {
        Serial.println("NONE");
    }
    else
    {
        Serial.println(getFlagTeamName(currentController));
    }
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
    Serial.println("FLAG Client stopped");
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
        Serial.print("[CLIENT] ");
        Serial.println("Already controlled by your team");
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
    Serial.print("[CLIENT] ");
    Serial.print("Started capturing for ");
    Serial.println(getFlagTeamName(team));
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
    Serial.print("[CLIENT] ");
    Serial.print("Point captured by ");
    Serial.println(getFlagTeamName(currentController));

    // Send capture message to server
    String msg = Protocol::buildCaptureMessage(myNodeId, (uint8_t)capturingTeam);
    if (network)
    {
        Serial.print("[CLIENT] ");
        Serial.println("Sending capture message to server");
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
    Serial.print("[CLIENT] ");
    Serial.println("========== GAME OVER ==========");
    Serial.print("Winner: ");
    Serial.println(getFlagTeamName(winner));
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