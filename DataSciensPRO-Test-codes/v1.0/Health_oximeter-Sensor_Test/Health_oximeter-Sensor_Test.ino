#include <Wire.h>
#include "MAX30105.h" // SparkFun MAX3010X library

MAX30105 particleSensor;
double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 100;  // Calculate SpO2 by this sampling interval
float ESpO2;   // Initial value of estimated SpO2
double FSpO2 = 0.7;  // Filter factor for estimated SpO2
double frate = 0.95; // Low pass filter for IR/red LED value to eliminate AC component
#define TIMETOBOOT 3000  // Wait for this time (msec) to output SpO2
#define SCALE 88.0  // Adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 100  // If you want to see heart beat more precisely, set SAMPLING to 1
#define FINGER_ON 30000  // If red signal is lower than this, it indicates your finger is not on the sensor
#define USEFIFO

bool max30105Initialized = false;

void setup() {
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  Serial.println();
  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {  // Use default I2C port, 400kHz speed
    max30105Initialized = true;
    // The LEDs are very low power and won't affect the temp reading much but
    // you may want to turn off the LEDs to avoid any local heating
    particleSensor.setup(0);            // Configure sensor. Turn off LEDs
    particleSensor.enableDIETEMPRDY();  // Enable the temp ready interrupt. This is required.
    Serial.println("MAX30105 initialized successfully.");
  } else {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
  }

  // Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x7F;  // Options: 0=Off to 255=50mA
  byte sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2;  // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 200;  // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411;  // Options: 69, 118, 215, 411
  int adcRange = 16384;  // Options: 2048, 4096, 8192, 16384

  // Set up the wanted parameters
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);  // Configure sensor with these settings
  particleSensor.enableDIETEMPRDY();
}

void loop() {
  if (!max30105Initialized) {
    Serial.println("Oxygen Level (%): 0.00%");
    return;  // Skip the rest of the loop if the sensor is not initialized
  }

  uint32_t ir = 0, red = 0, green = 0;
  double fred = 0, fir = 0;
  double SpO2 = 0; // Raw SpO2 before low pass filtered

#ifdef USEFIFO
  particleSensor.check();  // Check the sensor, read up to 3 samples

  while (particleSensor.available()) {  // Do we have new data?
    red = particleSensor.getFIFOIR();  // Read IR value
    ir = particleSensor.getFIFORed();  // Read Red value

    // Handle the case where the sensor is not connected
    if (ir == 0 || red == 0) {
      Serial.println("Sensor not connected or faulty.");
      break;
    }

    i++;
    fred = (double)red;
    fir = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate);  // Average red level by low pass filter
    aveir = aveir * frate + (double)ir * (1.0 - frate);  // Average IR level by low pass filter
    sumredrms += (fred - avered) * (fred - avered);  // Square sum of alternate component of red level
    sumirrms += (fir - aveir) * (fir - aveir);  // Square sum of alternate component of IR level

    if ((i % SAMPLING) == 0) {  // Slow down graph plotting speed for Arduino Serial plotter by thinning out
      if (millis() > TIMETOBOOT) {
        float ir_forGraph = (2.0 * fir - aveir) / aveir * SCALE;
        float red_forGraph = (2.0 * fred - avered) / avered * SCALE;
        // Truncation for Serial plotter's autoscaling
        if (ir_forGraph > 100.0) ir_forGraph = 100.0;
        if (ir_forGraph < 80.0) ir_forGraph = 80.0;
        if (red_forGraph > 100.0) red_forGraph = 100.0;
        if (red_forGraph < 80.0) red_forGraph = 80.0;

        float temperature = particleSensor.readTemperatureF();

        if (ir < FINGER_ON) {  // No finger on the sensor
          Serial.println("Oxygen Level (%): 0.00%");
          break;
        }
        if (ir > FINGER_ON) {
          Serial.print("Oxygen Level (%): ");
          Serial.print(ESpO2);
          Serial.println("%");
        }
      }
    }

    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      SpO2 = -23.3 * (R - 0.4) + 100;
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;  // Low pass filter
      sumredrms = 0.0;
      sumirrms = 0.0;
      i = 0;
      break;
    }
    particleSensor.nextSample();  // Move to next sample
  }
#endif
}
