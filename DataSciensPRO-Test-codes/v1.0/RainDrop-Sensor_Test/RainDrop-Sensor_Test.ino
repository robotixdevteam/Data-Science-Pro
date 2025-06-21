#define RainSensorPin 34
void setup() {
  Serial.begin(9600);
  pinMode(RainSensorPin, INPUT);
}

void loop() {
  int RainSensorValue = analogRead(RainSensorPin);
  //Map the sensor reading to the desired range (0 to 100 for percentage)
  int mappedRainDrop = map(RainSensorValue, 0, 4095, 0, 100);
  // Print the waterdrop level
  Serial.print("Rain Quantity (%): ");
  Serial.println(mappedRainDrop);
}
