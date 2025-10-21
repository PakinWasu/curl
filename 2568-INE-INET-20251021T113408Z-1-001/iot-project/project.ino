
  Smart Aquarium (ESP8266) - Firebase_ESP_Client (No TG commands)
  Sensors DHT22, BH1750, YF-S201 (Flow)
  Actuators Servo 360° (D5), Relay Pump (D6)
  Features
    - Auto Feed at 0600
    - Telegram ALERTS ONLY (no commands)
    - Firebase RTDB status (10s), history (10m), alertslogs
    - Web control via commands (feed  pump_on  pump_off)
    - Serial sensor log every 2s
 

#include ESP8266WiFi.h
#include WiFiClientSecureBearSSL.h
#include WiFiUdp.h
#include NTPClient.h
#include ArduinoJson.h
#include Servo.h
#include DHT.h
#include BH1750.h
#include Wire.h

 ==== Firebase ====
#include Firebase_ESP_Client.h
#include addonsTokenHelper.h
#include addonsRTDBHelper.h

 ========= USER CONFIG =========
#define WIFI_SSID  B415
#define WIFI_PASS  appletv415

 Telegram (ALERTS ONLY)
#define BOT_TOKEN  8195454313AAHRB0y7vNwoRAn7r_-lJEH3T-qsQrGyoqA
#define CHAT_ID    8296570636

 Firebase
#define API_KEY      AIzaSyCetfPmaEU7rS8xZZa5rzYFeRCirBwoUxw
#define DATABASE_URL httpsiot-project-90164-default-rtdb.asia-southeast1.firebasedatabase.app

 Timezone Bangkok
const long GMT_OFFSET_SECONDS = 7L  3600;

 Firebase Paths
const String FB_PATH_STATUS  = status;
const String FB_PATH_HISTORY = history;
const String FB_PATH_ALERTS  = alertslogs;

 ========= PIN MAP =========
 I2C D2 = SDA (GPIO4), D1 = SCL (GPIO5)
#define PIN_DHT    D4
#define DHTTYPE    DHT22
#define PIN_SERVO  D5
#define PIN_RELAY  D6
#define PIN_FLOW   D7

 ======== RELAY LOGIC (false = Active-LOW modules) ========
const bool RELAY_ACTIVE_HIGH = false;

 ========= SERVO CONFIG =========
Servo feederServo;
int NEUTRAL_US = 1500, TRIM = 0, SPEED_OFFSET = 240, T90_MS = 640;
int neutral() { return NEUTRAL_US + TRIM; }
void servoStop() { feederServo.writeMicroseconds(neutral()); Serial.println(F([SERVO] Stop)); }
void servoCW()   { feederServo.writeMicroseconds(neutral() + SPEED_OFFSET); Serial.println(F([SERVO] Spin CW)); }
void servoCCW()  { feederServo.writeMicroseconds(neutral() - SPEED_OFFSET); Serial.println(F([SERVO] Spin CCW)); }
void servoSpinFor(bool cw, unsigned long ms) { if (cw) servoCW(); else servoCCW(); delay(ms); servoStop(); }
void feedOnce()  { Serial.println(F([FEED] Feeding fish...)); servoSpinFor(true, T90_MS); delay(150); servoSpinFor(false, T90_MS); delay(80); Serial.println(F([FEED] Done feeding.)); }

 ========= OBJECTS =========
DHT dht(PIN_DHT, DHTTYPE);
BH1750 lightMeter;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, pool.ntp.org, GMT_OFFSET_SECONDS, 60  1000);

 Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

 ========= APP CONFIG =========
const unsigned long STATUS_INTERVAL_MS  = 10UL  1000UL;
const unsigned long HISTORY_INTERVAL_MS = 10UL  60UL  1000UL;

 Serial log throttle
const unsigned long SENSOR_LOG_MS = 2000;
unsigned long lastSensorLog = 0;

 Flow sensor
volatile unsigned long flowPulses = 0;
void IRAM_ATTR flowISR() { flowPulses++; }
const unsigned long FLOW_SAMPLE_MS = 1000;
unsigned long lastFlowSample = 0;
float lastFlowLpm = 0.0;

 Alerts
float TEMP_HIGH_C    = 35.0;
float HUMID_LOW_PCT  = 60.0;
float LUX_DARK_TH    = 50.0;
unsigned long NO_FLOW_TIMEOUT_MS = 10UL  1000UL;

bool alertHighTemp = false, alertLowHumid = false, alertDark = false, alertNoFlow = false;
unsigned long lastStatusAt = 0, lastHistoryAt = 0;

 Web commands polling
unsigned long lastCmdPoll = 0;
const unsigned long CMD_POLL_MS = 1000;
unsigned long lastCmdHandledTs = 0;

 ---------- SENSOR DATA ----------
struct Readings { float tC; float h; float lux; float flowLpm; bool isDark; };
Readings nowR;

 ---------- Reusable Firebase JSON ----------
FirebaseJson jStatus, jHistory, jLog;

 ---------- Debug Memory Helper (optional) ----------
void printMem(const char tag) {
  Serial.printf([MEM] %s free=%u, maxBlock=%u, frag=%u%%n,
    tag, ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(), ESP.getHeapFragmentation());
}

 ---------- UTIL ----------
String iso8601Now() {
  time_t raw = timeClient.getEpochTime();
  struct tm ti = gmtime(&raw);
  char buf[25]; strftime(buf, sizeof(buf), %Y-%m-%dT%H%M%S, ti);
  return String(buf);
}
void setRelay(bool on) {
  bool level = RELAY_ACTIVE_HIGH  on  !on;
  digitalWrite(PIN_RELAY, level  HIGH  LOW);
  Serial.printf([PUMP] %sn, on  ON  OFF);
}

 ---------- Telegram ALERT sender (no polling) ----------
bool sendTelegramAlert(const String &textPlain) {
  stdunique_ptrBearSSLWiFiClientSecure c(new BearSSLWiFiClientSecure());
  c-setInsecure();
  c-setBufferSizes(768, 384);   เล็กเพื่อประหยัดแรม (เฉพาะ TG)
  c-setTimeout(15000);

  const char host = api.telegram.org;
  if (!c-connect(host, 443)) {
    Serial.println(F([TG] connect FAIL));
    return false;
  }

  String payload = chat_id= + String(CHAT_ID) + &text= + textPlain + &parse_mode=;

  String req;
  req.reserve(256 + payload.length());
  req += F(POST bot); req += BOT_TOKEN; req += F(sendMessage HTTP1.1rn);
  req += F(Host api.telegram.orgrn);
  req += F(Content-Type applicationx-www-form-urlencodedrn);
  req += F(Connection closern);
  req += F(Content-Length ); req += String(payload.length()); req += F(rnrn);
  req += payload;

  c-print(req);

  unsigned long t0 = millis();
  while (c-connected() && millis() - t0  2000) {
    while (c-available()) { (void)c-read(); }
    delay(1);
  }
  c-stop();
  Serial.println(F([TG] alert sent));
  return true;
}

 ---------- Firebase writes ----------
void pushStatusToFirebase() {
  if (!Firebase.ready()) return;
  jStatus.clear();
  jStatus.set(tempC, nowR.tC);
  jStatus.set(humid, nowR.h);
  jStatus.set(lux, nowR.lux);
  jStatus.set(flowLpm, nowR.flowLpm);
  jStatus.set(pumpOn, digitalRead(PIN_RELAY) == (RELAY_ACTIVE_HIGH  HIGH  LOW));
  jStatus.set(dark, nowR.isDark);
  jStatus.set(updatedAt, iso8601Now());
  if (!Firebase.RTDB.setJSON(&fbdo, FB_PATH_STATUS.c_str(), &jStatus)) {
    Serial.printf([FIREBASE] status ERR %sn, fbdo.errorReason().c_str());
  } else Serial.println(F([FIREBASE] Updated status));
}

void pushHistoryToFirebase() {
  if (!Firebase.ready()) return;
  String path = FB_PATH_HISTORY +  + String(timeClient.getEpochTime());
  jHistory.clear();
  jHistory.set(tempC, nowR.tC);
  jHistory.set(humid, nowR.h);
  jHistory.set(lux, nowR.lux);
  jHistory.set(flowLpm, nowR.flowLpm);
  jHistory.set(pumpOn, digitalRead(PIN_RELAY) == (RELAY_ACTIVE_HIGH  HIGH  LOW));
  jHistory.set(dark, nowR.isDark);
  jHistory.set(ts, iso8601Now());
  if (!Firebase.RTDB.setJSON(&fbdo, path.c_str(), &jHistory)) {
    Serial.printf([FIREBASE] history ERR %sn, fbdo.errorReason().c_str());
  } else Serial.println(F([FIREBASE] Added history entry));
}

void logAlertToFirebase(const String &message) {
  if (!Firebase.ready()) return;
  String path = FB_PATH_ALERTS +  + String(timeClient.getEpochTime());
  jLog.clear(); jLog.set(message, message); jLog.set(ts, iso8601Now());
  if (!Firebase.RTDB.setJSON(&fbdo, path.c_str(), &jLog)) {
    Serial.printf([FIREBASE][LOG][ERR] %sn, fbdo.errorReason().c_str());
  } else Serial.println([FIREBASE][LOG]  + message);
}

 ---------- Alerts  Logic ----------
void notifyOnce(bool &flag, bool condition, const String &enterMsg, const String &clearMsg) {
  if (condition && !flag) { flag = true;  sendTelegramAlert(enterMsg); logAlertToFirebase(enterMsg); }
  else if (!condition && flag) { flag = false; if (clearMsg.length()) { sendTelegramAlert(clearMsg); logAlertToFirebase(clearMsg); } }
}

void readSensors() {
   DHT — ถ้ายังได้ 0.0 ให้เช็คสายไฟเลี้ยงพินและลอง DHT22 ให้ตรงรุ่น
  float h  = dht.readHumidity();
  float tC = dht.readTemperature();
  if (!isnan(h))  nowR.h  = h;
  if (!isnan(tC)) nowR.tC = tC;

   BH1750
  float lx = lightMeter.readLightLevel();
  if (!isnan(lx) && lx = 0.0f && lx  200000.0f) {
    nowR.lux = lx;
  } else {
    static unsigned long lastBhRetry = 0;
    if (millis() - lastBhRetry  2000) {
      lastBhRetry = millis();
      lightMeter.configure(BH1750CONTINUOUS_HIGH_RES_MODE);
      Serial.println(F([BH1750] re-configured after read error));
    }
  }

   Flow
  unsigned long ms = millis();
  if (ms - lastFlowSample = FLOW_SAMPLE_MS) {
    noInterrupts(); unsigned long pulses = flowPulses; flowPulses = 0; interrupts();
    lastFlowSample = ms;
    float hz = (float)pulses  (FLOW_SAMPLE_MS  1000.0f);
    lastFlowLpm = hz  7.5f;
  }
  nowR.flowLpm = lastFlowLpm;
  nowR.isDark = (nowR.lux = 0 && nowR.lux  LUX_DARK_TH);

  if (ms - lastSensorLog = SENSOR_LOG_MS) {
    lastSensorLog = ms;
    Serial.printf([SENSOR] T=%.1fC H=%.1f%% Lux=%.1f Flow=%.2fLmin Dark=%sn,
                  nowR.tC, nowR.h, nowR.lux, nowR.flowLpm, nowR.isDark  Yes  No);
  }
}

void checkAlertsAndActions() {
  if (!isnan(nowR.tC))
    notifyOnce(alertHighTemp, nowR.tC  TEMP_HIGH_C,
               🔥 Temp High  + String(nowR.tC, 1) + °C (  + String(TEMP_HIGH_C, 0) + °C),
               ✅ Temp Normal ( + String(nowR.tC, 1) + °C));

  if (!isnan(nowR.h))
    notifyOnce(alertLowHumid, nowR.h  HUMID_LOW_PCT,
               🌫 Humidity Low  + String(nowR.h, 1) + % (  + String(HUMID_LOW_PCT, 0) + %),
               ✅ Humidity Normal ( + String(nowR.h, 1) + %));

  notifyOnce(alertDark, nowR.isDark,
             🌙 It's Dark (Lux= + String(nowR.lux, 1) + ),
             💡 Light Detected (Lux= + String(nowR.lux, 1) + ));

  bool pumpOn = (digitalRead(PIN_RELAY) == (RELAY_ACTIVE_HIGH  HIGH  LOW));
  static unsigned long lastNoFlowCheck = 0;
  if (pumpOn) {
    if (nowR.flowLpm  0.05f) {
      if (lastNoFlowCheck == 0) lastNoFlowCheck = millis();
      if (millis() - lastNoFlowCheck = NO_FLOW_TIMEOUT_MS && !alertNoFlow) {
        setRelay(false);
        alertNoFlow = true;
        String msg = ⛔ No water flow while pump ON - Pump OFF;
        sendTelegramAlert(msg);
        logAlertToFirebase(msg);
      }
    } else {
      lastNoFlowCheck = 0;
      if (alertNoFlow) {
        alertNoFlow = false;
        String msg = ✅ Water flow normal;
        sendTelegramAlert(msg);
        logAlertToFirebase(msg);
      }
    }
  } else {
    lastNoFlowCheck = 0;
  }

   Auto-feed 0600
  time_t epoch = timeClient.getEpochTime();
  struct tm lt = localtime(&epoch);
  int y = lt-tm_year + 1900, m = lt-tm_mon + 1, d = lt-tm_mday, hh = lt-tm_hour, mm = lt-tm_min;
  static int lastAutoFeedY = -1, lastAutoFeedM = -1, lastAutoFeedD = -1;
  if (hh == 6 && mm == 0) {
    if (!(lastAutoFeedY == y && lastAutoFeedM == m && lastAutoFeedD == d)) {
      sendTelegramAlert(⏰ 0600 Auto Feeding);
      feedOnce();
      lastAutoFeedY = y; lastAutoFeedM = m; lastAutoFeedD = d;
      logAlertToFirebase(Auto feed at 0600);
      delay(800);
    }
  }
}

 ---------- Commands from Firebase (commands) ----------
void pollCommandsFromFirebase() {
  if (millis() - lastCmdPoll  CMD_POLL_MS) return;
  lastCmdPoll = millis();
  if (!Firebase.ready()) return;

   1) commandslatest
  if (Firebase.RTDB.getJSON(&fbdo, commandslatest)) {
    FirebaseJson json = fbdo.toFirebaseJson();
    FirebaseJsonData jd;
    unsigned long ts = 0; String type;

    json.get(jd, ts);   if (jd.success) ts = (unsigned long) jd.toint();
    json.get(jd, type); if (jd.success) type = jd.toString();

    if (ts  0 && type.length() && ts  lastCmdHandledTs) {
      Serial.printf([CMD][FB] latest %s (ts=%lu)n, type.c_str(), ts);
      if (type == feed) { sendTelegramAlert(🍽 Feed (from web)); feedOnce(); }
      else if (type == pump_on)  { setRelay(true);  sendTelegramAlert(🔌 Pump ON (from web)); }
      else if (type == pump_off) { setRelay(false); sendTelegramAlert(🔌 Pump OFF (from web)); }
      Firebase.RTDB.setString(&fbdo, (String(commandsack) + ts).c_str(), ok);
      lastCmdHandledTs = ts;
    }
  }

   2) sweep commands กันตกหล่น
  if (Firebase.RTDB.getJSON(&fbdo, commands)) {
    FirebaseJson all = fbdo.toFirebaseJson();
    int valueType; String key, value;

    size_t n = all.iteratorBegin();
    for (size_t i = 0; i  n; i++) {
      all.iteratorGet(i, valueType, key, value);
      if (key == latest  key == ack) continue;

      unsigned long ts = strtoul(key.c_str(), nullptr, 10);
      if (ts == 0  ts = lastCmdHandledTs) continue;

      FirebaseJson one; one.setJsonData(value);
      FirebaseJsonData jd; String type;
      one.get(jd, type); if (!jd.success) continue;
      type = jd.toString();

      Serial.printf([CMD][FB] queue %s (ts=%lu)n, type.c_str(), ts);
      if (type == feed) { sendTelegramAlert(🍽 Feed (from web)); feedOnce(); }
      else if (type == pump_on)  { setRelay(true);  sendTelegramAlert(🔌 Pump ON (from web)); }
      else if (type == pump_off) { setRelay(false); sendTelegramAlert(🔌 Pump OFF (from web)); }

      Firebase.RTDB.setString(&fbdo, (String(commandsack) + ts).c_str(), ok);
      lastCmdHandledTs = ts;
      yield();
    }
    all.iteratorEnd();
  }
}

 ---------- I2C scan (optional) ----------
void scanI2C() {
  byte err, addr; int n = 0;
  Serial.println(F([I2C] Scanning...));
  for (addr = 1; addr  127; addr++) {
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
    if (err == 0) { Serial.printf([I2C] Found 0x%02Xn, addr); n++; }
  }
  if (n == 0) Serial.println(F([I2C] No devices found));
}

 ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F(n[BOOT] Smart Aquarium (Firebase_ESP_Client, TG alerts only)));

  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, (RELAY_ACTIVE_HIGH  LOW  HIGH));  OFF
  feederServo.attach(PIN_SERVO);
  servoStop();

  dht.begin();
  Wire.begin(D2, D1);
  Wire.setClock(100000);
  scanI2C();

   BH1750 try 0x23 then 0x5C
  bool bhOK = false;
  for (uint8_t addr  {0x23, 0x5C}) {
    if (lightMeter.begin(BH1750CONTINUOUS_HIGH_RES_MODE, addr)) {
      Serial.printf([BH1750] init OK @0x%02Xn, addr);
      bhOK = true; break;
    } else {
      Serial.printf([BH1750] init FAIL @0x%02Xn, addr);
    }
  }
  if (!bhOK) Serial.println(F([BH1750] ERROR device not responding — check wiringADDR));

  pinMode(PIN_FLOW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_FLOW), flowISR, FALLING);

   WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print([WiFi] Connecting);
  while (WiFi.status() != WL_CONNECTED) { Serial.print(.); delay(400); yield(); }
  Serial.println(n[WiFi] Connected  + WiFi.localIP().toString());

   NTP
  timeClient.begin();
  timeClient.update();

   Firebase config
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

   Anonymous sign-in
  Serial.println(F([Firebase] Signing up anonymously...));
  if (Firebase.signUp(&config, &auth, , )) { Serial.println(F([Firebase] Anonymous sign-up OK)); signupOK = true; }
  else { Serial.printf([Firebase] Sign-up FAILED %sn, config.signer.signupError.message.c_str()); }

  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);

   ----- IMPORTANT Bigger TLS buffers for Firebase (fix BearSSL errors) -----
  fbdo.setBSSLBufferSize(4096, 1024);    ↑↑ เพิ่มเพื่อความเสถียร
  fbdo.setResponseSize(1024);
  config.cert.data = nullptr;            insecure (ลด RAM)

   First read & write
  readSensors();
  pushStatusToFirebase();
  pushHistoryToFirebase();

   แจ้งเตือนบูต (สั้น)
  sendTelegramAlert(🟢 Smart Aquarium Online);

  printMem(BOOT);
}

 ---------- LOOP ----------
void loop() {
  static unsigned long lastNtp = 0;
  if (millis() - lastNtp  60UL  1000UL) { timeClient.update(); lastNtp = millis(); }

  readSensors();
  checkAlertsAndActions();

  if (millis() - lastStatusAt  = STATUS_INTERVAL_MS)   { lastStatusAt  = millis(); pushStatusToFirebase(); }
  if (millis() - lastHistoryAt = HISTORY_INTERVAL_MS)  { lastHistoryAt = millis(); pushHistoryToFirebase(); }

  pollCommandsFromFirebase();  คำสั่งจากเว็บเท่านั้น

  delay(2);
}
