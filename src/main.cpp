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
#include <LedManager.h>
#include <DisplayLCD.h>
#include <Buzzer.h>

// somewhore to store msg for now
String msg = "";
String lastLoraMsg = "";

TeamId winner = TeamId::None;
int configState = 0;
bool MainNode = false;

DisplayLCD lcd;

GameState gameState;
ButtonManager buttonManager;
ControlPoint controlPoint(2);
DisplayOled displayOLED;
Buzzer buzzer(14);
Config config(5, 5, 5, 5);
Led blueLed(13);
Led yellowLed(12);
Clocker pointClock;
Clocker secondCLock;
Clocker gameClock;
Clocker networkClock;
SX1278 radio = new Module(18, 26, 23, 33);
LoRaMsg loramsg(config, controlPoint);
LoRaCom loraCom(radio, controlPoint, loramsg);
LoRaMsgHandler msgHandler(config, controlPoint, gameState, lastLoraMsg, winner, loramsg);
LoRaHandler lrradio(loraCom, msgHandler);

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
        loraCom.seqNum = loraCom.seqNum + 1;
        msg = loramsg.createNodeControlled(controlPoint.getNodeId(), capTeam, controlPoint.getGameMasterNode(), loraCom.seqNum);
        loraCom.sendMsgAckTo(msg, 0);
        Serial.println("Sending node controlled by message");
    }
}

void CapturePoint()
{
    if (buttonManager.blueButton.getState() == LOW && controlPoint.getControllingTeam() != TeamId::Blufor)
    {
        if (capturingTeam != TeamId::Blufor)
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
            buzzer.beepXtimes(200,3,200);
        }
        gracePeriodActive = false;
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        if (percentage > 1)
        {
            percentage = 1;
        }
        displayOLED.displayCapturing(capturingTeam, percentage);
        lcd.displayCapturing(capturingTeam, percentage);
    }
    if (buttonManager.yellowButton.getState() == LOW && controlPoint.getControllingTeam() != TeamId::YellowFor)
    {
        if (capturingTeam != TeamId::YellowFor)
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
            buzzer.beepXtimes(200,3,200);
        }
        gracePeriodActive = false;
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        if (percentage > 1)
        {
            percentage = 1;
        }
        displayOLED.displayCapturing(capturingTeam, percentage);
        lcd.displayCapturing(capturingTeam, percentage);
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
    lcd.begin();
    gameState = GameState::Network;
    controlPoint.addTeam(TeamId::Blufor);
    controlPoint.addTeam(TeamId::YellowFor);
    buttonManager.begin();
    lrradio.begin();

    // add itself to the list
    randomSeed(millis());
    int randomValue = abs(random());
    controlPoint.setNodeId(randomValue);
    controlPoint.addNode(controlPoint.getNodeId());
    displayOLED.displayInitLogo();
    delay(2000);
    buzzer.beep(300);
    Serial.println("Ready");
}

void loop()
{
    lrradio.loop();
    buttonManager.update();
    lcd.lcdLoop();
    buzzer.beepLoop();
    switch (gameState)
    {
    case GameState::Network:
        // here we just display network until idk button press or what ever
        if (buttonManager.blueButton.isPressed())
        {
            networkClock.reset();
            networkClock.start();
            loraCom.seqNum = loraCom.seqNum + 1;
            msg = loramsg.createNodeInfo(0, loraCom.seqNum);
            loraCom.sendMsgAckTo(msg, 0);
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
            lcd.displayNetworkStatus(controlPoint.getNodeCount(), controlPoint.getGameMaster(), false, lastLoraMsg);
        }
        else
        {
            displayOLED.displayNetworkStatus(controlPoint.getNodeCount(), controlPoint.getGameMaster(), false, lastLoraMsg);
            lcd.displayNetworkStatus(controlPoint.getNodeCount(), controlPoint.getGameMaster(), false, lastLoraMsg);
            networkClock.stop();
        }
        break;
    case GameState::Config:
        config.handleButtonPresses(buttonManager, configState);
        if (buttonManager.startButton.isPressed())
        {
            loraCom.seqNum = loraCom.seqNum + 1;
            msg = loramsg.createConfig(config, 0, loraCom.seqNum);
            loraCom.sendMsgAckToAll(msg);
            gameState = GameState::CountDownSetup;
        }
        displayOLED.displaySettings(config, configState);
        lcd.displaySettings(config, configState);
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
        lcd.displayCountdown(config.getCountdown());
        break;
    case GameState::StartGame:
        secondCLock.stop();
        secondCLock.reset();
        pointClock.start();
        gameClock.start();
        buzzer.beep(10000);
        gameState = GameState::Ongoing;
    case GameState::Ongoing:
        if (controlPoint.getControllingTeam() == TeamId::Blufor)
        {
            blueLed.on();
            yellowLed.off();
        }
        if (controlPoint.getControllingTeam() == TeamId::YellowFor)
        {
            blueLed.off();
            yellowLed.on();
        }
        if (controlPoint.getControllingTeam() == TeamId::None)
        {
            yellowLed.off();
            blueLed.off();
        }
        if (controlPoint.getGameMaster())
        {

            if (pointClock.getElapsedTime() >= 60000) // like the last one :/ will do later
            {
                Serial.println("Time elapsed: ");
                Serial.println(gameClock.getElapsedTimeInMinutes());
                controlPoint.increamentScore(1);
                loraCom.seqNum = loraCom.seqNum + 1;
                msg = loramsg.createScoreUpdate(controlPoint.getTeamPoints(TeamId::Blufor), controlPoint.getTeamPoints(TeamId::YellowFor), 0, loraCom.seqNum);
                loraCom.sendMsgAckToAll(msg);
                if (isGameOver())
                {
                    gameState = GameState::Finished;
                    gameClock.stop();
                    gameClock.reset();
                }
                pointClock.reset();
            }
        }
        // Capture Logic
        CapturePoint();
        if (buttonManager.yellowButton.getState() == HIGH && buttonManager.blueButton.getState() == HIGH)
        {
            displayOLED.displayGame(controlPoint, config.getDurration() - gameClock.getElapsedTimeInMinutes());
            lcd.displayGame(controlPoint, config.getDurration() - gameClock.getElapsedTimeInMinutes());
        }
        break;
    case GameState::Finished:
        winner = controlPoint.whoWon();
        Serial.println("GAME OVER");
        if (controlPoint.getGameMaster())
        {
            loraCom.seqNum = loraCom.seqNum + 1;
            msg = loramsg.createGameFinished(winner, controlPoint.getTeamPoints(TeamId::Blufor), controlPoint.getTeamPoints(TeamId::YellowFor), 0, loraCom.seqNum);
            loraCom.sendMsgAckToAll(msg);
        }
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
        buzzer.beepXtimes(1000,10,5000);
        gameState = GameState::WaitingForReset;
        break;
    case GameState::WaitingForReset:
        // here we just display winner until idk button press or what ever
        if (buttonManager.changeButton.isPressed())
        {
            gameState = GameState::Config;
        }
        displayOLED.displayFinished(winner, controlPoint);
        lcd.displayFinished(winner, controlPoint);
        break;
    default:
        // maybe just set state to config? not like there are any other ways default could happen
        gameState = GameState::Config;
        break;
    }
}