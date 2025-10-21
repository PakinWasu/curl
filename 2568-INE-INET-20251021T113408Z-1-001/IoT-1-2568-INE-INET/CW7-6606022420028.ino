#define ledPin D5
#define pirPin D4
int pirValue = 0;
int modetec = 0;
void getPirValue(void);
void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  digitalWrite(ledPin, LOW);
}
void loop() {
  getPirValue();
}



void getPirValue(void) {
  pirValue = digitalRead(pirPin);
  if (pirValue) {
    Serial.println("Motion Detec");
  }
  digitalWrite(ledPin, pirValue);
  Serial.println(pirValue);
  delay(1000);
}