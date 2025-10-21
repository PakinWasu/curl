#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <DHT.h>
#define DHTPIN D5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 OLED(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int trigPin = D6;
const int echoPin = D7;
#define Led_Red D9
#define Led_Green D10
float water = 0;
float WaterLevel = 0;
float t = 0;
float h = 0;
bool temperatureAlertSent = false;
bool humidityAlertSent = false;
bool waterLevelAlertSent = false;
bool coolingFanActivated = false;
bool heatingLampActivated = false;
// Wi-Fi
const char* ssid = "iot";           // ใส่ชื่อ Wi-Fi ของคุณ
const char* password = "00000000";  // ใส่รหัสผ่าน Wi-Fi ของคุณ

// Telegram
#define BOT_TOKEN "8195454313:AAHRB0y7vNwoRAn7r_-lJEH3T-qsQrGyoqA"  // ใส่ Token ของบอท Telegram
#define CHAT_ID "8296570636"                                        // ใส่ Chat ID ของคุณ

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
// Interval settings
unsigned long lastTimeBotRan = 0;
const unsigned long BOT_MTBS = 1000;  // check new messages every 1 sec
unsigned long lastSentTime = 0;
const unsigned long SEND_INTERVAL = 10000;  // send message every 10 sec
// Control variable
bool sendingEnabled = true;

float readDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return NAN;
  return duration * 0.034 / 2.0;
}
void setup() {
  Serial.begin(9600);
  dht.begin();
  pinMode(Led_Red, OUTPUT);
  pinMode(Led_Green, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  WiFi.begin(ssid, password);
  secured_client.setInsecure();  // ignore SSL cert

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if (!OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
  } else {
    Serial.println("ArdinoAll OLED Start Work !!!");
  }
  Serial.println(" Connected!");

  bot.sendMessage(CHAT_ID, "ESP8266 Bot ready", "");
  Serial.println("Bot ready!");
}

void loop() {
  // Check Telegram messages

  if (millis() > lastTimeBotRan + BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    sendWaterLevel();
    TempHumi();
    for (int i = 0; i < numNewMessages; i++) {
      String user_id = String(bot.messages[i].chat_id);
      if (user_id != CHAT_ID) continue;

      String text = bot.messages[i].text;

      if (text == "/ledon") {
        digitalWrite(LED_BUILTIN, LOW);
        bot.sendMessage(CHAT_ID, "LED is ON", "");
        Serial.println("LED turned ON");
      } else if (text == "/ledoff") {
        digitalWrite(LED_BUILTIN, HIGH);
        bot.sendMessage(CHAT_ID, "LED is OFF", "");
        Serial.println("LED turned OFF");
      } else if (text == "/stop") {
        sendingEnabled = false;
        bot.sendMessage(CHAT_ID, "Automatic messages STOPPED", "");
        Serial.println("Automatic messages stopped");
      } else if (text == "/start") {
        sendingEnabled = true;
        bot.sendMessage(CHAT_ID, "Automatic messages STARTED", "");
        Serial.println("Automatic messages started");
      }
    }
    if (t < 25 || t > 30) {
      if (!temperatureAlertSent) {
        bot.sendMessage(CHAT_ID, "Temperature Alert: The temperature is outside the range of 25°C to 30°C.", "");
        temperatureAlertSent = true;
      }
    } else {
      if (temperatureAlertSent) {
        bot.sendMessage(CHAT_ID, "Temperature is back to normal: within the range of 25°C to 30°C.", "");
        temperatureAlertSent = false;
      }
    }

    // เช็คเงื่อนไขความชื้น
    if (h < 60 || h > 75) {
      if (!humidityAlertSent) {
        bot.sendMessage(CHAT_ID, "Humidity Alert: The humidity is outside the range of 60% to 75%.", "");
        humidityAlertSent = true;
      }
    } else {
      if (humidityAlertSent) {
        bot.sendMessage(CHAT_ID, "Humidity is back to normal: within the range of 60% to 75%.", "");
        humidityAlertSent = false;
      }
    }

    // เช็คระดับน้ำ
    if (WaterLevel < 5) {
      if (!waterLevelAlertSent) {
        bot.sendMessage(CHAT_ID, "Water Level Alert: The water level is below 5 cm.", "");
        waterLevelAlertSent = true;
      }
    } else {
      if (waterLevelAlertSent) {
        bot.sendMessage(CHAT_ID, "Water level is back to normal: above 5 cm.", "");
        waterLevelAlertSent = false;
      }
    }

    if (t > 30) {
      if (!coolingFanActivated) {
        digitalWrite(Led_Green, HIGH);
        bot.sendMessage(CHAT_ID, "Cooling Fan Activated: The temperature has exceeded 30°C.", "");
        coolingFanActivated = true;
      }
    } else {
      if (coolingFanActivated) {
        digitalWrite(Led_Green, LOW);
        bot.sendMessage(CHAT_ID, "Cooling Fan Deactivated: The temperature is back to normal.", "");
        coolingFanActivated = false;
      }
    }

    if (t < 25) {
      if (!heatingLampActivated) {
        digitalWrite(Led_Red, HIGH);
        bot.sendMessage(CHAT_ID, "Heating Lamp Activated: The temperature has dropped below 25°C.", "");
        heatingLampActivated = true;
      }
    } else {
      if (heatingLampActivated) {
        digitalWrite(Led_Red, LOW);
        bot.sendMessage(CHAT_ID, "Heating Lamp Deactivated: The temperature is back to normal.", "");
        heatingLampActivated = false;
      }
    }
    OLED.clearDisplay();
    OLED.setTextColor(WHITE, BLACK);
    OLED.setTextSize(1);
    OLED.setCursor(0, 0);
    OLED.print("WaterLevel : ");
    OLED.println(String(WaterLevel, 2));
    OLED.setCursor(0, 15);
    OLED.print("Temp:");
    OLED.print(String(t, 1));
    OLED.print(" Humi:");
    OLED.println(String(h, 1));
    OLED.display();

    lastTimeBotRan = millis();
  }
  // Send message every 10 seconds if enabled
  // if (sendingEnabled && (millis() > lastSentTime + SEND_INTERVAL)) {
  //   if (bot.sendMessage(CHAT_ID, "ESP8266 is running. Sending message every 10 seconds!", "")) {
  //     Serial.println("Message sent successfully!");
  //   } else {
  //     Serial.println("Failed to send message.");
  //   }
  //   lastSentTime = millis();
  // }
}
void sendWaterLevel() {
  water = readDistanceCM();
  WaterLevel = 10 - water;
  if (!isnan(water)) {
    Serial.print("water: ");
    Serial.println(water);
    Serial.print("water Level : ");
    Serial.println(WaterLevel);
  } else {
    Serial.println("water read failed");
  }
}
void TempHumi() {
  h = dht.readHumidity();
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temp: ");
  Serial.println(t);
  Serial.print("Humi: ");
  Serial.println(h);
}