#ifndef BUTTON_MANAGER_C
#define BUTTON_MANAGER_C

#include <ezButton.h>

class ButtonManager {
public:
    ezButton playerButton1;
    ezButton playerButton2;

    ezButton configButton1;
    ezButton configButton2;

    ButtonManager()
        : playerButton1(2), playerButton2(3), 
          configButton1(4), configButton2(5)  
    {}

    void begin() {
        playerButton1.setDebounceTime(50);
        playerButton2.setDebounceTime(50);
        configButton1.setDebounceTime(50);
        configButton2.setDebounceTime(50);
    }

    void update() {
        playerButton1.loop();
        playerButton2.loop();
        configButton1.loop();
        configButton2.loop();
    }
};

#endif