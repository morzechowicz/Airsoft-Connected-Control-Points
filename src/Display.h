#ifndef DISPLAY_H  // Header guard to prevent multiple inclusions
#define DISPLAY_H

#include <Config.h>

class Display
{
public:
    virtual void displaySettings(Config config);
    virtual void displayCountdown(int countdown);
    virtual void displayGame(ControlPoint controlPoint);
    virtual void displayCapturing(TeamId capturingTeam,TeamId currentTeam);
    virtual void displayFinished(TeamId winner,ControlPoint controlPoint);
};

#endif