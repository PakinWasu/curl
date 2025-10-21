#include <ESP8266WiFi.h>
#include "AXWIFI.h"
#include "DHT.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BH1750.h>
#include <WiFiClientSecure.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_RESET -1
// Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
BH1750 lightMeter;
#define ON_Board_LED 2
const char* ssid = "INETHOUSE_2.4G";
const char* password = "2002031998";
const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "AKfycbwh9Yo-vlEjn17iNa-mMNrM6sExNVuHhUG_4YLxVdclKNDaO9roi7row1ETH4MYihqZPA";

long now = millis();
long lastMeasure = 0;
float h = 0;
float t = 0;
float lux = 0;

void setup() {
  Serial.begin(9600);
  delay(500);
  Wire.begin(14, 12);
  ax.begin();
  ax.SledTypeGRB();
  ax.SledShow(0, 0, 0, 0);
  ax.SledShow(1, 0, 0, 0);
  dht.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 started correctly");
  } else {
    Serial.println("Error initializing BH1750");
  }
  WiFi.begin(ssid, password);
  Serial.println("");
  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
  }
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  client.setInsecure();
}
void loop() {


  now = millis();
  if (now - lastMeasure > 600000) {
    lastMeasure = now;
    h = dht.readHumidity();
    t = dht.readTemperature();
    lux = lightMeter.readLightLevel();
    if (isnan(h) || isnan(t) || isnan(lux)) {
      Serial.println(F("Failed to read from DHT & BH1750 sensor!"));
      return;
    }
    Serial.print(F("Temperature Now: "));
    Serial.print(t);
    Serial.print(F(" °C "));
    Serial.print(F("Humidity Now: "));
    Serial.print(h);
    Serial.print(F(" % "));
    Serial.print("Light Level: ");
    Serial.print(lux);
    Serial.println(" lx");
    sendData(t, h, lux);
  }
}

void sendData(float value, float value2, float value3) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);

  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------
  //----------------------------------------Proses dan kirim data
  float string_temp = value;
  float string_humi = value2;
  float string_lux = value3;

  String url = "/macros/s/" + GAS_ID + "/exec?temp=" + string_temp + "&humi=" + string_humi + "&lux=" + string_lux;
  // 2 variables
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");
  Serial.println("request sent");
  //----------------------------------------
  //---------------------------------------
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
}
//===============================================
