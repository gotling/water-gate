#include "sensor.h"
#include "globals.h"

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

DHT dht(DHTPIN, DHTTYPE);
float temperatureOffset = 0;
int humidityOffset = 0;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature owSensors(&oneWire);

unsigned long sensorTime = millis() - SENSOR_INTERVAL;
unsigned long hygroTime = millis() - SENSOR_INTERVAL;

/**
 * Turn on power to hygro sensors in preparation for reading their values
 */
void primeHygro(bool state) {
  hygroActive = state;
  digitalWrite(HYG_VCC_PIN, hygroActive);
}

/**
 * Read value from the hygro sensors
 */
void readHygro() {
  hyg1 += hygToPercentage(analogRead(HYG_1_PIN));
  hyg2 += hygToPercentage(analogRead(HYG_2_PIN));
  hyg3 += hygToPercentage(analogRead(HYG_3_PIN));
}

/**
 * Value returned from sensor is analog between 0 (maximum) and 4095 (minimum).
 * @returns float percentage
 */
float hygToPercentage(short value) {
  return (ANALOG_MAX-value)/40.95;
}

/**
 * Read temperature and humidity from DHT sensor
 * Assignes to global variables if valid value
 */
void readTempHum() {
  float newT = dht.readTemperature();
  float newH = dht.readHumidity();
  
  if (!isnan(newT)) {
    temperature += (newT + temperatureOffset) / 100 * 100;
  }
  
  if (!isnan(newH)) {
    humidity += newH + humidityOffset;
  }

  // oneWire sensor
  owSensors.requestTemperatures();
  float newSoilTemperature = owSensors.getTempCByIndex(0);

  if (!isnan(newSoilTemperature) && newSoilTemperature > -127) {
    soilTemperature += newSoilTemperature / 100 * 100;
  }
}

/**
 * Read battery voltage and convert to float
 */
void readVoltage() {
  analogVoltage = analogRead(BATTERY_PIN);
 
  // Max 14.5 = 4096
  voltage += (float)analogVoltage / VOLTAGE_MULTIPLIER;
}

/**
 * Read level sensors. Connected pulled high so values are inverted.
 * Set global variable level or -1 if invalid value is read
 */
void readLevel() {
    short l2 = !digitalRead(BTN_LEVEL_2L);
    short l5 = !digitalRead(BTN_LEVEL_5L);

    if ((l2 == LOW) && (l5 == LOW))
      level = 0;
    else if ((l2 == HIGH) && (l5 == LOW))
      level = 2;
    else if ((l2 == HIGH) && (l5 == HIGH))
      level = 5;
    else
      level = -1; // Should not be possible
}

/**
 * Initialize DHT and OneWire
 */
void setupSensor() {
  pinMode(HYG_VCC_PIN, OUTPUT);
  digitalWrite(HYG_VCC_PIN, LOW);
  pinMode(HYG_1_PIN, INPUT);
  pinMode(HYG_2_PIN, INPUT);
  pinMode(HYG_3_PIN, INPUT);

  pinMode(BATTERY_PIN, INPUT);

  owSensors.begin();
  dht.begin();
}

/**
 * Prepare for reading of sensor data and read sensor data.
 * @returns bool true if data has been read
 */
bool readSensor() {
  // Prepare for hygro reading
  if (millis() - sensorTime >= SENSOR_INTERVAL) {
    primeHygro(true);
    hygroTime = millis();
    sensorTime = millis();
  }

  // Read temperatures and hygro after waiting for warmup
  if (hygroActive && millis() - hygroTime >= HYGRO_WARMUP_TIME) {
    readingRound++;
    if (readingRound > 3) {
      readingRound = 1;
      temperature = humidity = voltage = hyg1 = hyg2 = hyg3 = soilTemperature = 0;
    }

    readTempHum();
    readLevel();

    readHygro();
    primeHygro(false);

    readVoltage();

    return true;
  }

  return false;
}