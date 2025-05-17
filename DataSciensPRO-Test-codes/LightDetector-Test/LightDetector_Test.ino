#define LDR_PIN 36
void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);
}
void loop() {
  // Read the raw analog value from the LDR (0 - 4095 for ESP32)
  int ldrValue = analogRead(LDR_PIN);
  // Convert the raw LDR value to lux
  float lux = ldrValue * (600.0 / 4095.0);
  // Invert the lux value to correct the behavior
  float invertedLux = max(0.0, 140.0 - lux);
 
  if (invertedLux == 140.0){
    invertedLux = 0.00;
  }
  // Print the corrected light intensity in lux
  Serial.print("Light Intensity (lx): ");
  Serial.println(invertedLux);
}
 