/* Client side utility declarations for esp32-weather-epd.
 * Copyright (C) 2022-2023  Luke Marzen
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

#include <Arduino.h>
#include <vector>
#include <time.h>
#include <string.h>
#include <WiFiType.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

wl_status_t startWiFi();
void killWiFi();
bool getNtpTime(tm *timeInfo);
bool printLocalTime(tm *timeInfo);
void printHeapUsage();
uint32_t readBatteryVoltage();
uint32_t calcBatPercent(uint32_t v, uint32_t minv, uint32_t maxv);
String getTimeStr(tm *timeInfo);
String getRefreshTimeStr(tm *timeInfo, bool timeSuccess);
const char *getWiFidesc(int rssi);
const char *getWifiStatusPhrase(wl_status_t status);
void disableBuiltinLED();