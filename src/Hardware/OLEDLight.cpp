#include "OLEDLight.h"

OLEDLight::OLEDLight()
{
}

void OLEDLight::begin()
{
    oledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize with the I2C addr 0x3C (for the 128x64)
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
