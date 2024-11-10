#include "components/status.h"
#include "components/statusbar.h"
#include "config.h"
#include "fonts/Poppins_Regular.h"

Status::Status(DisplayBuffer *buffer)
	: DisplayComponent(buffer, 0, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight), isImportant(false), icon(NULL)
{
}

void Status::render() const
{
	int maxTextWidth = width;

	if (isImportant)
	{
		buffer->setBackgroundColor(Color::Black);
		buffer->setForegroundColor(Color::White);
		buffer->fillBackground(x, y, width, height);
		buffer->drawRect(x + 20, y + 20, width - 40, height - 40, 10);

		maxTextWidth = width - 40;
	}

	int startX = x + width / 2;
	int startY = y + height / 2;

	buffer->setFont(&FONT_22pt8b);
	TextSize *size = buffer->getStringBounds(statusMsg, maxTextWidth, 3);
	startY -= size->height / 2;

	uint8_t alignment = Alignment::Center;

	if (icon != NULL)
	{
		startX -= 196 / 2;
		startY -= 196 / 2;
		buffer->drawBitmap(startX, startY, icon, 196, 196);

		startX += 196 / 2;
		startY += 196 - 40;

		alignment = Alignment::HorizontalCenter | Alignment::Top;
	}

	buffer->drawString(startX, startY, statusMsg, alignment, maxTextWidth, 3);
}
