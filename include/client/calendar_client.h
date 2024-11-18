#pragma once

#include <vector>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

namespace calendar_client
{
    enum BusyState : int
    {
        Free = 0,
        Tentative = 1,
        Busy = 2
    };

    class CalendarEntry
    {
    protected:
        String title;
        time_t start;
        time_t end;
        bool all_day;
        BusyState busy;
        bool important;
        String message;

    public:
        CalendarEntry() {};
        CalendarEntry(const JsonObject &json);

        String getTitle() const { return title; }
        time_t getStart() const { return start; }
        time_t getEnd() const { return end; }
        bool isAllDay() const { return all_day; }
        BusyState getBusy() const { return busy; }
        bool isImportant() const { return important; }
        String getMessage() const { return message; }
    };

    typedef std::vector<CalendarEntry> CalendarEntries;

    class CalendarClient
    {
    protected:
        WiFiClient client;
        String apiEndpoint;
        int apiPort;

        time_t last_updated;
        CalendarEntries entries;

    public:
        CalendarClient(String apiEndpoint, int apiPort) : apiEndpoint(apiEndpoint), apiPort(apiPort) {}
        int fetchCalendar();

        const CalendarEntry *getCurrentEvent(time_t now) const;
        const CalendarEntry *getNextEvent(time_t now) const;

        time_t getLastUpdated() const { return last_updated; }
        const CalendarEntries *getCalendarEntries() const { return &entries; }

        static const char *getHttpResponsePhrase(int code);

    protected:
        bool parseCalendar(HTTPClient &client);
    };
};