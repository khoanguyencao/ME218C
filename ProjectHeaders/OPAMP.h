/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef OPAMP_H
#define OPAMP_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "PackDataHandler.h"

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitOPAMPState, Alive, Dead, Deactivated
}OPAMPState_t;

enum { IS_TEAM, IS_BROADCAST };
static int RXMSG = IS_TEAM;

// Public Function Prototypes

bool InitOPAMP(uint8_t Priority);
bool PostOPAMP(ES_Event_t ThisEvent);
ES_Event_t RunOPAMP(ES_Event_t ThisEvent);

#endif /* OPAMP_H */

