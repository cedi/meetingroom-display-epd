#include <ctime>
#include <Preferences.h>

#include "components/calendar.h"
#include "components/statusbar.h"
#include "icons/32x32/calendar.h"
#include "icons/48x48/warning_icon.h"
#include "icons/32x32/x_symbol.h"
#include "config.h"
#include "fonts/Poppins_Regular.h"
Calendar::Calendar(DisplayBuffer *buffer)
	: DisplayComponent(buffer, buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight)
{
}

void Calendar::render() const
{
	int yOffset = y;
	for (std::vector<CalendarEntry>::const_iterator it = events.begin(); it != events.end(); it++)
	{
		(*it).render(x, yOffset);
		yOffset += (*it).height;
	}
}

std::vector<CalendarEntry>::iterator Calendar::getCurrentEvent()
{
	Preferences prefs;
	prefs.begin(NVS_NAMESPACE, true);
	time_t unixTimeNTP = prefs.getLong("unixTimeNTP");
	prefs.end();

	for (std::vector<CalendarEntry>::iterator it = events.begin(); it != events.end(); it++)
	{
		if (unixTimeNTP > it->entry.start && unixTimeNTP < it->entry.end) {
			return it;
		}
	}

	return events.end();
}

void CalendarEntry::render(int x, int y) const
{
	Preferences prefs;
	prefs.begin(NVS_NAMESPACE, true);
	time_t unixTimeNTP = prefs.getLong("unixTimeNTP");
	prefs.end();

	bool isPastEvent = entry.end < unixTimeNTP;
	int offsetX = x;
	int bitmapSize = 32;

	if (entry.important && !isPastEvent) {
		bitmapSize = 48;
		buffer->drawBitmap(offsetX + height / 2 - bitmapSize / 2, y + height / 2 - bitmapSize / 2, warning_icon, bitmapSize, bitmapSize);
	}
	else
	{
		buffer->drawBitmap(offsetX + height / 2 - bitmapSize / 2, y + height / 2 - bitmapSize / 2, calendar, bitmapSize, bitmapSize);
	}

	if(isPastEvent)
	{
		// for past events, we cross out the calendar icon
		buffer->drawBitmap(offsetX + height / 2 - bitmapSize / 2, y + height / 2 - bitmapSize / 2, x_symbol, bitmapSize, bitmapSize);
	}

	// ok, we drawed the 32x32 icon, let's leave some space around
	offsetX += height;

	buffer->setFont(&FONT_11pt8b);
	TextSize *size = buffer->getStringBounds(entry.title, width - 48, 1);
	Rect r = buffer->drawString(offsetX, y + height /2 - size->height / 2 -4, entry.title, Alignment::VerticalCenter | Alignment::Left, width - 48, 1);

	if(isPastEvent)
	{
		// for past events, we strike through the title
		buffer->drawLine(r.x, r.y-6+r.height/2, r.x + r.width, r.y-6+r.height/2);
	}


	tm tmFrom = {};
	tm tmTo = {};
	String fromStr;
	String toStr;

	tmFrom = *localtime(&entry.start);
	tmTo = *localtime(&entry.end);

	getTimeStr(fromStr, &tmFrom);
	getTimeStr(toStr, &tmTo);

	String timeString = fromStr + String(" bis ") + toStr;

	// Draw the hours
	buffer->setFont(&FONT_9pt8b);
	r = buffer->drawString(offsetX, y + height - 6, timeString, Alignment::Bottom | Alignment::Left, width - 48, 1);

	if(isPastEvent)
	{
		buffer->drawLine(r.x, r.y - r.height /2, r.x + r.width, r.y - r.height /2);
	}

	buffer->drawLine(x, y + height, x + width, y + height);
}
