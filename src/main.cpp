#include <Arduino.h>
#include <Team.h>
#include <Teams.h>
#include <RadioLib.h>
#include <ButtonManager.h>
#include <GameState.h>
#include <DisplayOled.h>
#include <Config.h>
#include <ControlPoint.h>
#include <Clocker.h>
#include <LoRaHandler.h>
#include "LoRaMsg.h"

// somewhore to store msg for now
String msg = "";
String lastLoraMsg = "";

TeamId winner = TeamId::None;
int configState = 0;
bool MainNode = false;

GameState gameState;
ButtonManager buttonManager;
ControlPoint controlPoint(2);
DisplayOled displayOLED;
Config config(15, 15, 15, 15);
Clocker pointClock;
Clocker secondCLock;
Clocker gameClock;
Clocker networkClock;
LoRaMsgHandler msgHandler(config, controlPoint, gameState, lastLoraMsg, winner);
SX1278 radio = new Module(18, 26, 23, 33);
LoRaCom loraCom(radio);
LoRaHandler lrradio(loraCom, msgHandler);
LoRaMsg loramsg(loraCom, config, controlPoint);

// TO DO:
//  displaying info on lcd and oled logic
//  lora logic mainly keeping points in main node and getting team changes from other node

bool isGameOver()
{
    // time overflow
    if (gameClock.getElapsedTimeInMinutes() > config.getDurration())
    {
        Serial.println("TIMES UP");
        return true;
    }
    // winning by points
    if (controlPoint.pointsTargetReached(config.getPointsTarget()))
    {
        Serial.println("POINTS REACHED");
        return true;
    }
    return false;
}

static Clocker captureClock;
static Clocker graceClock;
static TeamId capturingTeam = TeamId::None;
static bool gracePeriodActive = false;

void capturedPointSendLoRaUpdate(TeamId capTeam)
{
    if (!controlPoint.getGameMaster())
    {
        msg = loramsg.createNodeControlled(controlPoint.getNodeId(), capTeam);
        loraCom.sendMsg(msg);
        Serial.println("Sending node controlled by message");
    }
}

void CapturePoint()
{
    if (buttonManager.blueButton.getState() == LOW)
    {
        if (capturingTeam != TeamId::Blufor && controlPoint.getControllingTeam() != TeamId::Blufor)
        {
            capturingTeam = TeamId::Blufor;
            captureClock.start();
            captureClock.reset();
            Serial.println("Team Blue started capturing!");
        }
        if ((captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() / 2) && controlPoint.getControllingTeam() != TeamId::None && controlPoint.getControllingTeam() != TeamId::Blufor && config.getCaptureTime() > 5)
        {
            controlPoint.setControllingTeam(TeamId::None, controlPoint.getNodeId());
            capturedPointSendLoRaUpdate(TeamId::None);
            Serial.println("Point neutralized!");
        }
        if (captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() && controlPoint.getControllingTeam() != TeamId::Blufor)
        {
            controlPoint.setControllingTeam(TeamId::Blufor, controlPoint.getNodeId());
            capturedPointSendLoRaUpdate(TeamId::Blufor);
            Serial.println("Team Blue fully captured the point!");
        }
        gracePeriodActive = false;
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        if (percentage > 1)
        {
            percentage = 1;
        }
        displayOLED.displayCapturing(capturingTeam, percentage);
    }
    if (buttonManager.yellowButton.getState() == LOW)
    {
        if (capturingTeam != TeamId::YellowFor && controlPoint.getControllingTeam() != TeamId::YellowFor)
        {
            capturingTeam = TeamId::YellowFor;
            captureClock.start();
            captureClock.reset();
            Serial.println("Team Yellow started capturing!");
        }
        if ((captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() / 2) && controlPoint.getControllingTeam() != TeamId::None && controlPoint.getControllingTeam() != TeamId::YellowFor && config.getCaptureTime() > 5)
        {
            controlPoint.setControllingTeam(TeamId::None, controlPoint.getNodeId());
            capturedPointSendLoRaUpdate(TeamId::None);
            Serial.println("Point neutralized!");
        }
        if (captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() && controlPoint.getControllingTeam() != TeamId::YellowFor)
        {
            controlPoint.setControllingTeam(TeamId::YellowFor, controlPoint.getNodeId());
            capturedPointSendLoRaUpdate(TeamId::YellowFor);
            Serial.println("Team Yellow fully captured the point!");
        }
        gracePeriodActive = false;
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        if (percentage > 1)
        {
            percentage = 1;
        }
        displayOLED.displayCapturing(capturingTeam, percentage);
    }
    else
    {
        if (!gracePeriodActive)
        {
            gracePeriodActive = true;
            graceClock.reset();
            graceClock.start();
        }
        if (graceClock.getElapsedTime() > 500)
        {
            capturingTeam = TeamId::None;
            captureClock.stop();
            captureClock.reset();
            graceClock.stop();
            graceClock.reset();
            gracePeriodActive = false;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting up...");
    displayOLED.begin();
    gameState = GameState::Network;
    controlPoint.addTeam(TeamId::Blufor);
    controlPoint.addTeam(TeamId::YellowFor);
    buttonManager.begin();
    lrradio.begin();

    // add itself to the list
    randomSeed(analogRead(35));
    int randomValue = abs(random());
    controlPoint.setNodeId(randomValue);
    controlPoint.addNode(controlPoint.getNodeId());
    displayOLED.displayInitLogo();
    delay(2000);
    Serial.println("Ready");
}

void loop()
{
    lrradio.loop();
    buttonManager.update();
    switch (gameState)
    {
    case GameState::Network:
        // here we just display network until idk button press or what ever
        if (buttonManager.blueButton.isPressed())
        {
            networkClock.reset();
            networkClock.start();
            msg = loramsg.createNodeInfo();
            loraCom.sendMsg(msg);
            Serial.println("Sending node info");
        }
        if (buttonManager.yellowButton.isPressed())
        {
            if (controlPoint.getGameMaster())
            {
                controlPoint.setGameMaster(false);
                Serial.println("I am not the GameMaster anymore");
            }
            else
            {
                controlPoint.setGameMaster(true);
                Serial.println("I am the GameMaster now");
            }
        }
        if (buttonManager.changeButton.isPressed())
        {
            gameState = GameState::Config;
        }
        if (networkClock.getElapsedTimeInSeconds() > 5)
        {
            displayOLED.displayNetworkStatus(controlPoint.getNodeCount(), controlPoint.getGameMaster(), false, lastLoraMsg);
        }
        else
        {
            displayOLED.displayNetworkStatus(controlPoint.getNodeCount(), controlPoint.getGameMaster(), false, lastLoraMsg);
            networkClock.stop();
        }
        break;
    case GameState::Config:
        config.handleButtonPresses(buttonManager, configState);
        if (buttonManager.startButton.isPressed())
        {
            msg = loramsg.createConfig(config);
            loraCom.sendMsg(msg);
            gameState = GameState::CountDownSetup;
        }
        displayOLED.displaySettings(config, configState);
        break;
    case GameState::CountDownSetup:
        secondCLock.start();
        gameState = GameState::CountDown;
        break;
    case GameState::CountDown:
        if (secondCLock.getElapsedTime() > 1000) // for the time being this stays like this i cant be bothered to add another setting rihgt now also nobody cares
        {
            config.setCountdown(config.getCountdown() - 1);
            Serial.println("Countdown: ");
            Serial.println(config.getCountdown());
            // secondCLock.getElapsedTime(); // add this to countdown display later TODO
            secondCLock.reset();
        }
        if (config.getCountdown() <= 0)
        {
            gameState = GameState::StartGame;
        }
        displayOLED.displayCountdown(config.getCountdown());
        break;
    case GameState::StartGame:
        secondCLock.stop();
        secondCLock.reset();
        pointClock.start();
        gameClock.start();
        gameState = GameState::Ongoing;
    case GameState::Ongoing:
        if (controlPoint.getGameMaster())
        {

            if (pointClock.getElapsedTime() >= 60000) // like the last one :/ will do later
            {
                Serial.println("Time elapsed: ");
                Serial.println(gameClock.getElapsedTimeInMinutes());
                controlPoint.increamentScore(1);
                msg = loramsg.createScoreUpdate(controlPoint.getTeamPoints(TeamId::Blufor), controlPoint.getTeamPoints(TeamId::YellowFor));
                loraCom.sendMsg(msg);
                if (isGameOver())
                {
                    gameState = GameState::Finished;
                    gameClock.stop();
                    gameClock.reset();
                    msg = loramsg.createGameFinished(winner, controlPoint.getTeamPoints(TeamId::Blufor), controlPoint.getTeamPoints(TeamId::YellowFor));
                    loraCom.sendMsg(msg);

                    winner = controlPoint.whoWon();
                    Serial.println("GAME OVER");
                    if (winner == TeamId::Blufor)
                    {
                        Serial.print("BLUFOR WON");
                    }
                    if (winner == TeamId::YellowFor)
                    {
                        Serial.print("YELLOWFOR WON");
                    }
                    if (winner == TeamId::Draw)
                    {
                        Serial.print("DRAW");
                    }
                    if (winner == TeamId::None)
                    {
                        Serial.print("a NONE in a WINNER if? how queer! Ive never seen such a thing ");
                        Serial.println("I guess we make an none now");
                        Serial.println("for real i have no idea what to put here");
                    }
                    pointClock.stop();
                }
                pointClock.reset();
            }
        }
        // Capture Logic
        CapturePoint();
        if (buttonManager.yellowButton.getState() == HIGH && buttonManager.blueButton.getState() == HIGH)
        {
            displayOLED.displayGame(controlPoint, config.getDurration() - gameClock.getElapsedTimeInMinutes());
        }
        break;
    case GameState::Finished:
        // here we just display winner until idk button press or what ever
        if (buttonManager.changeButton.isPressed())
        {
            gameState = GameState::Config;
        }
        displayOLED.displayFinished(winner, controlPoint);
        break;
    default:
        // maybe just set state to config? not like there are any other ways default could happen
        gameState = GameState::Config;
        break;
    }
}