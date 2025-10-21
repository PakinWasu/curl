#include "DHT.h"
#define Led_R D7
#define Led_G D6
#define Led_Y D5
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
int tem_prv = 0;
void setup() {
  pinMode(Led_R, OUTPUT);
  pinMode(Led_G, OUTPUT);
  pinMode(Led_Y, OUTPUT);
  Serial.begin(9600);
  dht.begin();
  int tem_prv = dht.readTemperature();
}

void loop() {
  delay(2000);
  float h = dht.readHumidity();
  int t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Temperature Now: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(F("Previous Temperature : "));
  Serial.print(tem_prv);
  Serial.println(F("°C "));
  if (t > tem_prv) {
    digitalWrite(Led_R, HIGH);
    delay(500);
    digitalWrite(Led_Y, HIGH);
    delay(500);
    digitalWrite(Led_G, HIGH);
    delay(1000);
    tem_prv = t;
  }
  if (t < tem_prv) {
    digitalWrite(Led_G, HIGH);
    delay(500);
    digitalWrite(Led_Y, HIGH);
    delay(500);
    digitalWrite(Led_R, HIGH);
    delay(1000);
    tem_prv = t;
  }
  digitalWrite(Led_R, LOW);
  digitalWrite(Led_G, LOW);
  digitalWrite(Led_Y, LOW);


  // float hif = dht.computeHeatIndex(f, h);
  // float hic = dht.computeHeatIndex(t, h, false);



  // Serial.print(F("Humidity: "));
  // Serial.print(h);

  // Serial.print(f);
  // Serial.print(F("°F  Heat index: "));
  // Serial.print(hic);
  // Serial.print(F("°C "));
  // Serial.print(hif);
  // Serial.println(F("°F"));
}