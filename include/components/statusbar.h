#pragma once

#include <vector>
#include "components.h"

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

    virtual void render(int x, int y) const = 0;
};

class StatusBar : public DisplayComponent
{
private:
    std::vector<StatusBarComponent*> leftBound;
    std::vector<StatusBarComponent*> rightBound;

public:
    StatusBar(DisplayBuffer *buffer);

    virtual void render() const override;

    const static int StatusBarHeight = 24;
};

class BatteryPercentage : public StatusBarComponent
{
public:
    BatteryPercentage(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 24, height){}
    virtual void render(int x, int y) const override;

protected:
    const uint8_t *getBatBitmap(uint32_t batPercent) const;
};

class WiFiStatus : public StatusBarComponent
{
public:
    WiFiStatus(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 24, height){}
    virtual void render(int x, int y) const override;

protected:
    const uint8_t *getWiFiBitmap(int rssi) const;
};

class DateTime : public StatusBarComponent
{
public:
    DateTime(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 155, height){}
    virtual void render(int x, int y) const override;
};