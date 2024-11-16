/* Client side utilities for esp32-weather-epd.
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

// built-in C++ libraries
#include <cstring>
#include <vector>

// arduino/esp32 libraries
#include <Arduino.h>
#include <esp_sntp.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <time.h>
#include <WiFi.h>

// additional libraries
#include <Adafruit_BusIO_Register.h>
#include <ArduinoJson.h>

// header files
#include "client_utils.h"
#include "config.h"
#include "display_utils.h"
#ifndef USE_HTTP
#include <WiFiClient.h>
#endif

#ifdef USE_HTTP
static const uint16_t OWM_PORT = 80;
#else
static const uint16_t OWM_PORT = 443;
#endif

// Power-on and connect WiFi.
wl_status_t startWiFi()
{
	WiFi.mode(WIFI_STA);
	Serial.printf("%s '%s'", "Connecting to", WIFI_SSID);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	// timeout if WiFi does not connect in WIFI_TIMEOUT ms from now
	unsigned long timeout = millis() + WIFI_TIMEOUT;
	wl_status_t connection_status = WiFi.status();

	while ((connection_status != WL_CONNECTED) && (millis() < timeout))
	{
		Serial.print(".");
		delay(50);
		connection_status = WiFi.status();
	}
	Serial.println();

	if (connection_status != WL_CONNECTED)
	{
		Serial.printf("%s '%s'\n", "Could not connect to", WIFI_SSID);
		return connection_status;
	}

	Serial.println("IP: " + WiFi.localIP().toString());
	Serial.printf("RSSI: %d\n", WiFi.RSSI());
	return connection_status;
}

// Disconnect and power-off WiFi.
void killWiFi()
{
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
}

/* Prints the local time to serial monitor.
 *
 * Returns true if getting local time was a success, otherwise false.
 */
bool printLocalTime(tm *timeInfo)
{
	int attempts = 0;
	while (!getLocalTime(timeInfo) && attempts++ < 3)
	{
		Serial.println("Failed to get the time!");
		return false;
	}
	Serial.println(timeInfo, "%A, %B %d, %Y %H:%M:%S");
	return true;
} // printLocalTime

/* Waits for NTP server time sync, adjusted for the time zone specified in
 * config.cpp.
 *
 * Returns true if time was set successfully, otherwise false.
 *
 * Note: Must be connected to WiFi to get time from NTP server.
 */
bool waitForSNTPSync(tm *timeInfo)
{
	// Wait for SNTP synchronization to complete
	unsigned long timeout = millis() + NTP_TIMEOUT;
	if ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) && (millis() < timeout))
	{
		Serial.print("Waiting for SNTP synchronization.");
		delay(100); // ms
		while ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) && (millis() < timeout))
		{
			Serial.print(".");
			delay(100); // ms
		}
		Serial.println();
	}
	return printLocalTime(timeInfo);
} // waitForSNTPSync

/* Prints debug information about heap usage.
 */
void printHeapUsage()
{
	Serial.println("[debug] Heap Size       : " + String(ESP.getHeapSize()) + " B");
	Serial.println("[debug] Available Heap  : " + String(ESP.getFreeHeap()) + " B");
	Serial.println("[debug] Min Free Heap   : " + String(ESP.getMinFreeHeap()) + " B");
	Serial.println("[debug] Max Allocatable : " + String(ESP.getMaxAllocHeap()) + " B");
	return;
}
