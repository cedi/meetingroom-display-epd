#pragma once

#include <vector>
#include "components.h"

class CalendarEntry
{
protected:
    DisplayBuffer *buffer;
    bool pastEvent;

public:
    const int width;
    const int height;

public:
    CalendarEntry(DisplayBuffer *buffer, int width)
        : CalendarEntry(buffer, width, false)
    {
    }

    CalendarEntry(DisplayBuffer *buffer, int width, bool pastEvent)
        : buffer(buffer), pastEvent(pastEvent), width(width), height(48)
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
