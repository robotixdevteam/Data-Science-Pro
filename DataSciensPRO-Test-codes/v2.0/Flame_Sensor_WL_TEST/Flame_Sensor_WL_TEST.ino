#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include "BluetoothSerial.h"

// ===== OTA Wi-Fi AP Settings =====
const char* apSSID = "DSPRO-####";
const char* apPassword = "123456789";
IPAddress local_IP(192, 168, #, #);
IPAddress gateway(192, 168, #, #);
IPAddress subnet(255, 255, 255, 0);

// ===== Sensor Pins =====
#define flame_PIN 27
#define batteryPin 39

// ===== Battery Voltage Divider =====
const float R1 = 150.0;
const float R2 = 560.0;
const float MIN_BATTERY_VOLTAGE = 3.7;

// ===== LED =====
#define LED_PIN_2 15
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// ===== State Variables =====
BluetoothSerial SerialBT;
bool initialCheckDone = false;
bool otaDisabled = false;
int rawValue;
float voltage, actualVoltage;

// ===== OTA Setup =====
void OTA_Setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

  Serial.println("Access Point Started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  ArduinoOTA
    .onStart([]() { Serial.println("Start updating..."); })
    .onEnd([]() { Serial.println("\nEnd"); })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.setPassword("1234");
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void setup() {
  Serial.begin(9600);
  SerialBT.begin("DSPRO-####");

  pinMode(flame_PIN, INPUT);
  pinMode(batteryPin, INPUT);

  FastLED.addLeds<NEOPIXEL, LED_PIN_2>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  OTA_Setup();
  delay(3000);
}

void loop() {
  if (!otaDisabled) {
    ArduinoOTA.handle();
  }

  voltage = readVoltage();
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

      // ===== Disable OTA and Wi-Fi After Initial Check =====
      ArduinoOTA.end();                // Stop OTA
      delay(100);
      WiFi.softAPdisconnect(true);     // Disconnect AP
      delay(100);
      WiFi.mode(WIFI_OFF);             // Turn off Wi-Fi
      delay(100);
      otaDisabled = true;
    } else {
      setBlue();
    }
  }

  int flame = readFlameSensor();
  String data = "Flame Proximity Index (no unit): " + String(flame);
  SerialBT.println(data);
  Serial.println(data);
  delay(1000);
}

// ===== Flame Sensor Averaging =====
int readFlameSensor() {
  int sum = 0;
  const int numSamples = 10;
  for (int i = 0; i < numSamples; i++) {
    sum += analogRead(flame_PIN);
    delayMicroseconds(50);
  }
  int avgIR = sum / numSamples;
  int flameValue = map(avgIR, 0, 4095, 0, 100);
  return constrain(flameValue, 0, 100);
}

// ===== Battery Voltage =====
float readVoltage() {
  rawValue = analogRead(batteryPin);
  voltage = (float)rawValue / 4095.0 * 3.7;
  actualVoltage = voltage * (R1 + R2) / R2;
  return actualVoltage;
}

// ===== LED Control =====
void setRed() {
  leds[0] = CRGB::Red;
  FastLED.show();
}
void setGreen() {
  leds[0] = CRGB::Green;
  FastLED.show();
}
void setBlue() {
  leds[0] = CRGB::Blue;
  FastLED.show();
}
