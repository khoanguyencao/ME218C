/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef TVS_H
#define TVS_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "PackDataHandler.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitTVSPState, Aiming, Targetted, Down
} TVSState_t;

// Public Function Prototypes

bool InitTVS(uint8_t Priority);
bool PostTVS(ES_Event_t ThisEvent);
ES_Event_t RunTVS(ES_Event_t ThisEvent);

bool CheckButtonEvents(); // Event checker to check button down

#endif /* TVS_H */



