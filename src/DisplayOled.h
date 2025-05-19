#ifndef DISPLAY_OLED_H  // Header guard to prevent multiple inclusions
#define DISPLAY_OLED_H

#include <Display.h>


class DisplayOled : public Display
{
public:
    void displaySettings(Config config) override;
    void displayCountdown(int countdown) override;
    void displayGame(ControlPoint controlPoint) override;
    void displayCapturing(TeamId capturingTeam,TeamId currentTeam) override;
    void displayFinished(TeamId winner,ControlPoint controlPoint) override;
};

#endif