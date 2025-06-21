#include "DHT.h"
#define DHTPIN 27
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  // Set the pin modes for the sensors
  pinMode(DHTPIN, INPUT);
}

void loop() {

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity)) {
    humidity = 0.0;
  }
  if (isnan(temperature)) {
    temperature = 0.0;
  }
  Serial.print("Humidity (%): ");
  Serial.print(humidity);
  Serial.print(", ");

  Serial.print("Temperature (^C): ");
  Serial.println(temperature);
}