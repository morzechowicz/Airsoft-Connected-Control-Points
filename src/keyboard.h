#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 3;

class Keyboard
{
public:
    Keyboard();

    void keyboardloop();
private:
    char keys[ROWS][COLS] = {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}};
    byte rowPins[ROWS] = {5, 4, 3, 2};
    byte colPins[COLS] = {8, 7, 6};

    Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

    unsigned long loopCount;
    unsigned long startTime;
    String msg;
};

#endif // KEYBOARD_H