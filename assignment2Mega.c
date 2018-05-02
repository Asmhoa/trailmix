// Assignment 2
// Code for ATMega
// Eddy, Kyuri, Amol

#include <stdio.h>
#include <stdbool.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include "TimerThree.h"

int unoCounter = 0;

// Define number of tasks
#define NUM_TASKS 6 //Need to ensure this is accurate else program breaks

/* INITIALIZATION - SETUP TFT DISPLAY */
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Elegoo_TFTLCD tft;

/* INITIALIZATION - GLOBAL VARIABLES */
    // Measurements
    unsigned int temperatureRaw, systolicPressRaw,
        diastolicPressRaw, pulseRateRaw;

    // Display
    double tempCorrected; 
    double systolicPressCorrected;
    double diastolicPressCorrected;
    double pulseRateCorrected;

    // Status
    unsigned short batteryState;

    // Alarms
    unsigned char bpOutOfRange, tempOutOfRange,
        pulseOutOfRange;

    // Warning
    bool bpHigh, tempHigh, pulseLow, battLow;

/* INITIALIZATION - MAKE TASK BLOCK STRUCTURE */
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
    double* temperatureCorrectedPtr;
    double* sysCorrectedPtr;
    double* diasCorrectedPtr;
    double* prCorrectedPtr;
}; typedef struct DataForComputeStruct ComputeTaskData;

struct DataForDisplayStruct {
    double* temperatureCorrectedPtr;
    double* sysCorrectedPtr;
    double* diasCorrectedPtr;
    double* prCorrectedPtr;
    unsigned short* batteryStatePtr;
}; typedef struct DataForDisplayStruct DisplayTaskData;

struct DataForWarningAlarmStruct {
    unsigned int* temperatureRawPtr;
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


/* INITIALIZATION - MAKE INSTANCES OF TCB */
TCB MeasureTask;
TCB ComputeTask;
TCB DisplayTask;
TCB AnnunciateTask;
TCB StatusTask;
TCB NullTask;

// Data
MeasureTaskData dataForMeasure;
ComputeTaskData dataForCompute;
DisplayTaskData dataForDisplay;
WarningAlarmTaskData dataForWarningAlarm;
StatusTaskData dataForStatus;


/* INITIALIZATION - FUNCTION DEFINITIONS */
void measureDataFunc(void* data) {
    MeasureTaskData* dataToMeasure = (MeasureTaskData*)data;
    MeasureTaskData dataStruct = *dataToMeasure;

    unsigned int temperatureRaw = *dataStruct.temperatureRawPtr;
    unsigned int systolicPressRaw = *dataStruct.systolicPressRawPtr;
    unsigned int diastolicPressRaw = *dataStruct.diastolicPressRawPtr;
    unsigned int pulseRateRaw = *dataStruct.pulseRateRawPtr;

    if (temperatureRaw < 50) {
        tempCrossedFifty = true;   
    }
    
    if (pulseRateRaw < 40) {
        pulseCrossedForty = true;
    }
    
    /*
    if (systolicComplete && diastolicComplete) {
        systolicPressRaw = 80;
        diastolicPressRaw = 80;
        systolicComplete = false;
        diastolicComplete = false;
    }*/
    
    // Perform measurement every 5 counts
    if (unoCounter % 5 == 0) {
        
        // Check if its an Even number of function call
        bool even = (unoCounter % 2 == 0);
        
        // Temperature
        if ((temperatureRaw > 50 || temperatureRaw < 15) && tempCrossedFifty) {
            int temp = tempChange[0];
            tempChange[0] = -1 * tempChange[1];
            tempChange[1] = -1 * temp;
        }
        if (even) {
            temperatureRaw += tempChange[0];
        } else {
            temperatureRaw += tempChange[1];
        }
        
        // Systolic: Resets to 80 at the end of sys-dias cycle
        if (systolicPressRaw <= 100) {
            if (even) {
                systolicPressRaw += 3;
            } else {
                systolicPressRaw--;
            }
        } else {
            systolicComplete = true;
            if(diastolicComplete) {
                systolicPressRaw = 20;
                diastolicPressRaw = 80;
                diastolicComplete = false;
                systolicComplete = false;
            }
        }
        
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

        // Pulse
        if ((pulseRateRaw > 40 || pulseRateRaw < 15) && pulseCrossedForty) {
            int temp = pulseChange[0];
            pulseChange[0] = -1 * pulseChange[1];
            pulseChange[1] = -1 * temp; 
        }
        if (even) {
            pulseRateRaw += pulseChange[0];
        } else {
            pulseRateRaw += pulseChange[1];
        }

        *dataStruct.temperatureRawPtr = temperatureRaw;
        *dataStruct.systolicPressRawPtr = systolicPressRaw;
        *dataStruct.diastolicPressRawPtr = diastolicPressRaw;
        *dataStruct.pulseRateRawPtr = pulseRateRaw;
    }
}

void computeDataFunc(void* x) {
    if (unoCounter % 5 == 0) {
        // Dereferencing void pointer to ComputeStruct
        ComputeTaskData* data = (ComputeTaskData*)x;
        ComputeTaskData dataStruct = *data;
        
        // Computing and converting temperatureRaw to unsigned char* (Celcius)
        double correctedTemp = 5 + 0.75 * *dataStruct.temperatureRawPtr;
        *dataStruct.temperatureCorrectedPtr = correctedTemp;
        
        // Computing and converting systolic pressure to unsigned char*
        double correctedSys = 9 + 2 * *dataStruct.systolicPressRawPtr;
        *dataStruct.sysCorrectedPtr = correctedSys;
        
        // Computing and converting diastolic pressure to unsigned char*
        double correctedDia =  6 + 1.5 * *dataStruct.diastolicPressRawPtr;
        *dataStruct.diasCorrectedPtr = correctedDia;

        // Computing and converting pulse rate to unsigned char*
        double correctedPr =  8 + 3 * *dataStruct.pulseRateRawPtr;
        *dataStruct.prCorrectedPtr = correctedPr;
    }
}

void displayDataFunc(void* x) {
    if (unoCounter % 5 == 0) { // Every 5 seconds
        DisplayTaskData* data = (DisplayTaskData*)x;
        DisplayTaskData dataStruct = *data;
        tft.setTextSize(1);
        tft.fillScreen(BLACK);
        tft.setCursor(0, 0);
        tempOutOfRange ? tft.setTextColor(RED) : tft.setTextColor(GREEN);
        tft.println("Temperature: " + (String)*dataStruct.temperatureCorrectedPtr + " C");
        bpOutOfRange ? tft.setTextColor(RED) : tft.setTextColor(GREEN);        
        tft.println("Systolic pressure: " + (String)*dataStruct.sysCorrectedPtr + " mm Hg");
        tft.println("Diastolic pressure: " + (String)*dataStruct.diasCorrectedPtr + " mm Hg");
        pulseOutOfRange ? tft.setTextColor(RED) : tft.setTextColor(GREEN);        
        tft.println("Pulse rate: " + (String)*dataStruct.prCorrectedPtr + " BPM");
        battLow ? tft.setTextColor(RED) : tft.setTextColor(GREEN);
        tft.println("Battery: " + (String)*dataStruct.batteryStatePtr);
    }
}

void annunciateDataFunc(void* x) {
    // Dereferencing void pointer to WarningStruct
    WarningAlarmTaskData* data = (WarningAlarmTaskData*)x;
    WarningAlarmTaskData dataStruct = *data;
    
    // check temperature:
    double normalizedTemp = 5 + 0.75 * *dataStruct.temperatureRawPtr;
    if (normalizedTemp < 36.1 || normalizedTemp > 37.8) {
        // TURN TEXT RED
        tempOutOfRange = 1;
        if (normalizedTemp > 40) {
            tempHigh = true;
        } else {
            tempHigh = false;
        }
    } else {
        tempOutOfRange = 0;
        tempHigh = false;
    }

    // check systolic: Instructions unclear, googled for healthy and 
    // unhealthy systolic pressures. 
    // SysPressure > 130 == High blood pressure
    // SysPressure > 160 == CALL A DOCTOR
    int normalizedSystolic = 9 + 2 * *dataStruct.systolicPressRawPtr;
    if (normalizedSystolic > 130) {
        // TURN TEXT RED
        bpOutOfRange = 1;
        if (normalizedSystolic > 160) {
            bpHigh = true;
        } else {
            bpHigh = false;
        }
    } else {
        bpOutOfRange = 0;
        bpHigh = false;
    }
    
    // check diastolic
    // DiasPressure > 90 == High blood pressure
    // DiasPressure > 120 == CALL A DOCTOR

            /*  IF PRESSURE CRITICALLY HIGH, need interrupt flashing warnings -----------------------------------
                User can acknowledge it to DISABLE INTERRUPT
            */

    double normalizedDiastolic = 6 + 1.5 * *dataStruct.diastolicPressRawPtr;
    if (normalizedDiastolic > 90) {
        // TURN TEXT RED
        bpOutOfRange = 1;
        if (normalizedDiastolic > 120) {
            bpHigh = true;
        } else {
            bpHigh = false;
        }
    } else {
        bpOutOfRange = 0;
        bpHigh = false;
    }
    
    // pulse rate 
    // HEALTHY PULSE => Between 60 and 100
    // IF PULSE < 30, WARNING

        /*  IF PULSE CRITICALLY LOW, need interrupt flashing warnings -----------------------------------
            User can acknowledge it to DISABLE INTERRUPT
        */

    int normalizedPulse = 8 + 3 * *dataStruct.pulseRateRawPtr;
    if (normalizedPulse < 60 || normalizedPulse > 100) {
        pulseOutOfRange = 1;
        if (normalizedPulse < 30) {
            pulseLow = true;
        } else {
            pulseLow = false;
        }
    } else {
        pulseOutOfRange = 0;
        pulseLow = false;
    }
    
    // battery
    // HEALTHY BATTERY LEVEL => Battery level above 20%
    // //printf("BATT LEVEL: %f\n", batteryLevel);

        /*  IF BATTERY CRITICALLY LOW, need interrupt flashing warnings -----------------------------------
            User can acknowledge it to DISABLE INTERRUPT
        */

    if (batteryState < 40) {
        battLow = true;
        // printf("LOW BATT: %f\n", batteryLevel);
    } else {
        battLow = false;
    }
}

void statusDataFunc(void* x) {
    if(unoCounter % 5 == 0) {
        // Dereferencing void pointer to WarningStruct
        StatusTaskData* data = (StatusTaskData*)x;
        StatusTaskData dataStruct = *data;
        
        *dataStruct.batteryStatePtr = 200 - (int)(unoCounter / 5);
    }
}

// Delay for 100ms and update counter/time on peripheral system
void updateCounter(void) {
    unoCounter++;
}

/* INITIALIZATION */
void setup(void) {
    // Setup communication
    Serial1.begin(9600);

    // Initialise Timer3
    Timer3.initialize(1000000);
    Timer3.attachInterrupt(updateCounter());
    
    // Configure display
        Serial.begin(9600);
        Serial.println(F("TFT LCD test"));


        #ifdef USE_Elegoo_SHIELD_PINOUT
            Serial.println(F("Using Elegoo 2.4\" TFT Arduino Shield Pinout"));
        #else
            Serial.println(F("Using Elegoo 2.4\" TFT Breakout Board Pinout"));
        #endif

        Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

        tft.reset();

        uint16_t identifier = tft.readID();
        if(identifier == 0x9325) {
            Serial.println(F("Found ILI9325 LCD driver"));
        } else if(identifier == 0x9328) {
            Serial.println(F("Found ILI9328 LCD driver"));
        } else if(identifier == 0x4535) {
            Serial.println(F("Found LGDP4535 LCD driver"));
        }else if(identifier == 0x7575) {
            Serial.println(F("Found HX8347G LCD driver"));
        } else if(identifier == 0x9341) {
            Serial.println(F("Found ILI9341 LCD driver"));
        } else if(identifier == 0x8357) {
            Serial.println(F("Found HX8357D LCD driver"));
        } else if(identifier==0x0101)
        {     
            identifier=0x9341;
            Serial.println(F("Found 0x9341 LCD driver"));
        }
        else if(identifier==0x1111)
        {     
            identifier=0x9328;
            Serial.println(F("Found 0x9328 LCD driver"));
        }
        else {
            Serial.print(F("Unknown LCD driver chip: "));
            Serial.println(identifier, HEX);
            Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
            Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
            Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
            Serial.println(F("If using the breakout board, it should NOT be #defined!"));
            Serial.println(F("Also if using the breakout, double-check that all wiring"));
            Serial.println(F("matches the tutorial."));
            identifier=0x9328;
        
        }
        tft.begin(identifier);

    // Set measurements to initial values
    // char* and char values are already set as global variables
    temperatureRaw = 75;
    systolicPressRaw = 80;
    diastolicPressRaw = 80;
    pulseRateRaw = 50;
    batteryState = 200;
    bpHigh = false;
    tempHigh = false;
    pulseLow = false;
    battLow = false;

    // Point data in data structs to correct information
        // Measure
        MeasureTaskData dataForMeasureTMP;
        dataForMeasureTMP.temperatureRawPtr = &temperatureRaw; 
        dataForMeasureTMP.systolicPressRawPtr = &systolicPressRaw;
        dataForMeasureTMP.diastolicPressRawPtr = &diastolicPressRaw;
        dataForMeasureTMP.pulseRateRawPtr = &pulseRateRaw;
        dataForMeasure = dataForMeasureTMP;

        // Compute
        ComputeTaskData dataForComputeTMP;
        dataForComputeTMP.temperatureRawPtr = &temperatureRaw; 
        dataForComputeTMP.systolicPressRawPtr = &systolicPressRaw;
        dataForComputeTMP.diastolicPressRawPtr = &diastolicPressRaw;
        dataForComputeTMP.pulseRateRawPtr = &pulseRateRaw;
        dataForComputeTMP.temperatureCorrectedPtr = &tempCorrected; // Already a pointer
        dataForComputeTMP.sysCorrectedPtr = &systolicPressCorrected;
        dataForComputeTMP.diasCorrectedPtr = &diastolicPressCorrected;
        dataForComputeTMP.prCorrectedPtr = &pulseRateCorrected;
        dataForCompute = dataForComputeTMP;

        // Display
        DisplayTaskData dataForDisplayTMP;
        dataForDisplayTMP.temperatureCorrectedPtr = &tempCorrected; // Already a pointer
        dataForDisplayTMP.sysCorrectedPtr = &systolicPressCorrected;
        dataForDisplayTMP.diasCorrectedPtr = &diastolicPressCorrected;
        dataForDisplayTMP.prCorrectedPtr = &pulseRateCorrected;
        dataForDisplayTMP.batteryStatePtr = &batteryState;
        dataForDisplay = dataForDisplayTMP;

        // WarningAlarm
        WarningAlarmTaskData dataForWarningAlarmTMP;
        dataForWarningAlarmTMP.temperatureRawPtr = &temperatureRaw; 
        dataForWarningAlarmTMP.systolicPressRawPtr = &systolicPressRaw;
        dataForWarningAlarmTMP.diastolicPressRawPtr = &diastolicPressRaw;
        dataForWarningAlarmTMP.pulseRateRawPtr = &pulseRateRaw;
        dataForWarningAlarmTMP.batteryStatePtr = &batteryState;
        dataForWarningAlarm = dataForWarningAlarmTMP;

        // Status
        StatusTaskData dataForStatusTMP;
        dataForStatusTMP.batteryStatePtr = &batteryState;
        dataForStatus = dataForStatusTMP;

    // Assign values in TCB's
    // Measure
    TCB MeasureTaskTMP; // Getting an error if I try to use MeasureTask directly
    MeasureTaskTMP.taskFuncPtr = &measureDataFunc;
    MeasureTaskTMP.taskDataPtr = &dataForMeasure;
    MeasureTask = MeasureTaskTMP;

    // Compute
    TCB ComputeTaskTMP;
    ComputeTaskTMP.taskFuncPtr = &computeDataFunc;
    ComputeTaskTMP.taskDataPtr = &dataForCompute;
    ComputeTask = ComputeTaskTMP;
    
    // Display
    TCB DisplayTaskTMP;
    DisplayTaskTMP.taskFuncPtr = &displayDataFunc;
    DisplayTaskTMP.taskDataPtr = &dataForDisplay;
    DisplayTask = DisplayTaskTMP;

    // Warning/Alarm
    TCB AnnunciateTaskTMP;
    AnnunciateTaskTMP.taskFuncPtr = &annunciateDataFunc;
    AnnunciateTaskTMP.taskDataPtr = &dataForWarningAlarm;
    AnnunciateTask = AnnunciateTaskTMP;

    // Status
    TCB StatusTaskTMP;
    StatusTaskTMP.taskFuncPtr = &statusDataFunc;
    StatusTaskTMP.taskDataPtr = &dataForStatus;
    StatusTask = StatusTaskTMP;

    // NULL TCB
    TCB NullTaskTMP;
    NullTaskTMP.taskFuncPtr = NULL;
    NullTaskTMP.taskDataPtr = NULL;
    NullTask = NullTaskTMP;

}

void loop(void) {
    unsigned long start = micros(); 
    
    /* SCHEDULE */
    TCB tasksArray[NUM_TASKS];
    tasksArray[0] = MeasureTask;
    tasksArray[1] = ComputeTask;
    tasksArray[2] = DisplayTask;
    tasksArray[3] = AnnunciateTask;
    tasksArray[4] = StatusTask;
    tasksArray[5] = NullTask;

    for(int i = 0; i < NUM_TASKS; i++) { // QUEUE
        tasksArray[i].taskFuncPtr(tasksArray[i].taskDataPtr);
    }
    updateCounter();
}