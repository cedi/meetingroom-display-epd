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

    clearDisplay();
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
    else if (hasAlignment(alignment, Alignment::Left))
    {
        x = x;
    }

    if (hasAlignment(alignment, Alignment::VerticalCenter))
    {
        y = y + h / 2;
    }
    else if (hasAlignment(alignment, Alignment::Top))
    {
        y = y + h;
    }
    else if (hasAlignment(alignment, Alignment::Bottom))
    {
        y = y;
    }

    display->setCursor(x, y);
    display->print(text);
}

TextSize* DisplayBuffer::getStringBounds(const String &text)
{
  int16_t x1, y1;
  uint16_t w, h;
  display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

  TextSize *size = new TextSize();
  size->width = w;
  size->height = h;
  return size;
}

void DisplayBuffer::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t width, int16_t height)
{
    display->drawInvertedBitmap(x, y, bitmap, width, height, foregroundColor);
}

void DisplayBuffer::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t thickness)
{
    display->fillRect(x, y, thickness, h, foregroundColor); // left column
    display->fillRect(x + w - thickness, y, thickness, h, foregroundColor); // right column

    display->fillRect(x, y, w, thickness, foregroundColor); // top column
    display->fillRect(x, y + h - thickness, w, thickness, foregroundColor); // bottom column
}