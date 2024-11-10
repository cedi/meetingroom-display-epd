#pragma once

#include <vector>
#include "components.h"

class CalendarEntry
{
protected:
    DisplayBuffer *buffer;

public:
    int width;
    int height;

public:
    CalendarEntry(DisplayBuffer *buffer, int width)
        : buffer(buffer), width(width), height(48)
    {
    }

    virtual void render(int x, int y) const;
};

class Calendar : public DisplayComponent
{
private:
    std::vector<CalendarEntry*> events;

public:
    Calendar(DisplayBuffer *buffer);

    virtual void render() const override;
};
