/****************************************************************************

  Header file for template Flat Sate Machine
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef ScrollingService_H
#define ScrollingService_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
typedef enum
{
  InitOLEDState, NotWriting, Writing
}ScrollingServiceState_t;

// Public Function Prototypes

bool InitOLEDService(uint8_t Priority);
bool PostOLEDService(ES_Event_t ThisEvent);
ES_Event_t RunOLEDService(ES_Event_t ThisEvent);

bool Check4NextPage(void);

#endif /* ScrollingService_H */

