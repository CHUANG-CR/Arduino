#include <Arduino.h>

#define SPEAKER_PIN 16
#define PIN_TRIG 18
#define PIN_ECHO 5

void setup(){
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(PIN_TRIG, LOW);
}

void loop(){
  // send trigger pulse
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH);
  Serial.println(duration);
  Serial.println("Distance in CM:");
  Serial.println(duration / 58.0);
  Serial.println("Distance in inches:");
  Serial.println(duration / 148.0);

  unsigned long buzzDelay = duration / 100;
  if (buzzDelay < 1) buzzDelay = 1;

  tone(SPEAKER_PIN, 2000);
  delay(buzzDelay);
  noTone(SPEAKER_PIN);
  delay(buzzDelay);
}
