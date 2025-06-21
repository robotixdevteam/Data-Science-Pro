const int vibrationSensorPin = 34;
void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  pinMode(vibrationSensorPin, INPUT);
  Serial.println("Vibration sensor test");
}
void loop() {
  int vibrationValue = analogRead(vibrationSensorPin);
  float vibrationMagnitude = map(vibrationValue, 0, 4095, 0, 20);
  Serial.print("Vibration Level (IPS): ");
  Serial.println(vibrationMagnitude);
  delay(100);
}