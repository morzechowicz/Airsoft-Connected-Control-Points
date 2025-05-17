#ifndef DISPLAY_LCD_H  // Header guard to prevent multiple inclusions
#define DISPLAY_LCD_H

#include <Display.h>

class DispalyLCD : public Display
{
public:
    void displaySettings() override;
    void displayCountdown() override;
    void displayGame() override;
    void displayFinished() override;
};

#endif