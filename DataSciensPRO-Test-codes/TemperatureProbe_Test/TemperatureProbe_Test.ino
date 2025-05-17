#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
void setup() {
  Serial.begin(9600);
  sensors.begin();
  pinMode(ONE_WIRE_BUS, INPUT);
}
void loop() {
  //print the liquid temperature
  sensors.requestTemperatures();
  Serial.print("Liquid Temperature (^C): ");
  float probe=sensors.getTempCByIndex(0);
  Serial.print(probe);
  Serial.println(" ");
}