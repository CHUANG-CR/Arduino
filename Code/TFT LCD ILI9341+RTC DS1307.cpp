#include "RTClib.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

RTC_DS1307 rtc;

#define TFT_DC 5
#define TFT_CS 17
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const uint16_t myImage[5000] PROGMEM = {};
void setup () {
  Serial.begin(115200);
   tft.begin();
   tft.setCursor(50, 0); 
   tft.print("4B3G0118 Chuang-Qi-Ren");
   tft.drawCircle(120, 180, 80, ILI9341_WHITE);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
}

void loop () {
  DateTime now = rtc.now();
  Serial.print("Current time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  tft.setCursor(10, 310);
  tft.fillRect(10, 310, 220, 220, ILI9341_BLACK);
  tft.setTextSize(1); 
  tft.print(" time: ");
  tft.print(now.year(), DEC);
  tft.print('/');
  tft.print(now.month(), DEC);
  tft.print('/');
  tft.print(now.day(), DEC);
  tft.print(" (");
  tft.print(daysOfTheWeek[now.dayOfTheWeek()]);
  tft.print(") ");
  tft.print(now.hour(), DEC);
  tft.print(':');
  tft.print(now.minute(), DEC);
  tft.print(':');
  tft.print(now.second(), DEC);
  tft.println();
Serial.println();
  delay(3000);
}
