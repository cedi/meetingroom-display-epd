#include "components/status.h"
#include "components/statusbar.h"
#include "fonts/FreeSans.h"

Status::Status(DisplayBuffer *buffer)
    : DisplayComponent(buffer, 0, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight)
    , isImportant(false)
    , icon(NULL)
{
}

void Status::render() const
{
    buffer->drawRect(x, y, width, height);

    if (isImportant)
    {
        buffer->setBackgroundColor(Color::Black);
        buffer->setForegroundColor(Color::White);
        buffer->fillBackground(x, y, width, height);
        buffer->drawRect(x + 20, y + 20, width - 40, height - 40, 10);
    }

    int startX = x + width / 2;
    int startY = y + height / 2;

    buffer->setFont(&FONT_22pt8b);
    uint8_t alignment = Alignment::Center;

    if (icon != NULL)
    {
        TextSize *size = buffer->getStringBounds("Status");

        int totalHeight = 196 + size->height;

        startX -= 196/2;
        startY -= totalHeight/2;
        buffer->drawBitmap(startX, startY, icon, 196, 196);

        startX = x + width / 2;
        startY += 196 - size->height;

        alignment = Alignment::HorizontalCenter | Alignment::Top;
    }

    buffer->drawString(startX, startY, "Status", alignment);
}
