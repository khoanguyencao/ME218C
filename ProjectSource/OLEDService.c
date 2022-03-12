/****************************************************************************
 Module
   ScrollingService.c

 Description
   This state machine scrolls letters across an OLED display

****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_General.h"
#include "OLEDService.h"
#include "ZENER.h"
#include "dbprintf.h"


#include "../u8g2Headers/u8g2TestHarness_main.h"
#include "../u8g2Headers/common.h"
#include "../u8g2Headers/spi_master.h"
#include "../u8g2Headers/u8g2.h"
#include "../u8g2Headers/u8x8.h"
#include "string.h"

/*----------------------------- Module Defines ----------------------------*/
// Fonts
#define MAIN_FONT                   u8g2_font_oskool_tr

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/


/*---------------------------- Module Variables ---------------------------*/

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

// Create variables used to initialize OLED:
extern uint8_t u8x8_pic32_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
extern uint8_t u8x8_byte_pic32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

static u8g2_t u8g2;

// Start with a blank buffer string
static char buffer [14] = "     ooo      ";     // 14 characters fit on the display

// To be used in the event checker 
static uint8_t prevPageState;

// Create deferral queue for spaces
static ES_Event_t ExtraCharacterQueue[10]; 

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitTemplateFSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

 Author
     J. Edward Carryer, 10/23/11, 18:55
****************************************************************************/
bool InitOLEDService(uint8_t Priority)
{
  ES_Event_t ThisEvent;

  MyPriority = Priority;
  // put us into the Initial PseudoState
  
  ES_InitDeferralQueueWith( ExtraCharacterQueue, ARRAY_SIZE(ExtraCharacterQueue));
  
  // Set up the SPI
  SPI_Init(); 
  // post the initial transition event
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
     PostTemplateFSM

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostOLEDService(ES_Event_t ThisEvent)
{
  return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTemplateFSM

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   uses nested switch/case to implement the machine.
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event_t RunOLEDService(ES_Event_t ThisEvent)
{
  ES_Event_t ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

switch (ThisEvent.EventType)
{
    case ES_INIT:
    {
        // build up the u8g2 structure with the proper values for our display
        u8g2_Setup_ssd1306_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_pic32_hw_spi, 
                                           u8x8_pic32_gpio_and_delay);
        // pass all that stuff on to the display to initialize it
        u8g2_InitDisplay(&u8g2);
        // turn off power save so that the display will be on
        u8g2_SetPowerSave(&u8g2, 0);
        // choose the font. this one is mono-spaced and has a reasonable size
        u8g2_SetFont(&u8g2, MAIN_FONT);
        
        // overwrite the background color of newly written characters
        u8g2_SetFontMode(&u8g2, 0);
        
        // clear screen
        u8g2_ClearBuffer(&u8g2);

        // Draw the string on the display
        //u8g2_DrawStr(&u8g2, 0, 37, buffer);
        u8g2_SendBuffer(&u8g2);
    } break;
    

    case ES_TIMEOUT:
    {
        if (ThisEvent.EventParam == ZENER_UPDATE_TIMER)
        {
            // set up for a screen update always start an update with u8g2_FirstPage
            u8g2_ClearBuffer(&u8g2);

            u8g2_SetDrawColor(&u8g2, 2);
            u8g2_SetBitmapMode(&u8g2,1);

            //u8g2_DrawXBM(&u8g2, getPlayerX(), getPlayerY(), player_image_width, player_image_height, player_image_bits);
            u8g2_DrawPixel(&u8g2, getPlayerX(), getPlayerY());

            uint8_t i = 0;

            for(i; i < getNumberOfEnemies(); i++) {
                if(enemyIsAlive(i)) {
                    struct point_t enemyLocation = getEnemyLocation(i);
                    u8g2_DrawPixel(&u8g2, enemyLocation.x, enemyLocation.y);
                }
            }

            u8g2_UpdateDisplay(&u8g2);// update display            
        }
    }
    break;

    default:
    {
        DB_printf("Event missed in ScrollingService\r\n");
        DB_printf("Event Type: %d", ThisEvent.EventType);
    }
}  // end events switch
    
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

