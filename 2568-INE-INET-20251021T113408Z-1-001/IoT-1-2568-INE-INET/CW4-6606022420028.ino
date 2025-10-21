#define Led_Y D5
int state = 0;
void setup() {
  pinMode(Led_Y, OUTPUT);
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Input 1 to Turn LED on and 0 to off");
}

void loop() {
  if (Serial.available()) {
    state = Serial.parseInt();
    if (state == 1) {
      digitalWrite(Led_Y, HIGH);
      Serial.println("Trun On");
    }
    if (state == 0) {
      digitalWrite(Led_Y, LOW);
      Serial.println("Trun Off");
    }
  }
}
