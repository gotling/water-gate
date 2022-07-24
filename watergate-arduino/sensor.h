#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

#include "globals.h"

// DHT temperature and humidity
#define DHTTYPE DHT22

void primeHygro(bool state);
float hygToPercentage(short value);
void readHygro();
void readTempHum();
void setupSensor();

#endif