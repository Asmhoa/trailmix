// Assignment 2
// Code for Uno
// Eddy, Kyuri, Amol

#include <stdio.h>
#include <stdbool.h>

// GLOBAL COUNTER FOR UNO
int unoCounter = 0;
char currentCharBuff[] = NULL;
char requestingTaskID[] = NULL;
char requestedFunction = NULL;
int numElements = -1;

// Declare variables

// Function prototypes
void measureFromValues(void *data);

void setup() {
  Serial.begin(9600);
}

void loop() {
    //  read incoming byte from the mega
    if(Serial.available() > 0) {
        Serial.readBytesUntil('>', currentCharBuff, Serial.available());
        Serial.println(currentCharBuff);
        Serial.readBytesUntil(',', requestingTaskID, Serial.available());
        Serial.println("Requesting TaskID = " + requestingTaskID);
        Serial.readBytesUntil(',', requestedFunction, Serial.available());
        Serial.println("Requested Function = " + requestedFunction);
        if(0 == strcmp(requestingTaskID, "measureDataFunc")) {

        } else if(0 == strcmp(requestingTaskID, "computeDataFunc")) {

        } else if(0 == strcmp(requestingTaskID, "annunciateDataFunc")) {

        } else if(0 == strcmp(requestingTaskID, "statusDataFunc")) {

        }
    }


    // if(Serial.available() > 2) {
    //     // Read first character to decide function to run
    //     char identifier = Serial.read(); //i.e. M
    //     Serial.println(identifier);
    //     int decAddress = Serial.parseInt();
    //     Serial.println(long(decAddress), HEX);
    //     void *p = (void*)(long(decAddress), HEX);
    //     Serial.println(*((*(MeasureTaskData*)p).temperatureRawPtr));

    }
}