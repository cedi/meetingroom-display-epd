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

    const static int StatusBarHeight = 25;
};

class BatteryPercentage : public StatusBarComponent
{
public:
    BatteryPercentage(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 24, height){}
    virtual void render(int x, int y) const override;
};

class WiFiStatus : public StatusBarComponent
{
public:
    WiFiStatus(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 24, height){}
    virtual void render(int x, int y) const override;
};

class DateTime : public StatusBarComponent
{
public:
    DateTime(DisplayBuffer *buffer, int height) : StatusBarComponent(buffer, 115, height){}
    virtual void render(int x, int y) const override;
};