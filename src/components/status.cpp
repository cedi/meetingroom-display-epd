#include "components/status.h"
#include "components/statusbar.h"
#include "config.h"

#include "icons/196x196/warning_icon.h"
#include "icons/196x196/wi_time_2.h" // TODO: Use 128x128
#include "fonts/Poppins_Regular.h"

Status::Status(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient)
	: DisplayComponent(buffer, 0, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight),
	  calClient(calClient)
{
}

void Status::render(time_t now) const
{
	int maxTextWidth = width;

	const calendar_client::CalendarEntry *currentEvent = calClient->getCurrentEvent(now);

	bool isImportant = false;
	String statusMsg = "Frei";
	const unsigned char *icon = NULL;
	int iconSize = 0;

	if (currentEvent != NULL)
	{

#if DEBUG_LEVEL >= 1
		Serial.printf("Current Event: %s, busy: %d (isBusy: %d)", currentEvent->getTitle().c_str(), currentEvent->getBusy(), calendar_client::Busy);
#endif
		if (currentEvent->getBusy() == calendar_client::Busy)
		{
			icon = wi_time_2;
			iconSize = 196;
		}

		if (currentEvent->isImportant())
		{
			icon = warning_icon;
			iconSize = 196;
			isImportant = true;
		}

		statusMsg = currentEvent->getMessage() != "" ? currentEvent->getMessage() : currentEvent->getTitle();
	}

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
		startX -= iconSize / 2;
		startY -= iconSize / 2;
		buffer->drawBitmap(startX, startY - 10, icon, iconSize, iconSize);

		startX += iconSize / 2;
		startY += iconSize;

		alignment = Alignment::HorizontalCenter | Alignment::Bottom;
	}

	buffer->drawString(startX, startY + 10, statusMsg, alignment, maxTextWidth, 3);
}
