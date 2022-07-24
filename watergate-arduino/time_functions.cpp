#include "time_functions.h"
#include "globals.h"

#include <time.h>

int bootCount = 0;
short waterFailSafe = AUTO_WATER_FAIL_SAFE;

/**
 * Handle different wakeup reasons
 * @returns true if wakeup caused by manual input
 */
bool check_wakeup_reason() {
  bootCount++;
  Serial.println("Boot number: " + String(bootCount));

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      return true;
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      configTime(gmtOffset_sec, daylightOffset_sec, NULL);
      waterFailSafe--;
      Serial.printf("Wakeup caused by timer. Water fail-safe: %d\n", waterFailSafe);
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }
  return false;
}

/**
 * Calculate number of seconds till next hour if NTP data available
 * @returns int seconds till next hour or 3600 if unkwnon
 */
int secondsTillNextHour() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time, default to 3600");
    return 3600;
  }

  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);
  int minute = atoi(timeMinute);

  char timeScond[3];
  strftime(timeScond, 3, "%S", &timeinfo);
  int second = atoi(timeScond);

  return (60 - minute) * 60 - second;
}

/**
 * Debug logging of time
 */
void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

/**
 * Get the current hour from NTP data
 * @returns short the hour or -1
 */
short getHour() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return -1;
  }

  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  return atoi(timeHour);
}

/**
 * Setup NTP concfguration
 */
void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}