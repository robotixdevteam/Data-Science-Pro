#include <Wire.h>
#include "MAX30105.h"
MAX30105 particleSensor;
bool max30105Initialized = false;
 
void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
  // Initialize MAX30105 sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {  // Use default I2C port, 400kHz speed
    max30105Initialized = true;
    particleSensor.setup(0);            // Configure sensor. Turn off LEDs
    particleSensor.enableDIETEMPRDY();  // Enable the temp ready interrupt. This is required.
  } else {
    Serial.println("MAX30102 was not found. Please check wiring/power.");
  }
}
 
void loop() {
  if (max30105Initialized) {
    float temperatureF = particleSensor.readTemperatureF();
    Serial.print("Temperature (^F): ");
    Serial.print(temperatureF, 4);
  } else {
    Serial.print("Temperature (^F): 0");
  }
  Serial.println();
}
