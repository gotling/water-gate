#ifndef GLOBALS_H
#define GLOBALS_H

#define ANALOG_MAX 4095
#define VOLTAGE_MULTIPLIER 269
#define uS_TO_S_FACTOR 1000000LL  /* Conversion factor for micro seconds to seconds */

// PIN assignments
// Hygro sensors
#define HYG_VCC_PIN 25  // Green
#define HYG_1_PIN 34    // Orange/White
#define HYG_2_PIN 32    // Green/White
#define HYG_3_PIN 35    // Blue/White

// Level
#define BTN_LEVEL_2L 16 // Blue/White
#define BTN_LEVEL_5L 17 // Blue

// DHT22 and DS18B20
#define DHTPIN 5
#define ONE_WIRE_PIN 27 // Brown/White

// Battery voltage
#define BATTERY_PIN 33

// LED
#define LED_ACTION 14

// NTP
constexpr const char* ntpServer = "se.pool.ntp.org";
constexpr const long gmtOffset_sec = 3600;
constexpr const int daylightOffset_sec = 3600;

// State
extern bool hygroActive;
extern float hyg1;
extern float hyg2;
extern float hyg3;
extern float temperature;
extern short humidity;
extern float soilTemperature;
extern short level;

extern short analogVoltage;
extern float voltage;

#endif