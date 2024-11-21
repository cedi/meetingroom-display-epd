#include "display.h"

#include "components/statusbar.h"
#include "components/calendar.h"
#include "components/status.h"
#include "fonts/Poppins_Regular.h"

#include "config.h"

#if defined(DISP_3C) || defined(DISP_7C)
Display::Display(int8_t pin_epd_pwr, int8_t pin_epd_sck, int8_t pin_epd_miso, int8_t pin_epd_mosi, int8_t pin_epd_cs, int16_t pin_epd_dc, int16_t pin_epd_rst, int16_t pin_epd_busy, calendar_client::CalendarClient *calClient, Color accentColor)
#else
Display::Display(int8_t pin_epd_pwr, int8_t pin_epd_sck, int8_t pin_epd_miso, int8_t pin_epd_mosi, int8_t pin_epd_cs, int16_t pin_epd_dc, int16_t pin_epd_rst, int16_t pin_epd_busy, calendar_client::CalendarClient *calClient)
#endif
	: pin_epd_pwr(pin_epd_pwr),
	  pin_epd_sck(pin_epd_sck),
	  pin_epd_miso(pin_epd_miso),
	  pin_epd_mosi(pin_epd_mosi),
	  pin_epd_cs(pin_epd_cs),
	  calClient(calClient),
	  initialized(false)
{
	// initialize power pin as output pin
	pinMode(pin_epd_pwr, OUTPUT);
	buffer = new DisplayBuffer(pin_epd_cs, pin_epd_dc, pin_epd_rst, pin_epd_busy);

#if defined(DISP_3C) || defined(DISP_7C)
	statusBar = new StatusBar(buffer, calClient, accentColor);
	calendar = new Calendar(buffer, calClient, accentColor);
	statusIndicator = new Status(buffer, calClient, accentColor);
#else
	statusBar = new StatusBar(buffer, calClient);
	calendar = new Calendar(buffer, calClient);
	statusIndicator = new Status(buffer, calClient);
#endif
}

void Display::init()
{
	// turn on the 3.3 power to the driver board
	digitalWrite(pin_epd_pwr, HIGH);

	// initialize epd display
	buffer->init();

	// remap spi
	SPI.end();
	SPI.begin(pin_epd_sck, pin_epd_miso, pin_epd_mosi, pin_epd_cs);

	// draw first page
	buffer->firstPage();

	initialized = true;
}

void Display::powerOff()
{
	if (!initialized)
	{
		return;
	}

	// turns powerOff() and sets controller to deep sleep
	// for minimum power use, ONLY if wakeable by RST (rst >= 0)
	buffer->hibernate();

	// turn off the 3.3 power to the driver board
	digitalWrite(pin_epd_pwr, LOW);
	initialized = false;
}

void Display::_render(time_t now) const
{
	statusBar->render(now);
	calendar->render(now);
	statusIndicator->render(now);

	buffer->drawLine(buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height());
}

void Display::_error(const uint8_t *bitmap_196x196, const String &title, const String &description) const
{
	int startX = buffer->width() / 2;
	int startY = buffer->height() / 2;

	if (!title.isEmpty())
	{
		buffer->setFont(&FONT_22pt8b);
		TextSize *size = buffer->getStringBounds(title, buffer->width(), 4);
		startY -= size->height / 2;
	}

	if (!description.isEmpty())
	{
		buffer->setFont(&FONT_18pt8b);
		TextSize *size = buffer->getStringBounds(description, buffer->width(), 4);
		startY -= size->height / 2;
	}

	uint8_t alignment = Alignment::HorizontalCenter | Alignment::Bottom;

	if (bitmap_196x196 == NULL)
	{
		// since we are displaying an error, we know our icons are 196x196
		int iconSize = 196;

		startX -= iconSize / 2;
		startY -= iconSize / 2;
		buffer->drawBitmap(startX, startY - 10, bitmap_196x196, iconSize, iconSize);

		startX += iconSize / 2;
		startY += iconSize;
	}

	if (!title.isEmpty())
	{
		buffer->setFont(&FONT_22pt8b);
		Rect textSize = buffer->drawString(startX, startY + 10, title, alignment, buffer->width(), 4);
		startY += textSize.height;
	}

	if (!description.isEmpty())
	{
		buffer->setFont(&FONT_18pt8b);
		buffer->drawString(startX, startY + 10, description, Alignment::Center);
	}
}