#pragma once

#include <string>

#include "components.h"
#include "client/calendar_client.h"

class Status : public DisplayComponent
{
private:
    calendar_client::CalendarClient *calClient;

public:
    Status(DisplayBuffer *buffer, calendar_client::CalendarClient *calClient);
    virtual void render(time_t now) const override;
};