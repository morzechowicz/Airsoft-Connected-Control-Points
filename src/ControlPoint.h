#ifndef CONTROL_POINT_H
#define CONTROL_POINT_H

#include "Team.h"
#include "Teams.h"
#include <cstddef>
#include <Arduino.h>

class ControlPoint
{
private:
    TeamId controllingTeamId;
    int nodeId;
    Team *teams;
    size_t maxTeams;
    size_t teamCount;
    bool leader = false;
    struct Nodes
    {
        int nodeId;
        TeamId controllingTeam;
    };


public:
    ControlPoint(size_t maxTeams) : controllingTeamId(TeamId::None), maxTeams(maxTeams)
    {
        Serial.println("ControlPoint initialized");
        teams = new Team[2];
    }
    Nodes nodes[20];
    size_t nodeCount = 0;

    void setControllingTeam(TeamId teamId) { controllingTeamId = teamId; }
    TeamId getControllingTeam() const { return controllingTeamId; }
    bool isControlled() const { return controllingTeamId != TeamId::None; }
    void increamentScore(int points);
    void addTeam(TeamId teamid);
    bool pointsTargetReached(int pointsTarget);
    TeamId whoWon();
    void addNode(int nodeId);
    void setNodeControllingTeam(int nodeId,TeamId teamId);
    

    int getTeamPoints(TeamId teamId);
    int getNodeId() {return nodeId;}
    bool getLeader() { return leader; }
    void setNodeId(int id) { nodeId = id; }
    void setLeader (bool isLeader) { leader = isLeader; }
};

#endif