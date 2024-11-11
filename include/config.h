#pragma once

// NON-VOLATILE STORAGE (NVS) NAMESPACE
#define NVS_NAMESPACE "meeting_epd"

// DEBUG
//   If defined, enables increase verbosity over the serial port.
//   level 0: basic status information, assists troubleshooting (default)
//   level 1: increased verbosity for debugging
//   level 2: print api responses to serial monitor
#define DEBUG_LEVEL 0

// PINS
// The configuration below is intended for use with the project's official
// wiring diagrams using the FireBeetle 2 ESP32-E microcontroller board.
//
// Note: LED_BUILTIN pin will be disabled to reduce power draw.  Refer to your
//       board's pinout to ensure you avoid using a pin with this shared
//       functionality.
//
// ADC pin used to measure battery voltage
#define PIN_BAT_ADC A2 // A0 for micro-usb firebeetle

// Pins for E-Paper Driver Board
#define PIN_EPD_BUSY 14 // 5 for micro-usb firebeetle
#define PIN_EPD_CS 13
#define PIN_EPD_RST 21
#define PIN_EPD_DC 22
#define PIN_EPD_SCK 18
#define PIN_EPD_MISO 19 // 19 Master-In Slave-Out not used, as no data from display
#define PIN_EPD_MOSI 23
#define PIN_EPD_PWR 26 // Irrelevant if directly connected to 3.3V

// WIFI
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "WIFIPSK"
#define WIFI_TIMEOUT  10000 // ms, WiFi connection timeout.

// HTTP
// The following errors are likely the result of insuffient http client tcp timeout:
//   -1   Connection Refused
//   -11  Read Timeout
//   -258 Deserialization Incomplete Input
#define HTTP_CLIENT_TCP_TIMEOUT 10000 // ms

// TIME
// For list of time zones see
// https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

// Time format used when displaying sunrise/set times. (Max 11 characters)
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
#define TIME_FORMAT "%H:%M" // 24-hour ex: 01:23   23:00

// Time format used when displaying axis labels. (Max 11 characters)
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
#define HOUR_FORMAT "%H" // 24-hour ex: 01   23

// Date format used when displaying date in top-right corner.
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
#define DATE_FORMAT "%a, %B %e" // ex: Sat, January 1

// Date/Time format used when displaying the last refresh time along the bottom
// of the screen.
// For more information about formatting see
// https://man7.org/linux/man-pages/man3/strftime.3.html
#define REFRESH_TIME_FORMAT "%F %H:%M"

// NTP_SERVER_1 is the primary time server, while NTP_SERVER_2 is a fallback.
// pool.ntp.org will find the closest available NTP server to you.
#define NTP_SERVER_1 "pool.ntp.org"
#define NTP_SERVER_2 "time.nist.gov"

// If you encounter the 'Failed To Fetch The Time' error, try increasing
// NTP_TIMEOUT or select closer/lower latency time servers.
#define NTP_TIMEOUT  20000 // ms

// Sleep duration in minutes. (aka how often esp32 will wake for an update)
// Aligned to the nearest minute boundary.
// For example, if set to 30 (minutes) the display will update at 00 or 30
// minutes past the hour. (range: [2-1440])
// Note: The OpenWeatherMap model is update every 10 minutes, so updating more
//       frequently than that is unnessesary.
#define SLEEP_DURATION 30 // minutes

// Bed Time Power Savings.
// If BED_TIME == WAKE_TIME, then this battery saving feature will be disabled.
// (range: [0-23])
#define BED_TIME 0	 // Last update at 00:00 (midnight) until WAKE_TIME.
#define WAKE_TIME 9 // Hour of first update after BED_TIME, 06:00.

// Note that the minute alignment of SLEEP_DURATION begins at WAKE_TIME even if
// Bed Time Power Savings is disabled.
// For example, if WAKE_TIME = 00 (midnight) and SLEEP_DURATION = 120, then the
// display will update at 00:00, 02:00, 04:00... until BED_TIME.
// If you desire to have your display refresh exactly once a day, you should set
// SLEEP_DURATION = 1440, and you can set the time it should update each day by
// setting both BED_TIME and WAKE_TIME to the hour you want it to update.

// BATTERY
// To protect the battery upon LOW_BATTERY_VOLTAGE, the display will cease to
// update until battery is charged again. The ESP32 will deep-sleep (consuming
// < 11μA), waking briefly check the voltage at the corresponding interval (in
// minutes). Once the battery voltage has fallen to CRIT_LOW_BATTERY_VOLTAGE,
// the esp32 will hibernate and a manual press of the reset (RST) button to
// begin operating again.
#define WARN_BATTERY_VOLTAGE 3535				   // (millivolts) ~20%
#define LOW_BATTERY_VOLTAGE 3462				   // (millivolts) ~10%
#define VERY_LOW_BATTERY_VOLTAGE 3442			   // (millivolts)  ~8%
#define CRIT_LOW_BATTERY_VOLTAGE 3404			   // (millivolts)  ~5%
#define LOW_BATTERY_SLEEP_INTERVAL  30	   // (minutes)
#define VERY_LOW_BATTERY_SLEEP_INTERVAL  120 // (minutes)

// Battery voltage calculations are based on a typical 3.7v LiPo.
#define MAX_BATTERY_VOLTAGE 4200 // (millivolts)
#define MIN_BATTERY_VOLTAGE 3000 // (millivolts)
