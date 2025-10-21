#include <Arduino.h>
#define Led_Red D7
#define Led_Green D6
int m1, m2;
int sensor = 0;
void setup() {
  pinMode(D7, OUTPUT);
  pinMode(D6, OUTPUT);
  Serial.begin(9600);
  m1 = millis();
  m2 = millis();
}

void loop() {
  if (millis() > m2 + 500) {
    m2 = millis();
    sensor = map(analogRead(A0), 0, 1024, 0, 100);
    Serial.println(sensor);
  }
  if (millis() > m1 + 2000) {
    m1 = millis();
    if (sensor % 2 == 0) {
      digitalWrite(Led_Green, HIGH);
      digitalWrite(Led_Red, LOW);

    } else {
      digitalWrite(Led_Red, HIGH);
      digitalWrite(Led_Green, LOW);
    }
  } 
  if (millis() > m1 + 1000) {
    digitalWrite(Led_Red, LOW);
    digitalWrite(Led_Green, LOW);
  }
}
