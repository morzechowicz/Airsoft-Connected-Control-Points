#ifndef CONTROL_POINT_H
#define CONTROL_POINT_H

#include "Team.h"
#include "Teams.h"
#include <cstddef>
#include <Arduino.h>

class ControlPoint
{
private:
    int nodeId;
    Team *teams;
    size_t maxTeams;
    size_t teamCount;
    bool GameMaster = false;
    int MasterNode = 0;



public:
    ControlPoint(size_t maxTeams) : maxTeams(maxTeams)
    {
        Serial.println("ControlPoint initialized");
        teams = new Team[2];
    }
    struct Nodes
    {
        int nodeId;
        TeamId controllingTeam;
    };
    Nodes nodes[20];
    size_t nodeCount = 0;

    void setControllingTeam(TeamId teamId, int nodeId);
    TeamId getControllingTeam();
    void increamentScore(int points);
    void addTeam(TeamId teamid);
    bool pointsTargetReached(int pointsTarget);
    TeamId whoWon();
    void addNode(int nodeId);
    void resetGame();
    

    int getTeamPoints(TeamId teamId);
    int getNodeId() {return nodeId;}
    void setTeamsScore(int teamBlueScore, int teamYellowScore);
    bool getGameMaster() { return GameMaster; }
    void setNodeId(int id) { nodeId = id; }
    void setGameMaster (bool isGameMaster) { GameMaster = isGameMaster; }
    int getNodeCount() { return nodeCount; }
    void setGameMasterNode(int node) {MasterNode = node;}
    int getGameMasterNode() { return MasterNode;}
};

#endif