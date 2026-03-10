#include "FLAGServer.h"

FLAGServer::FLAGServer(EventBus *eb, HardwareManager *hw, NetworkManager *net, const FLAGConfig &cfg)
    : BaseComponent(eb, hw, net),
      config(cfg),
      gameStartTime(0),
      lastScoreUpdate(0),
      gameRunning(false)
{
    Serial.print("Initializing FLAG with ");
    Serial.print(cfg.initTeamCount);
    Serial.println(" teams");

    // Initialize scoring 0 is none so skip it
    for (uint8_t i = 1; i < cfg.initTeamCount; i++)
    {
        score.AddTeam();

        Serial.print("Added ");
        Serial.println(getFlagTeamName((FlagTeam)i));
    }

    // if i get idea for using the network i will
    if (true)
    {
        Serial.println("Single node mode - network messages disabled");
    }
}

FLAGServer::~FLAGServer()
{
    eventBus->unsubscribe(KOTH_POINT_CAPTURED);
    eventBus->unsubscribe(PAUSE);
    eventBus->unsubscribe(GAME_STARTED);

    stopModeTask();
}

void FLAGServer::enterMode()
{
    Serial.println("=== FLAG Server Mode ===");

    // Reset game state
    gameStartTime = millis();
    lastScoreUpdate = millis();
    gameRunning = true;

    // Reset teams
    for (uint8_t i = 0; i < FlagTeam::MAX_TEAMS-1; i++)
    {
        score.teams[i].score = 0;
    }
    score.controler = FlagTeam::NONE;

    // Subscribe to events
    eventBus->subscribe(FLAG_CAPTURED, [this](Event e)
                        { onCaptureRequest(e); });
    eventBus->subscribe(PAUSE, [this](Event e)
                        { pauseGame(); });

    eventBus->publish(GAME_STARTED, 0);

    // Broadcast game start same as above
    // if (false)
    // {
    //     String startMsg = Protocol::buildGameStart();
    //     if (networkManager)
    //     {
    //         networkManager->broadcast(startMsg);
    //     }
    // }
    Serial.println("FLAG Server ready!");
}

void FLAGServer::exitMode()
{
    gameRunning = false;
    Serial.println("FLAG Server exiting...");
}

void FLAGServer::run()
{
    Serial.println("FLAGServer::run() starting main loop");

    unsigned long lastUpdate = millis();

    while (taskRunning)
    {
        if (gameRunning)
        {
            // Update score at intervals
            if (millis() - lastUpdate >= config.scoreIntervalMs)
            {
                updateScore();
                lastUpdate = millis();
                scoringInterval++;
            }

            // Check win conditions
            if (isGameOver())
            {
                FlagTeam winner = determineWinner();
                endGame(winner);
                break; // Exit loop
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    Serial.println("FLAGServer::run() exiting loop");
}

void FLAGServer::onCaptureRequest(Event e)
{
    // Extract packed data:
    uint8_t teamId = e.data1;
    FlagTeam team = (FlagTeam)teamId;

    processCaptureRequest(team);
}

void FLAGServer::processCaptureRequest(FlagTeam team)
{
    // Validate team
    if ((uint8_t)team > FlagTeam::MAX_TEAMS || (uint8_t)team < 0)
    {
        Serial.print("[SERVER] ");
        Serial.println("Invalid team in capture request");
        return;
    }

    // Check if already controlled by this team
    if (score.controler == team)
    {
        Serial.print("[SERVER] ");
        Serial.println(" already controlled by this team");
        return;
    }

    // Capture the node!
    score.controler = team;

    Serial.print("[SERVER] ");
    Serial.print(" captured by ");
    Serial.println(getFlagTeamName(team));
}

void FLAGServer::updateScore()
{
    // Add score to the controling team
    FlagTeam controler = score.controler;
    if (controler != FlagTeam::NONE)
    {
        score.teams[controler].score++;
    }
    Serial.print("[SERVER] ");
    Serial.print("Team: ");
    Serial.print(getFlagTeamName(controler));
    Serial.print(" controls points with");
    Serial.print(score.teams[controler].score);
    Serial.println(" points");
    // Broadcast score update
    // if (!config.singleNodeMode)
    // {
    //     broadcastScoreUpdate();
    // }
    eventBus->publish(FLAG_SCORE_UPDATE, score.controler, score.teams[controler].score);
}

void FLAGServer::broadcastScoreUpdate()
{
    // maybe later
}

void FLAGServer::checkWinConditions()
{
    if (isGameOver())
    {
        FlagTeam winner = determineWinner();
        endGame(winner);
    }
}

void FLAGServer::endGame(FlagTeam winner)
{
    Serial.print("[SERVER] ");
    Serial.println("========== GAME OVER ==========");
    Serial.print("Winner: ");
    Serial.println(getFlagTeamName(winner));
    Serial.print("Final Score - ");
    Serial.print(score.teams[winner].score);
    Serial.println("===============================");

    // Broadcast game over
    // String msg = Protocol::buildGameOver((uint8_t)winner);
    // if (false)
    // {
    //     if (networkManager)
    //     {
    //         networkManager->broadcast(msg);
    //     }
    // }

    // Publish local event
    eventBus->publish(GAME_OVER, (int)winner);

    gameRunning = false;
}

void FLAGServer::pauseGame()
{
    if (gameRunning)
    {
        gameRunning = !gameRunning;
    }
}

bool FLAGServer::isGameOver()
{
    // Check point limit
    for (int i = 0; i < config.initTeamCount; i++)
    {
        if (score.teams[i].score > config.maxPoints)
        {
            return true;
        }
    }

    // Check time limit
    if (scoringInterval >= config.maxTime)
    {
        return true;
    }

    return false;
}

FlagTeam FLAGServer::determineWinner()
{
    return score.getWinner();
}
