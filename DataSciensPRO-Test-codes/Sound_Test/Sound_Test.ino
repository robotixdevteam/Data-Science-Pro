const int soundSensorPin = 34;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
}

void loop() {
  int soundValue = analogRead(soundSensorPin);         // Read the analog value from the sound sensor
  int mappedValue = map(soundValue, 0, 1023, 0, 100);  // Map the value from 0-1023 to 0-130
  
  // Display sound level conditionally
  if (mappedValue < 60) {
    Serial.println("Sound Level (db): 0");
  } else {
    Serial.print("Sound Level (db): ");
    Serial.println(mappedValue);
  }
Serial.println(mappedValue);
  delay(100);  // Add a small delay to make the output readable
}
