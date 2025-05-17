const int flame_PIN = 27;
void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  pinMode(flame_PIN, INPUT);
  Serial.println("flame sensor test");
}
void loop() {
  int infrared_value = analogRead(flame_PIN);
  int flame = map(infrared_value, 0, 4095, 0, 100);
  Serial.print("Flame Proximity Index (no unit): ");
  Serial.println(flame);
}