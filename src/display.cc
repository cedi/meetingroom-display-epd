#include "display.h"

#include "components/statusbar.h"
#include "components/calendar.h"
#include "components/status.h"

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
	buffer->clearDisplay();

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

	buffer->drawVLine(buffer->width() / 2, StatusBar::StatusBarHeight, buffer->height());
}

void Display::_fullPageStatus(String icon, int16_t iconSize, const String &title, const String &description, time_t now) const
{
	int startX = buffer->width() / 2;
	int startY = buffer->height() / 2;

	if (now != 0)
	{
		statusBar->render(now);
		startY = StatusBar::StatusBarHeight + ((buffer->height() - StatusBar::StatusBarHeight) / 2);
	}

	uint8_t textAlignment = Alignment::HorizontalCenter | Alignment::Top;

	buffer->setFontSize(24);
	TextSize *titleSize = buffer->getStringBounds(title, buffer->width(), 1);
	int totalDrawingSize = titleSize->height;

	if (!description.isEmpty())
	{
		buffer->setFontSize(18);
		TextSize *descriptionSize = buffer->getStringBounds(title, buffer->width(), 3);
		totalDrawingSize += descriptionSize->height;
		totalDrawingSize += 5;
	}

	if (icon != "")
	{
		totalDrawingSize += iconSize;
		totalDrawingSize += 5;

		if (!description.isEmpty())
		{
			buffer->setFontSize(24);
			buffer->drawString(startX, startY - totalDrawingSize / 2, title, textAlignment, buffer->width(), 1);
		}

		buffer->drawIcon(startX, startY, icon, iconSize, Alignment::Center);
	}

	if (icon != "" && description.isEmpty())
	{
		buffer->setFontSize(24);
		buffer->drawString(startX, startY + totalDrawingSize / 2, title, textAlignment, buffer->width(), 1);
	}

	if (!description.isEmpty())
	{
		buffer->setFontSize(18);
		buffer->drawString(startX, startY + totalDrawingSize / 2, description, textAlignment, buffer->width(), 4);
	}
}