#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

#define DHTPIN D4  // D4 is GPIO2
#define DHTTYPE DHT22
#define analogPin A0
BH1750 lightMeter;
// WiFi Credentials
const char* WIFI_SSID = "B415";
const char* WIFI_PASSWORD = "appletv415";

// API Key, this will be your unique key to POST your data to your API
const char* PROJECT_API_KEY = "pakinsqldht";

// Domain Name with the API file
const char* HOST_NAME = "pakinlab.atwebpages.com";
const char* POST_API = "/sensordata.php";  // API Path

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);  // แก้ไขตรงนี้ให้ถูกต้องแล้ว

void setup() {
  Serial.begin(115200);
  dht.begin();
  Wire.begin(14, 12);
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 started correctly");
  } else {
    Serial.println("Error initializing BH1750");
  }
  // Connect to WiFi
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  float resis = analogRead(analogPin);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float lux = lightMeter.readLightLevel();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.println("--------------------------------------------");
  Serial.print("Temperature: ");
  Serial.println(t);
  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("Resistance: ");
  Serial.println(resis);
  Serial.print("Light Level: ");
  Serial.println(lux);
  upload_data(t, h, resis, lux);
  delay(30000);
}

void upload_data(float t, float h, float resis, float lux) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    String serverPath = "http://" + String(HOST_NAME) + String(POST_API);
    HTTPClient http;
    http.begin(client, serverPath);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "api_key=" + String(PROJECT_API_KEY) + "&temperature=" + String(t) + "&humidity=" + String(h) + "&resis=" + String(resis) + "&lux=" + String(lux);
    Serial.print("Sending POST data: ");
    Serial.println(httpRequestData);
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}