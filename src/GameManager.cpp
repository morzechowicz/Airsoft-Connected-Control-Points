#include "GameManager.h"


GameManager::GameManager() : localTeamsScore{{TEAM_BLUE, 0}, {TEAM_YELLOW, 0}} {}

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