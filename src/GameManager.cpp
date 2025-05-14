#include "GameManager.h"

GameManager::GameManager() : localTeamsScore{{TEAM_BLUE, 0},
                                             {TEAM_YELLOW, 0}},
                             totalTeamScore{{0, {0, 0}}},
                             maxScore(DEFAULT_MAX_SCORE), maxTime(DEFAULT_MAX_TIME), timeToStart(DEFAULT_TIME_TO_START), timeToCapture(DEFAULT_TIME_TO_CAPTURE), currentTime(0), currentSettingId(0),
                             currentGameState(GAME_STATE_CONFIG), pointControlledByTeam(TEAM_NONE), addPointFlag(false), addSecondFlag(false)
{
}
// i have and idea of preet battlefield with some point allready under control
// TODO: ask others what they think about this

void GameManager::updateTeamScore(int teamId, int score)
{
    for (int i = 0; i < 2; i++)
    {
        if (localTeamsScore[i].id == teamId)
        {
            localTeamsScore[i].score += score;
            break;
        }
    }
}

void GameManager::changeTeamControllingPoint(int teamId)
{
    if (teamId == TEAM_BLUE)
    {
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_BLUE, HIGH);
        pointControlledByTeam = TEAM_BLUE;
    }
    else if (teamId == TEAM_YELLOW)
    {
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_YELLOW, HIGH);
        pointControlledByTeam = TEAM_YELLOW;
    }
    else
    {
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_YELLOW, LOW);
        pointControlledByTeam = TEAM_NONE;
    }
}

void GameManager::updateTotalTeamScore(uint16_t NodeId, Team *singleScore)
{
    for (int i = 0; i < 10; i++)
    {
        if (totalTeamScore[i].NodeId == NodeId)
        {
            totalTeamScore[i].score[0] = singleScore[0];
            totalTeamScore[i].score[1] = singleScore[1];
            return;
        }
    }

    for (int i = 0; i < 10; i++)
    {
        if (totalTeamScore[i].NodeId == 0)
        {
            totalTeamScore[i].NodeId = NodeId;
            totalTeamScore[i].score[0] = singleScore[0];
            totalTeamScore[i].score[1] = singleScore[1];
            return;
        }
    }
}

int GameManager::getTotalScore(int TeamId)
{
    int totalScore = 0;
    for (int i = 0; i < 10; i++)
    {
        if (totalTeamScore[i].NodeId != 0)
        {
            totalScore += totalTeamScore[i].score[TeamId].score;
        }
    }
    return totalScore;
}

void GameManager::changeConfigAction(ezButton &changeConfigButton)
{
    if (changeConfigButton.isPressed())
    {
        Serial.println("Both buttons pressed");
        currentSettingId++;
        if (currentSettingId > 4)
        {
            currentSettingId = 0;
        }
    }
}

void GameManager::yellowButtonConfigAction(ezButton &teamBlueButton)
{
    if (teamBlueButton.isPressed())
    {
        Serial.println("Blue button released");
        switch (currentSettingId)
        {
        case 1:
            if (maxScore > 15)
            {
                maxScore = maxScore - 15;
            }
            break;
        case 2:
            if (maxTime > 15)
            {
                maxTime = maxTime - 15;
            }
            break;
        case 3:
            if (timeToStart > 30)
            {
                timeToStart = timeToStart - 30;
            }
            break;
        case 4:
            if (timeToCapture > 1)
            {
                timeToCapture = timeToCapture - 1;
            }
            break;
        default:
            break;
        }
    }
}

void GameManager::blueButtonConfigAction(ezButton &teamYellowButton)
{
    if (teamYellowButton.isPressed())
    {
        Serial.println("Yellow button released");
        switch (currentSettingId)
        {
        case 1:
            if (maxScore < 150)
            {
                maxScore = maxScore + 15;
            }
            break;
        case 2:
            if (maxScore < 300)
            {
                maxTime = maxTime + 15;
            }
            break;
        case 3:
            if (timeToStart < 600)
            {
                timeToStart = timeToStart + 30;
            }
            break;
        case 4:
            if (timeToCapture < 30)
            {
                timeToCapture = timeToCapture + 1;
            }
            break;
        default:
            break;
        }
    }
}

void GameManager::initializeLoop(ezButton &teamBlueButton, ezButton &teamYellowButton, ezButton &startGameButton, ezButton changeConfigButton, void (*sendGameStatus)(int))
{
    blueButtonConfigAction(teamBlueButton);
    yellowButtonConfigAction(teamYellowButton);
    changeConfigAction(changeConfigButton);
    startCountDownAction(startGameButton);
}

void GameManager::gameLoop(ezButton &teamBlueButton, ezButton &teamYellowButton, ezButton &startGameButton, uint16_t NodeId, uint16_t LeaderId)
{
    if (teamYellowButton.isPressed())
    {
        changeTeamControllingPoint(TEAM_YELLOW);
    }
    if (teamBlueButton.isPressed())
    {
        changeTeamControllingPoint(TEAM_BLUE);
    }
    if (addPointFlag)
    {
        if (pointControlledByTeam == TEAM_BLUE)
        {
            updateTeamScore(TEAM_BLUE, 1);
        }
        else if (pointControlledByTeam == TEAM_YELLOW)
        {
            updateTeamScore(TEAM_YELLOW, 1);
        }
        addPointFlag = false;
    }
    if (maxScore <= getTotalScore(TEAM_BLUE) || maxScore <= getTotalScore(TEAM_YELLOW))
    {
        endGameAction();
    }
    if (maxTime < 0)
    {
        endGameAction();
    }
}

void GameManager::minuteCallBack(GameManager *instance)
{
    instance->addPoint();
    instance->maxTime--;
}

void GameManager::addPoint()
{
    addPointFlag = true;
}

void GameManager::secondCallBack(GameManager *instance)
{
    instance->addSecond();
    instance->timeToStart--;
}

void GameManager::addSecond()
{
    addSecondFlag = true;
}
void GameManager::startCountDownAction(ezButton &button)
{
    if (button.isPressed())
    {
        Serial.println("Start countdown");
        currentGameState = GAME_STATE_COUNTDOWN;
        secondTicker.attach(1, secondCallBack, this);
    }
}

int GameManager::endGameAction()
{
    currentGameState = GAME_STATE_END;
    currentSettingId = 0;
    maxScore = DEFAULT_MAX_SCORE;
    maxTime = DEFAULT_MAX_TIME;
    timeToStart = DEFAULT_TIME_TO_START;
    timeToCapture = DEFAULT_TIME_TO_CAPTURE;
    if (totalTeamScore->score[0].score > totalTeamScore->score[1].score)
    {
        return TEAM_BLUE;
        changeTeamControllingPoint(TEAM_BLUE);
    }
    else if (totalTeamScore->score[0].score < totalTeamScore->score[1].score)
    {
        return TEAM_YELLOW;
        changeTeamControllingPoint(TEAM_YELLOW);
    }
    return TEAM_NONE;
}

void GameManager::countdownLoop(void (*sendGameStatus)(int))
{
    // what i want to do is to count down from timeToStart to 0
    // and when it reach 0 i want to start the game
    if (addSecondFlag)
    {
        Serial.print("Time to start: ");
        Serial.println(timeToStart);
        addSecondFlag = false;
    }

    if (timeToStart < 0)
    {
        Serial.println("Start game");
        currentGameState = GAME_STATE_PLAYING;
        sendGameStatus(currentGameState);
        minuteTicker.attach(60, minuteCallBack, this);
    }
}

void GameManager::endGameLoop()
{
    // Why i created this function?
    // does it even need a loop? loop for whtat? i need to thnik about this later
}