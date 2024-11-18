/* Main program for esp32-weather-epd.
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

#include <Arduino.h>
#include <Preferences.h>
#include <time.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <ctime>

#include "config.h"
#include "display_utils.h"
#include "client_utils.h"
#include "client/calendar_client.h"

#include "icons/196x196/wifi.h"
#include "icons/196x196/wifi_x.h"
#include "icons/196x196/wi_time_4.h"
#include "icons/196x196/biological_hazard_symbol.h"
#include "icons/196x196/battery_alert_90deg.h"
#include "icons/196x196/wi_cloud_down.h"

#include "display.h"

Preferences prefs;
calendar_client::CalendarClient calClient("192.168.0.32", 8080);
Display epd(PIN_EPD_PWR,
			PIN_EPD_SCK,
			PIN_EPD_MISO,
			PIN_EPD_MOSI,
			PIN_EPD_CS,
			PIN_EPD_DC,
			PIN_EPD_RST,
			PIN_EPD_BUSY,
			&calClient);

/* Put esp32 into ultra low-power deep sleep (<11μA).
 * Aligns wake time to the minute. Sleep times defined in config.cpp.
 */
void beginDeepSleep(unsigned long startTime, tm *timeInfo, bool useNtpTime = false, bool calendarFetched = false)
{
	// only call getLocalTime if we havent gotten the ntp time yet
	if (!useNtpTime)
	{
		if (!getLocalTime(timeInfo))
		{
			Serial.println("Failed to synchronize time before deep-sleep, referencing older time.");
		}
	}

	// To simplify sleep time calculations, the current time stored by timeInfo
	// will be converted to time relative to the WAKE_TIME. This way if a
	// SLEEP_DURATION is not a multiple of 60 minutes it can be more trivially,
	// aligned and it can easily be deterimined whether we must sleep for
	// additional time due to bedtime.
	// i.e. when curHour == 0, then timeInfo->tm_hour == WAKE_TIME
	int bedtimeHour = INT_MAX;
	if (BED_TIME != WAKE_TIME)
	{
		bedtimeHour = (BED_TIME - WAKE_TIME + 24) % 24;
	}

	// time is relative to wake time
	int curHour = (timeInfo->tm_hour - WAKE_TIME + 24) % 24;
	const int curMinute = curHour * 60 + timeInfo->tm_min;

	int sleepMinutes = SLEEP_DURATION;

	// Sleep duration is until the end of this meeting, or till the beginning of the next meeting
	if (calendarFetched)
	{
		time_t now = mktime(timeInfo);
		const calendar_client::CalendarEntry *currEvent = calClient.getCurrentEvent(now);

		if (currEvent != NULL)
		{
			sleepMinutes = difftime(currEvent->getEnd(), now) / 60; // calculate the minutes until the event ends
#if DEBUG_LEVEL >= 1
			Serial.println("[debug] sleeping till end of this event: " + currEvent->getTitle() + " (" + sleepMinutes + "min)");
#endif
		}
		else
		{
			const calendar_client::CalendarEntry *nextEvent = calClient.getNextEvent(now);
			if (nextEvent != NULL)
			{
				sleepMinutes = std::ceil(difftime(nextEvent->getStart(), now) / 60.0); // calculate the minutes until the next event starts
#if DEBUG_LEVEL >= 1
				Serial.println("[debug] sleeping till start of next event: " + nextEvent->getTitle() + " (" + sleepMinutes + "min)");
#endif
			}
		}
	}

	// estimated wake time, if this falls in a sleep period then sleepDuration
	// must be adjusted
	const int predictedWakeHour = ((curMinute + sleepMinutes) / 60) % 24;

	uint64_t sleepDuration;
	if (predictedWakeHour < bedtimeHour)
	{
		sleepDuration = sleepMinutes * 60 - timeInfo->tm_sec;
	}
	else
	{
		const int hoursUntilWake = 24 - curHour;
		sleepDuration = hoursUntilWake * 3600ULL - (timeInfo->tm_min * 60ULL + timeInfo->tm_sec);
	}

	// add extra delay to compensate for esp32's with fast RTCs.
	sleepDuration += 10ULL;

#if DEBUG_LEVEL >= 1
	printHeapUsage();
#endif

	Serial.println("Awake for " + String((millis() - startTime) / 1000.0, 3) + "s");
	Serial.println("Entering deep sleep for " + String(sleepDuration) + "s");
	esp_sleep_enable_timer_wakeup(sleepDuration * 1000000ULL);

	esp_deep_sleep_start();
}

// Program entry point.
void setup()
{
	unsigned long startTime = millis();
	Serial.begin(115200);

#if DEBUG_LEVEL >= 1
	printHeapUsage();
#endif

	disableBuiltinLED();

	// Open namespace for read/write to non-volatile storage
	prefs.begin(NVS_NAMESPACE, false);

	uint32_t batteryVoltage = readBatteryVoltage();
	Serial.print("Battery voltage");
	Serial.println(": " + String(batteryVoltage) + "mv");

	// When the battery is low, the display should be updated to reflect that, but
	// only the first time we detect low voltage. The next time the display will
	// refresh is when voltage is no longer low. To keep track of that we will
	// make use of non-volatile storage.
	bool lowBat = prefs.getBool("lowBat", false);

	// low battery, deep sleep now
	if (batteryVoltage <= LOW_BATTERY_VOLTAGE)
	{
		if (lowBat == false)
		{ // battery is now low for the first time
			prefs.putBool("lowBat", true);
			prefs.end();
			epd.init();
			epd.error(battery_alert_90deg, "Low Battery");
			epd.powerOff();
		}

		// critically low battery
		// don't set esp_sleep_enable_timer_wakeup();
		// We won't wake up again until someone manually presses the RST button.
		if (batteryVoltage <= CRIT_LOW_BATTERY_VOLTAGE)
		{
			Serial.println("Critically low battery voltage!");
			Serial.println("Hibernating without wake time!");
		}
		// very low battery
		else if (batteryVoltage <= VERY_LOW_BATTERY_VOLTAGE)
		{
			Serial.println("Very low battery voltage!");
			Serial.print("Entering deep sleep for " + String(VERY_LOW_BATTERY_SLEEP_INTERVAL) + "min");
			esp_sleep_enable_timer_wakeup(VERY_LOW_BATTERY_SLEEP_INTERVAL * 60ULL * 1000000ULL);
		}
		// low battery
		else
		{
			Serial.println("Low battery voltage!");
			Serial.println("Entering deep sleep for " + String(LOW_BATTERY_SLEEP_INTERVAL) + "min");
			esp_sleep_enable_timer_wakeup(LOW_BATTERY_SLEEP_INTERVAL * 60ULL * 1000000ULL);
		}

		esp_deep_sleep_start();
	}

	// battery is no longer low, reset variable in non-volatile storage
	if (lowBat == true)
	{
		prefs.putBool("lowBat", false);
	}

	// All data should have been loaded from NVS. Close filesystem.
	prefs.end();

	String statusStr = {};
	String tmpStr = {};
	tm timeInfo = {};

	// START WIFI
	wl_status_t wifiStatus = startWiFi();

	// WiFi Connection Failed
	if (wifiStatus != WL_CONNECTED)
	{
		killWiFi();
		epd.init();

		if (wifiStatus == WL_NO_SSID_AVAIL)
		{
			Serial.println("Network Not Available");
			epd.error(wifi_x, "Network Not Available");
		}
		else
		{
			Serial.println("WiFi Connection Failed");
			epd.error(wifi_x, "WiFi Connection Failed");
		}

		epd.powerOff();
		beginDeepSleep(startTime, &timeInfo);
	}

	// TIME SYNCHRONIZATION
	configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
	bool timeConfigured = getNtpTime(&timeInfo);
	if (!timeConfigured)
	{
		killWiFi();

		Serial.println("Time Synchronization Failed");
		epd.init();
		epd.error(wi_time_4, "Time Synchronization Failed");
		epd.powerOff();

		beginDeepSleep(startTime, &timeInfo);
	}

	int httpStatus = calClient.fetchCalendar();
	if (httpStatus != HTTP_CODE_OK)
	{
		killWiFi();

		Serial.printf("Fetching calendar failed. %d: %s", httpStatus, calendar_client::CalendarClient::getHttpResponsePhrase(httpStatus));

		epd.init();
		epd.error(wi_cloud_down, statusStr);
		epd.powerOff();

		beginDeepSleep(startTime, &timeInfo, true);
	}

	epd.init();
	epd.render(mktime(&timeInfo));
	epd.powerOff();

	// DEEP SLEEP
	beginDeepSleep(startTime, &timeInfo, true, true);
} // end setup

// This will never run
void loop()
{
}
