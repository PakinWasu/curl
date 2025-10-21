#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
WiFiClient client;
PubSubClient mqtt(client);

const char* WIFI_NAME = "iot"; // Name of the WiFi network
const char* WIFI_PASSWORD = "00000000";

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int DHT_PIN = D4; // Use D2 pin for DHT sensor on ESP8266

DHTesp dhtSensor;

void setup() {
  Serial.begin(9600);
  Serial.println("Hello, ESP8266!");

  WiFi.begin(WIFI_NAME, WIFI_PASSWORD); // Connect to the WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Wifi not connected");
  }
  Serial.println("Wifi connected!");
  Serial.println("Local IP: " + WiFi.localIP().toString()); // Print the local IP address


  mqtt.setServer(mqttServer, mqttPort);
  mqtt.connect("boss-7k"); // Instant with unique name for connection
  Serial.print("Connecting to MQTT ...");
  while (!mqtt.connected()) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("MQTT Connected");

  lcd.init();
  lcd.backlight();

  lcd.setCursor(2, 0);
  lcd.print("Hello Mutiara");
  lcd.setCursor(0, 1);
  lcd.print("Ini essay no 4");

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

void loop() {
  mqtt.loop();
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  Serial.println("Temp: " + String(data.temperature, 2) + "°C");
  Serial.println("Humidity: " + String(data.humidity, 1) + "%");
  Serial.println("---");

  String toPublish = String(data.temperature, 2) + " , " + String(data.humidity, 1);
  mqtt.publish("boss-7k/temp&humi", toPublish.c_str()); // change topic

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(data.temperature, 2) + " C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: " + String(data.humidity, 1) + "%");
  delay(2000); // Wait for a new reading from the sensor (DHT22 has ~0.5Hz sample rate)
}