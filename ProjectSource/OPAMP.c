/****************************************************************************
 Module
   OPAMP.c

 Revision
   1.0.0

 Description
   

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 05/07/21       kcao    Creation of file
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "PackDataHandler.h"
#include <stdbool.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PIC32_AD_Lib.h"
#include "OPAMP.h"
#include "TVS.h"
#include "ZENER.h"


/*----------------------------- Module Defines ----------------------------*/

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitIO();
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static OPAMPState_t CurrentState;
static uint8_t SecondCounter;
extern struct TeamData OPAMP_BUFFER;
extern struct TeamData TVS_BUFFER;
extern struct TeamData ZENER_BUFFER;
extern struct BroadcastData BROADCAST_BUFFER;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitOPAMP

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitOPAMP(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  CurrentState = InitOPAMPState;
  return true;
}

/****************************************************************************
 Function
     PostOPAMP

 Parameters
     EF_Event_t ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostOPAMP(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunOPAMP

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   Prints what letter you just pressed (for a and b only)
 Notes

 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunOPAMP(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  switch (CurrentState)
  {
    case InitOPAMPState:
    {
      switch (ThisEvent.EventType)
          {  
              case ES_ACTIVATE:
              {
                  InitIO();
                  SecondCounter = 0;
                  // Init transmit package
                  OPAMP_BUFFER.status = 0;
                  OPAMP_BUFFER.status = BIT3HI;
                  OPAMP_BUFFER.hull = 100;
                  OPAMP_BUFFER.shield = 200;
                  ES_Timer_InitTimer(OPAMP_TIMER, 200);
                  CurrentState = Alive;
              }
              break;

              case ES_DEACTIVATE:
              {
                  CurrentState = Deactivated;
              }
              break;

              default:
              {}
              break;
        }
    }
    break;

    case Alive:        
    {
        if ( ThisEvent.EventType != ES_NO_EVENT ) 
        {
            switch (ThisEvent.EventType)
            {  
                case ES_TIMEOUT:
                {
                  if (ThisEvent.EventParam == OPAMP_TIMER)
                  {
                      // Read potentiometers
                      uint32_t PotValues[3];
                      ADC_MultiRead(PotValues);
                      // Divide up the energy appropriately 
                      uint16_t TotalEnergy = PotValues[0] + PotValues[1] + PotValues[2];
                      uint16_t ShieldEnergy, WeaponEnergy, ThrustEnergy;
                      uint8_t EnergyAllotted = 64;
                      // Calculate energy for each 
                      ShieldEnergy = (PotValues[0] * EnergyAllotted) / TotalEnergy;
                      WeaponEnergy = (PotValues[1] * EnergyAllotted) / TotalEnergy;
                      ThrustEnergy = (PotValues[2] * EnergyAllotted) / TotalEnergy;
                      // Adds an extra value to thrust so nothing is wasted if total is lower than 64
                      uint8_t leftover = EnergyAllotted - ShieldEnergy - WeaponEnergy - ThrustEnergy;
                      ThrustEnergy += leftover;
                      // If everything is set to 0, there is an equal division
                      if (TotalEnergy = 0){
                        ShieldEnergy = 21;
                        WeaponEnergy = 21;
                        ThrustEnergy = 22;
                      }
                      // Recharge shield 
                      int16_t ShieldStrength = OPAMP_BUFFER.shield;
                      ShieldStrength += ShieldEnergy;
                      if (ShieldStrength > 200){
                        ShieldStrength = 200;
                      }
                      // Recharge thrust and weapon 
                      //printf("Shield: %u\n", ShieldEnergy);
                      //printf("Weapon: %u\n", WeaponEnergy);
                      //printf("Thrust: %u\n", ThrustEnergy);
                      //printf("------------------\n");
                      // Update transmit packet
                      OPAMP_BUFFER.weaponPower = WeaponEnergy;
                      OPAMP_BUFFER.navPower = ThrustEnergy;
                      OPAMP_BUFFER.shield = ShieldStrength;
                      
                      ES_Timer_InitTimer(2, 200);
                        printf("Shield Strength: %d\n", OPAMP_BUFFER.shield);
                        printf("Hull Strength: %d\n", OPAMP_BUFFER.hull);
                        printf("------------------\n");
                        //printTeamData(OPAMP_BUFFER);
                  }
                }
                break;

                case ES_HIT:
                {
                  printf("Hit!\n");   
                  uint8_t damage = 100;
                  int16_t ShieldStrength = OPAMP_BUFFER.shield;
                  int16_t HullStrength = OPAMP_BUFFER.hull;
                  if (ShieldStrength > 100){
                    ShieldStrength -= 100;
                  } else {
                    damage -= ShieldStrength;
                    ShieldStrength = 0;
                    HullStrength -= damage;
                  }
                  //printf("Shield Strength: %d\n", ShieldStrength);
                  //printf("Hull Strength: %d\n", HullStrength);
                  //printf("------------------\n");  
                  if (HullStrength <= 0) {
                    ES_Event_t DeadEvent;
                    DeadEvent.EventType = ES_DEAD;
                    PostOPAMP(DeadEvent);
                    PostTVS(DeadEvent);
                    PostZENER(DeadEvent);
                    OPAMP_BUFFER.shield = 0;
                    OPAMP_BUFFER.hull = 0;
                    OPAMP_BUFFER.weaponPower = 0;
                    OPAMP_BUFFER.navPower = 0;
                    OPAMP_BUFFER.shield = 0;
                  } else {
                        OPAMP_BUFFER.shield = ShieldStrength;
                        OPAMP_BUFFER.hull = HullStrength;
                  }
                }
                break;

                case ES_DEAD:
                {
                  // Turn on the DEAD LED 
                  LATAbits.LATA4 = 1;
                  CurrentState = Dead;
                  printf("Dead!\n");
                }
                break;
                
                case ES_READ:
                {
                    // If received from Zener, check whether hit and post ES_HIT
                    if (ThisEvent.EventParam == 3){
                        bool hit = ZENER_BUFFER.status & BIT0HI;
                        OPAMP_BUFFER.status = OPAMP_BUFFER.status | BIT7HI;
                        if (hit){
                            ES_Event_t HitEvent;
                            HitEvent.EventType = ES_HIT;
                            PostOPAMP(HitEvent);
                        }
                    }
                }
                break;

                case ES_DEACTIVATE:
                {
                    if ((OPAMP_BUFFER.shield == 0) && (OPAMP_BUFFER.hull == 0)){
                        ES_Event_t DeadEvent;
                        DeadEvent.EventType = ES_DEAD;
                        PostOPAMP(DeadEvent);
                        PostTVS(DeadEvent);
                        PostZENER(DeadEvent);
                        OPAMP_BUFFER.shield = 0;
                        OPAMP_BUFFER.hull = 0;
                        OPAMP_BUFFER.weaponPower = 0;
                        OPAMP_BUFFER.navPower = 0;
                        OPAMP_BUFFER.shield = 0;
                    } else {
                        printf("OPAMP Deactivated\n");
                        CurrentState = Deactivated;
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

    case Dead:        
    {
      // Do nothing - dead
    }
    break;

    case Deactivated:        
    {
      if (ThisEvent.EventType == ES_ACTIVATE)    
      {
        printf("OPAMP Activated\n");
        InitIO();
        SecondCounter = 0;
        ES_Timer_InitTimer(2, 200);
        CurrentState = Alive;
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
// RB12/RB13/RB15 = Analog inputs
// RA4 LED
static void InitIO(){
  // Set RB12, RB13 and RB15 as inputs
  TRISBbits.TRISB12 = 1;
  TRISBbits.TRISB13 = 1;
  TRISBbits.TRISB15 = 1;
  // Set RB12, RB13 and RB15 to analog 
  ANSELBbits.ANSB12 = 1;
  ANSELBbits.ANSB13 = 1;
  ANSELBbits.ANSB15 = 1;
  // Set RA4 as an output
  TRISAbits.TRISA4 = 0;
  LATAbits.LATA4 = 0;
  // Set up analog function using the ADC library (AN9/11/12)
  ADC_ConfigAutoScan((BIT9HI | BIT11HI | BIT12HI), 3);
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

