#ifndef OLEDLight_h
#define OLEDLight_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "../Config.h"

class OLEDLight {
public:
    OLEDLight();
    void begin();
    void update();
    void setBrightness(uint8_t brightness);
    void setDisplay(bool display);
private:
    Adafruit_SSD1306 oledDisplay;
    
    void writeCommand(uint8_t command);
    void writeData(uint8_t data);

    uint8_t _brightness;
    bool _display;
};

#endif // OLEDLight_h