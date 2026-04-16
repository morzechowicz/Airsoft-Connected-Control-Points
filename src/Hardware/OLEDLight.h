#ifndef OLEDLight_h
#define OLEDLight_h

#include <Arduino.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include "../Config.h"
#include "../Logging/LogManager.h"

class OLEDLight {
public:
    OLEDLight();
    void begin();
    void update();
    void setBrightness(uint8_t brightness);
    void setDisplay(bool display);

    void writeln(const char* text);
    void clear();
    void display();
private:
    SSD1306Wire* oledDisplay = nullptr;
#
    void writeCommand(uint8_t command);
    void writeData(uint8_t data);

    uint8_t _brightness;
    bool _display;
};

#endif // OLEDLight_h