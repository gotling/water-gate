#ifndef GLOBALS_H
#define GLOBALS_H

#define ANALOG_MAX 4095
#define VOLTAGE_MULTIPLIER 269

// PIN assignments
// Hygro sensors
#define HYG_VCC_PIN 25  // Green
#define HYG_1_PIN 34    // Orange/White
#define HYG_2_PIN 32    // Green/White
#define HYG_3_PIN 35    // Blue/White

// DHT22 and DS18B20
#define DHTPIN 5
#define ONE_WIRE_PIN 27 // Brown/White

// Battery voltage
#define BATTERY_PIN 33

// State
extern bool hygroActive;
extern float hyg1;
extern float hyg2;
extern float hyg3;
extern float temperature;
extern short humidity;
extern float soilTemperature;

extern short analogVoltage;
extern float voltage;

#endif