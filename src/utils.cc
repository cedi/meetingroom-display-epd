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
#include <cmath>

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
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <WiFiClient.h>

#ifndef USE_HTTP
#include <WiFiClient.h>
#endif

// header files
#include "utils.h"
#include "config.h"
#include "_strftime.h"

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
bool getNtpTime(tm *timeInfo)
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
}

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

// Returns battery voltage in millivolts (mv).
uint32_t readBatteryVoltage()
{
	esp_adc_cal_characteristics_t adc_chars;
	// __attribute__((unused)) disables compiler warnings about this variable
	// being unused (Clang, GCC) which is the case when DEBUG_LEVEL == 0.
	esp_adc_cal_value_t val_type __attribute__((unused));
	adc_power_acquire();
	uint16_t adc_val = analogRead(PIN_BAT_ADC);
	adc_power_release();

	// We will use the eFuse ADC calibration bits, to get accurate voltage
	// readings. The DFRobot FireBeetle Esp32-E V1.0's ADC is 12 bit, and uses
	// 11db attenuation, which gives it a measurable input voltage range of 150mV
	// to 2450mV.
	val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, 1100, &adc_chars);

#if DEBUG_LEVEL >= 1
	if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
	{
		Serial.println("[debug] ADC Cal eFuse Vref");
	}
	else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
	{
		Serial.println("[debug] ADC Cal Two Point");
	}
	else
	{
		Serial.println("[debug] ADC Cal Default");
	}
#endif

	uint32_t batteryVoltage = esp_adc_cal_raw_to_voltage(adc_val, &adc_chars);
	// DFRobot FireBeetle Esp32-E V1.0 voltage divider (1M+1M), so readings are
	// multiplied by 2.
	batteryVoltage *= 2;
	return batteryVoltage;
} // end readBatteryVoltage

// Returns battery percentage, rounded to the nearest integer.
// Takes a voltage in millivolts and uses a sigmoidal approximation to find an
// approximation of the battery life percentage remaining.
//
// This function contains LGPLv3 code from
// <https://github.com/rlogiacco/BatterySense>.
//
// Symmetric sigmoidal approximation
// <https://www.desmos.com/calculator/7m9lu26vpy>
// c - c / (1 + k*x/v)^3
uint32_t calcBatPercent(uint32_t v, uint32_t minv, uint32_t maxv)
{
	// slow
	// uint32_t p = 110 - (110 / (1 + pow(1.468 * (v - minv)/(maxv - minv), 6)));

	// steep
	// uint32_t p = 102 - (102 / (1 + pow(1.621 * (v - minv)/(maxv - minv), 8.1)));

	// normal
	uint32_t p = 105 - (105 / (1 + pow(1.724 * (v - minv) / (maxv - minv), 5.5)));
	return p >= 100 ? 100 : p;
}

// Gets string with the current time.
String getTimeStr(tm *timeInfo)
{
	String timeStr;
	char buf[48] = {};
	_strftime(buf, sizeof(buf), TIME_FORMAT, timeInfo);
	timeStr = buf;

	// remove double spaces. %e will add an extra space, ie. " 1" instead of "1"
	timeStr.replace("  ", " ");
	return timeStr;
}

// Gets string with the current date and time of the current refresh attempt.
String getRefreshTimeStr(tm *timeInfo, bool timeSuccess)
{
	String timeStr = "unknown";

	if (timeSuccess == false)
	{
		return timeStr;
	}

	char buf[48] = {};
	_strftime(buf, sizeof(buf), REFRESH_TIME_FORMAT, timeInfo);
	timeStr = buf;

	// remove double spaces.
	timeStr.replace("  ", " ");

	return timeStr;
}

// This function sets the builtin LED to LOW and disables it even during deep sleep.
void disableBuiltinLED()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	gpio_hold_en(static_cast<gpio_num_t>(LED_BUILTIN));
	gpio_deep_sleep_hold_en();
	return;
}
