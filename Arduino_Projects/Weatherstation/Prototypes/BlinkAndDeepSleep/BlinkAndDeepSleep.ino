/* Blink without Delay

 Turns on and off a light emitting diode (LED) connected to a digital
 pin, without using the delay() function.  This means that other code
 can run at the same time without being interrupted by the LED code.

 The circuit:
 * LED attached from pin 13 to ground.
 * Note: on most Arduinos, there is already an LED on the board
 that's attached to pin 13, so no hardware is needed for this example.

 created 2005
 by David A. Mellis
 modified 8 Feb 2010
 by Paul Stoffregen
 modified 11 Nov 2013
 by Scott Fitzgerald


 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
 */

#include "user_interface.h"

// constants won't change. Used here to set a pin number :
const int ledPin =  13;      // the number of the LED pin

// Mode of internal ADC
ADC_MODE(ADC_VCC);

void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
    // Wait for serial to initialize.
  while(!Serial) { }
  Serial.println("serial initialized");

  rst_info *rsti;
  rsti = ESP.getResetInfoPtr();
  Serial.println(String("ResetInfo.reason = ") + rsti->reason);
}

void loop() {
  // here is where you'd put code that needs to be running all the time.

   Serial.println("Let LED blink");

   letledblink(3);
   
//read VCC from internal ADC
float battery = ESP.getVcc();
  Serial.print("Battery: ");
  Serial.print(battery);

   Serial.println("Going into deep sleep for 10 seconds");
   ESP.deepSleep(10e6); // 10e6 is 10 seconds
   delay(100);
}

void letledblink (int times)
// let LED blink for a configurable number of times
{
  for (int i = 0; i <= (times+1); i++) 
   {
       digitalWrite(ledPin, HIGH);
       delay(500);
       digitalWrite(ledPin, LOW);
       delay(500);
   }
}


