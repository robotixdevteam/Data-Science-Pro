#include <FastLED.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// === Sensor Pins ===
#define soilmoisturePin 36
#define batteryPin      39

// === Battery Constants ===
const float R1 = 150.0;
const float R2 = 560.0;
const float MIN_BATTERY_VOLTAGE = 3.7;
float voltage;

// === LED Settings ===
#define LED_PIN 15
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];
bool initialCheckDone = false;

// === Bluetooth Object ===
BluetoothSerial SerialBT;

// === OTA Wi-Fi AP Settings ===
const char* apSSID = "DSPRO-####";
const char* apPassword = "123456789";
IPAddress local_IP(192, 168, #, #);
IPAddress gateway(192, 168, #, #);
IPAddress subnet(255, 255, 255, 0);

// === OTA Setup Function ===
void OTA_Setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

  ArduinoOTA.setPassword("1234");
  ArduinoOTA.begin();

  Serial.println("OTA Ready");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
}

// === Setup ===
void setup() {
  Serial.begin(9600);
  SerialBT.begin("DSPRO-####");

  pinMode(batteryPin, INPUT);
  pinMode(soilmoisturePin, INPUT);

  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  OTA_Setup();
  delay(3000);
}

// === Main Loop ===
void loop() {
  ArduinoOTA.handle();
  voltage = readVoltage();

  if (voltage < MIN_BATTERY_VOLTAGE) {
    setRed();
    Serial.println("Low Battery");
    SerialBT.println("Low Battery");
    return;
  } else {
    if (!initialCheckDone) {
      setGreen();
      initialCheckDone = true;
      delay(500);
    } else {
      setCyan();
    }
  }

  // === Read Soil Moisture ===
  int soilVal = analogRead(soilmoisturePin);
  int soilPct = map(soilVal, 0, 4095, 0, 100);  // ESP32 has 12-bit ADC

  // === Print Raw Value for Debugging ===
  Serial.print("Raw soil value: ");
  Serial.println(soilVal);

  // === Output Data ===
  String output = "Soil Moisture (%): " + String(soilPct);
  Serial.println(output);
  SerialBT.println(output);

  delay(2000);
}

// === Voltage Reading ===
float readVoltage() {
  int raw = analogRead(batteryPin);
  float v = ((float)raw / 4095.0) * 3.3;  // ADC reference is 3.3V
  return v * (R1 + R2) / R2;
}

// === LED Status Functions ===
void setRed() {
  leds[0] = CRGB::Red;
  FastLED.show();
}
void setGreen() {
  leds[0] = CRGB::Green;
  FastLED.show();
}
void setCyan() {
  leds[0] = CRGB::Cyan;
  FastLED.show();
}
