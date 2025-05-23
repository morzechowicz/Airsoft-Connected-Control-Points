#include <controlpoint.h>

void ControlPoint::addTeam(TeamId teamid)
{
    if (teamCount < maxTeams)
    {
        teams[teamCount] = Team(teamid, 0);
        teamCount++;
    }
    else
    {
        Serial.println("Max teams reached");
    }
}

void ControlPoint::increamentScore(int points)
{
    TeamId controllingTeam = controllingTeamId;
    if (controllingTeam != TeamId::None)
    {
        for (int i = 0; i < teamCount; ++i)
        {
            if (teams[i].getTeamId() == controllingTeam)
            {
                teams[i].addPoints(points); // yes i know ill change it later
                Serial.print("Team ");
                Serial.print((controllingTeam == TeamId::Blufor) ? "Blue" : "Yellow");
                Serial.println(" scored a point!");
                break;
            }
        }
    }
}

TeamId ControlPoint::whoWon() {
    int currentHighiest = 0;
    TeamId currentWinner = TeamId::None;


    for (int i = 0; i < teamCount; ++i) {
        if (teams[i].getTeamPoints() > currentHighiest) {
            currentHighiest = teams[i].getTeamPoints();
            currentWinner = teams[i].getTeamId();

        } else if (teams[i].getTeamPoints() == currentHighiest) {
            currentWinner = TeamId::Draw; 
        }
    }

    return currentWinner; 
}

bool ControlPoint::pointsTargetReached(int pointsTarget)
{
  for (int i = 0; i < teamCount; ++i)
  {
    if (teams[i].getTeamPoints() >= pointsTarget)
    {
      return true;
    }
  }
  return false;
}

int ControlPoint::getTeamPoints(TeamId teamId)
{
    for (int i = 0; i < teamCount; ++i)
    {
        if (teams[i].getTeamId() == teamId)
        {
            return teams[i].getTeamPoints();
        }
    }
    return 0;
}

void ControlPoint::addNode(int nodeId)
{
    bool present = false;
    for (size_t i = 0; i < nodeCount; i++)
    {
        if (nodes[i].nodeId == nodeId)
        {
            Serial.println("Node already exists");
            present = true;
        }
    }
    if(!present)
    {
        nodeCount++;
        nodes[nodeCount - 1].nodeId = nodeId;
        nodes[nodeCount - 1].controllingTeam = TeamId::None;
    }
    if(present)
    {
        Serial.println("Node already exists Id: ");
        Serial.print(nodeId);
    }
    
}

void ControlPoint::setNodeControllingTeam(int nodeId, TeamId teamId)
{
    for (size_t i = 0; i < nodeCount; i++)
    {
        if (nodes[i].nodeId == nodeId)
        {
            nodes[i].controllingTeam = teamId;
            Serial.print("Node ");
            Serial.print(nodeId);
            Serial.print(" is controlled by ");
            Serial.println((teamId == TeamId::Blufor) ? "Blue" : "Yellow");
        }
    }
}