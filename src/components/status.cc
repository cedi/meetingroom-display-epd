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

	Color fgSave = buffer->getForegroundColor();
	Color bgSave = buffer->getBackgroundColor();

	if (isImportant)
	{

#if defined(DISP_3C) || defined(DISP_7C)
		buffer->setForegroundColor(accentColor);
#elif defined(INVERT_AS_ACCENT) && INVERT_AS_ACCENT == true
		buffer->invert();
		buffer->fillBackground(x, y, width, height);
#endif

		buffer->drawRect(x + 20, y + 20, width - 40, height - 40, 10);
		maxTextWidth = width - 40;
	}

	// get the center of the screen
	uint8_t alignment = Alignment::Center;
	int startX = x + width / 2;
	int startY = y + height / 2;

	buffer->setFontSize(24);
	TextSize *size = buffer->getStringBounds(statusMsg, maxTextWidth, 3);

	if (icon != "")
	{
		// we take into account the Text-height to center both icon and text
		int iconYOffset = size->height / 2;

		// add extra padding between the two
		iconYOffset += 10;

		// draw the icon
		buffer->drawIcon(startX, startY - iconYOffset, icon, iconSize, Alignment::Center);

		// now move the Y start position down by the iconOffset
		// since this already includes our padding, we should be good
		// in terms of text spacing
		startY += iconYOffset;

		// Now, since we have the Icon, we can't position the text
		// vertically center, but let's do Top H-Center
		alignment = Alignment::HorizontalCenter | Alignment::Top;
	}

	buffer->drawString(startX, startY, statusMsg, alignment, maxTextWidth, 3);

	// reset display color
	buffer->setForegroundColor(fgSave);
	buffer->setBackgroundColor(bgSave);
}
