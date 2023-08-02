// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_PCF8523 rtc;

void setup () {
  Serial.begin(57600);

  #ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__));
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))+TimeSpan(0,7,0,0));

  rtc.start();

}

void loop () {
    DateTime now = rtc.now();
    Serial.println(now.timestamp());
    delay(1000);
}
