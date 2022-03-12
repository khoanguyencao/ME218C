/****************************************************************************
 Module
   TVS.c

 Revision
   1.0.0

 Description

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 04/28/21       bwan    Creation of file
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PIC32_AD_Lib.h"
#include "TVS.h"
#include "Math.h"
#include "CommsRx.h"
#include "CommsTx.h"
#include "Comms.h"
#include <xc.h>
#include <sys/attribs.h>
#include <stdbool.h>
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/
#define TESTING
#define LEN_MAX 572433.4
/*---------------------------- Module Functions ---------------------------*/
/* Helper functions */
static void initPins();		     // Initialize PIC32 for I/O
static void initUARTS();	     // Initialize PIC32 for UARTS
static void initPotTimer();    // Initialize PIC32 for checking pot values
static bool getBeamInfo();      // helper functions to update beam info from pots 
bool CheckButtonEvents();      // Event checker to check button down

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static TVSState_t CurrentState;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;
static bool LastButtonState;
static float potdirVal = 0;
static float potlenVal = 0;
static float dir = 0;     // in degrees
static float len = 0;    // in km
static uint32_t adcResults[2];
static int16_t beam_dx = 0;
static int16_t beam_dy = 0;
static uint16_t ship_x = 0;
static uint16_t ship_y = 0;
static uint8_t capBank = 200;
static uint8_t reqPower = 0;

/*------------------------------ Module Code ------------------------------*/

bool InitTVS(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  CurrentState = InitTVSPState;
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService(MyPriority, ThisEvent) == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool PostTVS(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}


ES_Event_t RunTVS(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  switch (CurrentState)
  {
    case InitTVSPState:
    {
        if (ThisEvent.EventType == ES_INIT)    
        {
            initPins();
            initPotTimer();
            CurrentState = Aiming;
        }
    }
    break;

    case Aiming:        
    {
        if ( ThisEvent.EventType != ES_NO_EVENT ) 
        {
            switch (ThisEvent.EventType)
            {
                case ES_READ:
                {
                    // if power in from opamp
                    if (ThisEvent.EventParam == 1){
                        if (OPAMP_BUFFER.weaponPower){
                            capBank += OPAMP_BUFFER.weaponPower;
                        }
                    } 
                    
                }
                break;
                
                case ES_DEACTIVATE:
                {
                    CurrentState = Down;
                }
                break;

                #ifdef TESTING
                case ES_NEW_KEY:
                {
                    if ('q' == ThisEvent.EventParam) {
                        LATBbits.LATB6 = 1;
                        printf("Ready to fire\n");
                        CurrentState = Targetted;
                    }
                    if ('s' == ThisEvent.EventParam) {
                        memset(&TVS_BUFFER, 0, sizeof(TVS_BUFFER));
                        TVS_BUFFER.weaponCap = 118;
                        TVS_BUFFER.beam_dx_lsb = -18;
                        TVS_BUFFER.beam_dx_msb = 8;
                        ES_Event_t event2Post;
                        event2Post.EventType = ES_TRANSMIT;
                        event2Post.EventParam = 3;
                        //printTeamData(TVS_BUFFER);
                        PostComms(event2Post);
                        printf("t sent\n");
                    }
                }
                break;
                #endif

                case ES_TIMEOUT:
                {
                    if (ThisEvent.EventParam == POT_TIMER) {
                        /* Reset Timer timeout for 0.2 sec */
                        ES_Timer_InitTimer(POT_TIMER, 200);
                        /* Update beam info */
                        if (getBeamInfo()){
                            LATBbits.LATB6 = 1;
                            CurrentState = Targetted;
                        }
                        
                    }
                }
                break;

                default:
                {}
                break;
            }
        }
    }
    break;

    case Targetted:        
    {
        if ( ThisEvent.EventType != ES_NO_EVENT ) 
        {
            switch (ThisEvent.EventType)
            { 
                #ifdef TESTING 
                case ES_NEW_KEY:
                {
                    if ('w' == ThisEvent.EventParam) {
                        LATBbits.LATB6 = 0;
                        printf("Target Lose\n");
                        CurrentState = Aiming;
                    } else if ('b' == ThisEvent.EventParam) {

                      printf("Beam Fired\n");
                      printf("Beam Dir: %.2f degrees\n", dir);
                      printf("Beam Len: %.2f km\n", len);

                      CurrentState = Aiming;
                      /* Reset Timer timeout for 0.2 sec */
                      ES_Timer_InitTimer(POT_TIMER, 200);
                    }
                }
                break;
                #endif

                case ES_REQ2FIRE:
                {
                    printf("Beam Fired\n");
                    printf("Beam Dir: %.2f degrees\n", dir);
                    printf("Beam Len: %.2f km\n", len);
                    
                    /* Dec cap bank */
                    capBank -= reqPower;
                    
                    /* send msg to ZENER w new params */
                    memset(&TVS_BUFFER, 0, sizeof(TVS_BUFFER));
                    TVS_BUFFER.beam_dx_lsb = beam_dx;
                    TVS_BUFFER.beam_dx_msb = (beam_dx >> 8);
                    TVS_BUFFER.beam_dy_lsb = beam_dy;
                    TVS_BUFFER.beam_dy_msb = (beam_dy >> 8);
                    ES_Event_t event2Post;
                    event2Post.EventType = ES_TRANSMIT;
                    event2Post.EventParam = 4;
                    PostComms(event2Post);
                    printf("new beam info -> ZENER\n");
                    
                    /* Send msg to OPAMP w new params */
                    memset(&TVS_BUFFER, 0, sizeof(TVS_BUFFER));
                    TVS_BUFFER.weaponCap = capBank;
                    ES_Event_t event3Post;
                    event3Post.EventType = ES_TRANSMIT;
                    event3Post.EventParam = 3;
                    PostComms(event3Post);
                    printf("new capBank info -> OPAMP\n");
                    
                    CurrentState = Aiming;
                    /* Reset Timer timeout for 0.2 sec */
                    ES_Timer_InitTimer(POT_TIMER, 200);
                }
                break;

                case ES_TIMEOUT:
                {
                    if (ThisEvent.EventParam == POT_TIMER) {
                        /* Reset Timer timeout for 0.2 sec */
                        ES_Timer_InitTimer(POT_TIMER, 200);
                        /* Update beam info */
                        if (!getBeamInfo()){
                            LATBbits.LATB6 = 0;
                            CurrentState = Aiming;
                        }
                    }
                }
                break;

                case ES_DEACTIVATE:
                {
                    CurrentState = Down;
                }
                break;

                default:
                {}
                break;
            }
        }
    }
    break;

    case Down:       
    {
        if (ThisEvent.EventType == ES_ACTIVATE)   
        {
          // Continued activation
          CurrentState = InitTVSPState;
        }
    }
    break;  

    default:
    {}
    break;
  }                                   
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/* Init Functions */
// SPI: (ZENER specific)
// SS1 to RB4
// SDI1 to RB5
// SDO1 to RB8
// SCK1 to RB14
//
// TODO: CHECK that 3 stations arent overwriting ansel or other bits
// 
// TVS I/O:
// Target Aimed LED = RB6 (output)   
// Request to fire Button = RB7 (input) 
// Direction of Beam Pot = RA4 (input)  -- TODO: NOW IS RA0
// Length of Beam Pot = RB10 (input)    -- TODO: NOW IS RA1
//
// UARTS2:
// U2TX = RB9 (output)
// U2RX = RB11 (input)
// 

static void initPins(){
    TRISBbits.TRISB6 = 0;       	// RB6 to output
    TRISBbits.TRISB7 = 1;       	// RB7 to input
    TRISAbits.TRISA4 = 1;       	// RA4 to input
    TRISBbits.TRISB10 = 1;      	// RB10 to input
    LATBbits.LATB6 = 0;		// Turn off LED upon restart	
    /* Sample port line and use it to initialize the LastInputState variable */
    LastButtonState = PORTBbits.RB7;	
}

static void initUARTS() {
  // init Pins
  TRISBbits.TRISB11 = 1;        // RB11 to input
  U2RXR = 0b0011;               // U2RX to RB11
  TRISBbits.TRISB9 = 0;        // RB9 to output
  RPB9R = 0b0010;               // RB9 to U2TX
  // init modes
  U2MODEbits.ON = 0;		// Turn off
  U2MODEbits.BRGH = 0;		// Set to high speed mode
  U2STA = 0;                  	// Clear status register
  U2BRG = 129;                 	// Set to 115200 baud rate: 20*(10^6)/(4*9600) - 1 = 519.83
  U2STAbits.URXEN = 1;      	// Enable UARTS receive 
  U2MODEbits.ON = 1;          	// Turn on
}

static void initPotTimer() { 
    /* init Pins */
    ANSELAbits.ANSA0 = 1;
    ANSELAbits.ANSA1 = 1;
    TRISAbits.TRISA0 = 1;         // RA0 to input
    TRISAbits.TRISA1 = 1;         // RA1 to input
    ADC_ConfigAutoScan((BIT0HI | BIT1HI), 2);

    /* Init Timer4 to timeout every 0.2 sec for checking pot value */
    ES_Timer_InitTimer(POT_TIMER, 200); 
}

bool CheckButtonEvents(){
    bool ReturnVal = false;
    bool CurrentButtonState;
    
    CurrentButtonState = PORTBbits.RB7;
    
    /* Set CurrentButtonState to state read from port pin */
    if ((CurrentButtonState != LastButtonState) && (CurrentButtonState == 1)){
        
        /* PostEvent ES_REQ2FIRE to TVS queue */
        ES_Event_t event2Post;
        event2Post.EventType = ES_REQ2FIRE;
        PostTVS(event2Post);
        ReturnVal = true;
        //printf("ButtonDown Detected!");
    }
    LastButtonState = CurrentButtonState;
    return ReturnVal;
}

static bool getBeamInfo() {
    /* Read pin to adc */ 
    ADC_MultiRead(adcResults);
    potdirVal = (float)(adcResults[0]* 3.3/1023.0);  // 3.3V/1023
    potlenVal = (float)(adcResults[1]* 3.3/1023.0);

    dir = potdirVal * 360 / 3.3; 
    len = potlenVal * LEN_MAX / 3.3;
    // if len within 4096?128000
    if ((len >= 4096) || (len <= 128000)) {
        reqPower = (uint8_t)(len * 97/61952 - 50/121);
        if (reqPower <= capBank) {
            beam_dx = (int16_t)(sin(dir/(180.0/3.14)) * len / 7.8125);
            beam_dy = (int16_t)(cos(dir/(180.0/3.14)) * len / 7.8125);
            return true;
        }
    } 
    return false;
}