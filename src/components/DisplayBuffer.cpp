#include "components/components.h"

// Function to test if a specific alignment flag is set
bool hasAlignment(uint8_t alignment, Alignment flag)
{
    return (alignment & flag) == flag;
}

void DisplayBuffer::firstPage()
{
    display->setRotation(0);
    display->setTextWrap(false);

    setTextSize(1);
    display->setFullWindow();
    setForegroundColor(Color::Black);
    setBackgroundColor(Color::White);

    // do
    // {
        clearDisplay();
    // } while (nextPage());
}

void DisplayBuffer::clearDisplay()
{
    display->setFullWindow();
    display->fillScreen(backgroundColor);
}

// Draw a String on x/y coordinate
void DisplayBuffer::drawString(int16_t x, int16_t y, const String &text, uint8_t alignment)
{
    int16_t x1, y1;
    uint16_t w, h;

    display->setTextColor(foregroundColor);
    display->getTextBounds(text, x, y, &x1, &y1, &w, &h);

    if (hasAlignment(alignment, Alignment::HorizontalCenter))
    {
        x = x - w / 2;
    }
    else if (hasAlignment(alignment, Alignment::Right))
    {
        x = x - w;
    }

    if (hasAlignment(alignment, Alignment::VerticalCenter))
    {
        y = y + h / 2;
    }
    else if (hasAlignment(alignment, Alignment::Top))
    {
        y = y + h;
    }

    display->setCursor(x, y);
    display->print(text);
}

void DisplayBuffer::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t width, int16_t height)
{
    display->drawInvertedBitmap(x, y, bitmap, width, height, foregroundColor);
}