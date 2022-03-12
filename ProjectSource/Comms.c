/****************************************************************************
 Module
   Comms.c

 Revision
   1.0.0

 Description

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 05/14/21       kcao    Creation of file
 05/19/21       kcao    Implementation of failsafe systems
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <xc.h>
#include <sys/attribs.h>
#include <string.h>
#include <stdbool.h>            // add
#include "PackDataHandler.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Types.h"
#include "PIC32_AD_Lib.h"
#include "Comms.h"
#include "CommsTx.h"
#include "CommsRx.h"
#include "OPAMP.h"
#include "TVS.h"
#include "ZENER.h"
#include "ES_CheckEvents.h"

/*----------------------------- Module Defines ----------------------------*/
#define OPAMP_MEICM 0x2191
#define TVS_MEICM 0x2184
#define ZENER_MEICM 0x2181
#define STATUS_MSGCNT 7    // 7 bytes per packet per receipt
#define TEAM_MSGCNT 19     // 19 bytes per packet tx'ed/ rx'ed
#define BROADCAST_MSGCNT 20    // 20 bytes per packet tx'ed/ rx'ed
#define TIMEOUT_PERIOD 1000
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitCommunication();
static void InitTimer();
static void rxData(uint8_t msg);
static void ActivateSystems();
static struct TeamTxPacket CheckAssumeCommand(struct TeamTxPacket PacketToSend);
extern struct TeamRxPacket realTeamRxPacket;    // stores real Team data
extern struct BroadcastRxPacket realBroadcastRxPacket;    // stores real broadcast data
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static CommsState_t CurrentState;
static uint8_t RoundRobinNum = 0;
static bool AssumeCommand = false;

// Self Variables
static SelfState_t ORIGINAL_SELF;           // stores the original self state 

// Self Stations
static bool SELF_OPAMP;
static bool SELF_TVS;
static bool SELF_ZENER;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitComms

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
bool InitComms(uint8_t Priority)
{
  ES_Event_t ThisEvent;
  MyPriority = Priority;
  CurrentState = InitPState;
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

/****************************************************************************
 Function
     PostComms

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
bool PostComms(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunComms

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
ES_Event_t RunComms(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  switch (CurrentState)
  {
    case InitPState:
    {
      if (ThisEvent.EventType == ES_INIT)    
      {
        InitTimer();
        InitCommunication();
        SELF = TVS;
        ActivateSystems();
        ORIGINAL_SELF = SELF;
        CurrentState = Idle;
 
      }
    }
    break;

    case Idle:        
    {

        if ( ThisEvent.EventType != ES_NO_EVENT ) 
        {
            switch (ThisEvent.EventType)
            {  
                case ES_TRANSMIT:
                {
                    /* 
                       Round robin system for transmits
                       0 = Global Broadcast
                       1 = OPAMP -> TVS 
                       2 = OPAMP -> Zener 
                       3 = TVS -> OPAMP 
                       4 = TVS -> Zener 
                       5 = Zener -> OPAMP 
                       6 = Zener -> TVS 
                    */
                    uint8_t TransmitNum = ThisEvent.EventParam;
                    // Broadcast Message
                    if ((TransmitNum == 0) && SELF_ZENER){
                        struct BroadcastTxPacket PacketToSend = broadcastCreatePacket(BROADCAST_BUFFER);
                        broadcastUARTWrite(PacketToSend);
                    } else {
                        ES_Event_t ReadEvent;
                        ReadEvent.EventType = ES_READ;
                        // Team Message
                        if ((TransmitNum == 1) && SELF_OPAMP){
                            struct TeamTxPacket PacketToSend = teamCreatePacket(OPAMP_BUFFER, TVS_MEICM);
                            PacketToSend = CheckAssumeCommand(PacketToSend);
                            teamUARTWrite(PacketToSend); 
                            ReadEvent.EventParam = 1;
                            PostTVS(ReadEvent);
                            PostZENER(ReadEvent);
                        } else if ((TransmitNum == 2) && SELF_OPAMP){
                            struct TeamTxPacket PacketToSend = teamCreatePacket(OPAMP_BUFFER, ZENER_MEICM);
                            PacketToSend = CheckAssumeCommand(PacketToSend);
                            teamUARTWrite(PacketToSend); 
                            ReadEvent.EventParam = 1;
                            PostTVS(ReadEvent);
                            PostZENER(ReadEvent);
                        } else if ((TransmitNum == 3) && SELF_TVS){
                            struct TeamTxPacket PacketToSend = teamCreatePacket(TVS_BUFFER, OPAMP_MEICM);
                            PacketToSend = CheckAssumeCommand(PacketToSend);
                            teamUARTWrite(PacketToSend);  
                            ReadEvent.EventParam = 2;
                            PostOPAMP(ReadEvent);
                            PostZENER(ReadEvent);
                        } else if ((TransmitNum == 4) && SELF_TVS){
                            struct TeamTxPacket PacketToSend = teamCreatePacket(TVS_BUFFER, ZENER_MEICM);
                            PacketToSend = CheckAssumeCommand(PacketToSend);
                            teamUARTWrite(PacketToSend);
                            ReadEvent.EventParam = 2;
                            PostOPAMP(ReadEvent);
                            PostZENER(ReadEvent);
                        } else if ((TransmitNum == 5) && SELF_ZENER){
                            struct TeamTxPacket PacketToSend = teamCreatePacket(ZENER_BUFFER, OPAMP_MEICM);
                            PacketToSend = CheckAssumeCommand(PacketToSend);
                            teamUARTWrite(PacketToSend);
                            ReadEvent.EventParam = 3;
                            PostOPAMP(ReadEvent);
                            PostTVS(ReadEvent);
                        } else if ((TransmitNum == 6) && SELF_ZENER){
                            struct TeamTxPacket PacketToSend = teamCreatePacket(ZENER_BUFFER, TVS_MEICM);
                            PacketToSend = CheckAssumeCommand(PacketToSend);
                            teamUARTWrite(PacketToSend);
                            ReadEvent.EventParam = 3;
                            PostOPAMP(ReadEvent);
                            PostTVS(ReadEvent);
                        }
                    }
                }
                break;

                case ES_RECEIVE:
                {
                    // Create a read event to tell systems to read
                    // Event Param: 1 = OPAMP, 2 = TVS, 3 = ZENER; 
                    //              F0 - F6: broadcasts from Team 0 to Team 6
                    ES_Event_t ReadEvent;
                    ReadEvent.EventType = ES_READ;
                    ReadEvent.EventParam = 0;
                    if (ThisEvent.EventParam == ISTEAM0){
                        ReadEvent.EventParam = 0xF0;
                        PostZENER(ReadEvent);
                    } else if (ThisEvent.EventParam == ISTEAM1){
                        ReadEvent.EventParam = 0xF1;
                        PostZENER(ReadEvent);
                    } else if (ThisEvent.EventParam == ISTEAM2){
                        ReadEvent.EventParam = 0xF2;
                        PostZENER(ReadEvent);
                    } else if (ThisEvent.EventParam == ISTEAM3){
                        ReadEvent.EventParam = 0xF3;
                        PostZENER(ReadEvent);
                    } else if (ThisEvent.EventParam == ISTEAM4){
                        ReadEvent.EventParam = 0xF4;
                        PostZENER(ReadEvent);
                    } else if (ThisEvent.EventParam == ISTEAM5){
                        ReadEvent.EventParam = 0xF5;
                        PostZENER(ReadEvent);
                    } else if (ThisEvent.EventParam == ISTEAM6){
                        ReadEvent.EventParam = 0xF6;
                        PostZENER(ReadEvent);
                    }
                    //ReadEvent.EventParam = 0;
                    // Team Message Received
                    if (ThisEvent.EventParam == ISBUDDY){
                        uint8_t status = realTeamRxPacket.LocalRxPacket.status;
                        // If we are not OPAMP, keep a copy of OPAMP struct 
                        if (!SELF_OPAMP && (status & BIT3HI)){
                            ES_Timer_InitTimer(OPAMP_TIMEOUT, TIMEOUT_PERIOD);
                            memcpy(&OPAMP_BUFFER, &realTeamRxPacket.LocalRxPacket, sizeof(struct TeamData));
                            ReadEvent.EventParam = 1;
                        }
                        // If we are not TVS, keep a copy of TVS struct 
                        if (!SELF_TVS && (status & BIT2HI)){
                            ES_Timer_InitTimer(TVS_TIMEOUT, TIMEOUT_PERIOD);
                            memcpy(&TVS_BUFFER, &realTeamRxPacket.LocalRxPacket, sizeof(struct TeamData));
                            ReadEvent.EventParam = 2;
                        }
                        // If we are not ZENER, keep a copy of ZENER struct 
                        if (!SELF_ZENER && (status & BIT4HI)){
                            ES_Timer_InitTimer(ZENER_TIMEOUT, TIMEOUT_PERIOD);
                            memcpy(&ZENER_BUFFER, &realTeamRxPacket.LocalRxPacket, sizeof(struct TeamData));
                            ReadEvent.EventParam = 3;
                        }

                        // Assume Command Events
                        if (status & BIT5HI){
                            ES_Event_t ReentryEvent;
                            ReentryEvent.EventType = ES_REENTRY;
                            // OPAMP Assume Command
                            if (status & BIT3HI){
                                ReentryEvent.EventParam = 1;
                            } 
                            // TVS Assume Command 
                            else if (status & BIT2HI){
                                ReentryEvent.EventParam = 2;
                            } 
                            // ZENER Assume Command 
                            else if (status & BIT4HI){
                                ReentryEvent.EventParam = 3;
                            }
                            PostComms(ReentryEvent);
                        }

                        // Send read events to each state machine 
                        if (SELF_OPAMP){
                            PostOPAMP(ReadEvent);
                        }
                        if (SELF_TVS){
                            PostTVS(ReadEvent);
                        }
                        if (SELF_ZENER){
                            PostZENER(ReadEvent);
                        }
                    }
                }
                break;

                case ES_SELF_DROPOUT: 
                {
                    printf("ES_SELF_DROPOUT\n");
                    // Update self states
                    ES_Event_t DeactivateEvent;
                    DeactivateEvent.EventType = ES_DEACTIVATE;
                    ES_Timer_StopTimer(OPAMP_TIMEOUT);
                    ES_Timer_StopTimer(TVS_TIMEOUT);
                    ES_Timer_StopTimer(ZENER_TIMEOUT);
                    if (SELF_OPAMP){
                        PostOPAMP(DeactivateEvent);
                        SELF_OPAMP = false;
                    }
                    if (SELF_TVS){
                        PostTVS(DeactivateEvent);
                        SELF_TVS = false;
                    }
                    if (SELF_ZENER){
                        PostZENER(DeactivateEvent);
                        SELF_ZENER = false;
                    }
                }
                break;

                case ES_SELF_REENTRY: 
                {
                    printf("ES_SELF_REENTRY\n");
                    ActivateSystems();
                    AssumeCommand = true;
                }
                break;
                
                // 1 = OPAMP dropped, 2 = TVS dropped, 3 = ZENER dropped 
                case ES_DROPOUT: 
                {
                    printf("ES_DROPOUT: %u\n", ThisEvent.EventParam);
                    ES_Event_t ActivateEvent;
                    ActivateEvent.EventType = ES_ACTIVATE;
                    // OPAMP activated
                    if (ThisEvent.EventParam == 1){
                        SELF_OPAMP = true;
                        PostOPAMP(ActivateEvent);
                        ES_Timer_StopTimer(OPAMP_TIMEOUT);
                    }
                    // TVS activated
                    if (ThisEvent.EventParam == 2){
                        SELF_TVS = true;
                        PostTVS(ActivateEvent);
                        ES_Timer_StopTimer(TVS_TIMEOUT);
                    }
                    // ZENER activated
                    if (ThisEvent.EventParam == 3){
                        SELF_ZENER = true;
                        PostZENER(ActivateEvent);
                        ES_Timer_StopTimer(ZENER_TIMEOUT);
                    }
                }
                break;

                case ES_REENTRY: 
                {
                    printf("ES_REENTRY: %u\n", ThisEvent.EventParam);
                    ES_Event_t DeactivateEvent;
                    DeactivateEvent.EventType = ES_DEACTIVATE;
                    // OPAMP re-entry
                    if (ThisEvent.EventParam == 1){
                        if (SELF_OPAMP){
                            SELF_OPAMP = false;
                            PostOPAMP(DeactivateEvent);
                            ES_Timer_InitTimer(OPAMP_TIMEOUT, TIMEOUT_PERIOD);
                        } 
                    }
                    // TVS re-entry
                    if (ThisEvent.EventParam == 2){
                        if (SELF_TVS){
                            SELF_TVS = false;
                            PostTVS(DeactivateEvent);
                            ES_Timer_InitTimer(TVS_TIMEOUT, TIMEOUT_PERIOD);
                        }
                    }
                    // ZENER re-entry
                    if (ThisEvent.EventParam == 3){
                        if (SELF_ZENER){
                            SELF_ZENER = false;
                            PostZENER(DeactivateEvent);
                            ES_Timer_InitTimer(ZENER_TIMEOUT, TIMEOUT_PERIOD);
                        }
                    }
                }
                break;

                case ES_TIMEOUT:
                {
                    ES_Event_t DropoutEvent;
                    DropoutEvent.EventType = ES_DROPOUT;
                    if (ThisEvent.EventParam == OPAMP_TIMEOUT){
                        DropoutEvent.EventParam = 1;
                    } else if (ThisEvent.EventParam == TVS_TIMEOUT){
                        DropoutEvent.EventParam = 2;
                    } else if (ThisEvent.EventParam == ZENER_TIMEOUT){
                        DropoutEvent.EventParam = 3;
                    }
                    PostZENER(DropoutEvent);
                }
                break;

                default:
                {}
                break;
            }
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
// RB9 is UX-TX, RB11 is UX-RX
// Transmit: Every 28ms, send a message
// Receive: Interrupt for receive and post 

static void InitCommunication(){
    TRISBbits.TRISB9 = 0;           // RB9 to output
    TRISBbits.TRISB11 = 1;          // RB11 to input
    RPB9R = 0b0010;                 // RB9 to U2TX
    U2RXR = 0b0011;                 // RB11 to U2RX
    // UART2 Settings 
    U2MODEbits.ON = 0;
    U2MODEbits.BRGH = 1;            // high-speed mode (4x baud clock)
    U2STA = 0;                      // clear status register
    U2BRG = 520;                    // 9600 baud rate
    // Receive Settings 
    U2STAbits.URXEN = 1;            // enable receive
    U2STAbits.URXISEL = 0b00;       // interrupts when Rx buffer not empty
    IPC9bits.U2IP = 7;              // set priority of interrupt to 7
    IEC1SET = _IEC1_U2RXIE_MASK;    // interrupt local enable 
    IFS1CLR = _IFS1_U2RXIF_MASK;    // clear interrupt flag 
    INTCONbits.MVEC = 1;            // Enable MVEC
    __builtin_enable_interrupts();  // Interrupt global enable 
    // Transmit Settings 
    U2STAbits.UTXEN = 1;            // enable transmit 
    U2MODEbits.ON = 1;              // turn peripheral on
}

static void InitTimer(){
    T2CONbits.ON = 0;               // Turn off timer 2
    T2CONbits.TCS = 0;              // Internal clock
    T2CONbits.TCKPS = 0b100;        // Prescaler of 16 for 200/7 ms
    TMR2 = 0;                       // Clear timer 
    PR2 = 35714;                    // Set period register for 200/7 ms
    IPC2bits.T2IP = 7;              // Set priority of interrupt to 7
    IEC0SET = _IEC0_T2IE_MASK;      // Interrupt local enable
    IFS0CLR = _IFS0_T2IF_MASK;      // Clear interrupt flag
    T2CONbits.ON = 1;               // Turn on timer 2
}

static void ActivateSystems(){
    ES_Event_t ActivateEvent;
    ActivateEvent.EventType = ES_ACTIVATE;
    ES_Event_t DeactivateEvent;
    DeactivateEvent.EventType = ES_DEACTIVATE;
    if (SELF == OPAMP){
        SELF_OPAMP = true;
        PostOPAMP(ActivateEvent);
        PostTVS(DeactivateEvent);
        PostZENER(DeactivateEvent);
        ES_Timer_InitTimer(TVS_TIMEOUT, TIMEOUT_PERIOD);
        ES_Timer_InitTimer(ZENER_TIMEOUT, TIMEOUT_PERIOD);
    } else if (SELF == TVS){
        SELF_TVS = true;
        PostOPAMP(DeactivateEvent);
        PostTVS(ActivateEvent);
        PostZENER(DeactivateEvent);
        ES_Timer_InitTimer(OPAMP_TIMEOUT, TIMEOUT_PERIOD);
        ES_Timer_InitTimer(ZENER_TIMEOUT, TIMEOUT_PERIOD);
    } else if (SELF == ZENER){
        SELF_ZENER = true;
        PostOPAMP(DeactivateEvent);
        PostTVS(DeactivateEvent);
        PostZENER(ActivateEvent);
        ES_Timer_InitTimer(OPAMP_TIMEOUT, TIMEOUT_PERIOD);
        ES_Timer_InitTimer(TVS_TIMEOUT, TIMEOUT_PERIOD);
    }
}

static struct TeamTxPacket CheckAssumeCommand(struct TeamTxPacket PacketToSend){
    // If assume command is active, will set the bit appropriately 
    if (AssumeCommand){
        PacketToSend.LocalTxPacket.status = (PacketToSend.LocalTxPacket.status | BIT5HI);
        AssumeCommand = false;
    }
    return PacketToSend;
}


/*------------------------------- Interrupts -------------------------------*/

void __ISR(_TIMER_2_VECTOR, IPL7SOFT) TransmitISR(){
  // clear flag 
  IFS0CLR = _IFS0_T2IF_MASK;
  TMR2 = 0;
  // post event to state machine
  ES_Event_t TransmitEvent;
  TransmitEvent.EventType = ES_TRANSMIT;
  TransmitEvent.EventParam = RoundRobinNum % 7;
  PostComms(TransmitEvent);
  RoundRobinNum++;
  if (RoundRobinNum == 7){
      RoundRobinNum = 0;
  }
}

void __ISR(_UART_2_VECTOR, IPL7SOFT) ReceiveISR(){
    if (IFS1bits.U2RXIF) {
        // clear flag
        IFS1CLR = _IFS1_U2RXIF_MASK;
        // if there was an overrun, clear it to reset the unit
        if(U2STAbits.OERR){ 
              U2STAbits.OERR = 0;
        }
        // read content of the receive register
        while (U2STAbits.URXDA) {
            uint8_t msg = U2RXREG;
            rxstoreBuf(msg);
        }
    }
}

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/