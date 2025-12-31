#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup() {
  Serial.begin(9600);
  if (!bno.begin()) {
    Serial.print("No BNO055 detected");
    while (1);
  }
  bno.setExtCrystalUse(true);
}

void loop() {
  sensors_event_t event;
  bno.getEvent(&event);
  Serial.print("Orientation X: "); Serial.print(event.orientation.x);
  Serial.print("\tY: "); Serial.print(event.orientation.y);
  Serial.print("\tZ: "); Serial.println(event.orientation.z);
  delay(100);
}