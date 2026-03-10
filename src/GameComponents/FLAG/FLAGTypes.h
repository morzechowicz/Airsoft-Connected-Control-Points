#ifndef FLAG_TYPES_H
#define FLAG_TYPES_H

#include <Arduino.h>

enum FlagTeam
{
    NONE,
    TEAM_A,
    TEAM_B,
    TEAM_C,
    TEAM_D,
    TEAM_E,
    TEAM_F,

    MAX_TEAMS
};

static const char *FlagTeamNames[] = {
    "NONE",
    "TEAM_A",
    "TEAM_B",
    "TEAM_C",
    "TEAM_D",
    "TEAM_E",
    "TEAM_F",
    "MAX_TEAMS"};

inline const char* getFlagTeamName(FlagTeam team) {
    return FlagTeamNames[team];
}

struct TeamScore
{
    FlagTeam team;
    uint16_t score;
};

struct FLAGGameScore
{
    TeamScore teams[MAX_TEAMS - 1];
    uint8_t teamsCount;
    FlagTeam controler;

    FLAGGameScore() : teamsCount(0), controler(FlagTeam::NONE)
    {
        for (int i = 0; i < FlagTeam::MAX_TEAMS-1; i++)
        {
            teams[i].team = (FlagTeam)i;
            teams[i].score = 0;
        }
    }

    void AddTeam()
    {
        if (teamsCount < MAX_TEAMS)
        {
            teamsCount++;
        }
    }

    FlagTeam getWinner()
    {
        FlagTeam winner;
        uint16_t winningScore = 0;
        for (int i = 1; i < FlagTeam::MAX_TEAMS-1; i++)
        {
            if (teams[i].score > winningScore)
            {
                winner = teams[i].team;
            }
        }
        return winner;
    }

    FlagTeam nextTeam(FlagTeam team, uint8_t teams)
    {
        uint8_t nextId = team + 1;
        if (nextId >= teams)
        {
            nextId = 1;
        }
        FlagTeam next = (FlagTeam)nextId;

        return next;
    }

    FlagTeam previousTeam(FlagTeam team, uint8_t teams)
    {
        uint8_t backId = team + 1;
        if (backId <= teams)
        {
            backId = teams;
        }
        FlagTeam back = (FlagTeam)backId;

        return back;
    }
};

struct FLAGConfig
{
    uint8_t initTeamCount;
    uint16_t maxTime;
    uint16_t maxPoints;

    unsigned long captureTime = 1;
    unsigned long scoreIntervalMs = 10000;
};
#endif