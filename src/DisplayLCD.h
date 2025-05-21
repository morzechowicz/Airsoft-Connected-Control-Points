#ifndef DISPLAY_LCD_H  // Header guard to prevent multiple inclusions
#define DISPLAY_LCD_H

#include <Display.h>
#include "ControlPoint.h"

class DispalyLCD : public Display
{
public:
    void displaySettings(Config config) override;
    void displayCountdown(int countdown) override;
    void displayGame(ControlPoint controlPoint) override;
    void displayCapturing(TeamId capturingTeam,TeamId currentTeam) override;
    void displayFinished(TeamId winner,ControlPoint controlPoint) override;
};

#endif