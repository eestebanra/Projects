// Use this file to adjust the RTC module's time to your computer's time.
#include "RTClib.h"
RTC_DS1307 rtc;
void setup () {
  #ifndef ESP8266
    while (!Serial);
  #endif
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}
void loop () {
}
