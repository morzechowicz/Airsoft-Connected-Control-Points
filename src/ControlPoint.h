#ifndef CONTROL_POINT_H
#define CONTROL_POINT_H

#include "Team.h"
#include "Teams.h"
#include <cstddef>
#include <Arduino.h>

class ControlPoint {
private:
    TeamId controllingTeamId;
    Team* teams;
    size_t maxTeams;
    size_t teamCount;

public:
    ControlPoint(size_t maxTeams) : controllingTeamId(TeamId::None),maxTeams(maxTeams)
    {
        Serial.println("ControlPoint initialized");
        teams = new Team[2];
    }

    void setControllingTeam(TeamId teamId) { controllingTeamId = teamId; }
    TeamId getControllingTeam() const { return controllingTeamId; }
    bool isControlled() const { return controllingTeamId != TeamId::None; }
    void increamentScore(int points);
    void addTeam(TeamId teamid);
    bool pointsTargetReached(int pointsTarget);
    TeamId whoWon();

    int getTeamPoints(TeamId teamId);
};

#endif