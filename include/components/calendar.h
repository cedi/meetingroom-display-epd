#pragma once

#include <vector>
#include "components.h"
#include "display_utils.h"

class CalendarEntry
{
protected:
    DisplayBuffer *buffer;

public:
    _CalendarEntry entry;
    const int width;
    const int height;

public:
    CalendarEntry(DisplayBuffer *buffer, int width, _CalendarEntry entry)
        : buffer(buffer), entry(entry), width(width), height(48)
    {
    }

    virtual void render(int x, int y) const;
};

class Calendar : public DisplayComponent
{
private:
    std::vector<CalendarEntry> events;

public:
    Calendar(DisplayBuffer *buffer);

    void addEvent(const _CalendarEntry& entry) { events.push_back(CalendarEntry(buffer, width, entry)); }
    virtual void render() const override;

    std::vector<CalendarEntry>::iterator getCurrentEvent();
    std::vector<CalendarEntry>::iterator last() { return events.end(); }
};
