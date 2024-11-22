#include "components/status.h"
#include "components/statusbar.h"
#include "config.h"

#if defined(DISP_3C) || defined(DISP_7C)
Status::Status(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient, Color accentColor)
	: DisplayComponent(buffer, 0, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight, accentColor),
#else
Status::Status(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient)
	: DisplayComponent(buffer, 0, StatusBar::StatusBarHeight, buffer->width() / 2, buffer->height() - StatusBar::StatusBarHeight),
#endif
	  calClient(calClient)

{
}

void Status::render(time_t now) const
{
	int maxTextWidth = width;

	const calendar_client::CalendarEntry *currentEvent = calClient->getCurrentEvent(now);

	bool isImportant = false;
	String statusMsg(TXT_FREE);
	String icon("");
	int iconSize = 0;

	if (currentEvent != NULL)
	{

#if DEBUG_LEVEL >= 2
		Serial.printf("[verbose] Current Event: %s, busy_state: %s\n",
					  currentEvent->getTitle().c_str(),
					  currentEvent->getBusy() == calendar_client::Busy ? "Busy" : currentEvent->getBusy() == calendar_client::Tentative ? "Tentative"
																																		: "Free");
#endif
		if (currentEvent->getBusy() == calendar_client::Busy)
		{
			icon = "wi_time_2";
			iconSize = 128;
		}

		if (currentEvent->isImportant())
		{
			icon = "warning_icon";
			iconSize = 196;
			isImportant = true;
		}

		statusMsg = currentEvent->getMessage() != "" ? currentEvent->getMessage() : currentEvent->getTitle();
	}

	buffer->setBackgroundColor(Color::White);
	buffer->setForegroundColor(Color::Black);
	Color fgSave = buffer->getForegroundColor();
	Color bgSave = buffer->getBackgroundColor();

	if (isImportant)
	{

#if defined(DISP_3C) || defined(DISP_7C)
		buffer->setForegroundColor(accentColor);
#else
		buffer->setBackgroundColor(Color::Black);
		buffer->fillBackground(x, y, width, height);
#endif

		buffer->drawRect(x + 20, y + 20, width - 40, height - 40, 10);
		maxTextWidth = width - 40;
	}

	int startX = x + width / 2;
	int startY = y + height / 2;

	buffer->setFontSize(24);
	TextSize *size = buffer->getStringBounds(statusMsg, maxTextWidth, 3);

	// do a -5 so we can get some spacing in beetween the icon and the text
	startY -= size->height / 2 - 10;

	uint8_t alignment = Alignment::Center;

	if (icon != "")
	{
		buffer->drawIcon(startX, startY - 10, icon, iconSize, Alignment::Center);

		startY += iconSize / 2;

		// do a +10 so we can get some spacing in beetween the icon and the text
		startY += 20;

		alignment = Alignment::HorizontalCenter | Alignment::Bottom;
	}

	buffer->drawString(startX, startY + 10, statusMsg, alignment, maxTextWidth, 3);

	// reset display color
	buffer->setForegroundColor(fgSave);
	buffer->setBackgroundColor(bgSave);
}
