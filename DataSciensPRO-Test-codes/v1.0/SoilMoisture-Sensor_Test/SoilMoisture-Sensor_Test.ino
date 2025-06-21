#define soilmoisturePin 36
void setup() {
  Serial.begin(9600);
  pinMode(soilmoisturePin, INPUT);
}

void loop() {
  int soilValue= analogRead(soilmoisturePin);
  //Map the sensor reading to the desired range (0 to 100 for percentage)
  int mappedsoilValue = map(soilValue, 0, 4095, 0, 100);
  // Print the soil moisture level
  Serial.print("Soil moisture(%): ");
  Serial.println(mappedsoilValue);
  delay(100);
}
