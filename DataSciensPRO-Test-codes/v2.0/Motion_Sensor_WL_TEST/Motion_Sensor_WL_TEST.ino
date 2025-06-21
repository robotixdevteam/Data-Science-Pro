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
#define pirPin 36
#define batteryPin 39

// ===== Battery Voltage Divider =====
const float R1 = 150.0;
const float R2 = 560.0;
const float MIN_BATTERY_VOLTAGE = 3.7;

// ===== LED Settings =====
#define LED_PIN_2 15
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// ===== State Variables =====
BluetoothSerial SerialBT;
bool initialCheckDone = false;
unsigned long startTime = 0;
int motionLevel = 0;
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

  pinMode(pirPin, INPUT);
  pinMode(batteryPin, INPUT);

  FastLED.addLeds<NEOPIXEL, LED_PIN_2>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  OTA_Setup();
  delay(3000);
}

void loop() {
  ArduinoOTA.handle();

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
    } else {
      setBlue();
    }
  }

  // ===== PIR Motion Detection =====
  if (digitalRead(pirPin) == HIGH) {
    if (startTime == 0) startTime = millis();
    unsigned long duration = millis() - startTime;
    motionLevel = map(duration, 0, 20000, 0, 100);
    motionLevel = constrain(motionLevel, 0, 100);
  } else {
    startTime = 0;
    motionLevel = 0;
  }

  // ===== Output via Bluetooth & Serial =====
  String data = "Motion Level (%): " + String(motionLevel);
  SerialBT.println(data);
  Serial.println(data);

  //delay(1000);
}

float readVoltage() {
  rawValue = analogRead(batteryPin);
  voltage = (float)rawValue / 4095.0 * 3.7;
  actualVoltage = voltage * (R1 + R2) / R2;
  return actualVoltage;
}

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
