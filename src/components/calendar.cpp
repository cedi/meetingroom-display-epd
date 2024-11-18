#include <ctime>
#include <Preferences.h>

#include "config.h"

#include "components/calendar.h"
#include "components/statusbar.h"

#include "icons/32x32/calendar.h"
#include "icons/48x48/wi_time_2.h"
#include "icons/48x48/warning_icon.h"
#include "icons/32x32/x_symbol.h"
#include "fonts/Poppins_Regular.h"

Calendar::Calendar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient)
	: DisplayComponent(buffer, buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight),
	  calClient(calClient),
	  entryHeight(48)
{
}

void Calendar::render(time_t now) const
{
	int yOffset = y;

#if DEBUG_LEVEL >= 2
	Serial.printf("[verbose] calendar_entries: %d\n", calClient->getCalendarEntries()->size());
#endif

	for (calendar_client::CalendarEntries::const_iterator calIt = calClient->getCalendarEntries()->begin();
		 calIt != calClient->getCalendarEntries()->end(); calIt++)
	{
		renderCalendarEntry(x, yOffset, *calIt, now);
		yOffset += entryHeight;
	}
}

void Calendar::renderCalendarEntry(int x, int y, const calendar_client::CalendarEntry &entry, time_t now) const
{
	bool isPastEvent = entry.getEnd() < now;
	bool isCurrentEvent = entry.getStart() < now && entry.getEnd() > now;

#if DEBUG_LEVEL >= 2
	Serial.printf("[verbose] rendering_calendar entry: %s. is_important: %d, is_current: %d, is_past: %d\n", entry.getTitle().c_str(), entry.isImportant(), isCurrentEvent, isPastEvent);
#endif

	int offsetX = x;
	int bitmapSize = 32;
	const unsigned char *icon = calendar;

	buffer->setBackgroundColor(Color::White);
	buffer->setForegroundColor(Color::Black);

	if (isCurrentEvent)
	{
		bitmapSize = 48;
		icon = wi_time_2;

		buffer->setBackgroundColor(Color::Black);
		buffer->setForegroundColor(Color::White);
		buffer->fillBackground(x, y, width, entryHeight);
	}
	else if (entry.isImportant() && !isPastEvent)
	{
		bitmapSize = 48;
		icon = warning_icon;
	}

	buffer->drawBitmap(offsetX + entryHeight / 2 - bitmapSize / 2, y + entryHeight / 2 - bitmapSize / 2, icon, bitmapSize, bitmapSize);

	if (isPastEvent)
	{
		// for past events, we cross out the calendar icon
		buffer->drawBitmap(offsetX + entryHeight / 2 - bitmapSize / 2, y + entryHeight / 2 - bitmapSize / 2, x_symbol, bitmapSize, bitmapSize);
	}

	// ok, we drawed the 32x32 icon, let's leave some space around
	offsetX += entryHeight;

	buffer->setFont(&FONT_11pt8b);
	TextSize *size = buffer->getStringBounds(entry.getTitle(), width - 48, 1);
	Rect r = buffer->drawString(offsetX, y + entryHeight / 2 - size->height / 2 - 4, entry.getTitle(), Alignment::VerticalCenter | Alignment::Left, width - 48, 1);

	if (isPastEvent)
	{
		// for past events, we strike through the title
		buffer->drawLine(r.x, r.y - 6 + r.height / 2, r.x + r.width, r.y - 6 + r.height / 2);
	}

	tm tmFrom = {};
	tm tmTo = {};
	String fromStr;
	String toStr;

	time_t entryStart = entry.getStart();
	time_t entryEnd = entry.getEnd();
	tmFrom = *localtime(&entryStart);
	tmTo = *localtime(&entryEnd);

	getTimeStr(fromStr, &tmFrom);
	getTimeStr(toStr, &tmTo);

	String timeString = fromStr + String(" bis ") + toStr;

	// Draw the hours
	buffer->setFont(&FONT_9pt8b);
	r = buffer->drawString(offsetX, y + entryHeight - 6, timeString, Alignment::Bottom | Alignment::Left, width - 48, 1);

	if (isPastEvent)
	{
		buffer->drawLine(r.x, r.y - r.height / 2, r.x + r.width, r.y - r.height / 2);
	}

	buffer->drawLine(x, y + entryHeight, x + width, y + entryHeight);
}
