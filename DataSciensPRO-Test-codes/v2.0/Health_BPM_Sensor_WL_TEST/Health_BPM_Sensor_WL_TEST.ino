#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BluetoothSerial.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <FastLED.h>

// OTA AP Credentials
const char* apSSID = "DSPRO-####";
const char* apPassword = "123456789";
IPAddress local_IP(192, 168, #, #);
IPAddress gateway(192, 168, #, #);
IPAddress subnet(255, 255, 255, 0);

// LED settings
#define LED_PIN 15
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// Battery monitoring
#define batteryPin 39
const float R1 = 150.0;
const float R2 = 560.0;
const float MIN_BATTERY_VOLTAGE = 3.7;

// Heart rate vars
MAX30105 particleSensor;
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

// Flags
bool initialCheckDone = false;
bool max30105Initialized = false;

// Bluetooth
BluetoothSerial SerialBT;

void OTA_Setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

  ArduinoOTA.setPassword("1234");
  ArduinoOTA
    .onStart([]() { Serial.println("Start updating"); })
    .onEnd([]() { Serial.println("OTA Update Complete"); })
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

  // Initialize MAX30105
  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    max30105Initialized = true;
    Serial.println("MAX30105 initialized. Place your finger.");
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
  } else {
    Serial.println("MAX30105 not found. Check wiring.");
  }
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
      setDeepPink();
    }
  }

  if (!max30105Initialized) {
    SerialBT.println("BPM(Beat per minute): 0");
    Serial.println("BPM(Beat per minute): 0");
    return;
  }

  long irValue = particleSensor.getIR();
  if (irValue == 0) {
    SerialBT.println("BPM(Beat per minute): 0");
    Serial.println("BPM(Beat per minute): 0");
    return;
  }

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  if (irValue < 50000) {
    SerialBT.println("BPM(Beat per minute): 0");
    Serial.println("BPM(Beat per minute): 0");
  } else {
    SerialBT.print("BPM(Beat per minute): ");
    SerialBT.println(beatAvg);
    Serial.print("BPM(Beat per minute): ");
    Serial.println(beatAvg);
  }
}

float readVoltage() {
  int raw = analogRead(batteryPin);
  float voltage = raw / 4095.0 * 3.7;
  return voltage * (R1 + R2) / R2;
}

void setRed() {
  leds[0] = CRGB::Red;
  FastLED.show();
}

void setGreen() {
  leds[0] = CRGB::Green;
  FastLED.show();
}

void setDeepPink() {
  leds[0] = CRGB::DeepPink;
  FastLED.show();
}
