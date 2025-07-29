#ifndef DISPLAY_H  // Header guard to prevent multiple inclusions
#define DISPLAY_H

#include <Config.h>
#include <ControlPoint.h>
#include <Teams.h>

class Display
{
public:
    virtual void displaySettings(Config config,int configState);
    virtual void displayCountdown(int countdown);
    virtual void displayGame(ControlPoint controlPoint, int timeLeft);
    virtual void displayCapturing(TeamId capturingTeam, float progres) ;
    virtual void displayFinished(TeamId winner,ControlPoint controlPoint);
    virtual void displayNetworkStatus(int nodesCount, bool leaderStatus, bool connecting, String &lastLoraMsg,int cpID);
    virtual void displayInitLogo();
};

#endif