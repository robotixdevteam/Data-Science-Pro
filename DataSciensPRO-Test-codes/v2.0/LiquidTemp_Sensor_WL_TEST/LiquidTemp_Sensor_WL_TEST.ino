#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include "BluetoothSerial.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== OTA Wi-Fi AP Settings =====
const char* apSSID = "DSPRO-####";
const char* apPassword = "123456789";
IPAddress local_IP(192, 168, #, #);
IPAddress gateway(192, 168, #, #);
IPAddress subnet(255, 255, 255, 0);

// ===== Sensor Pins =====
#define ONE_WIRE_BUS 5
#define batteryPin 39

// ===== Battery Divider =====
const float R1 = 150.0;
const float R2 = 560.0;
const float MIN_BATTERY_VOLTAGE = 3.7;

// ===== LED Settings =====
#define LED_PIN_2 15
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

// ===== State and Components =====
BluetoothSerial SerialBT;
bool initialCheckDone = false;
int rawValue;
float voltage, actualVoltage;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ===== OTA Setup =====
void OTA_Setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

  ArduinoOTA
    .onStart([]() {
      Serial.println("Start updating...");
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
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

  pinMode(ONE_WIRE_BUS, INPUT);
  pinMode(batteryPin, INPUT);

  sensors.begin();

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

  // Read DS18B20
  sensors.requestTemperatures();
  float liquidTemp = sensors.getTempCByIndex(0);
  if (liquidTemp == DEVICE_DISCONNECTED_C) {
    liquidTemp = 0.00;
  }

  // Combine and send output
  String output = "";
  output += "Liquid Temperature (^C): " + String(liquidTemp, 2);

  Serial.println(output);
  SerialBT.println(output);

  delay(1000);
}

float readVoltage() {
  rawValue = analogRead(batteryPin);
  voltage = (float)rawValue / 4095.0 * 3.3;  // Use 3.3V reference for ESP32 ADC
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
