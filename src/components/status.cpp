#include "components/status.h"
#include "components/statusbar.h"
#include "fonts/FreeSans.h"

Status::Status(DisplayBuffer *buffer)
    : DisplayComponent(buffer, 0, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight)
{
}

void Status::render() const
{
    buffer->drawRect(x, y, width, height);

    int startX = x + width / 2;
    int startY = y + height / 2;
    buffer->setFont(&FONT_22pt8b);
    buffer->drawString(startX, startY, "Status", Alignment::Center);
}
