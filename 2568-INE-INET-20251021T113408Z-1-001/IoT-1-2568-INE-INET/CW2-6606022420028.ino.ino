#define Led_R D7
#define Led_G D6
#define Led_Y D5
#define sw1 D4
#define sw2 D0
int count_c1 = 0;
int count_c2 = 0;
int count_c3 = 0;



void setup() {
  pinMode(Led_R, OUTPUT);
  pinMode(Led_G, OUTPUT);
  pinMode(Led_Y, OUTPUT);
  Serial.begin(9600);
  pinMode(sw1, INPUT_PULLUP);
  pinMode(sw2, INPUT_PULLUP);
}

void loop() {
  int s1 = digitalRead(sw1);
  int s2 = digitalRead(sw2);
  if (s1 == 0 and s2 == 0) {
    count_c3 += 1;
    Serial.print("count sw 1+2 = ");

    Serial.println(count_c3);

    if (count_c3 == 7) {
      for (int i = 0; i < 7; i++) {
        digitalWrite(Led_R, LOW);
        digitalWrite(Led_G, LOW);
        digitalWrite(Led_Y, HIGH);
        delay(500);
        digitalWrite(Led_R, LOW);
        digitalWrite(Led_G, LOW);
        digitalWrite(Led_Y, LOW);
        delay(500);
      }
      count_c3 = 0;
    }
  } else {
    if (s1 == 0) {
      count_c1 += 1;
      Serial.print("count sw 1 = ");

      Serial.println(count_c1);

      if (count_c1 == 5) {
        for (int i = 0; i < 5; i++) {
          digitalWrite(Led_R, HIGH);
          digitalWrite(Led_G, LOW);
          digitalWrite(Led_Y, LOW);
          delay(500);
          digitalWrite(Led_R, LOW);
          digitalWrite(Led_G, LOW);
          digitalWrite(Led_Y, LOW);
          delay(500);
        }
        count_c1 = 0;
      }
    }
    if (s2 == 0) {
      count_c2 += 1;
      Serial.print("count sw 2 = ");

      Serial.println(count_c2);

      if (count_c2 == 3) {
        for (int i = 0; i < 3; i++) {
          digitalWrite(Led_R, LOW);
          digitalWrite(Led_G, HIGH);
          digitalWrite(Led_Y, LOW);
          delay(500);
          digitalWrite(Led_R, LOW);
          digitalWrite(Led_G, LOW);
          digitalWrite(Led_Y, LOW);
          delay(500);
        }
        count_c2 = 0;
      }
    }
  }
  delay(200);
  digitalWrite(Led_R, LOW);
  digitalWrite(Led_G, LOW);
  digitalWrite(Led_Y, LOW);
}
