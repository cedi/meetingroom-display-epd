#pragma once

#include <GxEPD2_BW.h>

// Define the Alignment enum with bit flags
enum Alignment : uint8_t
{
    Left = 1 << 0,   // 0001
    Right = 1 << 1,  // 0010
    Top = 1 << 2,    // 0100
    Bottom = 1 << 3, // 1000

    HorizontalCenter = Left | Right,           // 0011
    VerticalCenter = Top | Bottom,             // 1100
    Center = HorizontalCenter | VerticalCenter // 1111
};

enum Color : uint16_t
{
    Black = GxEPD_BLACK,
    White = GxEPD_WHITE,

    // some controllers for b/w EPDs support grey levels
    DarkGrey = GxEPD_DARKGREY,
    LightGrey = GxEPD_LIGHTGREY
};

class DisplayBuffer
{
private:
    GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> *display;
    Color foregroundColor;
    Color backgroundColor;

public:
    DisplayBuffer(GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> *display) { this->display = display; }

    int16_t width() { return display->width(); }
    int16_t height() { return display->height(); }

    void firstPage();
    bool nextPage() { return display->nextPage(); }

    void setTextSize(uint8_t s) { this->display->setTextSize(s); }
    void setFont(const GFXfont *f) { display->setFont(f); }
    void setForegroundColor(Color c) { this->foregroundColor = c; }
    void setBackgroundColor(Color c) { this->backgroundColor = c; }

    void setFullWindow() { display->setFullWindow(); }
    void setPartialWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) { display->setPartialWindow(x, y, w, h); }

    void clearDisplay();

    void drawString(int16_t x, int16_t y, const String &text, uint8_t alignment = Alignment::Left);
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t width, int16_t height);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h) { display->drawRect(x, y, w, h, foregroundColor); }
};
