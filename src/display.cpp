#include "display.h"
#include "fonts/FreeSans.h"
#include "components/statusbar.h"
#include "components/calendar.h"
#include "components/status.h"

Display::Display(int8_t pin_epd_pwr, int8_t pin_epd_sck, int8_t pin_epd_miso, int8_t pin_epd_mosi, int8_t pin_epd_cs, int16_t pin_epd_dc, int16_t pin_epd_rst, int16_t pin_epd_busy)
    : display(GxEPD2_750_T7(pin_epd_cs, pin_epd_dc, pin_epd_rst, pin_epd_busy)),
      pin_epd_pwr(pin_epd_pwr),
      pin_epd_sck(pin_epd_sck),
      pin_epd_miso(pin_epd_miso),
      pin_epd_mosi(pin_epd_mosi),
      pin_epd_cs(pin_epd_cs)
{
    // initialize power pin as output pin
    pinMode(pin_epd_pwr, OUTPUT);
    buffer = new DisplayBuffer(&display);

    components.push_back(new StatusBar(buffer));
    components.push_back(new Calendar(buffer));
    components.push_back(new Status(buffer));
}

void Display::init()
{
    // turn on the 3.3 power to the driver board
    digitalWrite(pin_epd_pwr, HIGH);

    // initialize epd display
    display.init(115200, true, 10, false);

    // remap spi
    SPI.end();
    SPI.begin(pin_epd_sck, pin_epd_miso, pin_epd_mosi, pin_epd_cs);

    // draw first page
    buffer->firstPage();
}

bool Display::render() const
{
    do
    {
        for (std::vector<IDisplayComponent *>::const_iterator it = components.begin(); it != components.end(); it++)
        {
            (*it)->render();
        }
    } while (buffer->nextPage());

    return true;
}

void Display::powerOff()
{
    // turns powerOff() and sets controller to deep sleep
    // for minimum power use, ONLY if wakeable by RST (rst >= 0)
    display.hibernate();

    // turn off the 3.3 power to the driver board
    digitalWrite(pin_epd_pwr, LOW);
}

void Display::error(const uint8_t *bitmap_196x196, const String &title, const String &description)
{
    do
    {
        _error(bitmap_196x196, title, description);
    } while (buffer->nextPage());
}

void Display::_error(const uint8_t *bitmap_196x196, const String &title, const String &description)
{
    // buffer->clearDisplay();

    // since we are displaying an error, we know our icons are 196x196
    int16_t x = (buffer->width() / 2) - (196 / 2);       // center horizontally
    int16_t y = (buffer->height() / 2) - (196 / 2) - 21; // center vertically, but put it -21 to the top
    buffer->drawBitmap(x, y, bitmap_196x196, 196, 196);

    buffer->setFont(&FONT_22pt8b);

    x = buffer->width() / 2;                     // let's put it dead at the center and let drawString handle the centering
    y = (buffer->height() / 2) + (196 / 2) + 21; // Center, but add the icon hight + 21 for space

    if (description.isEmpty())
    {
        // TODO: drawMultiLineString...
        buffer->drawString(x, y, title, Alignment::Center);
    }
    else
    {
        buffer->drawString(x, y, title, Alignment::Center);
    }

    if (!description.isEmpty())
    {
        buffer->setFont(&FONT_18pt8b);
        y = buffer->height() / 2 + 196 / 2 + 21 + 60; // Center, but add the icon hight +21 for space +55 for the second line
        buffer->drawString(x, y, description, Alignment::Center);
    }
}