#ifndef DISPLAY_OLED_H  // Header guard to prevent multiple inclusions
#define DISPLAY_OLED_H

#include <Display.h>

class DispalyOled : public Display
{
public:
    void displaySettings() override;
    void displayCountdown() override;
    void displayGame() override;
    void displayFinished() override;
};

#endif