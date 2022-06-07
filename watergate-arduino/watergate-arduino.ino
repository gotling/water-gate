/**
 * Watergate - Automatic watering for grow house
 *
 * FTDebouncer by Ubi de Feo
 * https://github.com/ubidefeo/FTDebouncer
 * DHT sensor library by Adafruit 
 * https://github.com/adafruit/DHT-sensor-library
*/

#include <DHT.h>
#include <FTDebouncer.h>

// Hygro sensors
#define ANALOG_MAX 4095
#define SENSOR_PIN_1 13
#define SENSOR_VCC_1 14
#define SENSOR_DIGITAL_1 12
#define SENSOR_PIN_2 27

// DHT temperature and humidity
#define DHTPIN 22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float temperatureOffset = 0;
int humidityOffset = 0;

// Buttons
FTDebouncer pinDebouncer;
#define BTN_NUT 19
#define BTN_TEST 4

// MOSFET
#define MOSFET_NUT 23

// State
bool hygroActive = false;
short hygro1 = 0;
short hygro2 = 0;
long nutCounter = 0;
float temperature;
short humidity;

// Timers
#define SENSOR_INTERVAL 5000
unsigned long sensorTime = millis();
#define HYGRO_WARMUP_TIME 500
unsigned long hygroTime = millis();

void primeHygro(bool state) {
  hygroActive = state;
  digitalWrite(SENSOR_VCC_1, hygroActive);
}

void readHygro() {
  hygro1 = analogRead(SENSOR_PIN_1);
  hygro2 = analogRead(SENSOR_PIN_2);
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
}

// Button pressed
void onPinActivated(int pinNumber) {
  switch (pinNumber) {
    case BTN_NUT:
      nutCounter++;
      Serial.print("Nut counter: ");
      Serial.println(nutCounter);
      digitalWrite(MOSFET_NUT, LOW);
      break;
    case BTN_TEST:
      Serial.println("More nuts");
      digitalWrite(MOSFET_NUT, HIGH);
      break;
  }    
}

// Button released
void onPinDeactivated(int pinNumber) {
}

void serialLog() {
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" °C");
  Serial.print(" Humidity: ");
  Serial.print(humidity);
  Serial.print(" %");
  Serial.print(" Hygro 1: ");
  Serial.print(hygro1);
  Serial.print(" Hygro 2: ");
  Serial.println(hygro2);
}


void setup() {
  Serial.begin(115200);

  // Init the humidity sensor board
  pinMode(SENSOR_VCC_1, OUTPUT);
  digitalWrite(SENSOR_VCC_1, LOW);
  pinMode(SENSOR_PIN_1, INPUT);
  pinMode(SENSOR_DIGITAL_1, INPUT);
  pinMode(SENSOR_PIN_2, INPUT);

  // MOSFET
  pinMode(MOSFET_NUT, OUTPUT);
  digitalWrite(MOSFET_NUT, LOW);

  // Button
  pinDebouncer.addPin(BTN_NUT, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_TEST, HIGH, INPUT_PULLUP);
  pinDebouncer.begin();

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

    serialLog();
  }
}