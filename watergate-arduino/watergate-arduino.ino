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

#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#include "config.h"

// Global
#define ANALOG_MAX 4095
#define VOLTAGE_MULTIPLIER 269
#define NUT_TARGET 14

// Battery voltage
#define BATTERY_PIN 33 // Move from 12 to 33

// Hygro sensors
#define HYG_VCC_PIN 25 // Green
#define HYG_1_PIN 34 // Orange/White
#define HYG_2_PIN 32 // Green/White
#define HYG_3_PIN 35 // Blue/White

// DS18B20 temperature probe
#define ONE_WIRE_PIN 27 // Brown/White 33 move to 12 move to 27
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
#define BTN_ACTION 26

// LED
#define LED_ACTION 14

// MOSFET
#define MOSFET_NUT 2 // Green
#define MOSFET_PUMP 13

// Network
WiFiManager wm;
WiFiClientSecure client;
PubSubClient mqtt(client);

#define MQTT_SERVER "io.adafruit.com"
#define MQTT_PORT 8883
// Set the following in config.h
//#define MQTT_USER ""
//#define MQTT_PASSWORD ""
//#define MQTT_TOPIC ""

// Sleep
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  28800  // 8 Hours
#define WAKE_TIME_SHORT 60 // In seconds, when just measuring
#define WAKE_TIME_LONG 600 // In seconds, when watering

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int bootTimerCount = 0;

// State
bool hygroActive = false;
float hyg1 = 0;
float hyg2 = 0;
float hyg3 = 0;
long nutCounter = 0;
float temperature;
short humidity;
float soilTemperature;
short analogVoltage;
float voltage;
bool actionPump = false;
bool actionNut = false;

// Timers
#define SENSOR_INTERVAL 15000
unsigned long sensorTime = millis();
#define HYGRO_WARMUP_TIME 500
unsigned long hygroTime = millis();
unsigned long wakeTime = millis();
#define  MAX_PUMP_TIME 180000 // 3 minutes
unsigned long pumpStartTime = millis();
unsigned long timeToStayAwake;

// Automation
#define AUTO_WATER_X_BOOT 8 // If booting 1 time per hour, set to 8

typedef enum {
  ACTION,
  LED,
  NUT,
  PUMP,
  LEVEL,
  TIMER
} Event;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void check_wakeup_reason(){
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 :
      Serial.println("Wakeup caused by external signal using RTC_IO");
      timeToStayAwake = WAKE_TIME_LONG;
      break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:
      ++bootTimerCount;
      Serial.printf("Wakeup caused by timer: %d\n", bootTimerCount);
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void primeHygro(bool state) {
  hygroActive = state;
  digitalWrite(HYG_VCC_PIN, hygroActive);
}

float hygToPercentage(short value) {
  return (ANALOG_MAX-value)/40.95;
}

void readHygro() {
  hyg1 = hygToPercentage(analogRead(HYG_1_PIN));
  hyg2 = hygToPercentage(analogRead(HYG_2_PIN));
  hyg3 = hygToPercentage(analogRead(HYG_3_PIN));
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

void pump(bool start) {
  if (start) {
    pumpStartTime = millis();
    actionPump = true;
    digitalWrite(MOSFET_PUMP, HIGH);
    mqttSendEvent(PUMP, 1);
  } else {
    actionPump = false;
    digitalWrite(MOSFET_PUMP, LOW);
    mqttSendEvent(PUMP, 0);
  }
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
      mqttSendEvent(NUT, nutCounter);
      break;
    case BTN_TEST:
      Serial.println("More nuts");
      digitalWrite(MOSFET_NUT, HIGH);
      break;
    case BTN_LEVEL_2L:
      Serial.println("Liquid: > 2l");
      // If nutrition should be added, do it now
      if (actionNut) {
        addNutrition();
        actionNut = false;
      }
      mqttSendEvent(LEVEL, 2);
      break;
    case BTN_LEVEL_5L:
      Serial.println("Liquid: > 5l");
      // Stop pump
      pump(false);
      mqttSendEvent(LEVEL, 5);
      break;
    case BTN_ACTION:
      
      // Cancel pump and nutrition
      if (actionNut) {
        Serial.println("Action: Cancel");
        
        pump(false);
        actionNut = false;
        
        digitalWrite(LED_ACTION, LOW);
        
        digitalWrite(MOSFET_NUT, LOW);
        mqttSendEvent(ACTION, 0);
        break;
      }

      // If already pumping
      if (actionPump) {
        Serial.println("Action: Enable nutrition");
        // Add nutrition when water level reached
        actionNut = true;
        digitalWrite(LED_ACTION, HIGH);
        mqttSendEvent(ACTION, -2);
      } else {
        // Enable pump
        Serial.println("Action: Start pump");
        pump(true);
        mqttSendEvent(ACTION, -1);
      }
      break;
  }    
}

// Button released
void onPinDeactivated(int pinNumber) {
  switch (pinNumber) {
     case BTN_LEVEL_2L:
      Serial.println("Liquid: < 2l");
      mqttSendEvent(LEVEL, 0);
      break;
    case BTN_LEVEL_5L:
      Serial.println("Liquid: < 5l");
      mqttSendEvent(LEVEL, 2);
      break;
    case BTN_ACTION:
      mqttSendEvent(BUTTON, 0);
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
  Serial.print("% Hygro 2: ");
  Serial.print(hyg2);
  Serial.print("% Hygro 3: ");
  Serial.print(hyg3);
  Serial.print("% Analog voltage: ");
  Serial.print(analogVoltage);
  Serial.print(" Voltage: ");
  Serial.println(voltage);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Init the humidity sensor board
  pinMode(HYG_VCC_PIN, OUTPUT);
  digitalWrite(HYG_VCC_PIN, LOW);
  pinMode(HYG_1_PIN, INPUT);
  pinMode(HYG_2_PIN, INPUT);
  pinMode(HYG_3_PIN, INPUT);

  // MOSFET
  pinMode(MOSFET_PUMP, OUTPUT);
  digitalWrite(MOSFET_PUMP, LOW);
  pinMode(MOSFET_NUT, OUTPUT);
  digitalWrite(MOSFET_NUT, LOW);

  // Button
  pinDebouncer.addPin(BTN_NUT, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_TEST, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_LEVEL_2L, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_LEVEL_5L, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_ACTION, HIGH, INPUT_PULLUP);
  pinDebouncer.begin();

  // LED
  pinMode(LED_ACTION, OUTPUT);
  digitalWrite(LED_ACTION, LOW);

  // Battery voltage
  pinMode(BATTERY_PIN, INPUT);

  // OneWire
  owSensors.begin();

  // DHT
  dht.begin();

  // Setup WiFiManager
  setupWiFi();

  // Setup Serial
  Serial.println("Ready");

  // Setup deep sleep
  check_wakeup_reason();

  // Wake up on button press
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 0);

  // Auto water every x boot caused by timer
  if ((bootTimerCount > 0) && (bootTimerCount % AUTO_WATER_X_BOOT == 0)) {
    pump(true);
    timeToStayAwake = WAKE_TIME_LONG;
    Serial.printf("Auto water of plants. Staying awake for %d seconds\n", timeToStayAwake);
  } else {
    timeToStayAwake = WAKE_TIME_SHORT;
    Serial.printf("Measurement only. Staying awake for %d seconds\n", timeToStayAwake);
  }

  // Wake up on timer
  esp_sleep_enable_timer_wakeup((TIME_TO_SLEEP - timeToStayAwake) * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for " + String(TIME_TO_SLEEP - timeToStayAwake) + " seconds");
}

void loop() {
  pinDebouncer.update();
  wm.process();
  mqtt.loop();

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
    if (WiFi.status() == WL_CONNECTED)
      mqttSend();
  }

  // Stops pump after 3 minutes if sensors has not triggered yet
  if (actionPump && (millis() - pumpStartTime >= MAX_PUMP_TIME)) {
    Serial.println("Maximum pump time exceeded");
    pump(false);
  }

  // Time to sleep
  // TODO: Check that no action is being performed
  if (millis() - wakeTime >= timeToStayAwake * 1000) {
    Serial.printf("Going to sleep for %d seconds", TIME_TO_SLEEP);
    esp_deep_sleep_start();
  }
}