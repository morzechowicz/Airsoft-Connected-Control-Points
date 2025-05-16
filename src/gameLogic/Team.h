#ifndef TEAM_H  // Header guard to prevent multiple inclusions
#define TEAM_H

#include <Teams.h>

class Team
{
private:
    TeamId teamId;     
    int teamPoints;
    
public:
    Team() : teamId(), teamPoints(0) {}
    Team(TeamId id, int points) : teamId(id), teamPoints(points) {}

    void addPoints(int points);

    TeamId getTeamId() const { return teamId; }
    int getTeamPoints() const { return teamPoints; }

    void setTeamPoints(int points) { teamPoints = points; }
};

#endif 