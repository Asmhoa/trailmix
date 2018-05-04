#include <TimerOne.h>

int sensorPin;    
int sensorValue;
int pulseCount;
int pulseRate;
float voltageReading;

void setup() {
  // put your setup code here, to run once:
  // Fields for readFromFnGen
  sensorPin = A0;    // select the input pin for the potentiometer
  pulseCount = 0;
  voltageReading = 0;
  pulseRate = 0;
  Serial.begin(9600);

  Timer1.initialize(1000000);
  Timer1.attachInterrupt(updatePulseRate); // calculate average every minute
}

void loop() {
  // put your main code here, to run repeatedly:
  readFromFnGen();  
  //Serial.println(voltageReading);
  Serial.println(pulseCount);
}


// TODO: Write a function that reads 0 - 3.3v signals from function generator
// Function generator to generate a 0-3.3v squarewave. Attach function generator
// to A0 pin on Uno with a GPIO
void readFromFnGen() {
  // Set Amplitude to 1.950
  // Set Offset to 650mV
  voltageReading = analogRead(sensorPin) * (5.0 / 1023.0);
  if (voltageReading >= 3) {
      pulseCount++;
  }
}

void updatePulseRate() {
  pulseRate = pulseCount;
  // SEND PULSERATE TO MEGA
  
  pulseCount = 0;
}
