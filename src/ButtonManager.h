#ifndef BUTTON_MANAGER_C
#define BUTTON_MANAGER_C

#include <ezButton.h>

class ButtonManager {
public:
    ezButton blueButton;
    ezButton yellowButton;

    ezButton changeButton;
    ezButton startButton;

    ButtonManager()
        : blueButton(2), yellowButton(3), 
          changeButton(4), startButton(5)  
    {}

    void begin() {
        blueButton.setDebounceTime(50);
        yellowButton.setDebounceTime(50);
        changeButton.setDebounceTime(50);
        startButton.setDebounceTime(50);
    }

    void update() {
        blueButton.loop();
        yellowButton.loop();
        changeButton.loop();
        startButton.loop();
    }
};

#endif