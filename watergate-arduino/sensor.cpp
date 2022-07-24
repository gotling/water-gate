#include "sensor.h"

#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

DHT dht(DHTPIN, DHTTYPE);
float temperatureOffset = 0;
int humidityOffset = 0;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature owSensors(&oneWire);

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
  hyg1 = hygToPercentage(analogRead(HYG_1_PIN));
  hyg2 = hygToPercentage(analogRead(HYG_2_PIN));
  hyg3 = hygToPercentage(analogRead(HYG_3_PIN));
}

/**
 * Value returned from sensor is analog between 0 (maximum) and 4095 (minimum).
 * (float) percentage
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
    temperature = (newT + temperatureOffset) / 100 * 100;
  }
  
  if (!isnan(newH)) {
    humidity = newH + humidityOffset;
  }

  // oneWire sensor
  owSensors.requestTemperatures();
  float newSoilTemperature = owSensors.getTempCByIndex(0);

  if (!isnan(newSoilTemperature) && newSoilTemperature > -127) {
    soilTemperature = newSoilTemperature / 100 * 100;
  }
}

/**
 * Initialize DHT and OneWire
 */
void setupSensor() {
  owSensors.begin();
  dht.begin();
}