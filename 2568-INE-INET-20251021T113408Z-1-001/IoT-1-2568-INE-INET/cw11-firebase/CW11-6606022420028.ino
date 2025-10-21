#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>
#include <time.h>

// WiFi configuration
#define WIFI_SSID "B415"
#define WIFI_PASSWORD "appletv415"

// Firebase configuration
#define API_KEY "AIzaSyBHZYKZ_8PSr3W33axZOUc_3d4bDbPn7Ng"
#define DATABASE_URL "fir-dht-8e4f3-default-rtdb.asia-southeast1.firebasedatabase.app"

// DHT sensor configuration
#define DHTPIN D4
#define DHTTYPE DHT22

// Time configuration
int timezone = 7;
char ntp_server1[20] = "ntp.ku.ac.th";
char ntp_server2[20] = "fw.eng.ku.ac.th";
char ntp_server3[20] = "time.uni.net.th";
int dst = 0;

DHT dht(DHTPIN, DHTTYPE);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(9600);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");

  // Set Firebase configurations
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  auth.user.email = "esp8266@test.com";
  auth.user.password = "12345678";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set up time
  configTime(timezone * 3600, dst, ntp_server1, ntp_server2, ntp_server3);
  Serial.println("Waiting for time...");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTime synced!");

  dht.begin();
}

void loop() {
  // Read temperature and humidity from DHT22 sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(500);
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.println();

  // Create a FirebaseJson object and set the data
  FirebaseJson json;
  json.set("temperature", t);
  json.set("humidity", h);
  json.set("time", NowString());

  // Send JSON data to Firebase
  if (Firebase.RTDB.pushJSON(&fbdo, "/logDHT", &json)) { // <-- Here we pass a pointer to FirebaseJson
    Serial.println("Data sent to Firebase successfully");
  } else {
    Serial.printf("Error sending data: %s\n", fbdo.errorReason().c_str());
  }

  delay(120000);  // Delay for 30 seconds before next reading
}

// Function to get the current time as a string
String NowString() {
  time_t now = time(nullptr);
  struct tm* newtime = localtime(&now);

  String tmpNow = "";
  tmpNow += String(newtime->tm_year + 1900);
  tmpNow += "-";
  tmpNow += String(newtime->tm_mon + 1);
  tmpNow += "-";
  tmpNow += String(newtime->tm_mday);
  tmpNow += " ";
  tmpNow += String(newtime->tm_hour);
  tmpNow += ":";
  tmpNow += String(newtime->tm_min);
  tmpNow += ":";
  tmpNow += String(newtime->tm_sec);
  return tmpNow;
}
