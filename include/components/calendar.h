#pragma once

#include <vector>
#include "components.h"
#include "display_utils.h"
#include "client/calendar_client.h"

class Calendar : public DisplayComponent
{
private:
    calendar_client::CalendarClient *calClient;
    const int entryHeight;

public:
    Calendar(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient);

    virtual void render(time_t now) const override;

protected:
    virtual void renderCalendarEntry(int x, int y, const calendar_client::CalendarEntry &entry, time_t now) const;
};
