// C++ code
//

#define ledRed 9
#define ledGreen 10
int inches = 0;
int cm = 0;
long readUlatrasonicDistance(int triggerPin , int echoPin)
{
  pinMode(triggerPin,OUTPUT);
  digitalWrite(triggerPin,LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin,LOW);
  pinMode(echoPin,INPUT);
  return pulseIn(echoPin,HIGH);

}
void setup()
{
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  Serial.begin(9600);

}

void loop()
{
  cm = 0.01723* readUlatrasonicDistance(7,7);
  inches = (cm/2.54);
  Serial.print(inches);
    Serial.print("in, ");
 Serial.print(cm);
    Serial.println("cm");
  if(cm<50){
    digitalWrite(ledRed,HIGH);
        digitalWrite(ledGreen,LOW);
  }else{
    digitalWrite(ledRed,LOW);
        digitalWrite(ledGreen,HIGH);
  }
  
 

}