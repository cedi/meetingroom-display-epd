#include "components/calendar.h"
#include "components/statusbar.h"
#include "icons/32x32/calendar.h"
#include "icons/32x32/x_symbol.h"
#include "config.h"
#include "fonts/Poppins_Regular.h"

Calendar::Calendar(DisplayBuffer *buffer)
	: DisplayComponent(buffer, buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight)
{
	events.push_back(new CalendarEntry(buffer, width, true));
	events.push_back(new CalendarEntry(buffer, width, true));
	events.push_back(new CalendarEntry(buffer, width));
	events.push_back(new CalendarEntry(buffer, width));
}

void Calendar::render() const
{
	int yOffset = y;
	for (std::vector<CalendarEntry *>::const_iterator it = events.begin(); it != events.end(); it++)
	{
		(*it)->render(x, yOffset);
		yOffset += (*it)->height;
	}
}

void CalendarEntry::render(int x, int y) const
{
	buffer->drawBitmap(x + height / 2 - 32 / 2, y + height / 2 - 32 / 2, calendar, 32, 32);
	if(pastEvent)
	{
		buffer->drawBitmap(x + height / 2 - 32 / 2, y + height / 2 - 32 / 2, x_symbol, 32, 32);
	}

	buffer->setFont(&FONT_11pt8b);
	buffer->drawString(x + 48, y + 1, "Random Meeting", Alignment::Top | Alignment::Left);

	if(pastEvent)
	{
		TextSize *size = buffer->getStringBounds("Random Meeting");
		buffer->drawLine(x + 48, y + size->height/2 + 6, x+48+size->width, y + size->height/2 + 6);
	}

	buffer->setFont(&FONT_9pt8b);
	buffer->drawString(x + 48, y + height - 6, "17:30 bis 18:30", Alignment::Bottom | Alignment::Left);

	if(pastEvent)
	{
		TextSize *size = buffer->getStringBounds("17:30 bis 18:30");
		buffer->drawLine(x + 48, y + height - size->height + 1, x+48+size->width, y + height - size->height + 1);
	}

	buffer->drawLine(x, y + height, x + width, y + height);
}
