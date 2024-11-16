#include <cmath>
#include <vector>
#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "display_utils.h"

#include "_strftime.h"
#include "config.h"

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
} // end calcBatPercent

// Gets string with the current date.
void getDateStr(String &s, tm *timeInfo)
{
	char buf[48] = {};
	_strftime(buf, sizeof(buf), DATE_FORMAT, timeInfo);
	s = buf;

	// remove double spaces. %e will add an extra space, ie. " 1" instead of "1"
	s.replace("  ", " ");
	return;
}

// Gets string with the current time.
void getTimeStr(String &s, tm *timeInfo)
{
	char buf[48] = {};
	_strftime(buf, sizeof(buf), TIME_FORMAT, timeInfo);
	s = buf;

	// remove double spaces. %e will add an extra space, ie. " 1" instead of "1"
	s.replace("  ", " ");
	return;
}

// Gets string with the current date and time of the current refresh attempt.
void getRefreshTimeStr(String &s, bool timeSuccess, tm *timeInfo)
{
	if (timeSuccess == false)
	{
		s = "unknown";
		return;
	}

	char buf[48] = {};
	_strftime(buf, sizeof(buf), REFRESH_TIME_FORMAT, timeInfo);
	s = buf;

	// remove double spaces.
	s.replace("  ", " ");
	return;
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

// Perform an HTTP GET request to ical server
// Returns the HTTP Status Code.
int getCalendar(WiFiClient &client, time_t *last_updated, CalendarEntries *calendarEntries)
{
	int attempts = 0;
	bool rxSuccess = false;

	int httpResponse = 0;
	while (!rxSuccess && attempts < 3)
	{
		wl_status_t connection_status = WiFi.status();
		if (connection_status != WL_CONNECTED)
		{
			// -512 offset distinguishes these errors from httpClient errors
			return -512 - static_cast<int>(connection_status);
		}

		HTTPClient http;
		http.setConnectTimeout(HTTP_CLIENT_TCP_TIMEOUT); // default 10s
		http.setTimeout(HTTP_CLIENT_TCP_TIMEOUT);		 // default 10s
		http.addHeader(String("Content-Type"), String("application/protobuf"));

		http.begin(client, String("192.168.0.32"), 8080, String("/calendar"));
		httpResponse = http.GET();
		Serial.println("HTTP Response: " + String(httpResponse, DEC));

		if (httpResponse == HTTP_CODE_OK)
		{
			decodeCalendarResponse(http, last_updated, calendarEntries);
			rxSuccess = true;
		}

		client.stop();
		http.end();
		++attempts;
	}

	return httpResponse;
}

bool decodeCalendarResponse(HTTPClient &client, time_t *last_updated, CalendarEntries *calendarEntries)
{
	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, client.getStream());

#if DEBUG_LEVEL >= 1
	Serial.println("[debug] doc.overflowed() : " + String(doc.overflowed()));
#endif
#if DEBUG_LEVEL >= 2
	serializeJsonPretty(doc, Serial);
#endif

	if (error)
	{
		return false;
	}

	*last_updated = doc["last_updated"].as<time_t>();
	Serial.printf("[debug] lastUpdated: %ld\n", *last_updated);

	_CalendarEntry calEntry;
	for (JsonObject entry : doc["entries"].as<JsonArray>())
	{
		if (entry.containsKey("title"))
		{
			calEntry.title = String(entry["title"].as<const char *>());
		}

		if (entry.containsKey("message"))
		{
			calEntry.message = String(entry["message"].as<const char *>());
		}

		if (entry.containsKey("start"))
		{
			calEntry.start = entry["start"].as<time_t>();
		}

		if (entry.containsKey("end"))
		{
			calEntry.end = entry["end"].as<time_t>();
		}

		if (entry.containsKey("all_day"))
		{
			calEntry.all_day = entry["all_day"].as<bool>();
		}

		if (entry.containsKey("busy"))
		{
			calEntry.busy = entry["busy"].as<BusyState>();
		}

		if (entry.containsKey("important"))
		{
			calEntry.important = entry["important"].as<bool>();
		}
		else
		{
			calEntry.important = false;
		}

		calendarEntries->push_back(calEntry);
	}

	return true;
}