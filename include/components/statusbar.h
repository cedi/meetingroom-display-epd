#pragma once

#include <vector>
#include "components.h"
#include "client/calendar_client.h"

class StatusBarComponent
{
protected:
    DisplayBuffer *buffer;

public:
    int width;
    int height;

public:
    StatusBarComponent(DisplayBuffer *buffer, int width, int height)
        : buffer(buffer), width(width), height(height)
    {
    }

    virtual void render(int x, int y, time_t now) const = 0;
};

class StatusBar : public DisplayComponent
{
private:
    std::vector<StatusBarComponent *> leftBound;
    std::vector<StatusBarComponent *> rightBound;

public:
    StatusBar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient);

    virtual void render(time_t now) const override;

    const static int StatusBarHeight = 24;
};

class BatteryPercentage : public StatusBarComponent
{
public:
    BatteryPercentage(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 24, height) {}
    virtual void render(int x, int y, time_t now) const override;

protected:
    const uint8_t *getBatBitmap(uint32_t batPercent) const;
};

class WiFiStatus : public StatusBarComponent
{
public:
    WiFiStatus(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 24, height) {}
    virtual void render(int x, int y, time_t now) const override;

protected:
    const uint8_t *getWiFiBitmap(int rssi) const;
};

class DateTime : public StatusBarComponent
{
private:
    calendar_client::CalendarClient *calClient;

public:
    DateTime(DisplayBuffer *buffer, int height, calendar_client::CalendarClient *calClient) : StatusBarComponent(buffer, 155, height), calClient(calClient) {}
    virtual void render(int x, int y, time_t now) const override;
};