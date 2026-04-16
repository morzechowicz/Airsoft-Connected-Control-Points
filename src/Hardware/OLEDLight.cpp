#include "OLEDLight.h"

OLEDLight::OLEDLight()
{
}

void OLEDLight::begin()
{
    oledDisplay = new SSD1306Wire(0x3C, 17, 18, GEOMETRY_64_32);

    if (!oledDisplay->init())
    { // Initialize with the I2C addr 0x3C (for the 128x64)
        LOG_ERROR("OLEDLight", "Failed to initialize OLED display");
    }
    else
    {
        LOG_INFO("OLEDLight", "OLED display initialized successfully");
    }
    vTaskDelay(300);
    oledDisplay->setContrast(255);
    oledDisplay->setFont(ArialMT_Plain_10);
    

    clear();
    writeln("SPAS");
    writeln("FORWARDER");
    display();
}

void OLEDLight::update()
{
}

void OLEDLight::setBrightness(uint8_t brightness)
{
}

void OLEDLight::setDisplay(bool display)
{
}

void OLEDLight::writeln(const char *text)
{
    oledDisplay->println(text);
}

void OLEDLight::clear()
{
    oledDisplay->cls();
}

void OLEDLight::display()
{
    oledDisplay->display();
}
