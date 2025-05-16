#ifndef DISPLAY_H  // Header guard to prevent multiple inclusions
#define DISPLAY_H

class Display
{
public:
    virtual void displaySettings();
    virtual void displayCountdown();
    virtual void displayGame();
    virtual void displayFinished();
};

#endif