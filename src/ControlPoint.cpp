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
    // this will go through all teams array and add points if they controll point
    for (int i = 0; i < nodeCount; i++)
    {
        if (nodes[i].controllingTeam != TeamId::None)
        {
            for (int j = 0; j < teamCount; j++)
            {
                if (teams[j].getTeamId() == nodes[i].controllingTeam)
                {
                    teams[j].addPoints(points);
                    Serial.print("Team ");
                    Serial.print(static_cast<int>(teams[j].getTeamId()));
                    Serial.print(" scored ");
                    Serial.println(points);
                }
            }
        }
    }
}

TeamId ControlPoint::whoWon()
{
    int currentHighiest = 0;
    TeamId currentWinner = TeamId::None;

    for (int i = 0; i < teamCount; ++i)
    {
        if (teams[i].getTeamPoints() > currentHighiest)
        {
            currentHighiest = teams[i].getTeamPoints();
            currentWinner = teams[i].getTeamId();
        }
        else if (teams[i].getTeamPoints() == currentHighiest)
        {
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

void ControlPoint::setTeamsScore(int teamBlueScore, int teamYellowScore)
{
    {
        teams[0].setTeamPoints(teamBlueScore);
        teams[1].setTeamPoints(teamYellowScore);
    }
}

void ControlPoint::addNode(int nodeId)
{
    bool present = false;
    for (size_t i = 0; i < nodeCount; i++)
    {
        if (nodes[i].nodeId == nodeId)
        {
            present = true;
        }
    }
    if (!present)
    {
        nodeCount++;
        nodes[nodeCount - 1].nodeId = nodeId;
        nodes[nodeCount - 1].controllingTeam = TeamId::None;
        Serial.print("Node added Id: ");
        Serial.print(nodeId);
        Serial.print(" Node count: ");
        Serial.println(nodeCount);
    }
    if (present)
    {
        Serial.println("Node already exists Id: ");
        Serial.print(nodeId);
    }
}

void ControlPoint::setControllingTeam(TeamId teamId, int nodeId)
{
    for (size_t i = 0; i < nodeCount; i++)
    {
        if (nodes[i].nodeId == nodeId)
        {
            controllingTeamId = teamId;
            nodes[i].controllingTeam = teamId;
            Serial.print("Node ");
            Serial.print(nodeId);
            Serial.print(" is controlled by ");
            switch (teamId)
            {
            case TeamId::Blufor:
                Serial.println("Blue");
                break;
            case TeamId::YellowFor:
                Serial.println("Yellow");
                break;
            case TeamId::None:
                Serial.println("None");
                break;
            default:
                break;
            }
        }
    }
}
