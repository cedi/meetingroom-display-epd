#include "components/statusbar.h"

#include <WiFi.h>
#include <Preferences.h>

#include "config.h"
#include "display_utils.h"
#include "fonts/FreeSans.h"

#include "icons/32x32/wi_refresh_32x32.h"

StatusBar::StatusBar(DisplayBuffer *buffer)
    : DisplayComponent(buffer, 0, 0, buffer->width(), StatusBarHeight)
{
    rightBound.push_back(new BatteryPercentage(buffer, height));
    rightBound.push_back(new WiFiStatus(buffer, height));

    leftBound.push_back(new DateTime(buffer, height));
}

void StatusBar::render() const
{
    buffer->drawRect(x, y, width, height);

    int xOffset = 0;
    for (std::vector<StatusBarComponent *>::const_iterator it = leftBound.begin(); it != leftBound.end(); it++)
    {
        (*it)->render(xOffset, 0);
        xOffset += (*it)->width;
    }

    xOffset = width;
    for (std::vector<StatusBarComponent *>::const_iterator it = rightBound.begin(); it != rightBound.end(); it++)
    {
        xOffset -= (*it)->width;
        (*it)->render(xOffset, y);
    }
}

void BatteryPercentage::render(int x, int y) const
{
    buffer->drawRect(x, y, width, height);

    // battery - (expecting 3.7v LiPo)
    uint32_t batPercent = calcBatPercent(3.6, 3.3, 4.2);
    buffer->drawBitmap(x, y, getBatBitmap24(batPercent), 24, 24);
}

void WiFiStatus::render(int x, int y) const
{
    buffer->drawRect(x, y, width, height);
    int rssi = WiFi.RSSI();
    buffer->drawBitmap(x, y, getWiFiBitmap24(rssi), 24, 24);
}

void DateTime::render(int x, int y) const
{
    Preferences prefs;
    prefs.begin(NVS_NAMESPACE, true);
    String refreshTimeStr = prefs.getString("refreshTimeStr");
    String dateStr = prefs.getString("dateStr");
    prefs.end();

    buffer->drawRect(x, y, width, height);

    // I should be using a 24x24 icon here, but I found the
    // 24x24 is a little too small for my liking. So I'm using
    // the bigger 32x32 version, but I have to adjust the offsets
    // just a little so it is center :)
    buffer->drawBitmap(x - 3, y - 3, wi_refresh_32x32, 32, 32);

    buffer->setFont(&FONT_8pt8b);
    buffer->drawString(x + 24, y + height/2 -1, refreshTimeStr, Alignment::VerticalCenter | Alignment::Left);
}