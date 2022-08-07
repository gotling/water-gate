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
    Serial.printf("Blink led: %b\n", ledState);
    ledState != ledState;
    led(ledState);
    ledTime = millis();
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
  Serial.print(voltage);
  Serial.print(" Level: ");
  Serial.println(level);
}