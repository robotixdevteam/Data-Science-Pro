#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BluetoothSerial.h>
#include <Wire.h>
#include "MAX30105.h"
#include <FastLED.h>

// OTA SoftAP config
const char* apSSID = "DSPRO-####";
const char* apPassword = "123456789";
IPAddress local_IP(192, 168, #, #);
IPAddress gateway(192, 168, #, #);
IPAddress subnet(255, 255, 255, 0);

// LED for battery status
#define LED_PIN 15
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// MAX30105 sensor setup
MAX30105 particleSensor;

// SpO2 calculation variables
double avered = 0, aveir = 0, sumirrms = 0, sumredrms = 0;
int i = 0, Num = 100;
float ESpO2 = 95.0;
double FSpO2 = 0.7;
double frate = 0.95;
#define TIMETOBOOT 3000
#define SCALE 88.0
#define SAMPLING 100
#define FINGER_ON 30000
#define USEFIFO

// Battery monitoring
#define batteryPin 39
const float R1 = 150.0;
const float R2 = 560.0;
const float MIN_BATTERY_VOLTAGE = 3.7;

bool max30105Initialized = false;
bool initialCheckDone = false;

// Bluetooth
BluetoothSerial SerialBT;

void OTA_Setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

  ArduinoOTA.setPassword("1234");
  ArduinoOTA
    .onStart([]() { Serial.println("Start updating"); })
    .onEnd([]() { Serial.println("\nEnd"); })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress * 100) / total);
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  Serial.println("OTA Ready. AP IP: " + WiFi.softAPIP().toString());
}

void setup() {
  Serial.begin(9600);
  pinMode(batteryPin, INPUT);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear(); FastLED.show();

  OTA_Setup();
  SerialBT.begin("DSPRO-####");

  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    max30105Initialized = true;
    particleSensor.setup(0);  // Turn off LEDs initially
    particleSensor.enableDIETEMPRDY();
    Serial.println("MAX30105 initialized.");
  } else {
    Serial.println("MAX30105 not found.");
  }

  // Recommended MAX30105 config
  particleSensor.setup(0x7F, 4, 2, 200, 411, 16384);
}

void loop() {
  ArduinoOTA.handle();

  float voltage = readVoltage();
  if (voltage < MIN_BATTERY_VOLTAGE) {
    setRed();
    SerialBT.println("Low Battery");
    Serial.println("Low Battery");
    return;
  } else {
    if (!initialCheckDone) {
      setGreen();
      initialCheckDone = true;
      delay(500);
    } else {
      setLavender();
    }
  }

  if (!max30105Initialized) {
    SerialBT.println("Oxygen Level (%): 0.00%");
    return;
  }

  uint32_t ir = 0, red = 0;
  double fred = 0, fir = 0, SpO2 = 0;

#ifdef USEFIFO
  particleSensor.check();

  while (particleSensor.available()) {
    red = particleSensor.getFIFOIR();
    ir = particleSensor.getFIFORed();

    if (ir == 0 || red == 0) {
      SerialBT.println("Sensor not connected or faulty.");
      Serial.println("Sensor not connected or faulty.");
      break;
    }

    i++;
    fred = (double)red;
    fir = (double)ir;

    avered = avered * frate + red * (1.0 - frate);
    aveir = aveir * frate + ir * (1.0 - frate);
    sumredrms += (fred - avered) * (fred - avered);
    sumirrms += (fir - aveir) * (fir - aveir);

    if ((i % SAMPLING) == 0 && millis() > TIMETOBOOT) {
      if (ir < FINGER_ON) {
        SerialBT.println("Oxygen Level (%): 0.00");
        Serial.println("Oxygen Level (%): 0.00");
        break;
      } else {
        SerialBT.print("Oxygen Level (%): ");
        SerialBT.println(ESpO2);
      }
    }

    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      SpO2 = -23.3 * (R - 0.4) + 100;
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;
      sumredrms = 0;
      sumirrms = 0;
      i = 0;
      break;
    }

    particleSensor.nextSample();
  }
#endif
}

float readVoltage() {
  int raw = analogRead(batteryPin);
  float v = raw / 4095.0 * 3.7;
  return v * (R1 + R2) / R2;
}

void setRed() {
  leds[0] = CRGB::Red;
  FastLED.show();
}

void setGreen() {
  leds[0] = CRGB::Green;
  FastLED.show();
}

void setLavender() {
  leds[0] = CRGB::Lavender;
  FastLED.show();
}
