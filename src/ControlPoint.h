#ifndef CONTROL_POINT_H
#define CONTROL_POINT_H

#include "Team.h"
#include "Teams.h"
#include <cstddef>

class ControlPoint {
private:
    TeamId controllingTeamId;
    Team localTeams[2];

public:
    ControlPoint() : controllingTeamId(TeamId::None) {}

    void setControllingTeam(TeamId teamId) { controllingTeamId = teamId; }
    TeamId getControllingTeam() const { return controllingTeamId; }
    bool isControlled() const { return controllingTeamId != TeamId::None; }

    void setLocalTeams(const Team* team, size_t teamCount);
};

#endif