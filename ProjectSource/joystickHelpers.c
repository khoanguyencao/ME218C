/****************************************************************************
 Module
   joystick.c

 Revision
   1.0.1

 Description
   This module contains an initialization function and two event checker for the joystick

 Notes
   Joystick Y is connected to RB2 (pin 6) on PIC32
   Joystick X is connected to RB3 (pin 7) on PIC32
 * 
 * for currentJoystick and previousJoystick values:
 * default position: 0
 * toggle up: positive value up to around 500
 * toggle down: negative value down to around -500

 History
 When           Who     What/Why
 -------------- ---     --------
 11/03/20 15:21 abn     initial version
****************************************************************************/

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


//#include "testService.h"


// this will pull in the symbolic definitions for events, which we will want
// to post in response to detecting events
#include "ES_Configure.h"
// This gets us the prototype for ES_PostAll
#include "ES_Framework.h"
// this will get us the structure definition for events, which we will need
// in order to post events in response to detecting events
#include "ES_Events.h"
// if you want to use distribution lists then you need those function
// definitions too.
#include "ES_PostList.h"
// This include will pull in all of the headers from the service modules
// providing the prototypes for all of the post functions
#include "ES_ServiceHeaders.h"
// this test harness for the framework references the serial routines that
// are defined in ES_Port.c
#include "ES_Port.h"
// include our own prototypes to insure consistency between header &
// actual functionsdefinition
#include "EventCheckers.h"

#include "joystickHelpers.h"

#include "PIC32_AD_Lib.h"
#include "pinsHAL.h"
#include "ZENER.h"
#include "dbprintf.h"

#define USE_OLD_BINNING 1

///* function prototypes */
//void initJoystick();
//bool Check4JoystickToggleY();
//bool Check4JoystickToggleX();
//int16_t getJoystickY();
//int16_t getJoystickX();

/* module variable */
static uint16_t calibrateJoystickY; // joystickY raw center position
static int16_t calibrationFactorY;
static uint16_t calibrateJoystickX; // joystickX raw center position
static int16_t calibrationFactorX;
static int16_t previousJoystickY; // store previousJoystickY value for event checker
static int16_t previousJoystickX; // store previousJoystickX value for event checker
static int16_t currentJoystickY; // joystickY value: 0 is center, negative is down, positive is up
static int16_t currentJoystickX; // joystickX value: 0 is center, negative is down, positive is up
static uint32_t joystickPosition[2]; // array for ADC_MultiRead to store joystick raw values

static int8_t previousYBin;
static int8_t previousXBin;

/* initialize joystick */
void initJoystick() {
    
    ANSELBbits.ANSB2 = 1; // enable analog on RB2 for joystickY
    TRISBbits.TRISB2 = 1; // set RB2 to be an input
    
    ANSELBbits.ANSB3 = 1; // enable analog on RB3 for joystickX
    TRISBbits.TRISB3 = 1; // set RB3 to be an input
    
    pinMode(18,OUTPUT); // debugging pin
    
    ADC_ConfigAutoScan( (BIT4HI | BIT5HI), 2);  // initialize RB2 and RB3 to be analog inputs

    /* declare variables to calculate calibration average */
    uint8_t sampleCounter; 
    uint8_t validSamplesY = 0;
    uint8_t validSamplesX = 0;
    uint8_t totalSamples = 10; // change this value to take more samples for calibration average
    uint16_t sumX = 0;
    uint16_t sumY = 0;
    
    bool firstEntry = true;
    uint8_t initLoop;
    
    /* calibrate joystick to account for drift by averaging */
    for (sampleCounter = 0; sampleCounter < totalSamples; sampleCounter++) {
        ADC_MultiRead(joystickPosition); // read analog values from RB2 and RB3
        
        if (firstEntry)
        {
            for (initLoop = 0; initLoop < 100; initLoop++){;}
            firstEntry = false;
        }
        
        sumY += joystickPosition[0]; // read joystickY position value and include it in average calculation
        sumX += joystickPosition[1]; // read joystickX position value and include it in average calculation  
        // The first couple ADC_MultiRead returns 0
        // with this if statement we only count non-zero values
        if (sumY != 0)
        {
            validSamplesY++;
        }
        if (sumX != 0)
        {
            validSamplesX++;
        }
    }
//    DB_printf("\nsumX: %d\n", sumX);
    // The first couple ADC_MultiRead returns 0, as such we divide by (validSamples) to provide a proper mean
    calibrateJoystickY = sumY / (validSamplesY); // calculate average joystickY center value to set calibration offset
    calibrateJoystickX = sumX / (validSamplesX); // calculate average joystickX center value to set calibration offset
    calibrationFactorY = calibrateJoystickY - 512;
    calibrationFactorX = calibrateJoystickX - 512;
    //printf("CAL Y: %d\r\n", calibrationFactorY);
    
    /* set previousJoystickY and previousJoystickX variable to calibration joystick values */
    previousJoystickY = calibrateJoystickY/10;
    calibrateJoystickX = calibrateJoystickX;
    
    previousXBin = 0;
    previousYBin = 0;
}

/* event checkers */

#if 0 
bool Check4JoystickToggleY() {
    bool returnVal = false;
    ADC_MultiRead(joystickPosition); // read joystick position    
    currentJoystickY = joystickPosition[0] - calibrationFactorY;
   
    
    // Calculate current Y Bin:
    int8_t CurrentYBin = (currentJoystickY) / 8; // bin into 0-7
    if (CurrentYBin > 63) {  // combine bins 3 & 4 
        CurrentYBin --; 
    }
    CurrentYBin -= 63; // Set center positions to 0
    if (CurrentYBin > 63) { // remove any extraneous bins that may result from calibration
        CurrentYBin = 63;
    } else if (CurrentYBin < -63) {
        CurrentYBin = -63;
    }
    
    // Check for Y Bin change:
    if(CurrentYBin != previousYBin) {
        returnVal = true;
        setPlayerDdy(CurrentYBin);   // update player dy
//        ES_Event_t ThisEvent;
//        if(CurrentYBin < previousYBin){
//            ThisEvent.EventType = ES_JOYSTICK_UP;
//            if (CurrentYBin == -3) {        // if joystick is in topmost bin
//                ThisEvent.EventParam = 1;
//            } else {
//                ThisEvent.EventParam = 0;
//            }
//            PostJoystickFSM(ThisEvent);
//        } else {
//            ThisEvent.EventType = ES_JOYSTICK_DOWN;
//            if (CurrentYBin == 3) {         // if joystick is in bottommost bin
//                ThisEvent.EventParam = 1;
//            } else {
//                ThisEvent.EventParam = 0;
//            }
//            PostJoystickFSM(ThisEvent);
//        }
    }
    previousYBin = CurrentYBin;
    
    return returnVal;            

}
#endif


#if 0 
bool Check4JoystickToggleX() {
    bool returnVal = false;
    ADC_MultiRead(joystickPosition); // read joystick position    
    currentJoystickX = joystickPosition[1] - calibrationFactorX;
    
    // Calculate current X Bin
    int8_t CurrentXBin = (currentJoystickX) / 8; // bin into 0-7
    if (CurrentXBin > 63) {  // combine bins 3 & 4 
        CurrentXBin --; 
    }
    CurrentXBin -= 63; // Set center bins to 0
    if (CurrentXBin > 63) { //remove extraneous bins from calibration
        CurrentXBin = 63;
    } else if (CurrentXBin < -63) {
        CurrentXBin = -63;
    }
    
    
    // Check for change in X Bin
    if(CurrentXBin != previousXBin) {
        returnVal = true;
        
        setPlayerDdx(CurrentXBin); // update player dx
//        ES_Event_t ThisEvent;
//        if(CurrentXBin < previousXBin){ // joystick moved left
//            ThisEvent.EventType = ES_JOYSTICK_LEFT;
//            if (CurrentXBin == -3) {    // if joystick is in the leftmost bin
//                ThisEvent.EventParam = 1;
//            } else {
//                ThisEvent.EventParam = 0;
//            }
//            PostJoystickFSM(ThisEvent);
//        } else { // joystick moved right
//            ThisEvent.EventType = ES_JOYSTICK_RIGHT;
//            if (CurrentXBin == 3) {     // if joystick is in rightmost bin
//                ThisEvent.EventParam = 1;
//            } else {
//                ThisEvent.EventParam = 0;
//            }
//            PostJoystickFSM(ThisEvent);
//        }

    }

    previousXBin = CurrentXBin;
    return returnVal;  
}
#endif

struct joyInfo_t retrieveJoystickInfo(void)
{
    struct joyInfo_t currentInfo;
    
    ADC_MultiRead(joystickPosition); // read joystick position  
    
    currentJoystickX = joystickPosition[1] - calibrationFactorX;
    currentJoystickY = joystickPosition[0] - calibrationFactorY;
    
    
    // Calculate current X Bin
    int8_t CurrentXBin = (currentJoystickX) / 8; // bin into 0-7
    if (CurrentXBin > 63) {  // combine bins 3 & 4 
        CurrentXBin --; 
    }
    CurrentXBin -= 63; // Set center bins to 0
    if (CurrentXBin > 63) { //remove extraneous bins from calibration
        CurrentXBin = 63;
    } else if (CurrentXBin < -63) {
        CurrentXBin = -63;
    }

    currentInfo.x = CurrentXBin;

    // Calculate current Y Bin:
    int8_t CurrentYBin = (currentJoystickY) / 8; // bin into 0-7
    if (CurrentYBin > 63) {  // combine bins 3 & 4 
        CurrentYBin --; 
    }
    CurrentYBin -= 63; // Set center positions to 0
    if (CurrentYBin > 63) { // remove any extraneous bins that may result from calibration
        CurrentYBin = 63;
    } else if (CurrentYBin < -63) {
        CurrentYBin = -63;
    }
    
    currentInfo.y = CurrentYBin;
    
    return currentInfo;
}
