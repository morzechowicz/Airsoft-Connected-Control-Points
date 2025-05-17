#include <Arduino.h>
#include <Team.h>
#include <Teams.h>
#include <LoRaManager.h>
#include <RadioLib.h>
#include <ButtonManager.h>
#include <GameState.h>
#include <DisplayOled.h>
#include <Config.h>
#include <GameLogic.h>
#include <ControlPoint.h>
#include <Clocker.h>

GameState gameState;
Team blue(TeamId::Blufor, 0);
Team yellow(TeamId::YellowFor, 0);
SX1278 radio = new Module(18, 14, 26, 33);
LoRaManager loRaManager(radio);
ButtonManager buttonManager;
ControlPoint controlPoint;
DisplayOled displayOLED;
Team teams[2] = {blue, yellow};
Config config(15, 15, 15, 15);
GameLogic gameLogic;
Clocker pointClock;
Clocker secondCLock;
Clocker gameClock;

TeamId winner = TeamId::None;
int configState = 0;
bool MainNode = false;

bool isGameOver()
{
  // time overflow
  if (gameClock.getElapsedTime() > config.getDurration())
  {
    Serial.println("TIMES UP");
    return true;
  }
  // winning by points
  for (int i = 0; i < sizeof(teams); ++i)
  {
    if (teams[i].getTeamPoints() > config.getPointsTarget())
    {
      return true;
    }
  }
  return false;
}

TeamId whoWon()
{
  int currentHighiest = 0;
  TeamId currentWinner = TeamId::None;
  for (int i = 0; i < sizeof(teams); ++i)
  {
    if (teams[i].getTeamPoints() > currentHighiest)
    {
      currentHighiest = teams[i].getTeamPoints();
      if(teams[i].getTeamPoints() == currentHighiest)
      {
        currentWinner = TeamId::None; // maybe i should have one more type for draw?
      }else{
        currentWinner = teams[i].getTeamId();
      }
    }
  }

  return currentWinner;
}

void setup()
{
  Serial.begin(115200);
  gameState = GameState::Config;
  controlPoint.setLocalTeams(teams, 2);
}

void loop()
{
  switch (gameState)
  {
  case GameState::Config:
    config.handleButtonPresses(buttonManager, configState);
    if (buttonManager.configButton2.isPressed())
    {
      gameState = GameState::CountDown;
      secondCLock.start();
    }
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
    break;
  case GameState::Ongoing:
    if (pointClock.getElapsedTime() >= 60000) // like the last one :/ will do later
    {
      TeamId controllingTeam = controlPoint.getControllingTeam();
      if (controllingTeam != TeamId::None)
      {
        for (int i = 0; i < sizeof(teams); ++i)
        {
          if (teams[i].getTeamId() == controllingTeam)
          {
            teams[i].addPoints(1); // yes i know ill change it later
            Serial.print("Team ");
            Serial.print((controllingTeam == TeamId::Blufor) ? "Blue" : "Yellow");
            Serial.println(" scored a point!");
            break;
          }
        }
      }

      if (isGameOver())
      {
        gameState = GameState::Finished;
        gameClock.stop();
        gameClock.reset();

        winner = whoWon();
        Serial.println("GAME OVER");
        if(winner == TeamId::Blufor)
        {
          Serial.print("BLUFOR WON");
        }
         if(winner == TeamId::YellowFor)
        {
          Serial.print("YELLOWFOR WON");
        }
        if(winner == TeamId::None)
        {
          Serial.print("DRAW");
        }
      }
      pointClock.reset();
      pointClock.stop();
    }
    // Capture Logic
    static Clocker captureClock;                // Tracks how long a button is held
    static TeamId capturingTeam = TeamId::None; // Tracks which team is trying to capture

    if (buttonManager.playerButton1.isPressed())
    {
      if (capturingTeam != TeamId::Blufor)
      {
        capturingTeam = TeamId::Blufor;
        captureClock.reset(); // Start tracking capture time
        Serial.println("Team Blue started capturing!");
      }
      if (captureClock.getElapsedTime() >= config.getCaptureTime() / 2)
      {
        controlPoint.setControllingTeam(TeamId::None); // Neutralize the point
        Serial.println("Point neutralized!");
      }
      if (captureClock.getElapsedTime() >= config.getCaptureTime())
      {
        controlPoint.setControllingTeam(TeamId::Blufor); // Fully captured
        Serial.println("Team Blue fully captured the point!");
      }
    }
    else if (buttonManager.playerButton2.isPressed())
    {
      if (capturingTeam != TeamId::YellowFor)
      {
        capturingTeam = TeamId::YellowFor;
        captureClock.reset(); // Start tracking capture time
        Serial.println("Team Yellow started capturing!");
      }
      if (captureClock.getElapsedTime() >= config.getCaptureTime() / 2)
      {
        controlPoint.setControllingTeam(TeamId::None); // Neutralize the point
        Serial.println("Point neutralized!");
      }
      if (captureClock.getElapsedTime() >= config.getCaptureTime())
      {
        controlPoint.setControllingTeam(TeamId::YellowFor); // Fully captured
        Serial.println("Team Yellow fully captured the point!");
      }
    }
    else
    {
      // If no button is pressed, reset capture logic
      // YES I WANT IT THIS WAY
      capturingTeam = TeamId::None;
      captureClock.reset();
    }
    break;
  case GameState::Finished:
    //here we just display winner until idk button press or what ever
    if(buttonManager.configButton1.isPressed())
    {
      gameState = GameState::Config;
    }
    break;
  default:
    // maybe just set state to config? not like there are any other ways default could happen
    gameState = GameState::Config;
    break;
  }
}
