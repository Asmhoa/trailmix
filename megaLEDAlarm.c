#include <stdio.h>
#include <stdbool.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <assignment2Mega.c>

// Mega Alarm LED Pin
const int redAlarmLEDPin = 45; 

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(redAlarmLEDPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(redAlarmLEDPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(redAlarmLEDPin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}