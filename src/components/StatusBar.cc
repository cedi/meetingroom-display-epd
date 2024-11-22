#include "components/statusbar.h"

#include <WiFi.h>
#include <Preferences.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include "config.h"
#include "utils.h"

#if defined(DISP_3C) || defined(DISP_7C)
StatusBar::StatusBar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient, Color accentColor)
	: DisplayComponent(buffer, 0, 0, buffer->width(), StatusBarHeight, accentColor)
#else
StatusBar::StatusBar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient)
	: DisplayComponent(buffer, 0, 0, buffer->width(), StatusBarHeight)
#endif
{
#if defined(DISP_3C) || defined(DISP_7C)
	rightBound.push_back(new BatteryPercentage(buffer, height, accentColor));
	rightBound.push_back(new WiFiStatus(buffer, height, accentColor));
	leftBound.push_back(new DateTime(buffer, height, calClient, accentColor));
#else
	rightBound.push_back(new BatteryPercentage(buffer, height));
	rightBound.push_back(new WiFiStatus(buffer, height));
	leftBound.push_back(new DateTime(buffer, height, calClient));
#endif
}

void StatusBar::render(time_t now) const
{
	buffer->setBackgroundColor(Color::White);
	buffer->setForegroundColor(Color::Black);

	buffer->drawLine(x, y + height, x + width, y + height);

	int xOffset = 0;
	for (std::vector<StatusBarComponent *>::const_iterator it = leftBound.begin(); it != leftBound.end(); it++)
	{
		(*it)->render(xOffset, 0, now);
		xOffset += (*it)->getWidth() + 10;

		buffer->drawLine(xOffset, y, xOffset, y + height);
	}

	xOffset = width;
	for (std::vector<StatusBarComponent *>::const_iterator it = rightBound.begin(); it != rightBound.end(); it++)
	{
		xOffset -= (*it)->getWidth();
		buffer->drawLine(xOffset, y, xOffset, y + height);
		(*it)->render(xOffset, y, now);
	}

	xOffset = width / 2;
	buffer->setFontSize(9);

	tm timeInfo = *localtime(&now);
	String nowString = getRefreshTimeStr(&timeInfo, true);
	buffer->drawString(xOffset, y + height / 2 - 1, nowString, Alignment::Center);
}

String BatteryPercentage::getBatBitmap(uint32_t batPercent) const
{
	if (batPercent >= 93)
	{
		return "battery_full_90deg";
	}
	else if (batPercent >= 79)
	{
		return "battery_6_bar_90deg";
	}
	else if (batPercent >= 65)
	{
		return "battery_5_bar_90deg";
	}
	else if (batPercent >= 50)
	{
		return "battery_4_bar_90deg";
	}
	else if (batPercent >= 36)
	{
		return "battery_3_bar_90deg";
	}
	else if (batPercent >= 22)
	{
		return "battery_2_bar_90deg";
	}
	else if (batPercent >= 8)
	{
		return "battery_1_bar_90deg";
	}
	else // batPercent < 8
	{
		return "battery_0_bar_90deg";
	}
}

int BatteryPercentage::getWidth() const
{
	// battery - (expecting 3.7v LiPo)
	uint32_t batteryVoltage = readBatteryVoltage();
	uint32_t batPercent = calcBatPercent(batteryVoltage, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE);

	buffer->setFontSize(9);
	TextSize *textSize = buffer->getStringBounds(String(batPercent) + String("%"));

	if (textSize == NULL)
	{
		return 24;
	}

	return padding + textSize->width + padding + iconSize;
}

void BatteryPercentage::render(int x, int y, time_t now) const
{
	// battery - (expecting 3.7v LiPo)
	uint32_t batteryVoltage = readBatteryVoltage();
	uint32_t batPercent = calcBatPercent(batteryVoltage, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE);

	buffer->setBackgroundColor(Color::White);
	buffer->setForegroundColor(Color::Black);
	Color fgSave = buffer->getForegroundColor();
	Color bgSave = buffer->getBackgroundColor();

#if defined(DISP_3C) || defined(DISP_7C)
	if (batPercent <= 10)
	{
		buffer->setForegroundColor(accentColor);
	}
#endif

	buffer->setFontSize(9);
	int xOffset = x + padding;
	Rect textSize = buffer->drawString(xOffset, y + height / 2 - 1, String(batPercent) + String("%"), Alignment::VerticalCenter | Alignment::Left);

	xOffset += textSize.width + padding;
	buffer->drawIcon(xOffset, y + height / 2, getBatBitmap(batPercent), 24, Alignment::Left | Alignment::VerticalCenter);

	// reset display color
	buffer->setForegroundColor(fgSave);
	buffer->setBackgroundColor(bgSave);
}

int WiFiStatus::getWidth() const
{
	return padding + iconSize + padding;
}

void WiFiStatus::render(int x, int y, time_t now) const
{
	int rssi = WiFi.RSSI();

	buffer->setBackgroundColor(Color::White);
	buffer->setForegroundColor(Color::Black);
	Color fgSave = buffer->getForegroundColor();
	Color bgSave = buffer->getBackgroundColor();

#if defined(DISP_3C) || defined(DISP_7C)
	if (rssi < -70)
	{
		buffer->setForegroundColor(accentColor);
	}
#endif

	buffer->drawIcon(x + padding, y + height / 2, getWiFiBitmap(rssi), iconSize, Alignment::Left | Alignment::VerticalCenter);

	// reset display color
	buffer->setForegroundColor(fgSave);
	buffer->setBackgroundColor(bgSave);
}

// Returns 24x24 bitmap incidcating wifi status.
String WiFiStatus::getWiFiBitmap(int rssi) const
{
	if (rssi == 0)
	{
		return "wifi_x";
	}
	else if (rssi >= -50)
	{
		return "wifi";
	}
	else if (rssi >= -60)
	{
		return "wifi_3_bar";
	}
	else if (rssi >= -70)
	{
		return "wifi_2_bar";
	}
	else
	{ // rssi < -70
		return "wifi_1_bar";
	}
}

int DateTime::getWidth() const
{
	// get the lastRefreshTime from the server response (when was the)
	// calendar updated...
	time_t last_updated = calClient->getLastUpdated();
	tm timeInfo = *localtime(&last_updated);
	String refreshTimeStr = getRefreshTimeStr(&timeInfo, true);

	buffer->setFontSize(9);
	TextSize *textSize = buffer->getStringBounds(refreshTimeStr);

	if (textSize == NULL)
	{
		return 32;
	}

	return 24 + textSize->width + padding;
}

void DateTime::render(int x, int y, time_t now) const
{
	// get the lastRefreshTime from the server response (when was the)
	// calendar updated...
	time_t last_updated = calClient->getLastUpdated();
	tm timeInfo = *localtime(&last_updated);
	String refreshTimeStr = getRefreshTimeStr(&timeInfo, true);

	// I should be using a 24x24 icon here, but I found the
	// 24x24 is a little too small for my liking. So I'm using
	// the bigger 32x32 version, but I have to adjust the offsets
	// just a little so it is center :)
	buffer->drawIcon(x, y + height / 2, "wi_refresh", iconSize, Alignment::Left | Alignment::VerticalCenter);

	buffer->setFontSize(9);
	buffer->drawString(x + iconSize, y + height / 2 - 1, refreshTimeStr, Alignment::VerticalCenter | Alignment::Left);
}
