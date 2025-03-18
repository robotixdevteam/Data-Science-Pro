#define sensorPin1 18
#define sensorPin2 23  
 
const float dist = 0.3;
bool sensor1State = false;
bool sensor2State = false;
unsigned long interval = 0;
unsigned long startTime = 0;
double speed = 0;
const int timeout = 3000;
 
void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
 
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
}
 
void loop() {
  sensor1State = digitalRead(sensorPin1);
  sensor2State = digitalRead(sensorPin2);
  startTime = 0;
 
  if (sensor1State == 0) {
    startTime = millis();
    while (digitalRead(sensorPin2)) {
      if (millis() - startTime > timeout) {
        Serial.println("Timeout");
        return;
      }
      delay(3);
    }
 
    interval = millis() - startTime;
    if (interval > 3) {
      speed = ((dist) / (interval / 1000.0)) * (18 / 5.0);
      Serial.print("Speed Limit (kmph): ");
      Serial.print(speed);
    }
  }
  Serial.println();
}
