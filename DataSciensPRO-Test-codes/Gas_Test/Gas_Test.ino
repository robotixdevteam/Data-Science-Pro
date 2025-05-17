#define gasSensorPin 35

void setup() {
  Serial.begin(9600);
  pinMode(gasSensorPin, INPUT);
}

void loop() {
  int gasSensorValue = analogRead(gasSensorPin);
  float gasRatio = map(gasSensorValue, 0, 4095, 0, 100);
    Serial.print("Gas Volume (ppm): ");
    Serial.println(gasRatio);
}
