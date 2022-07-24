#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

#include "globals.h"

// DHT temperature and humidity
#define DHTTYPE DHT22

#define SENSOR_INTERVAL 15000
#define HYGRO_WARMUP_TIME 500

void primeHygro(bool state);
float hygToPercentage(short value);
void readHygro();
void readTempHum();
void readVoltage();
void setupSensor();
bool readSensor();

#endif