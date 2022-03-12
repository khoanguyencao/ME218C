/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef COMMS_H
#define COMMS_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Framework.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitPState, Idle, Transmitting, Receiving
}CommsState_t;

typedef enum 
{
  OPAMP, TVS, ZENER, NONE
} SelfState_t;

// Buffers are read/write if your system, otherwise should be read-only
struct TeamData OPAMP_BUFFER;
struct TeamData TVS_BUFFER;
struct TeamData ZENER_BUFFER;
struct BroadcastData BROADCAST_BUFFER;

SelfState_t SELF;

// Active Stations
bool OPAMP_ACTIVE;
bool TVS_ACTIVE;
bool ZENER_ACTIVE;

uint8_t TransmitNum;

// Public Function Prototypes

bool InitComms(uint8_t Priority);
bool PostComms(ES_Event_t ThisEvent);
ES_Event_t RunComms(ES_Event_t ThisEvent);

#endif /* COMMS_H */

