#include <time.h>
#include <esp_sntp.h>
#include "RTClib.h"

#include "time_functions.h"
#include "globals.h"

RTC_DS3231 rtc;
char t[32];
bool rtcReady = false;

int bootCount = 0;
short waterFailSafe = AUTO_WATER_FAIL_SAFE;

void time_sync_notification_cb(struct timeval *tv) {
  Serial.println("Time synchronized from NTP server, setting RTC clock");
  unsigned long unix_epoch = getEpochTime();// + gmtOffset_sec + daylightOffset_sec;  // Get epoch time
  if (rtcReady == true)
    rtc.adjust(DateTime(unix_epoch));
}

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
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
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
 * Get the current weekday day from NTP data
 * @returns short the day or -1
 */
short getWeekDay() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return -1;
  }

  char timeWeekDay[3];
  strftime(timeWeekDay, 3, "%w", &timeinfo);
  if (atoi(timeWeekDay) == 0)
    return 7;
  else
    return atoi(timeWeekDay);
}

// Function that gets current epoch time
unsigned long getEpochTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

/**
 * Setup NTP concfguration
 */
void setupTime() {
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void setupRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    return;
  }

  rtcReady = true;

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    unsigned long unix_epoch = getEpochTime();// + gmtOffset_sec + daylightOffset_sec;  // Get epoch time
    rtc.adjust(DateTime(unix_epoch));  // Set RTC time using NTP epoch time
  }

  DateTime rtcTime = rtc.now();
  struct timeval now = { .tv_sec = (long) rtcTime.unixtime(), .tv_usec = 0}; // &dstm.tm_ds
  settimeofday(&now, NULL);

  Serial.print("ESP: ");
  printLocalTime();
  sprintf(t, "RTC: %02d-%02d-%02d %02d:%02d:%02d", rtcTime.year(), rtcTime.month(), rtcTime.day(), rtcTime.hour(), rtcTime.minute(), rtcTime.second());
  Serial.println(t);
}