#ifndef OUTPUT_H
#define OUTPUT_H

#include <Arduino.h>

#define LED_INTERVAL 500

void ledSetup();
void led(bool on);
void ledBlink(short interval);
void ledProcess();
void serialLog();

#endif