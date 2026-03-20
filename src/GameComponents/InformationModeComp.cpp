#include "InformationModeComp.h"

InformationModeComp::InformationModeComp(EventBus *eventBus, HardwareManager *hardware, NetworkManager *network) : eventBus(eventBus),
                                                                                                                   hardware(hardware),
                                                                                                                   network(network)
{
}

void InformationModeComp::start()
{
    Serial.println("Starting Information Mode Component");
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
        gameTime = e.data1;
        lastKnownScore.yellowPoints = e.data2;
        lastKnownScore.bluePoints = e.data3;
        nodeCount = e.data4;
        for (int i = 0; i < nodeCount; i++) {
            lastKnownNodeStates[i].nodeId = e.teamPoints[i].nodeId;
            lastKnownNodeStates[i].controllingTeam = e.teamPoints[i].controllingTeam;
        }
        updateDisplay(); });
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

    if (gameActive)
    {
        hardware->lcd.displayText(("TIME: " + String(gameTime) + "s").c_str(), 0);
        hardware->lcd.displayText(("Y: " + String(lastKnownScore.yellowPoints) + " B: " + String(lastKnownScore.bluePoints)).c_str(), 1);
        hardware->lcd.displayText(("CP 3: " + String(lastKnownNodeStates[1].controllingTeam == Team::YELLOW ? "Y" : lastKnownNodeStates[1].controllingTeam == Team::BLUE ? "B"
                                                                                                                                                                         : "N"))
                                      .c_str(),
                                  2);
        hardware->lcd.displayText(("CP 4: " + String(lastKnownNodeStates[2].controllingTeam == Team::YELLOW ? "Y" : lastKnownNodeStates[2].controllingTeam == Team::BLUE ? "B"
                                                                                                                                                                         : "N"))
                                      .c_str(),
                                  3);
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
    Serial.println("Game Over! Winner: " + String((int)winner));
    // Here you can add code to update the display with the winner information
}

void InformationModeComp::onButtonPressed(Event e)
{
    Serial.println("Button Pressed: " + String(e.data1));
    // Here you can add code to handle button presses, e.g., switch display modes
}

void InformationModeComp::onButtonReleased(Event e)
{
    Serial.println("Button Released: " + String(e.data1));
    // Here you can add code to handle button releases if needed
}

void InformationModeComp::onNetworkMessage(Event e)
{
}