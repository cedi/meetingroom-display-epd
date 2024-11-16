#pragma once

#include <GxEPD2_BW.h>

// Define the Alignment enum with bit flags
enum Alignment : uint8_t
{
	Left = 1 << 0,	 // 0001
	Right = 1 << 1,	 // 0010
	Top = 1 << 2,	 // 0100
	Bottom = 1 << 3, // 1000

	HorizontalCenter = Left | Right,		   // 0011
	VerticalCenter = Top | Bottom,			   // 1100
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

struct TextSize
{
	uint16_t width;
	uint16_t height;
};

struct Rect : public TextSize
{
	uint16_t x;
	uint16_t y;
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

	Rect drawString(int16_t x, int16_t y, const String &text, uint8_t alignment = Alignment::Left);
	Rect drawString(int16_t x, int16_t y, const String &text, uint8_t alignment, uint16_t max_width, uint16_t max_lines);

	TextSize *getStringBounds(int16_t x, int16_t y, const String &text);
	TextSize *getStringBounds(const String &text) { return getStringBounds(0, 0, text); }
	TextSize *getStringBounds(int16_t x, int16_t y, const String &text, uint16_t max_width, uint16_t max_lines);
	TextSize *getStringBounds(const String &text, uint16_t max_width, uint16_t max_lines) { return getStringBounds(0, 0, text, max_width, max_lines);}

	void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t width, int16_t height);
	void drawRect(int16_t x, int16_t y, int16_t w, int16_t h) { display->drawRect(x, y, w, h, foregroundColor); }
	void fillBackground(int16_t x, int16_t y, int16_t w, int16_t h) { display->fillRect(x, y, w, h, backgroundColor); }
	void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t thickness);

	void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) { display->drawLine(x0, y0, x1, y1, foregroundColor); }
};
