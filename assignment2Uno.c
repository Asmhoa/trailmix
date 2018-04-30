// Assignment 2
// Code for Uno
// Eddy, Kyuri, Amol

// GLOBAL COUNTER FOR UNO
int unoCounter = 0;

// Structs
    struct TaskStruct {
        void (*taskFuncPtr)(void*);
        void* taskDataPtr;
    }; typedef struct TaskStruct TCB;


    /* INITIALIZATION - MAKE TASK DATA POINTER BLOCKS */
    struct DataForMeasureStruct {
        unsigned int* temperatureRawPtr;
        unsigned int* systolicPressRawPtr;
        unsigned int* diastolicPressRawPtr;
        unsigned int* pulseRateRawPtr;
    }; typedef struct DataForMeasureStruct MeasureTaskData;

    struct DataForComputeStruct {
        unsigned int* temperatureRawPtr;
        unsigned int* systolicPressRawPtr;
        unsigned int* diastolicPressRawPtr;
        unsigned int* pulseRateRawPtr;
        unsigned char* temperatureCorrectedPtr;
        unsigned char* sysCorrectedPtr;
        unsigned char* diasCorrectedPtr;
        unsigned char* prCorrectedPtr;
    }; typedef struct DataForComputeStruct ComputeTaskData;

    struct DataForDisplayStruct {
        unsigned char* temperatureCorrectedPtr;
        unsigned char* sysCorrectedPtr;
        unsigned char* diasCorrectedPtr;
        unsigned char* prCorrectedPtr;
        unsigned short* batteryStatePtr;
    }; typedef struct DataForDisplayStruct DisplayTaskData;

    struct DataForWarningAlarmStruct {
    int* temperatureRawPtr;
        unsigned int* systolicPressRawPtr;
        unsigned int* diastolicPressRawPtr;
        unsigned int* pulseRateRawPtr;
        unsigned short* batteryStatePtr;
    }; typedef struct DataForWarningAlarmStruct WarningAlarmTaskData;

    struct DataForStatusStruct {
        unsigned short* batteryStatePtr;
    }; typedef struct DataForStatusStruct StatusTaskData;

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
void measure(void *data);

void setup() {
  Serial.begin(9600);
}

void loop() {
    //  read incoming byte from the mega
    // if(Serial.available() == 1) {
    //     char inbyte = Serial.read();
    //     switch(inbyte) {
    //     case 'U':
    //         unoCounter++;
    //         break;
    //     }
    // if(unoCounter % 1000 == 0) {
    //     Serial.println(unoCounter);
    // }
    if(Serial.available() > 2) {
        // Read first character to decide function to run
        char identifier = Serial.read(); //i.e. M
        Serial.println(identifier);
        int decAddress = Serial.parseInt();
        Serial.println(long(decAddress), HEX);
        void *p = (void*)(long(decAddress), HEX);
        Serial.println(*((*(MeasureTaskData*)p).temperatureRawPtr));
        
//        switch(identifier) {
//            case 'U':
//                unoCounter++;
//                break;
//            case 'M':
////                measure(incomingAddress);
//                break;
//            case 'C':
//            case 'A':
//            case 'S':
//            break;
//        }
    }
}

void measure(void *data) {
    MeasureTaskData* dataToMeasure = (MeasureTaskData*)data;
    MeasureTaskData dataStruct = *dataToMeasure;

    unsigned int temperatureRaw = *dataStruct.temperatureRawPtr;
    unsigned int systolicPressRaw = *dataStruct.systolicPressRawPtr;
    unsigned int diastolicPressRaw = *dataStruct.diastolicPressRawPtr;
    unsigned int pulseRateRaw = *dataStruct.pulseRateRawPtr;

    Serial.println("Temperature is " + (String)temperatureRaw);
    if (temperatureRaw < 50) {
        tempCrossedFifty = true;
        
    }
    
    if (pulseRateRaw < 40) {
        pulseCrossedForty = true;
    }
    
    if (systolicComplete && diastolicComplete) {
        systolicPressRaw = 80;
        diastolicPressRaw = 80;
        systolicComplete = false;
        diastolicComplete = false;
        printf("<> Sys-Dias reset.\n");
    }
    
    // Perform measurement every 5 counts
    if (unoCounter % 5 == 0) {
        
        // Check if its an Even number of function call
        bool even = (unoCounter % 2 == 0);

        printf("Uno Counter: %d\n", unoCounter);
        
        // Temperature
        if ((temperatureRaw > 50 || temperatureRaw < 15) && tempCrossedFifty) {
            int temp = tempChange[0];
            tempChange[0] = -1 * tempChange[1];
            tempChange[1] = -1 * temp;
            printf("<> Temperature direction change: [%d, %d]\n", tempChange[0], tempChange[1]);
        }
        if (even) {
            temperatureRaw += tempChange[0];
        } else {
            temperatureRaw += tempChange[1];
        }
        printf("Temperature: %d\n", temperatureRaw);

        
        // Systolic: Resets to 80 at the end of sys-dias cycle
        if (systolicPressRaw <= 100) {
            if (even) {
                systolicPressRaw += 3;
            } else {
                systolicPressRaw--;
            }
        } else {
            systolicComplete = true;
        }
        printf("Systolic: %d\n", systolicPressRaw);
        
        
        // Diastolic: Resets to 80 at the end of sys-dias cycle
        if (diastolicPressRaw >= 40) {
            if (even) {
                diastolicPressRaw -= 2;
            } else {
                diastolicPressRaw++;
            }
        } else {
            diastolicComplete = true;
        }
        printf("Diastolic: %d\n", diastolicPressRaw);
        
        
        // Pulse
        if ((pulseRateRaw > 40 || pulseRateRaw < 15) && pulseCrossedForty) {
            int temp = pulseChange[0];
            pulseChange[0] = -1 * pulseChange[1];
            pulseChange[1] = -1 * temp; 
            printf("<> Pulse direction change: [%d, %d]\n", pulseChange[0], pulseChange[1]);
        }
        if (even) {
            pulseRateRaw += pulseChange[0];
        } else {
            pulseRateRaw += pulseChange[1];
        }
        printf("Pulse: %d\n", pulseRateRaw);

    }
}