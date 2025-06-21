#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BluetoothSerial.h>
#include <FastLED.h>

// ===== Wi-Fi AP Settings =====
const char* apSSID = "DSPRO-####";
const char* apPassword = "123456789";
IPAddress local_IP(192, 168, #, #);
IPAddress gateway(192, 168, #, #);
IPAddress subnet(255, 255, 255, 0);

// ===== Sensor Pins =====
#define sensorPin1 18
#define sensorPin2 23
#define batteryPin 39

// ===== Battery Voltage Divider =====
const float R1 = 150.0;
const float R2 = 560.0;
const float MIN_BATTERY_VOLTAGE = 3.7;

// ===== LED Setup =====
#define LED_PIN 15
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// ===== Speed Calculation Variables =====
const float dist = 0.3;  // meters
bool sensor1State = false;
bool sensor2State = false;
unsigned long interval = 0;
unsigned long startTime = 0;
double speed = 0;
const int timeout = 3000;  // ms

// ===== State & Bluetooth =====
bool initialCheckDone = false;
BluetoothSerial SerialBT;

// ===== OTA Setup =====
void OTA_Setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

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
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(9600);
  SerialBT.begin("DSPRO-####");

  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(batteryPin, INPUT);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  OTA_Setup();
  delay(3000);
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
      setPurple();
    }
  }

  // ===== Speed Sensor Logic =====
  sensor1State = digitalRead(sensorPin1);
  sensor2State = digitalRead(sensorPin2);
  startTime = 0;
  String speedStr = "";

  if (sensor1State == LOW) {
    startTime = millis();
    while (digitalRead(sensorPin2)) {
      if (millis() - startTime > timeout) {
        speedStr = "Timeout";
        break;
      }
      delay(3);
    }

    if (speedStr != "Timeout") {
      interval = millis() - startTime;
      if (interval > 3) {
        speed = (dist / (interval / 1000.0)) * 3.6;  // m/s to km/h
        speedStr = String(speed, 2);
      }
    }
  }

  // ===== Print and Send All Sensor Data =====
  String data = "Speed (km/h): " + (speedStr != "" ? speedStr : "0");
  Serial.println(data);
  SerialBT.println(data);

  delay(1000);
}

// ===== Voltage & LED Functions =====
float readVoltage() {
  int rawValue = analogRead(batteryPin);
  float voltage = (float)rawValue / 4095.0 * 3.7;
  float actualVoltage = voltage * (R1 + R2) / R2;
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
void setPurple() {
  leds[0] = CRGB::Purple;
  FastLED.show();
}
