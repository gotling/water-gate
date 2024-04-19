#include "output.h"
#include "globals.h"

short ledInterval = 500;
long ledTime = 0;
bool ledState = LOW;

void ledSetup() {
  pinMode(LED_ACTION, OUTPUT);
  digitalWrite(LED_ACTION, LOW);
  ledTime = millis();
}

void led(bool on) {
  digitalWrite(LED_ACTION, on);
}

void ledBlink(short interval) {
  if (interval <= 0)
    led(false);
  ledInterval = interval;
}

void ledProcess() {
  if ((ledInterval > 0) && (millis() - ledTime >= ledInterval)) {
    ledState = !ledState;
    led(ledState);
    ledTime = millis();
  }
}

void serialLog() {
  Serial.print("Temp: ");
  Serial.print(temperature / readingRound);
  Serial.print(" °C");
  Serial.print(" Humidity: ");
  Serial.print(humidity / readingRound);
  Serial.print(" % ");
  Serial.print("Soil Temp: ");
  Serial.print(soilTemperature / readingRound);
  Serial.print(" °C");
  Serial.print(" Hygro 1: ");
  Serial.print(hyg1 / readingRound);
  Serial.print("% Hygro 2: ");
  Serial.print(hyg2 / readingRound);
  Serial.print("% Hygro 3: ");
  Serial.print(hyg3 / readingRound);
  Serial.print("% Analog voltage: ");
  Serial.print(analogVoltage) / readingRound;
  Serial.print(" Voltage: ");
  Serial.print(voltage / readingRound);
  Serial.print(" Level: ");
  Serial.println(level);
}