


#define trigPin D1
#define echoPin D2

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.0343


long duration;
float distanceCm;
float distanceInch;
float distanceMeter;

void setup() {

  Serial.begin(9600);        // Starts the serial communication
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = (duration * SOUND_VELOCITY) / 2;
  distanceInch = distanceCm / 2.54;
  distanceMeter = distanceCm / 100;
  // Convert to inches


  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.print(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.print(distanceInch);
  Serial.print("Distance (m): ");
  Serial.println(distanceMeter);




  delay(1000);
}
