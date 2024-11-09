/* Display helper utilities for esp32-weather-epd.
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

#include <cmath>
#include <vector>
#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#include <aqi.h>

#include "_strftime.h"
#include "config.h"

// icon header files
#include "icons/16x16/wi_na_16x16.h"
#include "icons/24x24/wi_na_24x24.h"
#include "icons/32x32/wi_na_32x32.h"
#include "icons/48x48/wi_na_48x48.h"
#include "icons/64x64/wi_na_64x64.h"
#include "icons/196x196/wi_na_196x196.h"

#include "icons/24x24/battery_full_90deg_24x24.h"
#include "icons/24x24/battery_6_bar_90deg_24x24.h"
#include "icons/24x24/battery_5_bar_90deg_24x24.h"
#include "icons/24x24/battery_4_bar_90deg_24x24.h"
#include "icons/24x24/battery_3_bar_90deg_24x24.h"
#include "icons/24x24/battery_2_bar_90deg_24x24.h"
#include "icons/24x24/battery_1_bar_90deg_24x24.h"
#include "icons/24x24/battery_0_bar_90deg_24x24.h"

#include "icons/24x24/wifi_x_24x24.h"
#include "icons/24x24/wifi_24x24.h"
#include "icons/24x24/wifi_3_bar_24x24.h"
#include "icons/24x24/wifi_2_bar_24x24.h"
#include "icons/24x24/wifi_1_bar_24x24.h"


/* Returns battery voltage in millivolts (mv).
 */
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
  val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db,
                                      ADC_WIDTH_BIT_12, 1100, &adc_chars);

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

/* Returns battery percentage, rounded to the nearest integer.
 * Takes a voltage in millivolts and uses a sigmoidal approximation to find an
 * approximation of the battery life percentage remaining.
 * 
 * This function contains LGPLv3 code from 
 * <https://github.com/rlogiacco/BatterySense>.
 * 
 * Symmetric sigmoidal approximation
 * <https://www.desmos.com/calculator/7m9lu26vpy>
 *
 * c - c / (1 + k*x/v)^3
 */
uint32_t calcBatPercent(uint32_t v, uint32_t minv, uint32_t maxv)
{
  // slow
  //uint32_t p = 110 - (110 / (1 + pow(1.468 * (v - minv)/(maxv - minv), 6)));

  // steep
  //uint32_t p = 102 - (102 / (1 + pow(1.621 * (v - minv)/(maxv - minv), 8.1)));

  // normal
  uint32_t p = 105 - (105 / (1 + pow(1.724 * (v - minv)/(maxv - minv), 5.5)));
  return p >= 100 ? 100 : p;
} // end calcBatPercent

// Returns 24x24 bitmap incidcating battery status.
const uint8_t *getBatBitmap24(uint32_t batPercent)
{
  if (batPercent >= 93)
  {
    return battery_full_90deg_24x24;
  }
  else if (batPercent >= 79)
  {
    return battery_6_bar_90deg_24x24;
  }
  else if (batPercent >= 65)
  {
    return battery_5_bar_90deg_24x24;
  }
  else if (batPercent >= 50)
  {
    return battery_4_bar_90deg_24x24;
  }
  else if (batPercent >= 36)
  {
    return battery_3_bar_90deg_24x24;
  }
  else if (batPercent >= 22)
  {
    return battery_2_bar_90deg_24x24;
  }
  else if (batPercent >= 8)
  {
    return battery_1_bar_90deg_24x24;
  }
  else
  {  // batPercent < 8
    return battery_0_bar_90deg_24x24;
  }
} // end getBatBitmap24

// Gets string with the current date.
void getDateStr(String &s, tm *timeInfo)
{
  char buf[48] = {};
  _strftime(buf, sizeof(buf), DATE_FORMAT, timeInfo);
  s = buf;

  // remove double spaces. %e will add an extra space, ie. " 1" instead of "1"
  s.replace("  ", " ");
  return;
} // end getDateStr

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
} // end getRefreshTimeStr

// Returns the wifi signal strength descriptor text for the given RSSI.
const char *getWiFidesc(int rssi)
{
  if (rssi == 0)
  {
    return "no WiFi connection"; //TXT_WIFI_NO_CONNECTION;
  }
  else if (rssi >= -50)
  {
    return "excellent"; //TXT_WIFI_EXCELLENT;
  }
  else if (rssi >= -60)
  {
    return "good"; //TXT_WIFI_GOOD;
  }
  else if (rssi >= -70)
  {
    return "fair"; //TXT_WIFI_FAIR;
  }
  else
  {  // rssi < -70
    return "weak"; //TXT_WIFI_WEAK;
  }
}

// Returns 24x24 bitmap incidcating wifi status.
const uint8_t *getWiFiBitmap24(int rssi)
{
  if (rssi == 0)
  {
    return wifi_x_24x24;
  }
  else if (rssi >= -50)
  {
    return wifi_24x24;
  }
  else if (rssi >= -60)
  {
    return wifi_3_bar_24x24;
  }
  else if (rssi >= -70)
  {
    return wifi_2_bar_24x24;
  }
  else
  {  // rssi < -70
    return wifi_1_bar_24x24;
  }
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
