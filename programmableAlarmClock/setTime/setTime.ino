/*  Programmable Alarm Clock
    Developed by Esteban Rosero
    Electrical Engineering Co-op Student at the University of Alberta
*/
// Use this file to set the RTC module's time to your computer's time.

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
