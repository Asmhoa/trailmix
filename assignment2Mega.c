// Assignment 2
// Code for ATMega
// Eddy, Kyuri, Amol

#include <stdio.h>
#include <stdbool.h>
#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <TimerThree.h> // SYSTEM TICK
#include <arduinoFFT.h> //

arduinoFFT FFT = arduinoFFT();

/* SERIAL COMMUNICATIONS CONSTANT */
#define START_MESSAGE '>'
#define END_TERM ','
#define END_MESSAGE '<'
#define EKG_SAMPLES 256
#define SAMPLING_FREQUENCY 7566

double unoCounter = 0.5;
int dismissCounter = 0;
uint16_t identifier = 0;
char requestingTaskID = 0;
char requestedFunction = 0;
int incomingData = 0;
const int sizeBuf = 8;


// ================================================ TFT ================================================================
    /* INITIALIZATION - SETUP TFT DISPLAY */
    #define LCD_CS A3 // Chip Select goes to Analog 3
    #define LCD_CD A2 // Command/Data goes to Analog 2
    #define LCD_WR A1 // LCD Write goes to Analog 1
    #define LCD_RD A0 // LCD Read goes to Analog 0
    #define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

    #define YP A3  // must be an analog pin, use "An" notation!
    #define XM A2  // must be an analog pin, use "An" notation!
    #define YM 9   // can be a digital pin
    #define XP 8   // can be a digital pin

    // Assign human-readable names to some common 16-bit color values:
    #define BLACK   0x0000
    #define BLUE    0x001F
    #define RED     0xF800
    #define GREEN   0x07E0
    #define CYAN    0x07FF
    #define MAGENTA 0xF81F
    #define YELLOW  0xFFE0
    #define WHITE   0xFFFF
    #define ORANGE  0xFD40

    // Color definitions
    #define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
    #define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
    #define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
    #define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
    #define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
    #define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
    #define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
    #define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
    #define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
    #define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
    #define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
    #define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
    #define ILI9341_RED         0xF800      /* 255,   0,   0 */
    #define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
    #define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
    #define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
    #define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
    #define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
    #define ILI9341_PINK        0xF81F

    #define MINPRESSURE 10
    #define MAXPRESSURE 1000

    //Touch For New ILI9341 TP
    #define TS_MINX 120
    #define TS_MAXX 900

    #define TS_MINY 70
    #define TS_MAXY 920
    // We have a status line for like, is FONA working
    #define STATUS_X 10
    #define STATUS_Y 65

    Elegoo_GFX_Button menuButtons[4];

    Elegoo_GFX_Button backButton[1]; 
    Elegoo_GFX_Button dismissButton[1];

    Elegoo_GFX_Button measureSelectButtons[5]; // Increased size by one to accommodate EKG
    // Elegoo_GFX_Button AcknowledgeButton;

    Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

    TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// ================================================ TFT END ================================================================

// ============================================ GLOBAL VARIABLES ===========================================================
    // Measurements
    unsigned int temperatureRawBuf[8] = {75, 75, 75, 75, 75, 75, 75, 75};
    unsigned int bloodPressRawBuf[16] = {80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80};
    unsigned int pulseRateRawBuf[8] = {0};
    unsigned int respirationRateRawBuf[8] = {0};
        // EKG: raw buf
    unsigned int EKGRawBuf[EKG_SAMPLES] = {0};

    int currTemp;
    int currSys;
    int currDia;
    int currPr;
    int currResp;
    int currEKG;
    char mode = 'N';

    int warningLED = 45;

    // index for flags (0 -> 7) : measure, compute, display, annunciate, status, keypad, EKG Capture, EKG Process
    // TODO: WHAT WOULD BE THE INITIAL FLAG FOR EKG CAPTURE?
    unsigned short addFlags[8] = {1, 1, 1, 0, 0, 1, 0, 0};
    unsigned short removeFlags[8] = {0};


    // Display
    double tempCorrectedBuf[8] = {0};
    double bloodPressCorrectedBuf[16] = {0};
    double pulseRateCorrectedBuf[8] = {0};
    double respirationRateCorrectedBuf[8] = {0};
        // EKG: corrected freq buf
    unsigned int EKGFreqBuf[16] = {0};

    // Status
    unsigned short batteryState = 200;

    // Alarms
    unsigned char bpOutOfRange = 0,
        bpOutOfRange2 = 0,
        tempOutOfRange = 0,
        pulseOutOfRange = 0,
        rrOutOfRange = 0,
            // EKG: out of range
        EKGOutOfRange = 0;

    // Warning
    bool bpHigh = false,
        bpHigh2 = false,
        tempHigh = false,
        pulseLow = false,
        rrLow = false,
        battLow = false,
        state = false,
        dismiss = false,
            // EKG: too high anfd too low
        EKGLow = false,
        EKGHigh = false;

    // TFT Keypad
    unsigned short functionSelect = 0,
        measurementSelection = 5, alarmAcknowledge = 0;

    /* INITIALIZATION - MAKE TASK BLOCK STRUCTURE */
    struct TaskStruct {
        void (*taskFuncPtr)(void*);
        void* taskDataPtr;
        struct TaskStruct* next;
        struct TaskStruct* prev;
    }; typedef struct TaskStruct TCB;


    // HEAD OF TCB DOUBLE LINKED LIST 
    TCB* linkedListHead = NULL;
    // Traversing Pointer
    TCB* currPointer = NULL;


    /* INITIALIZATION - MAKE TASK DATA POINTER BLOCKS */
    struct DataForMeasureStruct {
        unsigned int* temperatureRawPtr;
        unsigned int* bloodPressRawPtr;
        unsigned int* pulseRateRawPtr;
        unsigned int* respirationRateRawPtr;
        unsigned short* measurementSelectionPtr;
    }; typedef struct DataForMeasureStruct MeasureTaskData;

    struct DataForComputeStruct {
        unsigned int* temperatureRawPtr;
        unsigned int* bloodPressRawPtr;
        unsigned int* pulseRateRawPtr;
        unsigned int* respirationRateRawPtr;
            // EKG: Pointer to raw buf - EKGRawBuf
        unsigned int* EKGRawBufPtr;
        double* temperatureCorrectedPtr;
        double* bloodPressCorrectedPtr;
        double* prCorrectedPtr;
        double* respirationRateCorrectedPtr;
            // EKG: Pointer to corrected freq buf - EKGFreqBuf
        unsigned int* EKGFreqBufPtr;
        unsigned short* measurementSelectionPtr;
    }; typedef struct DataForComputeStruct ComputeTaskData;

    struct DataForDisplayStruct {
        double* temperatureCorrectedPtr;
        double* bloodPressCorrectedPtr;
        double* prCorrectedPtr;
        double* respirationRateCorrectedPtr;
            // EKG: Pointer to corrected freq buf - EKGFreqBuf
        unsigned int* EKGFreqBufPtr;
        unsigned short* batteryStatePtr;
    }; typedef struct DataForDisplayStruct DisplayTaskData;

    struct DataForWarningAlarmStruct {
        double* temperatureCorrectedPtr;
        double* bloodPressCorrectedPtr;
        double* prCorrectedPtr;
        double* respirationRateCorrectedPtr;
            // EKG: Pointer to corrected freq buf - EKGFreqBuf
        unsigned int* EKGFreqBufPtr;
        unsigned short* batteryStatePtr;
    }; typedef struct DataForWarningAlarmStruct WarningAlarmTaskData;

    struct DataForStatusStruct {
        unsigned short* batteryStatePtr;
    }; typedef struct DataForStatusStruct StatusTaskData;

    struct DataForKeypadStruct {
        char* modePtr;
        unsigned short* measurementSelectionPtr;
        unsigned short* alarmAcknowledgePtr;
    }; typedef struct DataForKeypadStruct KeypadTaskData;

    struct DataForCommsStruct {
        unsigned short* measurementSelectionPtr;
        double* temperatureCorrectedPtr;
        double* bloodPressCorrectedPtr;
        double* prCorrectedPtr;
        double* respirationRateCorrectedPtr;
            // EKG: Pointer to raw buf - EKGRawBuf: WHY!??!?
        unsigned int* EKGRawBufPtr;
    }; typedef struct DataForCommsStruct CommsTaskData;

    // EKG Data struct
    struct DataForEKGStruct {
            // EKG: Pointer to raw buf - EKGRawBuf
        unsigned int* EKGRawBufPtr;
            // EKG: Pointer to corrected freq buf - EKGFreqBuf
        unsigned int* EKGFreqBufPtr;
    }; typedef struct DataForEKGStruct EKGTaskData;

    // TODO: Command Data Struct
    struct DataForCommandStruct {

    }; typedef struct DataForcCommandStruct CommandTaskData;

    // TODO: Remote Comm Data Struct
    struct DataForRemoteCommStruct {

    }; typedef struct DataForRemoteCommStruct RemoteCommTastData;

    // TODO: Traffic Management Data Struct
    struct DataForTrafficStruct {

    }; typedef struct DataForTrafficStruct TrafficTaskData;


    /* INITIALIZATION - MAKE INSTANCES OF TCB */
    TCB MeasureTask;
    TCB ComputeTask;
    TCB DisplayTask;
    TCB AnnunciateTask;
    TCB StatusTask;
    TCB KeypadTask;
        // EKG: Capture Task
    TCB EKGCaptureTask;
        // EKG: Process Task
    TCB EKGProcessTask;
    TCB NullTask;

    // Data
    MeasureTaskData dataForMeasure;
    ComputeTaskData dataForCompute;
    DisplayTaskData dataForDisplay;
    WarningAlarmTaskData dataForWarningAlarm;
    StatusTaskData dataForStatus;
    KeypadTaskData dataForKeypad;
    CommsTaskData dataForComms;
        // EKG: data struct initialization
    EKGTaskData dataForEKG;

// ============================================ GLOBAL VARIABLES END ===========================================================

/* INITIALIZATION - FUNCTION DEFINITIONS */
void requestMessage(String taskID, String funcToRun, String data) {
    Serial1.println(START_MESSAGE + taskID + END_TERM + funcToRun + END_TERM
        + data + END_MESSAGE);
}

// This function should only be run after a request message statement
void parseMessage() {
    while(0 == Serial1.available()) {
        // It freezes the system till the UNO responds
    }

    //  read incoming byte from the mega
    while(Serial1.available() > 0) {
        char currChar = Serial1.read();
        if ('>' == currChar) {
            delay(20);
            requestingTaskID = (char)Serial1.read();
            Serial1.read(); //Read over terminator

            delay(20);
            requestedFunction = Serial1.read();
            Serial1.read();

            delay(20);
            incomingData = Serial1.parseInt();
            Serial1.read();

            // Serial.println(incomingData);
        }
    }
}


void EKGCaptureDataFunc(void* data) {
    EKGTaskData* dataToMeasure = (EKGTaskData*)data;
    EKGTaskData dataStruct = *dataToMeasure;

    // Request EKG measurement
    requestMessage("M", "E", "");

    // Now set the flag to run EKG Process
    addFlags[7] = 1;

    // Parse EKG return message. Fetch data for buffer, get all 256.
    while (0 == Serial1.available()) {
        // It freezes the system till the UNO responds
    }
    int i = 0;
    // TODO: What if this clashes with counterUpdate sending a U to uno every half a second? 
    while (Serial1.available() > 0) {
        delay(20);
        if(i < EKG_SAMPLES) {
            *(dataStruct.EKGRawBufPtr + i) = Serial1.parseInt();
            //EKGRawBuf[i] = Serial1.parseInt();
            Serial1.read();
            i++;
        }
        // Serial.println(incomingData);
    }
}

void EKGProcessDataFunc(void* data) {

    EKGTaskData* dataToMeasure = (EKGTaskData*)data;
    EKGTaskData dataStruct = *dataToMeasure;

    unsigned int EKGFreqBufTemp[16];
    for (int i = 0; i < 16; i++) {
        EKGFreqBufTemp[i] = *(dataStruct.EKGFreqBufPtr + i);
    }

    // Use Fast Fourier Transform here to get frequency
    double vReal[EKG_SAMPLES];
    double vImag[EKG_SAMPLES];
    // Transferring analog reader values to vReal for FFT
    for (int i = 0; i < EKG_SAMPLES; i++) {
        //vReal[i] = EKGRawBuf[i];
        vReal[i] = *(dataStruct.EKGRawBufPtr + i);
        vImag[i] = 0;
    }
    FFT.Windowing(vReal, EKG_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, EKG_SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, EKG_SAMPLES);
    double peak = FFT.MajorPeak(vReal, EKG_SAMPLES, SAMPLING_FREQUENCY);

    // Store this in the EKGFreqBuf, keep a 16-element buffer    
    for (int i = 15; i > 0; i--) {
        //EKGFreqBuf[i] = EKGFreqBuf[i - 1];
        EKGFreqBufTemp[i] = EKGFreqBufTemp[i-1];
    }
    EKGFreqBufTemp[0] = (int) peak;
    for (int i = 0; i < 16; i++) {
        *(dataStruct.EKGFreqBufPtr + i) = EKGFreqBufTemp[i];
    }
}

void measureDataFunc(void* data) {
    // Dereference data to use it
    // Serial.println("Measure task started");
    // delay(20);
    MeasureTaskData* dataToMeasure = (MeasureTaskData*)data;
    MeasureTaskData dataStruct = *dataToMeasure;
    
    // Required temporary variables
    int i;
    unsigned int temperatureRawBufTemp[sizeBuf];
    unsigned int systolicPressRawBuf[sizeBuf];
    unsigned int diastolicPressRawBuf[sizeBuf];
    unsigned int pulseRateRawBufTemp[sizeBuf];
    unsigned int respirationRateRawBufTemp[sizeBuf];
    int prMeasured;
    int tempMeasured;
    int respMeasured;

    // Populate temporary variables
    for(i = 0; i < sizeBuf * 2; i++) {
        if(i < sizeBuf) {
            temperatureRawBufTemp[i] = *(dataStruct.temperatureRawPtr + i);
            systolicPressRawBuf[i] = *(dataStruct.bloodPressRawPtr + i);
            pulseRateRawBufTemp[i] = *(dataStruct.pulseRateRawPtr + i);
            respirationRateRawBufTemp[i] = *(dataStruct.respirationRateRawPtr + i);
        } else {
            diastolicPressRawBuf[i- sizeBuf] = *(dataStruct.bloodPressRawPtr + i);
        }
    }
    
    // Get most recent values --- SEND THESE TO UNO
    currTemp = temperatureRawBufTemp[0];
    currSys = systolicPressRawBuf[0];
    currDia = diastolicPressRawBuf[0];
    currPr = pulseRateRawBufTemp[0];
    currResp = respirationRateRawBufTemp[0];

    // Tell the Uno to do all the work
    switch(measurementSelection) {
        case 0:
            requestMessage("M", "T", String(currTemp));
            parseMessage();
            tempMeasured = incomingData;
            delay(20);
            break;
        case 1:
            requestMessage("M", "S", String(currSys));
            parseMessage();
            currSys = incomingData;
            delay(20);
            requestMessage("M", "D", String(currDia));
            parseMessage();
            currDia = incomingData;
            break;
        case 2:
            requestMessage("M", "P", String(currPr));
            parseMessage();
            prMeasured = incomingData;
            delay(20);
            break;
        case 3:
            // Added new Respiration case
            requestMessage("M", "R", String(currResp));
            parseMessage();
            respMeasured = incomingData;
            delay(20);
            break;
        case 4:
            // TODO: Run EKG TCB?? Or should Measurement also have the power to request for EKG reading
            addFlags[6] = 1;
            // currEKG =
        case 5:
            requestMessage("M", "T", String(currTemp));
            parseMessage();
            tempMeasured = incomingData;
            delay(20);
            requestMessage("M", "S", String(currSys));
            parseMessage();
            currSys = incomingData;
            delay(20);
            requestMessage("M", "D", String(currDia));
            parseMessage();
            currDia = incomingData;
            delay(20);
            requestMessage("M", "R", String(currResp));
            parseMessage();
            respMeasured = incomingData;
            delay(20);
            requestMessage("M", "P", String(currPr));
            parseMessage();
            prMeasured = incomingData;
            delay(20);
            // TODO: Run EKG TCB?? Or should Measurement also have the power to request for EKG reading
            addFlags[6] = 1;
            // currEKG = 
            delay(20);
            break;
    }
    
    // RESET message
    requestingTaskID = 0;
    requestedFunction = 0;
    incomingData = 0;

    // Check out of range for pulse rate
    float lowPr = currPr * 0.85;
    float highPr = currPr * 1.15;
    
    if((prMeasured < lowPr ) || (prMeasured > highPr)) {
        currPr = prMeasured;
    }

    // Check out of range for temperature
    float lowTemp = currTemp * 0.85;
    float highTemp = currTemp * 1.15;
    
    if((tempMeasured < lowTemp ) || (tempMeasured > highTemp)) {
        currTemp = tempMeasured;
    }

    // Check out of range for respiration rate
    float lowResp = currResp * 0.85;
    float highResp = currResp * 1.15;

    if((respMeasured < lowResp) || (respMeasured > highResp)) {
        currResp = respMeasured;
    }
        
        
    // Shift older values back in the temporary buffer
    for(i = sizeBuf - 1; i > 0; i--) {
        systolicPressRawBuf[i] = systolicPressRawBuf[i - 1];
        diastolicPressRawBuf[i] = diastolicPressRawBuf[i - 1];
        if(currTemp != temperatureRawBufTemp[0]) {
            temperatureRawBufTemp[i] = temperatureRawBufTemp[i-1];
        }
        if(currPr != pulseRateRawBufTemp[0]) {
            pulseRateRawBufTemp[i] = pulseRateRawBufTemp[i - 1];
        }
        if(currResp != respirationRateRawBufTemp[0]) {
            respirationRateRawBufTemp[i] = respirationRateRawBufTemp[i - 1];
        }
    }

    // Update new values
    temperatureRawBufTemp[0] = currTemp;
    systolicPressRawBuf[0] = currSys;
    diastolicPressRawBuf[0] = currDia;
    pulseRateRawBufTemp[0] = currPr;
    respirationRateRawBufTemp[0] = currResp;
    
    // Update the data pointers
    for(i = 0; i < sizeBuf * 2; i++) {
        if(i < sizeBuf) {
            *(dataStruct.temperatureRawPtr + i) = temperatureRawBufTemp[i];
            *(dataStruct.bloodPressRawPtr + i) = systolicPressRawBuf[i];
            *(dataStruct.pulseRateRawPtr + i) = pulseRateRawBufTemp[i];
            *(dataStruct.respirationRateRawPtr + i) = respirationRateRawBufTemp[i];
        } else {
            *(dataStruct.bloodPressRawPtr + i) = diastolicPressRawBuf[i- sizeBuf];
        }
    }
    if(dismiss) {
       dismissCounter++; 
    }
    
    // Change Compute Flag to addTask when new data is measured
    addFlags[1] = 1;
    // Serial.println("Measure task Ended");
    // delay(20);
}

void computeDataFunc(void* x) {
    // Serial.println("!Compute task started");
    // Dereferencing void pointer to ComputeStruct
    ComputeTaskData* data = (ComputeTaskData*)x;
    ComputeTaskData dataStruct = *data;

    int rawTemp = *(dataStruct.temperatureRawPtr);
    int rawSys = *(dataStruct.bloodPressRawPtr);
    int rawDia = *(dataStruct.bloodPressRawPtr + 8);
    int rawPr = *(dataStruct.pulseRateRawPtr);
    int rawResp = *(dataStruct.respirationRateRawPtr);

    // Shift older values back in the computed values buffer
    for(int i = sizeBuf - 1; i > 0; i--) {
        tempCorrectedBuf[i] = tempCorrectedBuf[i - 1];
        bloodPressCorrectedBuf[i] = bloodPressCorrectedBuf[i - 1];
        bloodPressCorrectedBuf[i + sizeBuf] = bloodPressCorrectedBuf[i + sizeBuf - 1];
        pulseRateCorrectedBuf[i] = pulseRateCorrectedBuf[i - 1];
        respirationRateCorrectedBuf[i] = respirationRateCorrectedBuf[i - 1];
    }

    // Computing and converting temperatureRaw to unsigned char* (Celsius)
    double correctedTemp = 5 + 0.75 * rawTemp;
    tempCorrectedBuf[0] = correctedTemp;
    
    // Computing and converting systolic pressure to unsigned char*
    double correctedSys = 9 + 2 * rawSys;
    bloodPressCorrectedBuf[0] = correctedSys;
    
    // Computing and converting diastolic pressure to unsigned char*
    double correctedDia =  6 + 1.5 * rawDia;
    bloodPressCorrectedBuf[sizeBuf] = correctedDia;

    // Computing and converting pulse rate to unsigned char*
    double correctedPr =  8 + 3 * rawPr;
    pulseRateCorrectedBuf[0] = correctedPr;

    double rrCorrected = 7 + 3 * rawResp;
    respirationRateCorrectedBuf[0] = rrCorrected;
    // Serial.println("?Compute task ended");
    // delay(20);
}


// ENTER THIS ONLY WHEN ANNUNCIATION IS PRESSED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void displayDataFunc(void* x) {

    // Serial.println("Display task started");
    DisplayTaskData* data = (DisplayTaskData*)x;
    DisplayTaskData dataStruct = *data;

    tft.setTextSize(2);
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);

    // Create buttons
    menuButtons[0].initButton(&tft, 125, 55, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "MENU", 2);
    menuButtons[0].drawButton();

    menuButtons[1].initButton(&tft, 125, 125, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "ANNUNC", 2);
    menuButtons[1].drawButton();

    menuButtons[2].initButton(&tft, 125, 195, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "EM1", 2);
    menuButtons[2].drawButton();

    menuButtons[3].initButton(&tft, 125, 265, 180, 50, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "EM2", 2);
    menuButtons[3].drawButton();
    // Serial.println("Display task ended");
    // delay(20);
}

void annunciateDataFunc(void* x) {
    // Serial.println("Annunciate started");
    // delay(20);
    
    // measurementSelection = 4; // Get all values

    // Dereferencing void pointer to WarningStruct
    WarningAlarmTaskData* data = (WarningAlarmTaskData*)x;
    WarningAlarmTaskData dataStruct = *data;

    tft.setTextSize(2); // Is font size too small?
    tft.fillScreen(BLACK);
    tft.setCursor(0, 0);

    switch(measurementSelection) {
        case 0: // Temperature
            tempOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);
            if (tempHigh) {
                tft.setTextColor(RED); // Add acknowledgement event??
            } 
            tft.println("Temp: " + (String)*dataStruct.temperatureCorrectedPtr + " C");
            break;
        case 1: // Blood pressure
            // Sys
            bpOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);    
            if (bpHigh) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Sysl.: " + (String)*dataStruct.bloodPressCorrectedPtr + " mm Hg");
            // Dia
            bpOutOfRange2 ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);    
            if (bpHigh2) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Diasl.: " + (String)*(dataStruct.bloodPressCorrectedPtr + 8) + " mm Hg");
            break;
        case 2: // Pulse
            pulseOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);        
            if (pulseLow) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Pulse: " + (String)*dataStruct.prCorrectedPtr + " BPM");
            break;
        case 3: // Respiration
            rrOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);        
            if (rrLow) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Resp.: " + (String)*dataStruct.respirationRateCorrectedPtr + " RR");
            break;
        case 4: // EKG
            EKGOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);        
            // if (EKGLow || EKGHigh) { 
            //     tft.setTextColor(RED); // Add acknowledgement event??
            // }
            tft.println("EKG: " + (String)*dataStruct.EKGFreqBufPtr + " HZ");
        case 5: // Everything
            mode = 'A';

             // Temperature
            tempOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);
            if (tempHigh) {
                tft.setTextColor(RED); // Add acknowledgement event??
            } 
            tft.println("Temp: " + (String)*dataStruct.temperatureCorrectedPtr + " C");

            // Blood Pressure
            bpOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);    
            if (bpHigh) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Sysl.: " + (String)*dataStruct.bloodPressCorrectedPtr + " mm Hg");
            // Blood Pressure
            bpOutOfRange2 ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);    
            if (bpHigh2) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Diasl.: " + (String)*(dataStruct.bloodPressCorrectedPtr + 8) + " mm Hg");

            // Pulse
            pulseOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);        
            if (pulseLow) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Pulse: " + (String)*dataStruct.prCorrectedPtr + " BPM");

            // Respiration Rate
            rrOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);        
            if (rrLow) { 
                tft.setTextColor(RED); // Add acknowledgement event??
            }
            tft.println("Resp.: " + (String)*dataStruct.respirationRateCorrectedPtr + " RR");

            // EKG Readings
            EKGOutOfRange ? tft.setTextColor(ORANGE) : tft.setTextColor(GREEN);        
            // if (EKGLow || EKGHigh) { 
            //     tft.setTextColor(RED); // Add acknowledgement event??
            // }
            tft.println("EKG: " + (String)*dataStruct.EKGFreqBufPtr + " HZ");

            // Battery
            battLow ? tft.setTextColor(RED) : tft.setTextColor(GREEN);
            tft.println("Battery: " + (String)*dataStruct.batteryStatePtr);
            break;
        default:
            break;
    }

    if ('A' == mode) {
        addFlags[0] = 1;
        addFlags[1] = 1;
        addFlags[2] = 0;
        addFlags[3] = 1;
        addFlags[4] = 1;
        addFlags[5] = 1;
        addFlags[6] = 0;
        addFlags[7] = 0;

        // Back Button
        backButton[0].initButton(&tft, 125, 265, 180, 35, ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE, "MAIN", 2);
        backButton[0].drawButton();
    }
    // delay(50);


    // Acknowledge button SHOULD ONLY APPEAR WHEN THERE IS AN ALARM RINGING
    // AcknowledgeButton.initButton(&tft, 120, 265, 240, 70, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "DISMISS", 2);
    // AcknowledgeButton.drawButton();

    // bool staySensingPress = true; 
    // while(staySensingPress) {
    //     TSPoint p = ts.getPoint();
    //     if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    //         // scale from 0->1023 to tft.width
    //         p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    //        // p.y = (tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
    //         p.y = (map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
    //     }
    //     if (AcknowledgeButton.contains(p.x, p.y)) {
    //             AcknowledgeButton.press(true); // tell the button it is pressed
    //             dismiss = true;
    //             //Dismiss the alarm for five seconds

    //             //If signal value remains out of range for five measurements, resume alarm annunciation

    //     } else {
    //             AcknowledgeButton.press(false);  // tell the button it is NOT pressed
    //             dismiss = false;
    //     }
    // }

    // check temperature:
    double normalizedTemp = *dataStruct.temperatureCorrectedPtr;
    if ((normalizedTemp < (36.1 * 0.95)) || (normalizedTemp > (37.8 * 1.05))) {
        // TURN TEXT RED---------
        tempOutOfRange = 1;
        if ((normalizedTemp < (36.1 * 0.85)) || (normalizedTemp > (37.8 * 1.15))) {
            tempHigh = true;
            // Trigger alarm event

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
    int normalizedSystolic = *dataStruct.bloodPressCorrectedPtr;
    if ((normalizedSystolic > (120 * 0.95)) || (normalizedSystolic > (130 * 1.05))) {
        // TURN TEXT RED
        bpOutOfRange = 1;
        if ((normalizedSystolic < (120 * 0.8)) || (normalizedSystolic > (130 * 1.2))) {
            bpHigh = true;
            // Trigger alarm event

        } else {
            bpHigh = false;
        }
    } else {
        bpOutOfRange = 0;
        bpHigh = false;
    }

    
    /*  IF PRESSURE CRITICALLY HIGH, need interrupt flashing warnings -----------------------------------
        User can acknowledge it to DISABLE INTERRUPT
    */

    double normalizedDiastolic = *(dataStruct.bloodPressCorrectedPtr + 8);
    if ((normalizedDiastolic < (70 * 0.95)) || (normalizedDiastolic > (80 * 1.05))) {
        // TURN TEXT RED
        bpOutOfRange2 = 1;
        if ((normalizedDiastolic < (70 * 0.8)) || (normalizedDiastolic > (80 * 1.2))) {
            bpHigh2 = true;
            // Trigger alarm event

        } else {
            bpHigh2 = false;
        }
    } else {
        bpOutOfRange2 = 0;
        bpHigh2 = false;
    }
    
    // pulse rate 
    // HEALTHY PULSE => Between 60 and 100
    // IF PULSE < 30, WARNING

        /*  IF PULSE CRITICALLY LOW, need interrupt flashing warnings -----------------------------------
            User can acknowledge it to DISABLE INTERRUPT
        */

    int normalizedPulse = *dataStruct.prCorrectedPtr;
    if ((normalizedPulse < (60 * 0.95)) || (normalizedPulse > (100 * 1.05))) {
        pulseOutOfRange = 1;
        if ((normalizedPulse < (60 * 0.85)) || (normalizedPulse > (100 * 1.15))) {
            pulseLow = true;
            // Trigger alarm event

        } else {
            pulseLow = false;
        }
    } else {
        pulseOutOfRange = 0;
        pulseLow = false;
    }

    int normalizedResp = *dataStruct.respirationRateCorrectedPtr;
    if ((normalizedResp < (12 * 0.95)) || (normalizedResp > (25 * 1.05))) {
        rrOutOfRange = 1;
        if ((normalizedResp < (12 * 0.85)) || (normalizedResp > (25 * 1.15))) {
            rrLow = true;
            // Trigger alarm event

        } else {
            rrLow = false;
        }
    } else {
        rrOutOfRange = 0;
        rrLow = false;
    }

    if (batteryState < 40) {
        battLow = true;
        // Trigger alarm event
    } else {
        battLow = false;
    }

    int normalizedEKG = *dataStruct.EKGFreqBufPtr;
    if(normalizedEKG < 35 || normalizedEKG > 3750) {
        EKGOutOfRange = 1;
    }

    Serial.println("Annunciate ended");


    //DISMISS BUTTON
    if((bpHigh || bpHigh2 || tempHigh || pulseLow || rrLow) && dismissCounter == 0 && !dismiss) {
        dismissButton[0].initButton(&tft, 125, 195, 180, 35, ILI9341_WHITE, ILI9341_RED, ILI9341_WHITE, "DISMISS", 2);
        dismissButton[0].drawButton();
    } else if (dismiss && dismissCounter < 5) {
        bpHigh = false;
        bpHigh2 = false;
        tempHigh = false;
        pulseLow = false;
        rrLow = false;
    } else {
        // BACK TO ALARM OR NORMAL
        dismissCounter = 0;
        dismiss = false;
    }

}

void statusDataFunc(void* x) {
    // Dereferencing void pointer to WarningStruct
    StatusTaskData* data = (StatusTaskData*)x;
    StatusTaskData dataStruct = *data;
    
    *(dataStruct.batteryStatePtr) = 200 - (int)(unoCounter / 5);
}

void KeypadDataFunc(void* x) {
    // Serial.println("Keypad Task started");
    KeypadTaskData* data = (KeypadTaskData*)x;
    KeypadTaskData dataStruct = *data;

    if('A' == mode) {

        int tempCounter = unoCounter;
        bool keepSensing = true;
        while (keepSensing && unoCounter - tempCounter <= 2) {
            digitalWrite(13, HIGH);
            TSPoint p = ts.getPoint();
            digitalWrite(13, LOW);
            pinMode(XM, OUTPUT);
            pinMode(YP, OUTPUT);
            if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
                // scale from 0->1023 to tft.width
                p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
                p.y = (map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
            }

            if (backButton[0].contains(p.x, p.y)) {
                backButton[0].press(true); // tell the button it is pressed
                // keepSensing = false;
            } else {
                backButton[0].press(false);  // tell the button it is NOT pressed
            }

            if (backButton[0].justPressed()) {
                // GO BACK TO MAIN MENU, SHOULD USE SCHEDULER??
                tft.setCursor(0,0);
                tft.fillScreen(BLACK);
                Serial.println("BACK BUTTON IS PRESSED");
                mode = 'B';
                measurementSelection = 6; // Changed from 5 to 6

                // ADDFLAG FOR DISPLAY AND TOUCHPAD? NEED TO BRING USER BACK TO MAIN MENU
                addFlags[0] = 1;
                addFlags[1] = 1;
                addFlags[2] = 1;
                addFlags[3] = 0;
                addFlags[4] = 0;
                addFlags[5] = 1;
                addFlags[6] = 0;
                addFlags[7] = 0;
            }

            if (dismissButton[0].contains(p.x, p.y)) {
                dismissButton[0].press(true); // tell the button it is pressed
                dismiss = true;
            } else {
                dismissButton[0].press(false);  // tell the button it is NOT pressed
            }
            
            if (dismissButton[0].justPressed()) {
                dismissButton[0].initButton(&tft, 0, 0, 0, 0, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "", 2);
                dismissButton[0].drawButton();
                tft.setCursor(0,0);
                // STAY IN ANNUNCIATE BUT REMOVE ALARM
                bpHigh = false;
                bpHigh2 = false;
                tempHigh = false;
                pulseLow = false;
                rrLow = false;
                Serial.println("DISMISS BUTTON IS PRESSED");
            }
        }
    } else if ('B' == mode) {
        mode = 'N';
        addFlags[0] = 1;
        addFlags[1] = 1;
        addFlags[2] = 1;
        addFlags[3] = 0;
        addFlags[4] = 0;
        addFlags[5] = 1;
        addFlags[6] = 0;
        addFlags[7] = 0;
    } else {
        bool keepSensing = true;
        while (keepSensing) {
            digitalWrite(13, HIGH);
            TSPoint p = ts.getPoint();
            digitalWrite(13, LOW);

            pinMode(XM, OUTPUT);
            pinMode(YP, OUTPUT);
            
            if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
                // scale from 0->1023 to tft.width
                p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
                p.y = (tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
            }

            // go thru all the buttons, checking if they were pressed
            for (uint8_t b = 0; b < 4; b++) {
                if (menuButtons[b].contains(p.x, p.y)) {
                    // Serial.print("Pressing: "); Serial.println(b);
                    menuButtons[b].press(true);  // tell the button it is pressed
                    keepSensing = false;

                } else {
                    menuButtons[b].press(false);  // tell the button it is NOT pressed
                }
            }

            for (uint8_t b = 0; b < 4; b++) {
                if (menuButtons[b].justPressed()) {
                    if(b == 3)  { // Menu
                        tft.setCursor(0,0);
                        tft.fillScreen(BLACK);
                        mode = 'N';
                        menuView();
                        break;
                    } else if (b == 2) { // Annunciate
                        tft.setCursor(0,0);
                        tft.fillScreen(BLACK);
                        // Setting flags in schedule 
                        // First measure everything, then annunciate everything
                        measurementSelection = 5;
                        // TODO: CORRECT THESE FLAGS, need to include EKG
                        addFlags[0] = 1;
                        addFlags[1] = 1;
                        addFlags[3] = 1;
                        addFlags[6] = 1;
                        //addFlags[7] = 1;
                        break;
                    } 
                }
            }
        }
    }
    // delay(20);
    // Serial.println("Keypad Task ended");
}

void menuView() {
    // Serial.println("Menu View started");
    measureSelectButtons[0].initButton(&tft, 125, 40, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Temp", 2);
    measureSelectButtons[0].drawButton();

    measureSelectButtons[1].initButton(&tft, 125, 95, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Blood Pr.", 2);
    measureSelectButtons[1].drawButton();

    measureSelectButtons[2].initButton(&tft, 125, 150, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Pulse", 2);
    measureSelectButtons[2].drawButton();

    measureSelectButtons[3].initButton(&tft, 125, 205, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "Resp.", 2);
    measureSelectButtons[3].drawButton();

    measureSelectButtons[4].initButton(&tft, 125, 260, 180, 35, ILI9341_WHITE, ILI9341_BLUE, ILI9341_WHITE, "EKG", 2);
    measureSelectButtons[4].drawButton(); 
    
    // Mode is N
    bool staySensingPress = true;
    while (staySensingPress) {

        digitalWrite(13, HIGH);
        TSPoint p = ts.getPoint();
        digitalWrite(13, LOW);

        pinMode(XM, OUTPUT);
        pinMode(YP, OUTPUT);

        if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
            // scale from 0->1023 to tft.width
            p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
            p.y = (tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
        }

        for (uint8_t b = 0; b < 5; b++) {
            if (measureSelectButtons[b].contains(p.x, p.y)) {
                // Serial.print("Pressing: "); Serial.println(b);
                measureSelectButtons[b].press(true); // tell the button it is pressed
                tft.setCursor(0,0);
                tft.fillScreen(BLACK);
                addFlags[0] = 1;
                addFlags[1] = 1;
                if(0 == b) {
                    addFlags[6] = 1;
                }
                staySensingPress = false;
            } else {
                menuButtons[b].press(false);  // tell the button it is NOT pressed
            }
        }

        for (uint8_t b = 0; b < 5; b++) {
            // TODO: FIX THE FLAGS ARRAY
            if (measureSelectButtons[b].justPressed()) {
                mode = 'A';
                if (4 == b) { // Temperature
                    staySensingPress = false;
                    measurementSelection = 0;
                    addFlags[3] = 1;
                    // Go to Annunciate

                } else if (3 == b) { // BP
                    staySensingPress = false;
                    measurementSelection = 1;
                    addFlags[3] = 1;
                    // Go to Annunciate

                } else if (2 == b) { // Pulse
                    measurementSelection = 2;
                    staySensingPress = false;
                    addFlags[3] = 1;
                    // Go to Annunciate

                } else if (1 == b) { // respiration
                    measurementSelection = 3;
                    staySensingPress = false;
                    addFlags[3] = 1;
                    // Go to Annunciate
                } else if (0 == b) { // EKG
                    measurementSelection = 4;
                    staySensingPress = false;
                    addFlags[3] = 1;
                }
            }
        }
    }
}

// Delay for 500ms and update counter/time on peripheral system
void updateCounter(void) {
    Serial1.println('U');
    unoCounter += 0.5;


    //FLASH FOR WARNING
    if(tempOutOfRange) { // EVERY 1 SEC
        digitalWrite(warningLED, state);
        if(0 == (int)unoCounter % 1) {
           state = !state; 
        }
    } else if(bpOutOfRange || bpOutOfRange2) { // EVERY 0.5 SEC
        digitalWrite(warningLED, state);
        state = !state;
    } else if(pulseOutOfRange) { // EVERY 2 SEC
        digitalWrite(warningLED, state);
        if(0 == (int)unoCounter % 2) {
            state = !state;
        } 
    } else {
        state = false;
    }
}

/* INITIALIZATION */
void setup(void) {
    // Setup communication
    Serial1.begin(9600);

    // Initialise Timer3
    Timer3.initialize(500000);
    Timer3.attachInterrupt(updateCounter);
    
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

    identifier = tft.readID();
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

    pinMode(warningLED, OUTPUT);
    

    // Set measurements to initial values
    // char* and char values are already set as global variables
    bpHigh = false;
    tempHigh = false;
    pulseLow = false;
    battLow = false;
    bpHigh2 = false;
    rrLow = false;
    state = false;
    dismiss = false;

    // Point data in data structs to correct information
    // Measure
    MeasureTaskData dataForMeasureTMP;
    dataForMeasureTMP.temperatureRawPtr = temperatureRawBuf; 
    dataForMeasureTMP.bloodPressRawPtr = bloodPressRawBuf;
    dataForMeasureTMP.pulseRateRawPtr = pulseRateRawBuf;
    dataForMeasureTMP.respirationRateRawPtr = respirationRateRawBuf;
    dataForMeasureTMP.measurementSelectionPtr = &measurementSelection;
    dataForMeasure = dataForMeasureTMP;

    // Compute
    ComputeTaskData dataForComputeTMP;
    dataForComputeTMP.temperatureRawPtr = temperatureRawBuf;
    dataForComputeTMP.bloodPressRawPtr = bloodPressRawBuf;
    dataForComputeTMP.pulseRateRawPtr = pulseRateRawBuf;
    dataForComputeTMP.EKGRawBufPtr = EKGRawBuf;
    dataForComputeTMP.temperatureCorrectedPtr = tempCorrectedBuf; // Already a pointer
    dataForComputeTMP.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
    dataForComputeTMP.prCorrectedPtr = pulseRateCorrectedBuf;
    dataForComputeTMP.respirationRateCorrectedPtr = respirationRateCorrectedBuf;
    dataForComputeTMP.EKGFreqBufPtr = EKGFreqBuf;
    dataForComputeTMP.measurementSelectionPtr = &measurementSelection;
    dataForCompute = dataForComputeTMP;

    // Display
    DisplayTaskData dataForDisplayTMP;
    dataForDisplayTMP.temperatureCorrectedPtr = tempCorrectedBuf; // Already a pointer
    dataForDisplayTMP.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
    dataForDisplayTMP.prCorrectedPtr = pulseRateCorrectedBuf;
    dataForDisplayTMP.respirationRateCorrectedPtr = respirationRateCorrectedBuf;
    dataForDisplayTMP.EKGFreqBufPtr = EKGFreqBuf;
    dataForDisplayTMP.batteryStatePtr = &batteryState;
    dataForDisplay = dataForDisplayTMP;

    // WarningAlarm
    WarningAlarmTaskData dataForWarningAlarmTMP;
    dataForWarningAlarmTMP.temperatureCorrectedPtr = tempCorrectedBuf;
    dataForWarningAlarmTMP.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
    dataForWarningAlarmTMP.prCorrectedPtr = pulseRateCorrectedBuf;
    dataForWarningAlarmTMP.respirationRateCorrectedPtr = respirationRateCorrectedBuf;
    dataForWarningAlarmTMP.EKGFreqBufPtr = EKGFreqBuf;
    dataForWarningAlarmTMP.batteryStatePtr = &batteryState;
    dataForWarningAlarm = dataForWarningAlarmTMP;

    // Status
    StatusTaskData dataForStatusTMP;
    dataForStatusTMP.batteryStatePtr = &batteryState;
    dataForStatus = dataForStatusTMP;

    // TFT Keypad
    KeypadTaskData dataForKeypadTMP;
    dataForKeypadTMP.modePtr = &mode;
    dataForKeypadTMP.measurementSelectionPtr = &measurementSelection;
    dataForKeypadTMP.alarmAcknowledgePtr = &alarmAcknowledge;
    dataForKeypad = dataForKeypadTMP;

    // Communications
    CommsTaskData dataForCommsTMP;
    dataForCommsTMP.measurementSelectionPtr = &measurementSelection;
    dataForCommsTMP.temperatureCorrectedPtr = tempCorrectedBuf;
    dataForCommsTMP.bloodPressCorrectedPtr = bloodPressCorrectedBuf;
    dataForCommsTMP.prCorrectedPtr = pulseRateCorrectedBuf;
    dataForCommsTMP.respirationRateCorrectedPtr = respirationRateCorrectedBuf;
    dataForCommsTMP.EKGRawBufPtr = EKGRawBuf;
    dataForComms = dataForCommsTMP;

    // EKG
    EKGTaskData dataForEKGTMP;
    dataForEKGTMP.EKGRawBufPtr = EKGRawBuf;
    dataForEKGTMP.tEKGFreqBufPtr = EKGFreqBuf;
    dataForEKG = dataForEKGTMP;
    
    // Assign values in TCB's
    // Measure
    TCB MeasureTaskTMP; // Getting an error if I try to use MeasureTask directly
    MeasureTaskTMP.taskFuncPtr = &measureDataFunc;
    MeasureTaskTMP.taskDataPtr = &dataForMeasure;
    MeasureTaskTMP.next = NULL;
    MeasureTaskTMP.prev = NULL;
    MeasureTask = MeasureTaskTMP;

    // Compute
    TCB ComputeTaskTMP;
    ComputeTaskTMP.taskFuncPtr = &computeDataFunc;
    ComputeTaskTMP.taskDataPtr = &dataForCompute;
    ComputeTaskTMP.next = NULL;
    ComputeTaskTMP.prev = NULL;
    ComputeTask = ComputeTaskTMP;
    
    // Display
    TCB DisplayTaskTMP;
    DisplayTaskTMP.taskFuncPtr = &displayDataFunc;
    DisplayTaskTMP.taskDataPtr = &dataForDisplay;
    DisplayTaskTMP.next = NULL;
    DisplayTaskTMP.prev = NULL;
    DisplayTask = DisplayTaskTMP;

    // Warning/Alarm
    TCB AnnunciateTaskTMP;
    AnnunciateTaskTMP.taskFuncPtr = &annunciateDataFunc;
    AnnunciateTaskTMP.taskDataPtr = &dataForWarningAlarm;
    AnnunciateTaskTMP.next = NULL;
    AnnunciateTaskTMP.prev = NULL;
    AnnunciateTask = AnnunciateTaskTMP;

    // Status
    TCB StatusTaskTMP;
    StatusTaskTMP.taskFuncPtr = &statusDataFunc;
    StatusTaskTMP.taskDataPtr = &dataForStatus;
    StatusTaskTMP.next = NULL;
    StatusTaskTMP.prev = NULL;
    StatusTask = StatusTaskTMP;

    // Keypad
    TCB KeypadTaskTMP;
    KeypadTaskTMP.taskFuncPtr = &KeypadDataFunc;
    KeypadTaskTMP.taskDataPtr = &dataForKeypad;
    KeypadTaskTMP.next = NULL;
    KeypadTaskTMP.prev = NULL;
    KeypadTask = KeypadTaskTMP;

    // EKG Capture TCB
    TCB EKGCaptureTaskTMP;
    EKGCaptureTaskTMP.taskFuncPtr = &EKGCaptureDataFunc;
    EKGCaptureTaskTMP.taskDataPtr = &dataForEKG;
    EKGCaptureTaskTMP.next = NULL;
    EKGCaptureTaskTMP.prev = NULL;
    EKGCaptureTask = EKGCaptureTaskTMP;

    // EKG Process TCB
    TCB EKGProcessTaskTMP;
    EKGProcessTaskTMP.taskFuncPtr = &EKGProcessDataFunc;
    EKGProcessTaskTMP.taskDataPtr = &dataForEKG;
    EKGProcessTaskTMP.next = NULL;
    EKGProcessTaskTMP.prev = NULL;
    EKGProcessTask = EKGProcessTaskTMP;

    // NULL TCB
    TCB NullTaskTMP;
    NullTaskTMP.taskFuncPtr = NULL;
    NullTaskTMP.taskDataPtr = NULL;
    NullTaskTMP.next = NULL;
    NullTaskTMP.prev = NULL;
    NullTask = NullTaskTMP;

    //displayDataFunc(&dataForDisplay);

    // User End Cases:
        // For annunciate: SHOW EVERYTHING
        // annunciateDataFunc(&dataForWarningAlarm);

        // For BP - Measurement selection would've been done at menuView
        // ONLY DISPLAY BP
        
        // For Temp - Measurement selection would've been done at menuView
        // First set Temp for measurement
        // ONLY DISPLAY TEMP

        // For Pulse - Measurement selection would've been done at menuView
        // First set Pulse for measurement
        // ONLY DISPLAY PULSE

        // For Respiration - Measurement selection would've been done at menuView
        // First set Respiration for measurement
        // ONLY DISPLAY RESPIRATION

        // For Display ALL DATA (update everything, NOt the menu display) - 
        // !!! -> MeasurementSelection = 4
        
}

// Modify this to traverse linkedList instead
void loop(void) {
    //annunciateDataFunc(&dataForWarningAlarm);
    //KeypadDataFunc(&dataForKeypad);
    scheduler();
}

void scheduler() {

    // FLAGS DEBUGGER
    if(0 == (int)unoCounter % 2) {
        Serial.print("[");
        for(int i = 0; i < 6; i++) {
            Serial.print(addFlags[i]); Serial.print(" ");
        }         
        Serial.println("]");
    }

    if(0 == (int)unoCounter % 5) {

        // Serial.println("five seconds have passed");
        for(int i = 0; i < 8; i++) { // checks add task flags
            // Serial.print(i); Serial.print(": "); Serial.println(addFlags[i]);
            if(addFlags[i]) {
                runTask(i, true); // insert task
                delay(20);
                addFlags[i] = 0;    
                removeFlags[i] = 1;
            }
        }
    }

    if(unoCounter > 5 && 0 == (int)unoCounter % 2) {
        if(addFlags[5]) {
            runTask(5, true); // insert task
            delay(20);
            addFlags[5] = 0;
            removeFlags[5] = 1;
        }
    }

    int index;
    if(2 == index || 3 == index || 6 == index || 7 == index) {
        if(addFlags[index]) {
            runTask(index, true); // insert task
            delay(20);
            addFlags[index] = 0;
            removeFlags[index] = 1;
        }
    }
    

    currPointer = linkedListHead;
    while(currPointer != NULL) {
        if(currPointer == &MeasureTask) {
            Serial.println("currPointer == measureTask");
        } else if (currPointer == &ComputeTask) {
            Serial.println("currPointer == computeTask");
        }  else if (currPointer == &AnnunciateTask) {
            Serial.println("currPointer == annunciateTask");
        } else if(currPointer == &DisplayTask) {
            Serial.println("currPointer == displayTask");
        } else if (currPointer == &StatusTask) {
            Serial.println("currPointer == statusTask");
        } else if (currPointer == &KeypadTask) {
            Serial.println("currPointer == keypadTask");
        } else if (currPointer == &EKGCaptureTask) {
            Serial.println("currPointer == EKGCaptureTask");
        } else if (currPointer == &EKGProcessTask) {
            Serial.println("currPointer == EKGProcessTask");
        }
        delay(20);
       // currPointer->taskFuncPtr(currPointer->taskDataPtr);
        (*currPointer->taskFuncPtr)(currPointer->taskDataPtr);
        delay(20);
        currPointer = currPointer->next; // moves current pointer to the next task
    }

    for(int i = 0; i < 8; i++) { // checks remove task flags
        if(removeFlags[i]) {
            runTask(i, false); // remove task
            removeFlags[i] = 0;
        }
    }
}

void runTask(int taskID, bool insertTask) {
    switch(taskID) {
        case 0: 
            if(insertTask) {
                appendAtEnd(&MeasureTask);
                // Serial.println("MeasureTask added");
            } else {
                deleteNode(&MeasureTask);
                // Serial.println("MeasureTask deleted");
            }
        break;
        case 1:
            if(insertTask) {
                appendAtEnd(&ComputeTask);
                // Serial.println("ComputeTask added");
            } else {
                deleteNode(&ComputeTask);
                // Serial.println("ComputeTask deleted");
            }
        break;
        case 2:
            if(insertTask) {
                appendAtEnd(&DisplayTask);
                // Serial.println("DisplayTask added");
            } else {
                deleteNode(&DisplayTask);
                // Serial.println("DisplayTask deleted");
            }
        break;
        case 3:
            if(insertTask) {
                appendAtEnd(&AnnunciateTask);
                // Serial.println("AnnunciateTask added");
            } else {
                deleteNode(&AnnunciateTask);
                // Serial.println("AnnunciateTask deleted");
            }
        break;
        case 4:
            if(insertTask) {
                appendAtEnd(&StatusTask);
                // Serial.println("StatusTask added");
            } else {
                deleteNode(&StatusTask);
                // Serial.println("StatusTask deleted");
            }
        break;
        case 5:
            if(insertTask) {
                appendAtEnd(&KeypadTask);
                // Serial.println("KeyPadTask added");
            } else {
                deleteNode(&KeypadTask);
                // Serial.println("KeyPadTask deleted");
            }
        break;
        case 6:
            if(insertTask) {
                appendAtEnd(&EKGCaptureTask);
            } else {
                deleteNode(&EKGCaptureTask);
            }
        break;
        case 7:
            if(insertTask) {
                appendAtEnd(&EKGProcessTask);
            } else {
                deleteNode(&EKGProcessTask);
            }
        break;
    }
}



// ------------------------------------Double Linked List Fns-------------------------------

// // Add to front
// void push(TCB newTCB) {
//     TCB* newNode = &newTCB;
//     /* 1. Make next of new node as head and previous as NULL */
//     newNode->next = linkedListHead;
//     newNode->prev = NULL;
 
//     /* 2. change prev of head node to new node */
//     if(linkedListHead !=  NULL) {
//       linkedListHead->prev = newNode ;
//     }
 
//     /* 3. move the head to point to the new node */
//     linkedListHead = newNode;
// }

// WORKS
// Add to back
void appendAtEnd(TCB* newNode) {
    // Set up second pointer to be moved to the end of the list, will be used in 3
    //TCB* newNode = newTCB;
    TCB* lastRef = linkedListHead;  
  
    /* 1. This new node is going to be the last node, so
          make next of it as NULL*/
    newNode->next = NULL;
 
    /* 2. If the Linked List is empty, then make the new
          node as head */
    if (linkedListHead == NULL) {
        newNode->prev = NULL;
        linkedListHead = newNode;
        return;
    }

    /* 3. Else traverse till the last node */
    while (lastRef->next != NULL) {
        lastRef = lastRef->next;
    }     
    
    /* 4. Change the next of last node */
    lastRef->next = newNode;

    /* 5. Make last node as previous of new node */
    newNode->prev = lastRef;

    return;
}

// Add within list
void insertAfterNode(TCB* prevNodeRef, TCB* newNode) {
    /* 1. check if the given prev_node is NULL */
    if (prevNodeRef == NULL) {
        Serial.println("the given previous node cannot be NULL");
        return;
    }
  
    /* 2. Make next of new node as next of prev_node */
    newNode->next = prevNodeRef->next;
 
    /* 3. Make the next of prev_node as new_node */
    prevNodeRef->next = newNode;
 
    /* 4. Make prev_node as previous of new_node */
    newNode->prev = prevNodeRef;
 
    /* 5. Change previous of new_node's next node */
    if (newNode->next != NULL) {
      newNode->next->prev = newNode;
    }
}

// Delete
/* Function to delete a node in a Doubly Linked List.
   head_ref --> pointer to head node pointer.
   del  -->  pointer to node to be deleted. */
void deleteNode(TCB* del) {
  /* base case */
  if (linkedListHead == NULL || del == NULL)
    return;
 
  /* If node to be deleted is head node */
  if (linkedListHead == del) {
    linkedListHead = del->next;
  }
 
  /* Change next only if node to be deleted is NOT the last node */
  if (del->next != NULL) {
    del->next->prev = del->prev;
  }
 
  /* Change prev only if node to be deleted is NOT the first node */
  if (del->prev != NULL) {
    del->prev->next = del->next;  
  }   
 
  return;
}     