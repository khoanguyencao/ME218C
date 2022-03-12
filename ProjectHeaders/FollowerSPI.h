/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef FollowerSPI_H
#define FollowerSPI_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitPState, YesBaton
}TemplateState_t;

// Public Function Prototypes

bool InitSPIFollower(uint8_t Priority);
bool PostSPIFollower(ES_Event_t ThisEvent);
ES_Event_t RunSPIFollower(ES_Event_t ThisEvent);
TemplateState_t QuerySPIFollower(void);

#endif /* FollowerSPI_H */

