/**
 * Watergate - Automatic watering for growhouse
 *
 * FTDebouncer by Ubi de Feo
 * https://github.com/ubidefeo/FTDebouncer
 * DHT sensor library by Adafruit 
 * https://github.com/adafruit/DHT-sensor-library
 * DallasTemperature by Miles Burton
 * https://github.com/milesburton/Arduino-Temperature-Control-Library
 * OneWire by Jim Studt, ...
 * https://www.pjrc.com/teensy/td_libs_OneWire.html
 */

#include <DHT.h>
#include <FTDebouncer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Global
#define ANALOG_MAX 4095
#define VOLTAGE_MULTIPLIER 269
#define NUT_TARGET 14

// Battery voltage
#define BATTERY_PIN 12

// Hygro sensors
#define HYG_VCC_PIN 25 // Green
#define HYG_1_PIN 34 // Orange/White
#define HYG_2_PIN 32 // Green/White
#define HYG_3_PIN 35 // Blue/White

// DS18B20 temperature probe
#define ONE_WIRE_PIN 33 // Brown/White
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature owSensors(&oneWire);

// DHT temperature and humidity
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float temperatureOffset = 0;
int humidityOffset = 0;

// Buttons
FTDebouncer pinDebouncer;
#define BTN_NUT 4 // Green/White
#define BTN_TEST 15 // 
#define BTN_LEVEL_2L 16 // Blue/White
#define BTN_LEVEL_5L 17 // Blue

// MOSFET
#define MOSFET_NUT 2 // Green
#define MOSFET_PUMP 13

// State
bool hygroActive = false;
short hyg1 = 0;
short hyg2 = 0;
short hyg3 = 0;
long nutCounter = 0;
float temperature;
short humidity;
float soilTemperature;
short analogVoltage;
float voltage;

// Timers
#define SENSOR_INTERVAL 5000
unsigned long sensorTime = millis();
#define HYGRO_WARMUP_TIME 500
unsigned long hygroTime = millis();

void primeHygro(bool state) {
  hygroActive = state;
  digitalWrite(HYG_VCC_PIN, hygroActive);
}

void readHygro() {
  hyg1 = analogRead(HYG_1_PIN);
  hyg2 = analogRead(HYG_2_PIN);
  hyg3 = analogRead(HYG_3_PIN);
}

void readVoltage() {
  analogVoltage = analogRead(BATTERY_PIN);
 
  // Max 14.5 = 4096
  voltage = (float)analogVoltage / VOLTAGE_MULTIPLIER;
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

  if (!isnan(newSoilTemperature)) {
    soilTemperature = newSoilTemperature / 100 * 100;
  }
}

void addNutrition() {
  nutCounter = 0;
  digitalWrite(MOSFET_NUT, HIGH);
}

// Button pressed
void onPinActivated(int pinNumber) {
  switch (pinNumber) {
    case BTN_NUT:
      nutCounter++;
      Serial.print("Nut: ");
      Serial.println(nutCounter);
      if (nutCounter >= NUT_TARGET) {
        digitalWrite(MOSFET_NUT, LOW);
        digitalWrite(LED_ACTION, LOW);
        Serial.println("Nut: Target reached");
      }
      break;
    case BTN_TEST:
      Serial.println("More nuts");
      digitalWrite(MOSFET_NUT, HIGH);
      break;
    case BTN_LEVEL_2L:
      Serial.println("Liquid: > 2l");
      // If nutrition should be added, do it now
      break;
    case BTN_LEVEL_5L:
      Serial.println("Liquid: > 5l");
      // Stop pump
      break;
  }    
}

// Button released
void onPinDeactivated(int pinNumber) {
  switch (pinNumber) {
     case BTN_LEVEL_2L:
      Serial.println("Liquid: < 2l");
      break;
    case BTN_LEVEL_5L:
      Serial.println("Liquid: < 5l");
      break;
  }
}

void serialLog() {
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C");
  Serial.print(" Humidity: ");
  Serial.print(humidity);
  Serial.print(" % ");
  Serial.print("Soil Temp: ");
  Serial.print(soilTemperature);
  Serial.print(" °C");
  Serial.print(" Hygro 1: ");
  Serial.print(hyg1);
  Serial.print(" Hygro 2: ");
  Serial.print(hyg2);
  Serial.print(" Hygro 3: ");
  Serial.print(hyg3);
  Serial.print(" Analog voltage: ");
  Serial.print(analogVoltage);
  Serial.print(" Voltage: ");
  Serial.println(voltage);
}


void setup() {
  Serial.begin(115200);

  // Init the humidity sensor board
  pinMode(HYG_VCC_PIN, OUTPUT);
  digitalWrite(HYG_VCC_PIN, LOW);
  pinMode(HYG_1_PIN, INPUT);
  pinMode(HYG_2_PIN, INPUT);
  pinMode(HYG_3_PIN, INPUT);

  // MOSFET
  pinMode(MOSFET_NUT, OUTPUT);
  digitalWrite(MOSFET_NUT, LOW);

  // Button
  pinDebouncer.addPin(BTN_NUT, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_TEST, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_LEVEL_2L, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_LEVEL_5L, HIGH, INPUT_PULLUP);
  pinDebouncer.begin();

  // Battery voltage
  pinMode(BATTERY_PIN, INPUT);

  // OneWire
  owSensors.begin();

  // DHT
  dht.begin();

  // Setup Serial
  Serial.println("Ready");
}

void loop() {
  pinDebouncer.update();

  // Read DHT22 and prepare for hygro reading
  if (millis() - sensorTime >= SENSOR_INTERVAL) {
    readTempHum();
    sensorTime = millis();

    primeHygro(true);
    hygroTime = millis();
  }

  // Read hygro after waiting for warmup
  if (hygroActive && millis() - hygroTime >= HYGRO_WARMUP_TIME) {
    readHygro();
    primeHygro(false);

    readVoltage();
    serialLog();
  }
}