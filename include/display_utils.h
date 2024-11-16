/* Display helper utility declarations for esp32-weather-epd.
 * Copyright (C) 2022-2024  Luke Marzen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include <time.h>
#include <string.h>
#include <WiFiType.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

/* Enum definitions */
typedef enum _BusyState {
    Busy = 0,
    Tentative = 1,
    Free = 2
} BusyState;

/* Struct definitions */
typedef struct {
    String title;
    /* Represents seconds of UTC time since Unix epoch
 1970-01-01T00:00:00Z. Must be from 0001-01-01T00:00:00Z to
 9999-12-31T23:59:59Z inclusive. */
    time_t start;
    time_t end;
    bool all_day;
    BusyState busy;
    bool important;
    String message;
} _CalendarEntry;

typedef std::vector<_CalendarEntry> CalendarEntries;

uint32_t readBatteryVoltage();
uint32_t calcBatPercent(uint32_t v, uint32_t minv, uint32_t maxv);
void getDateStr(String &s, tm *timeInfo);
void getTimeStr(String &s, tm *timeInfo);
void getRefreshTimeStr(String &s, bool timeSuccess, tm *timeInfo);
const char *getWiFidesc(int rssi);
const char *getWifiStatusPhrase(wl_status_t status);
void disableBuiltinLED();
int getCalendar(WiFiClient &client, time_t *last_updated, CalendarEntries *calendarEntries);
bool decodeCalendarResponse(HTTPClient &client, time_t *last_updated, CalendarEntries *calendarEntries);