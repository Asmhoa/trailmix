// Assignment 2
// Code for Uno
// Eddy, Kyuri, Amol

#include <stdio.h>
#include <stdbool.h>

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
}

void loop() {
    parseMessage();
    if(0 == strcmp(requestingTaskID, 'M')) { //Measure
        // Check if its an Even number of function call
        bool even = (unoCounter % 2 == 0);
        switch(requestedFunction) { 
            case 'T':
                measureTemp(data);
                respondMessage("M", "T", "6");
                break;
            case 'S':
                measureSys(data);
                respondMessage("M", "S", "5");
                break;
            case 'D':
                measureDia(data);
                respondMessage("M", "D", "3");
                break;
            case 'P':
                measurePr(data);
                respondMessage("M", "P", "2");
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

void measureTemp(int currTemp) {
    if(0 == unoCounter % 5) {
        if (currTemp < 50) {
        tempCrossedFifty = true;   
        }

        if ((currTemp > 50 || currTemp < 15) && tempCrossedFifty) {
            int temp = tempChange[0];
            tempChange[0] = -1 * tempChange[1];
            tempChange[1] = -1 * temp;
        }

        if (even) {
            currTemp += tempChange[0];
        } else {
            currTemp += tempChange[1];
        }
    }
}

void measureSys(int currSys) {
    if(0 == unoCounter % 5) {
        // Systolic: Resets to 80 at the end of sys-dias cycle
        if (currSys <= 100) {
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
}

void measureDia(int currDia) {
    if(0 == unoCounter % 5) {
        // Diastolic: Resets to 80 at the end of sys-dias cycle
        if (currDia >= 40) {
            if (even) {
                currDia -= 2;
            } else {
                currDia++;
            }
        } else {
            diastolicComplete = true;
            
        }
    }
}

void measurePr(int currPr) {
    if(0 == unoCounter % 5) {
        if (currPr < 40) {
            pulseCrossedForty = true;
        }

                // Pulse
        if ((currPr > 40 || currPr < 15) && pulseCrossedForty) {
            int temp = pulseChange[0];
            pulseChange[0] = -1 * pulseChange[1];
            pulseChange[1] = -1 * temp; 
        }
        if (even) {
            currPr += pulseChange[0];
        } else {
            currPr += pulseChange[1];
        }
        
        float low = currPr * 0.85;
        float high = currPr * 1.15;
    }
}