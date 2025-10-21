#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
WiFiClient client;
PubSubClient mqtt(client);

const char* WIFI_NAME = "iot";
const char* WIFI_PASSWORD = "00000000";

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int DHT_PIN = D4; // DHT sensor
const int R = D5;
const int G = D6;
const int Y = D7;

DHTesp dhtSensor;

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("Message arrived [" + String(topic) + "]: " + message);

  // ตัวอย่างการควบคุมไฟ
  if (String(topic) == "boss-7k/led") {
    if (message == "R_ON") digitalWrite(R, HIGH);
    else if (message == "R_OFF") digitalWrite(R, LOW);

    else if (message == "G_ON") digitalWrite(G, HIGH);
    else if (message == "G_OFF") digitalWrite(G, LOW);

    else if (message == "Y_ON") digitalWrite(Y, HIGH);
    else if (message == "Y_OFF") digitalWrite(Y, LOW);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Hello, ESP8266!");

  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(Y, OUTPUT);

  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Wifi not connected");
  }
  Serial.println("Wifi connected!");
  Serial.println("Local IP: " + WiFi.localIP().toString());

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(callback);

  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT ...");
    if (mqtt.connect("boss-7k")) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(mqtt.state());
      delay(2000);
    }
  }

  // Subscribe to LED control topic
  mqtt.subscribe("boss-7k/led");

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
  mqtt.publish("boss-7k/temp&humi", toPublish.c_str());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(data.temperature, 2) + " C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: " + String(data.humidity, 1) + "%");

  delay(2000);
}
