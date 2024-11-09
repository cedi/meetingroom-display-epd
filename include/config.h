#pragma once

#include <cstdint>
#include <Arduino.h>


// FONTS
// A handful of popular Open Source typefaces have been included with this
// project for your convenience. Change the font by selecting its corresponding
// header file.
//
//   FONT           HEADER FILE              FAMILY          LICENSE
//   FreeMono       FreeMono.h               GNU FreeFont    GNU GPL v3.0
//   FreeSans       FreeSans.h               GNU FreeFont    GNU GPL v3.0
//   FreeSerif      FreeSerif.h              GNU FreeFont    GNU GPL v3.0
//   Lato           Lato_Regular.h           Lato            SIL OFL v1.1
//   Montserrat     Montserrat_Regular.h     Montserrat      SIL OFL v1.1
//   Open Sans      OpenSans_Regular.h       Open Sans       SIL OFL v1.1
//   Poppins        Poppins_Regular.h        Poppins         SIL OFL v1.1
//   Quicksand      Quicksand_Regular.h      Quicksand       SIL OFL v1.1
//   Raleway        Raleway_Regular.h        Raleway         SIL OFL v1.1
//   Roboto         Roboto_Regular.h         Roboto          Apache v2.0
//   Roboto Mono    RobotoMono_Regular.h     Roboto Mono     Apache v2.0
//   Roboto Slab    RobotoSlab_Regular.h     Roboto Slab     Apache v2.0
//   Ubuntu         Ubuntu_R.h               Ubuntu font     UFL v1.0
//   Ubuntu Mono    UbuntuMono_R.h           Ubuntu font     UFL v1.0
//
// Adding new fonts is relatively straightforward, see fonts/README.
//
// Note:
//   The layout of the display was designed around spacing and size of the GNU
//   FreeSans font, but this project supports the ability to modularly swap
//   fonts. Using a font other than FreeSans may result in undesired spacing or
//   other artifacts.
#define FONT_HEADER "fonts/FreeSans.h"

// STATUS BAR EXTRAS
//   Extra information that can be displayed on the status bar. Set to 1 to
//   enable.
#define STATUS_BAR_EXTRAS_BAT_VOLTAGE 0
#define STATUS_BAR_EXTRAS_WIFI_RSSI   0

// BATTERY MONITORING
//   You may choose to power your weather display with or without a battery.
//   Low power behavior can be controlled in config.cpp.
//   If you wish to disable battery monitoring set this macro to 0.
#define BATTERY_MONITORING 1

// NON-VOLATILE STORAGE (NVS) NAMESPACE
#define NVS_NAMESPACE "meeting_epd"

// DEBUG
//   If defined, enables increase verbosity over the serial port.
//   level 0: basic status information, assists troubleshooting (default)
//   level 1: increased verbosity for debugging
//   level 2: print api responses to serial monitor
#define DEBUG_LEVEL 0

// Set the below constants in "config.cpp"
extern const uint8_t PIN_BAT_ADC;
extern const uint8_t PIN_EPD_BUSY;
extern const uint8_t PIN_EPD_CS;
extern const uint8_t PIN_EPD_RST;
extern const uint8_t PIN_EPD_DC;
extern const uint8_t PIN_EPD_SCK;
extern const uint8_t PIN_EPD_MISO;
extern const uint8_t PIN_EPD_MOSI;
extern const uint8_t PIN_EPD_PWR;
extern const uint8_t PIN_BME_SDA;
extern const uint8_t PIN_BME_SCL;
extern const uint8_t PIN_BME_PWR;
extern const uint8_t BME_ADDRESS;
extern const char *WIFI_SSID;
extern const char *WIFI_PASSWORD;
extern const unsigned long WIFI_TIMEOUT;
extern const unsigned HTTP_CLIENT_TCP_TIMEOUT;
extern const String OWM_APIKEY;
extern const String OWM_ENDPOINT;
extern const String OWM_ONECALL_VERSION;
extern const String LAT;
extern const String LON;
extern const String CITY_STRING;
extern const char *TIMEZONE;
extern const char *TIME_FORMAT;
extern const char *HOUR_FORMAT;
extern const char *DATE_FORMAT;
extern const char *REFRESH_TIME_FORMAT;
extern const char *NTP_SERVER_1;
extern const char *NTP_SERVER_2;
extern const unsigned long NTP_TIMEOUT;
extern const int SLEEP_DURATION;
extern const int BED_TIME;
extern const int WAKE_TIME;
extern const int HOURLY_GRAPH_MAX;
extern const uint32_t WARN_BATTERY_VOLTAGE;
extern const uint32_t LOW_BATTERY_VOLTAGE;
extern const uint32_t VERY_LOW_BATTERY_VOLTAGE;
extern const uint32_t CRIT_LOW_BATTERY_VOLTAGE;
extern const unsigned long LOW_BATTERY_SLEEP_INTERVAL;
extern const unsigned long VERY_LOW_BATTERY_SLEEP_INTERVAL;
extern const uint32_t MAX_BATTERY_VOLTAGE;
extern const uint32_t MIN_BATTERY_VOLTAGE;
