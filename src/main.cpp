#include <Arduino.h>
#include <Team.h>
#include <Teams.h>
#include <LoRaManager.h>
#include <RadioLib.h>
#include <ButtonManager.h>
#include <GameState.h>
#include <DisplayOled.h>
#include <Config.h>
#include <ControlPoint.h>
#include <Clocker.h>

GameState gameState;
SX1278 radio = new Module(18, 14, 26, 33);
// LoRaManager loRaManager(radio);
ButtonManager buttonManager;
ControlPoint controlPoint(2);
DisplayOled displayOLED;
Config config(15, 15, 15, 15);
Clocker pointClock;
Clocker secondCLock;
Clocker gameClock;

// TO DO:
//  displaying info on lcd and oled logic
//  lora logic mainly keeping points in main node and getting team changes from other node

TeamId winner = TeamId::None;
int configState = 0;
bool MainNode = false;

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
        if ((captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() / 2) && controlPoint.getControllingTeam() != TeamId::None)
        {
            controlPoint.setControllingTeam(TeamId::None);
            Serial.println("Point neutralized!");
        }
        if (captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() && controlPoint.getControllingTeam() != TeamId::Blufor)
        {
            controlPoint.setControllingTeam(TeamId::Blufor);
            Serial.println("Team Blue fully captured the point!");
        }
        gracePeriodActive = false; 
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        displayOLED.displayCapturing(capturingTeam, (percentage * 100));
    }
    else if (buttonManager.yellowButton.getState() == LOW)
    {
        if (capturingTeam != TeamId::YellowFor && controlPoint.getControllingTeam() != TeamId::YellowFor)
        {
            capturingTeam = TeamId::YellowFor;
            captureClock.start();
            captureClock.reset();
            Serial.println("Team Yellow started capturing!");
        }
        if ((captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() / 2) && controlPoint.getControllingTeam() != TeamId::None)
        {
            controlPoint.setControllingTeam(TeamId::None);
            Serial.println("Point neutralized!");
        }
        if (captureClock.getElapsedTimeInSeconds() >= config.getCaptureTime() && controlPoint.getControllingTeam() != TeamId::YellowFor)
        {
            controlPoint.setControllingTeam(TeamId::YellowFor);
            Serial.println("Team Yellow fully captured the point!");
        }
        gracePeriodActive = false; 
        float percentage = static_cast<float>(captureClock.getElapsedTimeInSeconds()) / static_cast<float>(config.getCaptureTime());
        displayOLED.displayCapturing(capturingTeam, (percentage * 100));
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
    gameState = GameState::Config;
    controlPoint.addTeam(TeamId::Blufor);
    controlPoint.addTeam(TeamId::YellowFor);
    buttonManager.begin();
    Serial.println("Ready");
}

void loop()
{
    switch (gameState)
    {
    case GameState::Config:
        config.handleButtonPresses(buttonManager, configState);
        if (buttonManager.startButton.isPressed())
        {
            gameState = GameState::CountDown;
            secondCLock.start();
        }
        displayOLED.displaySettings(config, configState);
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
            secondCLock.stop();
            secondCLock.reset();
            gameState = GameState::Ongoing;
            pointClock.start();
            gameClock.start();
        }
        displayOLED.displayCountdown(config.getCountdown());
        break;
    case GameState::Ongoing:
        if (pointClock.getElapsedTime() >= 60000) // like the last one :/ will do later
        {
            Serial.println("Time elapsed: ");
            Serial.println(gameClock.getElapsedTimeInMinutes());
            controlPoint.increamentScore(1);
            if (isGameOver())
            {
                gameState = GameState::Finished;
                gameClock.stop();
                gameClock.reset();

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
    buttonManager.update();
}
