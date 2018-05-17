// Assignment 2
// Code for Uno
// Eddy, Kyuri, Amol

#include <stdio.h>
#include <stdbool.h>
#include <TimerOne.h>

#define START_MESSAGE '>'
#define END_TERM ','
#define END_MESSAGE '<'

// START-------------------------- DEBOUNCE VARIABLES / CONSTANTS ------------------------------ 
    const int debounceButtonPin = 2;    // the number of the pushbutton pin
    const int debounceledPin = 12;      // the number of the LED pin

    // Variables will change:
    int debounceLedState = HIGH;         // the current state of the output pin
    int debounceButtonState;             // the current reading from the input pin
    int debounceLastButtonState = LOW;   // the previous reading from the input pin

    // the following variables are unsigned longs because the time, measured in
    // milliseconds, will quickly become a bigger number than can be stored in an int.
    unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

    // Cuff Control Flag: TRUE == Increment, FALSE == Decrement
    bool increOrDecre = true;
// END--------------------------- DEBOUNCE VARIABLES / CONSTANTS ------------------------------ 

// START------------------------------ CUFF VARIABLES / CONSTANTS ------------------------------ 
    const int buttonPin = 4;    // the number of the pushbutton pin
    const int greenLedPin = 11;      // green LED lights up to increment cuff pressure
    const int redLedPin = 10;        // red LED lights up to decrement cuff pressure

    // Variables will change:
    int ledState = HIGH;         // the current state of the output pin
    int buttonState;             // the current reading from the input pin
    int lastButtonState = LOW;   // the previous reading from the input pin

    // the following variables are unsigned longs because the time, measured in
    // milliseconds, will quickly become a bigger number than can be stored in an int.
    unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
// END------------------------------- CUFF VARIABLES / CONSTANTS ------------------------------ 


// GLOBAL COUNTER FOR UNO
// Updated during interrupt service routines
int unoCounter = 0;
char requestingTaskID = 0;
char requestedFunction = 0;
int data = 0;
bool even = false;

// Declare variables
int currTemp = 75;
int currSys = 80;
int currDia = 80;
int currPr = 0;
// BLOOD PRESSURE FOR CUFFS
double currBp = 80;

// Fields for Analog Pulse Reader
int pulseSensorPin;         // Pulse from Fn Gen
int respirationSensorPin;   // Respiration from Fn Gen
int pulseCount;
int pulseRate;
int respirationCount;
int respirationRate;
float voltageReading;

// Arrays containing changes in temp and pulse, will be switched
// when upper/lower limits are hit. 0th elements are called at Even
// function calls.
int tempChange[2] = {-2, 1}; // initially falling towards 15
int pulseChange[2] = {1, -3}; // initially falling towards 15

// Initially both temperature and pulse are very high and must keep 
// falling until they cross 50 and 40 respectively before their 
// standard boundaries can work.
bool tempCrossedFifty = false;
bool pulseCrossedForty = false;

// Systolic completion
bool systolicComplete = false;
// Diastolic completion
bool diastolicComplete = false;

// Function prototypes
void measureTemp();
void measureSys(int currSys);
void measureDia(int currDia);
void measurePr(int currPr);
void readFromFnGen();
int updatePulseRate();

void respondMessage(String taskID, String funcToRun, String data) {
    Serial.println(START_MESSAGE + taskID + END_TERM + funcToRun + END_TERM
        + data + END_MESSAGE);
}

void parseMessage() {
    //  read incoming byte from the mega
    while(Serial.available() > 0) {
        char currChar = Serial.read();
        if('U' == currChar) {
            unoCounter++;
        } else if ('>' == currChar) {
            delay(20);
            requestingTaskID = (char)Serial.read();
            Serial.read(); //Read over terminator

            delay(20);
            requestedFunction = Serial.read();
            Serial.read();

            delay(20);
            data = Serial.parseInt();
            Serial.read();
        }
    }
}

void setup() {
    Serial.begin(9600);

    // Setup for Analog Pulse Reader
    pulseCount = 0;
    voltageReading = 0;
    pulseRate = 0;
    pulseSensorPin = A1;    // Temperature
    respirationSensorPin = A2;    
  
    // DEBOUNCE SWITCH
        pinMode(debounceButtonPin, INPUT);
        pinMode(debounceledPin, OUTPUT);
        // set initial LED state
        digitalWrite(debounceledPin, debounceLedState);


    // CUFF BUTTON
        pinMode(buttonPin, INPUT);
        pinMode(greenLedPin, OUTPUT);
        pinMode(redLedPin, OUTPUT);
        // set initial LED state
        digitalWrite(greenLedPin, LOW);
        digitalWrite(redLedPin, LOW);
}

bool sysTaken = false;
bool diaTaken = false;

void loop() {   
    // START======================================== COMMS =====================================
        parseMessage();
        if(0 == strcmp(requestingTaskID, 'M')) { //Measure
            // Check if its an Even number of function call
            bool even = (unoCounter % 2 == 0);
            switch(requestedFunction) { 
                case 'T':
                    // measureTemp();
                    respondMessage("M", "T", String(currTemp));
                    break;
                case 'S':
                    // measureSys(data);
                    respondMessage("M", "S", String(currSys));
                    break;
                case 'D':
                    // measureDia(data);
                    respondMessage("M", "D", String(currDia));
                    break;
                case 'P':
                    // measurePr();
                    respondMessage("M", "P", String(currPr));
                    break;
            }
        } else if(0 == strcmp(requestingTaskID, 'C')) { //Compute

        } else if(0 == strcmp(requestingTaskID, 'A')) { //Annunciate

        } else if(0 == strcmp(requestingTaskID, 'K')) { //Status

        }

        // Reset
        requestingTaskID = 0;
        requestedFunction = 0;
        data = 0;
        even = false;
    // END========================================== COMMS =====================================

    // START================================= DEBOUNCE SWITCH ================================== 
        // If LED on, incrementing blood pressure
        // If LED off, demcrementing blood pressure
        // read the state of the switch into a local variable:
        int debounceReading = digitalRead(debounceButtonPin);

        // check to see if you just pressed the button
        // (i.e. the input went from LOW to HIGH), and you've waited long enough
        // since the last press to ignore any noise:

        // If the switch changed, due to noise or pressing:
        if (debounceReading != debounceLastButtonState) {
            // reset the debouncing timer
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            // whatever the reading is at, it's been there for longer than the debounce
            // delay, so take it as the actual current state:

            // if the button state has changed:
            if (debounceReading != debounceButtonState) {
            debounceButtonState = debounceReading;

            // only toggle the LED if the new button state is HIGH
            if (debounceButtonState == HIGH) {
                debounceLedState = !debounceLedState;
                // Set Cuff Switch Boolean: Increment or Decrement mode
                Serial.println("Cuff Switched");
                increOrDecre = !increOrDecre;
            }
            }
        }
        // set the LED:
        digitalWrite(debounceledPin, debounceLedState);

        // save the reading. Next time through the loop, it'll be the lastButtonState:
        debounceLastButtonState = debounceReading;
    // END=================================== DEBOUNCE SWITCH ==================================

    // START================================== CUFF BUTTON ===================================== 
        int reading = digitalRead(buttonPin);

        // check to see if you just pressed the button
        // (i.e. the input went from LOW to HIGH), and you've waited long enough
        // since the last press to ignore any noise:

        // If the switch changed, due to noise or pressing:
        if (reading != lastButtonState) {
            // reset the debouncing timer
            lastDebounceTime2 = millis();
        }

        if ((millis() - lastDebounceTime2) > debounceDelay) {
            // whatever the reading is at, it's been there for longer than the debounce
            // delay, so take it as the actual current state:

            // if the button state has changed:
            if (reading != buttonState) {
            buttonState = reading;

            // only toggle the LED if the new button state is HIGH
            if (buttonState == HIGH) {
                ledState = !ledState;
                // INCREMENT or DECREMENT CUFF pressure based on CUFF SWITCH BOOLEAN
                if (increOrDecre) {
                currBp = currBp * 1.1;
                Serial.print("Increment: "); Serial.println(currBp);
                } else {
                currBp = currBp * 0.9;
                Serial.print("Decrement: "); Serial.println(currBp);
                }
            }
            }
        }

        // set the LED:
        if (increOrDecre) {
            digitalWrite(greenLedPin, reading);
        } else {
            digitalWrite(redLedPin, reading);
        }

        // save the reading. Next time through the loop, it'll be the lastButtonState:
        lastButtonState = reading;
    // END==================================== CUFF BUTTON ===================================== 

    // START================================= TEMPERATURE ANALOG (A0) ==========================
        // read the input on analog pin 0:
        double tempSensorValue = analogRead(A0) / 12;
        // No longer needs measureTemp()
        currTemp = tempSensorValue;
    // END=================================== TEMPERATURE ANALOG (A0) ==========================

    // START=============================== BLOOD PRESSURE =====================================
        // Model systolic and diastolic blood pressures
        if ((currBp > 110) && (currBp < 150)) {
            
            if (!sysTaken) {
                delayMicroseconds(5);
                Serial.println("Delayed for 5ms, SYS taken");
                sysTaken = true;
            }
            currSys = currBp; // Taking the measurement
            
        } else if ((currBp > 50) && (currBp < 80)) {
            
            if (!diaTaken) {
                delayMicroseconds(5);
                Serial.println("Delayed for 5ms, DIAS taken");
                diaTaken = true;
            }

            currDia = currBp; // Taking the measurement
        } else {
            sysTaken = false;
            diaTaken = false;
        }
    // END================================= BLOOD PRESSURE =====================================

    // START=============================== PULSE RATE (A1, FN GEN) =============================
        // READ ANALOG VOLTAGE READING 
        readFromFnGen(pulseSensorPin);
        if (unoCounter % 5 == 0) { // Updates every 5 seconds?
            pulseRate = pulseCount; // Should I divide this by 5 seconds to get a moving average?
            pulseCount = 0;
        }
    // END================================= PULSE RATE (A1, FN GEN) =============================

    // START=============================== RESPIRATION RATE (A2, FN GEN) =======================
        // READ ANALOG VOLTAGE READING 
        readFromFnGen(respirationSensorPin);
        if (unoCounter % 5 == 0) { // Updates every 5 seconds?
            respirationRate = respirationCount; // Should I divide this by 5 seconds to get a moving average?
            respirationCount = 0;
        }
    // END================================= RESPIRATION RATE (A2, FN GEN) =======================
}

// Function generator to generate a 0-3.3v squarewave. Attach function generator
// to A1 or A2 pin on Uno with a GPIO for PULSE || RESPIRATION readings
void readFromFnGen(int sensorPin) {
    // Set Amplitude to 1.950
    // Set Offset to 650mV
    voltageReading = analogRead(sensorPin) * (5.0 / 1023.0);
    if (voltageReading >= 3) {
        if (sensorPin == pulseSensorPin) {
            pulseCount++;
        } else {
            respirationCount++;
        }
        
    }
}