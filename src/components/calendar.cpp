#include "components/calendar.h"
#include "components/statusbar.h"
#include "fonts/FreeSans.h"
#include "icons/48x48/wi_alien_48x48.h"

Calendar::Calendar(DisplayBuffer *buffer)
    : DisplayComponent(buffer, buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight)
{
    events.push_back(new CalendarEntry(buffer, width));
    events.push_back(new CalendarEntry(buffer, width));
    events.push_back(new CalendarEntry(buffer, width));
    events.push_back(new CalendarEntry(buffer, width));
}

void Calendar::render() const
{
    buffer->drawRect(x, y, width, height);

    int yOffset = y;
    for (std::vector<CalendarEntry *>::const_iterator it = events.begin(); it != events.end(); it++)
    {
        (*it)->render(x, yOffset);
        yOffset += (*it)->height;
    }
}

void CalendarEntry::render(int x, int y) const
{
    // Calendar entry bounds
    buffer->drawRect(x, y, width, height);

    // Icon Bounds
    buffer->drawBitmap(x, y, wi_alien_48x48, 48, 48);

    buffer->setFont(&FONT_11pt8b);
    buffer->drawString(x + 48, y + 2, "Random Meeting", Alignment::Top | Alignment::Left);

    buffer->setFont(&FONT_9pt8b);
    buffer->drawString(x + 48, y + height - 6, "17:30 bis 18:30", Alignment::Bottom | Alignment::Left);
}
