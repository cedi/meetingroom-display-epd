#pragma once

#include <vector>
#include <string>

#include "components/components.h"

class Display
{
private:
    GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display;
    DisplayBuffer *buffer;
    int8_t pin_epd_pwr;
    int8_t pin_epd_sck;
    int8_t pin_epd_miso;
    int8_t pin_epd_mosi;
    int8_t pin_epd_cs;

    std::vector<IDisplayComponent*> components;

public:
    Display(int8_t pin_epd_pwr, int8_t pin_epd_sck, int8_t pin_epd_miso, int8_t pin_epd_mosi, int8_t pin_epd_cs, int16_t pin_epd_dc, int16_t pin_epd_rst, int16_t pin_epd_busy);

    void init();

    // turn the power to the display off.
    // first puts the epd driver to deep sleep for, and then
    // cuts power to the power pin
    void powerOff();

    bool render() const;

    // Draw an error message to the display.
    // If only title is specified, content of tilte is wrapped across two lines
    void error(const uint8_t *bitmap_196x196, const String &title, const String &description = "");

protected:
    void _error(const uint8_t *bitmap_196x196, const String &title, const String &description = "");
};