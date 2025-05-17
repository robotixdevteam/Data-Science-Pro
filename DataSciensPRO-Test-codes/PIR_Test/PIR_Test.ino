const int pirPin = 36;  
unsigned long startTime = 0;
int piranalogValue = 0;
void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  }
void loop() {
 
   if (digitalRead(pirPin) == HIGH) {
    if (startTime == 0) {
      startTime = millis();  // Record the time when PIR goes HIGH
    }
    unsigned long duration = millis() - startTime;
    piranalogValue = map(duration, 0, 10000, 0, 100);  // Map duration to 0-255 range
    piranalogValue = constrain(piranalogValue, 0, 100 );  // Constrain value to 0-255
  } else {
    startTime = 0;  // Reset the timer when PIR goes LOW
    piranalogValue = 0;  // Set analog value to 0 when no motion is detected
  }
 // Serial.print("motion status: ");
  Serial.println(piranalogValue);  // Print simulated analog value to Serial Monitor
  delay(100);
}