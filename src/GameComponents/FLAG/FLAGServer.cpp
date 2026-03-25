#include "FLAGServer.h"

FLAGServer::FLAGServer(EventBus *eb, HardwareManager *hw, NetworkManager *net, const FLAGConfig &cfg)
    : BaseComponent(eb, hw, net),
      config(cfg),
      gameStartTime(0),
      lastScoreUpdate(0),
      gameRunning(false)
{
    LOG_INFO("FLAG_SERVER", "Initializing FLAG Server with %d teams", cfg.initTeamCount);

    // Initialize scoring 0 is none so skip it
    for (uint8_t i = 1; i < cfg.initTeamCount; i++)
    {
        score.AddTeam();
        LOG_INFO("FLAG_SERVER", "Added team: %s", getFlagTeamName((FlagTeam)i));

    }

    // if i get idea for using the network i will
    if (true)
    {
        LOG_DEBUG("FLAG_SERVER", "Single node mode - network messages disabled");
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
    LOG_INFO("FLAG_SERVER", "Entering FLAG Server Mode");  

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
    LOG_INFO("FLAG_SERVER", "FLAG Server ready!");
}

void FLAGServer::exitMode()
{
    gameRunning = false;
    LOG_INFO("FLAG_SERVER", "FLAG Server exiting...");
}

void FLAGServer::run()
{
    LOG_INFO("FLAG_SERVER", "FLAGServer::run() starting main loop");

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

    LOG_INFO("FLAG_SERVER", "FLAGServer::run() exiting loop");
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
        LOG_ERROR("FLAG_SERVER", "Invalid team in capture request");
        return;
    }

    // Check if already controlled by this team
    if (score.controler == team)
    {
        LOG_INFO("FLAG_SERVER", " already controlled by this team");
        return;
    }

    // Capture the node!
    score.controler = team;

    LOG_INFO("FLAG_SERVER", " captured by %s", getFlagTeamName(team));
}

void FLAGServer::updateScore()
{
    // Add score to the controling team
    FlagTeam controler = score.controler;
    if (controler != FlagTeam::NONE)
    {
        score.teams[controler].score++;
    }
    LOG_INFO("FLAG_SERVER", "Team: %s controls points with %d points", getFlagTeamName(controler), score.teams[controler].score);
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
    LOG_INFO("FLAG_SERVER", "========== GAME OVER ==========");
    LOG_INFO("FLAG_SERVER", "Winner: %s", getFlagTeamName(winner));
    LOG_INFO("FLAG_SERVER", "Final Score - %d", score.teams[winner].score);

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
