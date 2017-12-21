/*
 *  Read from DHT 12 and send data to serial monitor
 */
#include "WEMOS_DHT12.h"

DHT12 dht12;

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("Setup done");
}

void loop() {
  Serial.println("start reading sensor data");

if(dht12.get()==0){
    Serial.println(dht12.cTemp);
    Serial.println(dht12.humidity);
  }
  else
  {
    Serial.println("Error!");
  }

  // Wait a bit before scanning again
  delay(5000);
}
