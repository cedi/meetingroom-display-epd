#pragma once

#include <vector>
#include <string>

#include "components/statusbar.h"
#include "components/status.h"
#include "components/calendar.h"
#include "display_utils.h"

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

	StatusBar *statusBar;
	Calendar *calendar;
	Status *statusIndicator;

	bool initialized;

	// working with the display
public:
	Display(int8_t pin_epd_pwr, int8_t pin_epd_sck, int8_t pin_epd_miso, int8_t pin_epd_mosi, int8_t pin_epd_cs, int16_t pin_epd_dc, int16_t pin_epd_rst, int16_t pin_epd_busy);

	// turn on the display and initialize it
	void init();

	// turn the power to the display off.
	// first puts the epd driver to deep sleep for, and then
	// cuts power to the power pin
	void powerOff();

	// Display configuration
public:
	void setStatus(String message, bool isImportant = false, const uint8_t *icon = NULL);
	void setCalendar(const CalendarEntries& calendar);
	Calendar *getCalendar() { return calendar; }

	// Rendering functions
public:
	// renders the display in a single refresh cycle for the display
	void render() const
	{
		do
		{
			_render();
		} while (buffer->nextPage());
	}

	// Draw an error message to the display.
	// If only title is specified, content of tilte is wrapped across two lines
	void error(const uint8_t *bitmap_196x196, const String &title, const String &description = "") const
	{
		do
		{
			_error(bitmap_196x196, title, description);
		} while (buffer->nextPage());
	}

	// internal rendering functions.
	// they draw the display, but do not take care of the nextPage. That is done in the public variants
protected:
	void _error(const uint8_t *bitmap_196x196, const String &title, const String &description = "") const;
	void _render() const;
};