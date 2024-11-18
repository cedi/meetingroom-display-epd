#include "components/statusbar.h"

#include <WiFi.h>
#include <Preferences.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include "config.h"
#include "display_utils.h"
#include "fonts/Poppins_Regular.h"

#include "icons/24x24/battery_full_90deg.h"
#include "icons/24x24/battery_6_bar_90deg.h"
#include "icons/24x24/battery_5_bar_90deg.h"
#include "icons/24x24/battery_4_bar_90deg.h"
#include "icons/24x24/battery_3_bar_90deg.h"
#include "icons/24x24/battery_2_bar_90deg.h"
#include "icons/24x24/battery_1_bar_90deg.h"
#include "icons/24x24/battery_0_bar_90deg.h"
#include "icons/24x24/wifi_x.h"
#include "icons/24x24/wifi.h"
#include "icons/24x24/wifi_3_bar.h"
#include "icons/24x24/wifi_2_bar.h"
#include "icons/24x24/wifi_1_bar.h"
#include "icons/32x32/wi_refresh.h"

StatusBar::StatusBar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient)
	: DisplayComponent(buffer, 0, 0, buffer->width(), StatusBarHeight)
{
	rightBound.push_back(new BatteryPercentage(buffer, height));
	rightBound.push_back(new WiFiStatus(buffer, height));

	leftBound.push_back(new DateTime(buffer, height, calClient));
}

void StatusBar::render(time_t now) const
{
	buffer->drawLine(x, y+height, x + width, y+height);

	int xOffset = 0;
	for (std::vector<StatusBarComponent *>::const_iterator it = leftBound.begin(); it != leftBound.end(); it++)
	{
		(*it)->render(xOffset, 0, now);
		xOffset += (*it)->width;
	}

	xOffset = width;
	for (std::vector<StatusBarComponent *>::const_iterator it = rightBound.begin(); it != rightBound.end(); it++)
	{
		xOffset -= (*it)->width;
		(*it)->render(xOffset, y, now);
	}
}

// Returns 24x24 bitmap incidcating battery status.
const uint8_t *BatteryPercentage::getBatBitmap(uint32_t batPercent) const
{
	if (batPercent >= 93)
	{
		return battery_full_90deg;
	}
	else if (batPercent >= 79)
	{
		return battery_6_bar_90deg;
	}
	else if (batPercent >= 65)
	{
		return battery_5_bar_90deg;
	}
	else if (batPercent >= 50)
	{
		return battery_4_bar_90deg;
	}
	else if (batPercent >= 36)
	{
		return battery_3_bar_90deg;
	}
	else if (batPercent >= 22)
	{
		return battery_2_bar_90deg;
	}
	else if (batPercent >= 8)
	{
		return battery_1_bar_90deg;
	}
	else // batPercent < 8
	{
		return battery_0_bar_90deg;
	}
}

void BatteryPercentage::render(int x, int y, time_t now) const
{
	// battery - (expecting 3.7v LiPo)
	uint32_t batPercent = calcBatPercent(3.6, MIN_BATTERY_VOLTAGE, MAX_BATTERY_VOLTAGE);
	buffer->drawBitmap(x, y, getBatBitmap(batPercent), 24, 24);
}

void WiFiStatus::render(int x, int y, time_t now) const
{
	int rssi = WiFi.RSSI();
	buffer->drawBitmap(x, y, getWiFiBitmap(rssi), 24, 24);
}

// Returns 24x24 bitmap incidcating wifi status.
const uint8_t *WiFiStatus::getWiFiBitmap(int rssi) const
{
	if (rssi == 0)
	{
		return wifi_x;
	}
	else if (rssi >= -50)
	{
		return wifi;
	}
	else if (rssi >= -60)
	{
		return wifi_3_bar;
	}
	else if (rssi >= -70)
	{
		return wifi_2_bar;
	}
	else
	{ // rssi < -70
		return wifi_1_bar;
	}
}

void DateTime::render(int x, int y, time_t now) const
{
	// get the lastRefreshTime from the server response (when was the)
	// calendar updated...
	time_t last_updated = calClient->getLastUpdated();
	tm timeInfo = *localtime(&last_updated);
	String refreshTimeStr;
	getRefreshTimeStr(refreshTimeStr, true, &timeInfo);

	buffer->drawLine(x + width, y, x + width, y + height);

	// I should be using a 24x24 icon here, but I found the
	// 24x24 is a little too small for my liking. So I'm using
	// the bigger 32x32 version, but I have to adjust the offsets
	// just a little so it is center :)
	buffer->drawBitmap(x - 3, y - 3, wi_refresh, 32, 32);

	buffer->setFont(&FONT_8pt8b);
	buffer->drawString(x + 24, y + height / 2 - 1, refreshTimeStr, Alignment::VerticalCenter | Alignment::Left);
}
