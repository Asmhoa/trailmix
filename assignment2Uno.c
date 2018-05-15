// Assignment 2
// Code for Uno
// Eddy, Kyuri, Amol

#include <stdio.h>
#include <stdbool.h>
#include <TimerOne.h>

#define START_MESSAGE '>'
#define END_TERM ','
#define END_MESSAGE '<'

// GLOBAL COUNTER FOR UNO
// Updated during interrupt service routines
int unoCounter = 0;
char requestingTaskID = 0;
char requestedFunction = 0;
int data = 0;
bool even = false;

// Declare variables
int currTemp = 0;
int currSys = 0;
int currDia = 0;
int currPr = 0;

// Fields for Analog Pulse Reader
int sensorPin;    
int sensorValue;
int pulseCount;
int pulseRate;
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
void measureTemp(int currTemp);
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
    sensorPin = A0;    // select the input pin for the potentiometer
    pulseCount = 0;
    voltageReading = 0;
    pulseRate = 0;
}

void loop() {
    // READ ANALOG VOLTAGE READING 
    readFromFnGen();  

    parseMessage();
    if(0 == strcmp(requestingTaskID, 'M')) { //Measure
        // Check if its an Even number of function call
        bool even = (unoCounter % 2 == 0);
        switch(requestedFunction) { 
            case 'T':
                measureTemp(data);
                respondMessage("M", "T", String(currTemp));
                break;
            case 'S':
                measureSys(data);
                respondMessage("M", "S", String(currSys));
                break;
            case 'D':
                measureDia(data);
                respondMessage("M", "D", String(currDia));
                break;
            case 'P':
                measurePr();
                respondMessage("M", "P", String(currPr));
                break;
        }
    } else if(0 == strcmp(requestingTaskID, 'C')) { //Compute

    } else if(0 == strcmp(requestingTaskID, 'A')) { //Annunciate

    } else if(0 == strcmp(requestingTaskID, 'K')) { //Status

    }

    // RESET
    requestingTaskID = 0;
    requestedFunction = 0;
    data = 0;
    even = false;
}

void measureTemp(int currTempMega) {
    currTemp = 6;
}

void measureSys(int currSysMega) {
        // Systolic: Resets to 80 at the end of sys-dias cycle
        if (currSysMega <= 100) {
            if (even) {
                currSys += 3;
            } else {
                currSys--;
            }
        } else {
            systolicComplete = true;
            if(diastolicComplete) {
                currSys = 20;
                currDia = 80;
                diastolicComplete = false;
                systolicComplete = false;
            }
        }
}

void measureDia(int currDiaMega) {
        // Diastolic: Resets to 80 at the end of sys-dias cycle
        if (currDiaMega >= 40) {
            if (even) {
                currDia -= 2;
            } else {
                currDia++;
            }
        } else {
            diastolicComplete = true;
            
        }
}

void measurePr() {
        // if (currPr < 40) {
        //     pulseCrossedForty = true;
        // }

        //         // Pulse
        // if ((currPr > 40 || currPr < 15) && pulseCrossedForty) {
        //     int temp = pulseChange[0];
        //     pulseChange[0] = -1 * pulseChange[1];
        //     pulseChange[1] = -1 * temp; 
        // }
        // if (even) {
        //     currPr += pulseChange[0];
        // } else {
        //     currPr += pulseChange[1];
        // }
        
        // float low = currPr * 0.85;
        // float high = currPr * 1.15;

        // currPr = updatePulseRate();

        currPr = unoCounter % 5 * 10;
        pulseCount = 0;
}

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

// Sends Mega a pulse rate measured every 5 seconds. 
// int updatePulseRate() {
//     pulseRate = pulseCount;
//     pulseCount = 0;
//     // SEND PULSERATE TO MEGA
//     return pulseRate;
// }