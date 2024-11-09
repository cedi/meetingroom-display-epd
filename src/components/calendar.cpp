#include "components/calendar.h"
#include "components/statusbar.h"
#include "fonts/FreeSans.h"

Calendar::Calendar(DisplayBuffer *buffer)
    : DisplayComponent(buffer, buffer->width() / 2, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight)
{
}

void Calendar::render() const
{
    buffer->drawRect(x, y, width, height);

    int startX = x + width / 2;
    int startY = y + height / 2;
    buffer->setFont(&FONT_22pt8b);
    buffer->drawString(startX, startY, "Calendar", Alignment::Center);
}
