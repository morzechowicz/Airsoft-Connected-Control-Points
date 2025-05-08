#include "GameManager.h"


GameManager::GameManager() : localTeamsScore{ {TEAM_BLUE, 0}, {TEAM_YELLOW, 0} }, totalTeamScore{ {0, {0, 0}} } {}


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

void GameManager::changeLedColor(int teamId)
{
    if (teamId == TEAM_BLUE)
    {
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_BLUE, HIGH);
    }
    else if (teamId == TEAM_YELLOW)
    {
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_YELLOW, HIGH);
    }
    else
    {
        digitalWrite(LED_BLUE, LOW);
        digitalWrite(LED_YELLOW, LOW);
    }
}

void GameManager::updateTotalTeamScore(uint16_t NodeId,Team *singleScore)
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