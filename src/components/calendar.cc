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

#if defined(DISP_3C) || defined(DISP_7C)
Calendar::Calendar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient, Color accentColor)
	: DisplayComponent(buffer, buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight, accentColor),

#else
Calendar::Calendar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient)
	: DisplayComponent(buffer, buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight),
#endif
	  calClient(calClient),
	  entryHeight(48)
{
}

void Calendar::render(time_t now) const
{
	int yOffset = y;

	// If there are more calendar items than fit on the display,
	// we need to truncate them a bit so they'll fit
	int maxCalendarEntries = height / entryHeight;
	int calendarEntryCount = calClient->getCalendarEntries()->size();
	bool truncate = calendarEntryCount > maxCalendarEntries;
	int overflowingMeetings = 0;

#if DEBUG_LEVEL >= 1
	Serial.printf("[debug] calendar_entries: %d (max: %d)\n", calendarEntryCount, maxCalendarEntries);
#endif

	int itemCntr = 0;
	for (calendar_client::CalendarEntries::const_iterator calIt = calClient->getCalendarEntries()->begin();
		 calIt != calClient->getCalendarEntries()->end(); calIt++)
	{
		// if we must truncate, lets hide the the calendar item after now
		if (truncate && calIt->getEnd() < now)
		{
#if DEBUG_LEVEL >= 1
			Serial.printf("[debug] truncating past event: %s\n", calIt->getTitle().c_str());
#endif
			// and re-evaluate if we still have to truncate
			calendarEntryCount--;
			truncate = calendarEntryCount > maxCalendarEntries;
			continue;
		}

		if (itemCntr >= maxCalendarEntries)
		{
			overflowingMeetings = maxCalendarEntries - itemCntr + 2;
#if DEBUG_LEVEL >= 1
			Serial.printf("[debug] truncating %d future events due to no more available space on display.\n", overflowingMeetings);
#endif
			break;
		}

		renderCalendarEntry(x, yOffset, *calIt, now);
		yOffset += entryHeight;
		itemCntr++;
	}

	// If we have more meetings than we can display,
	// at least add an indicator that there's more to come at the bottom
	if (overflowingMeetings > 0)
	{
		int availableHeight = height - maxCalendarEntries * entryHeight;
		String remainingText("+" + String(overflowingMeetings) + " " + TXT_EVENTS + "...");

		buffer->setFont(&FONT_9pt8b);
		buffer->drawString(x + width / 2, yOffset + availableHeight / 2, remainingText, Alignment::Center, width, 1);
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
	Color fgSave = buffer->getForegroundColor();
	Color bgSave = buffer->getBackgroundColor();

	if (isCurrentEvent)
	{
		bitmapSize = 48;
		icon = wi_time_2;

#if defined(DISP_3C) || defined(DISP_7C)
		buffer->setForegroundColor(accentColor);
#else
		buffer->setBackgroundColor(Color::Black);
		buffer->setForegroundColor(Color::White);
		buffer->fillBackground(x, y, width, height);
#endif
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

	time_t entryStart = entry.getStart();
	time_t entryEnd = entry.getEnd();
	tmFrom = *localtime(&entryStart);
	tmTo = *localtime(&entryEnd);

	String fromStr = getTimeStr(&tmFrom);
	String toStr = getTimeStr(&tmTo);

	String timeString(fromStr + " " + TXT_UNTIL + " " + toStr);

	// Draw the hours
	buffer->setFont(&FONT_9pt8b);
	r = buffer->drawString(offsetX, y + entryHeight - 6, timeString, Alignment::Bottom | Alignment::Left, width - 48, 1);

	if (isPastEvent)
	{
		buffer->drawLine(r.x, r.y - r.height / 2, r.x + r.width, r.y - r.height / 2);
	}

	buffer->drawLine(x, y + entryHeight, x + width, y + entryHeight);

	// reset display color
	buffer->setForegroundColor(fgSave);
	buffer->setBackgroundColor(bgSave);
}
