#ifndef BUTTON_MANAGER_C
#define BUTTON_MANAGER_C

#include <ezButton.h>

class ButtonManager {
public:
    ezButton blueButton;
    ezButton yellowButton;

    ezButton changeButton;
    ezButton startButton;

    ButtonManager();

    void begin();

    void update();
};

#endif