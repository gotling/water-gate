#ifndef TIME_FUNCTIONS_H
#define TIME_FUNCTIONS_H

#include <Arduino.h>

// Sleep
#define TIME_TO_SLEEP  3600  // 1 hour
#define WAKE_TIME_SHORT 30 // In seconds, when just measuring
#define WAKE_TIME_LONG 120 // In seconds, when just measuring
#define WAKE_TIME_AFTER_EMPTY 60 // In seconds, when watering
#define AUTO_WATER_FAIL_SAFE 24 // Water if started from timer this many times without watering

RTC_DATA_ATTR extern short waterFailSafe;
RTC_DATA_ATTR extern int bootCount;

bool check_wakeup_reason();
int secondsTillNextHour();
void printLocalTime();
short getHour();
void setupTime();

#endif