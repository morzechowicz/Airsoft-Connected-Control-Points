#ifndef TEAM_H  // Header guard to prevent multiple inclusions
#define TEAM_H

class Team
{
private:
    int teamId;     
    int teamPoints;
    
public:
    Team() : teamId(0), teamPoints(0) {}
    Team(int id, int points) : teamId(id), teamPoints(points) {}

    void addPoints(int points);

    int getTeamId() const { return teamId; }
    int getTeamPoints() const { return teamPoints; }

    void setTeamPoints(int points) { teamPoints = points; }
};

#endif 