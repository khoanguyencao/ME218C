/* 
 * File:   playerFSM.c
 * Author: Kyle Denton
 *
 * Revision: 
 * 
 * Description: 
 */

/*----------------------------- Include Files -----------------------------*/
// This module
#include "../ProjectHeaders/ZENER.h"

// Hardware
#include <xc.h>
#include <proc/p32mx170f256b.h>
#include <sys/attribs.h> // for ISR macros
#include <math.h>

// Event & Services Framework
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"
#include "ES_Port.h"
#include "OLEDService.h"
#include "PackDataHandler.h"
#include "Comms.h"
#include "CommsRx.h"
#include "CommsTx.h"
#include "joystickHelpers.h"

/*----------------------------- Module Defines ----------------------------*/
//#define X_DIR               
//#define Y_DIR               

#define X_INIT              0x3412 
#define Y_INIT              0x7856 
#define MAX_NUMBER_TEAMS    7
#define OUR_TEAM            5
//#define REFRESH_TIME        200 // 5Hz
#define REFRESH_TIME        200 // faster debugging

#define TEAM_NUMBER     5
#define TN_FLAG         (TEAM_NUMBER<<5) // shift left five places         
#define ALIVE           BIT1HI
#define DEAD            BIT1LO
#define FIRED           BIT0HI
#define NOT_FIRED       BIT0LO
 
#define ZENER_BIT       BIT4HI
#define HIT             BIT0HI
#define CLEAR           BIT0LO 

#define GOTO(x,y)       printf("\033[%d;%dH",y,x);
#define CLEAR_SCREEN()  printf("\033[H\033[J");
#define BELL()          printf("\007");
#define CLR()             printf("\033]50;ClearScrollback\007");

/*---------------------------- Module Functions ---------------------------*/
void updateAcceleration(void);
void limitAcceleration(void);
void updateShipVelocity(void);
void limitVelocity(void);
//void broadcastInfo(void);
void updateShipPosition(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static playerState_t CurrentState;
static struct ship_t playerShip;

static uint8_t allocatedEnergy;
static struct enemy_t enemyArray [MAX_NUMBER_TEAMS];

//static uint8_t AD_Flag;
//static uint8_t WF_Flag;
static uint8_t HullStrength;
static uint8_t Beam_dx_msb;
static uint8_t Beam_dx_lsb;
static uint8_t Beam_dy_msb;
static uint8_t Beam_dy_lsb;
static uint8_t ShieldStrength;
static uint8_t HitBit;

//static struct BroadcastRxPacket realB0RxPacket;
//static struct BroadcastRxPacket realB1RxPacket;
//static struct BroadcastRxPacket realB2RxPacket;
//static struct BroadcastRxPacket realB3RxPacket;
//static struct BroadcastRxPacket realB4RxPacket;
//static struct BroadcastRxPacket realB5RxPacket;
//static struct BroadcastRxPacket realB6RxPacket;

static struct BroadcastRxPacket *BroadcastPtrArray[7];



/*------------------------------ Module Code ------------------------------*/
/****
 Function
     InitZENER

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
****/
bool InitZENER(uint8_t Priority)
{
    initJoystick();     // initialize joystick
//    CLEAR_SCREEN();
//    GOTO(0,0);
//    printf("1");
//    GOTO(1,1);
//    printf("2");
//    GOTO(50,1);
//    printf(".");
//    GOTO(100,1);
//    printf(".");
//    GOTO(150,1);
//    printf(".");
    
    allocatedEnergy = 64;
    
    playerShip.x = X_INIT;
    playerShip.ideal_dx = 0;
    playerShip.limited_dx = 0;
    playerShip.ideal_ddx = 0;
    playerShip.limited_ddx = 0;
    playerShip.y = Y_INIT;
    playerShip.ideal_dy = 0;
    playerShip.limited_dy = 0;
    playerShip.ideal_ddy = 0;
    playerShip.limited_ddy = 0;

    //AD_Flag = ALIVE;
    //WF_Flag = NOT_FIRED;
    HullStrength = 100;
    Beam_dx_msb = 0;
    Beam_dx_lsb = 0;
    Beam_dy_msb = 0;
    Beam_dy_lsb = 0;
    ShieldStrength = 0;
    
    // Starting Buffer
    //BROADCAST_BUFFER.status = TN_FLAG + AD_Flag + WF_Flag;
    BROADCAST_BUFFER.status = TN_FLAG + NOT_FIRED + ALIVE;
    BROADCAST_BUFFER.hull = 100;
    BROADCAST_BUFFER.ship_x_msb = (playerShip.x>>8);
    BROADCAST_BUFFER.ship_x_lsb = (playerShip.x&0xFF);
    BROADCAST_BUFFER.ship_y_msb = (playerShip.y>>8);
    BROADCAST_BUFFER.ship_y_lsb = (playerShip.y&0xFF);
    ZENER_BUFFER.status = ZENER_BIT + CLEAR;
    
    // FOR TESTING PURPOSES ONLY
    ZENER_BUFFER.hull = 0xAA;
    ZENER_BUFFER.beam_dx_lsb = 0x12;
    ZENER_BUFFER.beam_dx_msb = 0x34;
    ZENER_BUFFER.beam_dy_lsb = 0x56;
    ZENER_BUFFER.beam_dy_msb = 0x78;
    ZENER_BUFFER.weaponPower = 0x9A;
    ZENER_BUFFER.navPower = 0xBC;
    ZENER_BUFFER.weaponCap = 0xDE;
    ZENER_BUFFER.shield = 0xF0;
    
    BroadcastPtrArray[0] = &realB0RxPacket;
    BroadcastPtrArray[1] = &realB1RxPacket;
    BroadcastPtrArray[2] = &realB2RxPacket;
    BroadcastPtrArray[3] = &realB3RxPacket;
    BroadcastPtrArray[4] = &realB4RxPacket;
    BroadcastPtrArray[5] = &realB5RxPacket;
    BroadcastPtrArray[6] = &realB6RxPacket;
    
    
    struct enemy_t blankEnemy;
    blankEnemy.x = 0;
    blankEnemy.y = 0;
    blankEnemy.alive = false;
    
    
    uint8_t i = 0;
    for(i; i < MAX_NUMBER_TEAMS; i++) {
        blankEnemy.team = i;
        enemyArray[i] = blankEnemy;
    }
    
    ES_Event_t ThisEvent;
    MyPriority = Priority;              // Set the module's priority
    CurrentState = initPlayerState;     // Select initial state for FSM
    
    ThisEvent.EventType = ES_INIT;      // Initial Player Event
    
    return ES_PostToService(MyPriority, ThisEvent);
}

/****
 Function
     PostZENER

 Parameters
     EF_Event_t ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
****/
bool PostZENER(ES_Event_t ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/****
 Function
    RunZENER

 Parameters
   ES_Event_t : the event to process

 Returns
   ES_Event_t, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
 FSM maintains the position of the ship at each screen refresh and stores the 
 new wall's anchor point when the button is pressed
 
 Notes
   uses nested switch/case to implement the machine.
****/

ES_Event_t RunZENER(ES_Event_t ThisEvent)
{
    ES_Event_t ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    switch (CurrentState)
    {
        case initPlayerState:        // If current state is initial Psedudo State
        {
            switch (ThisEvent.EventType)
            {
                case ES_INIT:
                {

                } break;

                case ES_ACTIVATE:
                {
                    ES_Timer_InitTimer(ZENER_UPDATE_TIMER, REFRESH_TIME);
                    CurrentState = ActiveZener;
                } break;

                case ES_DEACTIVATE:
                {
                    CurrentState = DeactivatedZener;
                } break;

                default: 
                {} break;
            }   
        }
        break;

        case ActiveZener:        // Waiting for a new character to add to the string
        {
            switch (ThisEvent.EventType)
            {
                case ES_TIMEOUT:  
                {
                    if(ThisEvent.EventParam == ZENER_UPDATE_TIMER) {
                        updateAcceleration();
                        updateShipVelocity();
                        updateShipPosition(); 
                        ////printf("(%d,%d)\r\n",playerShip.x*4000,playerShip.y*4000);
                        //printf("Ideal Acc:(%d,%d)\r\n",playerShip.ideal_ddx,playerShip.ideal_ddy);
                        //broadcastInfo();
                        // TODO: POST ES_READ EVENTS TO OPAMP AND TVS for if this machine assumed control
                        PostOLEDService(ThisEvent);
                        ES_Timer_InitTimer(ZENER_UPDATE_TIMER, REFRESH_TIME);
                    } else if (ThisEvent.EventParam == DAMAGE_TIMER) {
                        ZENER_BUFFER.status = ZENER_BIT + CLEAR;
                    }
                }
                break;

                case ES_READ:
                {
                    if (ThisEvent.EventParam >= 0xF0) {
                        uint8_t teamNumber = (ThisEvent.EventParam & 0b111);
                        struct BroadcastRxPacket EnemyBroadcastPacket = *(BroadcastPtrArray[teamNumber]);
                        struct BroadcastData EnemyBroadcastData = EnemyBroadcastPacket.GlobalRxPacket;
                        if((EnemyBroadcastData.status>>5) != teamNumber) { // if there was a mismatch between the team number data and it's location
                            //printf("ENEMY BROADCAST DATA IN WRONG LOCATION\r\n");
                        } else {
                            if((EnemyBroadcastData.status & 0b11) == 0b11) { // if the enemy is alive and fired their weapon
                                uint16_t enemyX = (EnemyBroadcastData.ship_x_msb << 8) + EnemyBroadcastData.ship_x_lsb;
                                uint16_t enemyY = (EnemyBroadcastData.ship_y_msb << 8) + EnemyBroadcastData.ship_y_lsb;
                                int16_t laserDx = (EnemyBroadcastData.beam_dx_msb << 8) + EnemyBroadcastData.beam_dx_lsb;   
                                int16_t laserDy = (EnemyBroadcastData.beam_dy_msb << 8) + EnemyBroadcastData.beam_dy_lsb;
                                bool hit = check4Damage( enemyX, enemyY, laserDx, laserDy);
                                if (hit) {
                                    ZENER_BUFFER.status = ZENER_BIT + HIT;
                                    ES_Timer_InitTimer(DAMAGE_TIMER, REFRESH_TIME); // start timer for 200 ms to make sure damage is sent to OPAMP
                                }
                            }
                        }
                    } else if (ThisEvent.EventParam == 1){ // If received from OPAMP
                        BROADCAST_BUFFER.hull = OPAMP_BUFFER.hull;
                        BROADCAST_BUFFER.shield = OPAMP_BUFFER.shield;
                        allocatedEnergy = OPAMP_BUFFER.navPower;
                        printf("Allocated Energy: %3d\r\n", allocatedEnergy);
                        if (OPAMP_BUFFER.hull == 0) { // If hull strength is 0, we are dead
                            ES_Event_t Event2Post;
                            Event2Post.EventType = ES_DEAD;
                            PostZENER(Event2Post);
                        }
                    } else if (ThisEvent.EventParam == 0){ // If received from TVS
                        BROADCAST_BUFFER.beam_dx_msb = TVS_BUFFER.beam_dx_msb;
                        BROADCAST_BUFFER.beam_dx_lsb = TVS_BUFFER.beam_dx_lsb;
                        BROADCAST_BUFFER.beam_dy_msb = TVS_BUFFER.beam_dy_msb;
                        BROADCAST_BUFFER.beam_dy_lsb = TVS_BUFFER.beam_dy_lsb;
                        
                        int16_t dx = (TVS_BUFFER.beam_dx_msb<<8) + (TVS_BUFFER.beam_dx_lsb);
                        int16_t dy = (TVS_BUFFER.beam_dy_msb<<8) + (TVS_BUFFER.beam_dy_lsb);
                        
                        //printf("dx: %06d  |  dy: %06d\r\n");

                        if ((TVS_BUFFER.status & BIT1HI) == BIT1HI){ // If weapon was fired
                            //WF_Flag = FIRED;
                            printf("FIRED!!!\r\n");
                            BROADCAST_BUFFER.status = TN_FLAG + FIRED + ALIVE;
                        } else {
                            //WF_Flag = NOT_FIRED;
                            BROADCAST_BUFFER.status = TN_FLAG + NOT_FIRED + ALIVE;
                        }
                    }
                } break;

                case ES_DEACTIVATE:
                {
                    CurrentState = DeactivatedZener;
                } break;

                case ES_DEAD:
                {
                    BROADCAST_BUFFER.status = TN_FLAG + NOT_FIRED + DEAD;
                    //AD_Flag = DEAD;
                    CurrentState = DeadZener;
                } break;

                default:
                {} break;
            }  // end switch on CurrentEvent
        } break; // end ActiveZener case

        case DeactivatedZener:
        {
            if (ThisEvent.EventType == ES_ACTIVATE)    
            {
                ES_Timer_InitTimer(ZENER_UPDATE_TIMER, REFRESH_TIME);
                CurrentState = ActiveZener;
            }

        } break;

        case DeadZener:
        {

        } break;

        default:
          ;
    }                                   // end switch on Current State
    return ReturnEvent;
}

/****
 Function
     QueryZENER

 Parameters
     None

 Returns
     playerState_t The current state of the playerFSM state machine

 Description
     returns the current state of the playerFSM state machine
****/
playerState_t QueryZENER(void)
{
    return CurrentState;
}


/***************************************************************************
 private functions
 ***************************************************************************/
void updateAcceleration(void)
{
    struct joyInfo_t currentJoystickInfo = retrieveJoystickInfo();
    playerShip.ideal_ddx = currentJoystickInfo.x;
    playerShip.ideal_ddy = currentJoystickInfo.y;
    playerShip.thrust.idealx = currentJoystickInfo.x * 1000; // up to 64,000 km/s^2 
    playerShip.thrust.idealy = currentJoystickInfo.y * 1000; // up to 64,000 km/s^2
    limitAcceleration();
    
//    GOTO(0,2);
//    //printf("Ideal Acc: (%6d,%6d)\r\n", playerShip.thrust.idealx, playerShip.thrust.idealy);
    //printf("Accel: (%6d,%6d)\r\n", playerShip.limited_ddx, playerShip.limited_ddy);
    
    
}

void limitAcceleration(void)
{
//    double a = (double)playerShip.thrust.idealx;
//    double b = (double)playerShip.thrust.idealy;
//    double c_limit = (double)allocatedEnergy * 1000; // 1GW = 1000 km/s^2
//    
//    if (((a * a) + (b * b)) > (c_limit * c_limit)) { // If ideal acceleration magnitude exceeds what is possible given allocated energy
//        double idealMag = sqrt((a * a) + (b * b)); // Calculate what ideal magnitude is
//        double scaleFactor = idealMag / c_limit; // Calculate what factor the magnitude needs to scale down by
//        double a_scaled = ((double)playerShip.thrust.idealx / scaleFactor); // Scale down the acceleration components
//        double b_scaled = ((double)playerShip.thrust.idealy / scaleFactor);
//        playerShip.thrust.limitedx = (int32_t)(a_scaled); // Cast to integers (automatically rounds down)
//        playerShip.thrust.limitedx = (int32_t)(b_scaled);
//        //printf("a=%f, b=%f, clim=%f sf=%f, alim=%f, alim=%d, blim=%f, blim=%d\r\n", a, b, c_limit, scaleFactor, a_scaled, (int8_t)(a_scaled),b_scaled, (int8_t)(b_scaled) );
//        
//    } else {
//        playerShip.thrust.limitedx = playerShip.thrust.idealx;
//        playerShip.thrust.limitedy = playerShip.thrust.idealy;       
//    }
//    printf("Limit Acc: (%d,%d)\r\n", playerShip.thrust.limitedx, playerShip.thrust.limitedy);
    
    
    double a = (double)playerShip.ideal_ddx;
    double b = (double)playerShip.ideal_ddy;
    double c_limit = (double)allocatedEnergy;
    
    if (((a * a) + (b * b)) > (c_limit * c_limit)) { // If ideal acceleration magnitude exceeds what is possible given allocated energy
        double idealMag = sqrt((a * a) + (b * b)); // Calculate what ideal magnitude is
        double scaleFactor = idealMag / c_limit; // Calculate what factor the magnitude needs to scale down by
        double a_scaled = ((double)playerShip.ideal_ddx / scaleFactor); // Scale down the acceleration components
        double b_scaled = ((double)playerShip.ideal_ddy / scaleFactor);
        playerShip.limited_ddx = (int8_t)(a_scaled); // Cast to integers (automatically rounds down)
        playerShip.limited_ddy = (int8_t)(b_scaled);
        //printf("a=%f, b=%f, clim=%f sf=%f, alim=%f, alim=%d, blim=%f, blim=%d\r\n", a, b, c_limit, scaleFactor, a_scaled, (int8_t)(a_scaled),b_scaled, (int8_t)(b_scaled) );
        
    } else {
        playerShip.limited_ddx = playerShip.ideal_ddx;
        playerShip.limited_ddy = playerShip.ideal_ddy;       
    }
    
}

void updateShipVelocity(void)
{
    playerShip.ideal_dx += playerShip.limited_ddx;
    playerShip.ideal_dy += playerShip.limited_ddy;
    limitVelocity();
    
//    GOTO(0,3);
//    printf("Velocity:     (%6d,%6d)\r\n", playerShip.limited_dx, playerShip.limited_dy);
}

void limitVelocity(void)
{
    double a = (double)playerShip.ideal_dx;
    double b = (double)playerShip.ideal_dy;
    double c_limit = 38400; // 38400 * 7.8125 = 300,000 km/s (speed of light)
    
    if (((a * a) + (b * b)) > (c_limit * c_limit)) { // If ideal velocity magnitude exceeds the speed of light
    //if (false) {    
        double idealMag = sqrt((a * a) + (b * b)); // Calculate what ideal magnitude is
        double scaleFactor = idealMag / c_limit; // Calculate what factor the magnitude needs to scale down by
        double a_scaled = ((double)playerShip.ideal_dx / scaleFactor); // Scale down the velocity components
        double b_scaled = ((double)playerShip.ideal_dy / scaleFactor);
        playerShip.limited_dx = (int32_t)(a_scaled); // Cast to integers (automatically rounds down)
        playerShip.limited_dy = (int32_t)(b_scaled);
        //printf("a=%f, b=%f, clim=%f sf=%f, alim=%f, alim=%d, blim=%f, blim=%d\r\n", a, b, c_limit, scaleFactor, a_scaled, (int8_t)(a_scaled),b_scaled, (int8_t)(b_scaled) );
        
    } else {
        playerShip.limited_dx = playerShip.ideal_dx;
        playerShip.limited_dy = playerShip.ideal_dy;       
    }
}

void updateShipPosition(void)
{
    playerShip.x += playerShip.limited_dx;
    playerShip.y += playerShip.limited_dy;
    
//    GOTO(0,4);
//    printf("Position:     (%6d,%6d)\r\n", playerShip.x, playerShip.y);
    
    
    BROADCAST_BUFFER.ship_x_msb = (playerShip.x>>8);
    BROADCAST_BUFFER.ship_x_lsb = (playerShip.x&0xFF);
    BROADCAST_BUFFER.ship_y_msb = (playerShip.y>>8);
    BROADCAST_BUFFER.ship_y_lsb = (playerShip.y&0xFF);
}

// Used by OLED Service
uint8_t getPlayerX() {
    //return bug1.x;
    return (playerShip.x/512); // go from 16 bit number to 7 bit number
}

uint8_t getPlayerY() {
    //return bug1.y;
    return (playerShip.y/1024); // go from 16 bit number to 6 bit number
}

// Based on solution listed in https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
// Method modified to work in C language

bool check4Damage(uint16_t enemyX, uint16_t enemyY, int16_t laserDx, int16_t laserDy) {
    bool withinRange = false;
    int16_t A = playerShip.x - enemyX;
    int16_t B = playerShip.y - enemyY;
    
    int16_t dot_product = A * laserDx + B * laserDy;
    uint16_t length_squared = laserDx * laserDx + laserDy * laserDy;
    
    float param = -1; //in case of 0 length line
    if (length_squared != 0) {
        param = dot_product / length_squared;
    } 

    uint16_t closestPointX; 
    uint16_t closestPointY;

    if (param < 0) {
      closestPointX = enemyX;
      closestPointY = enemyY;
    }
    else if (param > 1) {
      closestPointX = enemyX + laserDx;
      closestPointY = enemyY + laserDy;
    }
    else {
      closestPointX = enemyX + (int16_t)(param * laserDx);
      closestPointY = enemyY + (int16_t)(param * laserDy);
    }

    int16_t ship2LaserDx = playerShip.x - closestPointX;
    int16_t ship2LaserDy = playerShip.y - closestPointY;
    float distanceShip2Laser = sqrt(ship2LaserDx * ship2LaserDx + ship2LaserDy * ship2LaserDx);
    if (distanceShip2Laser <= 160) {
        withinRange = true;
    }
    return withinRange;
}

uint8_t getNumberOfEnemies(void) {
    return MAX_NUMBER_TEAMS;
}

bool enemyIsAlive(uint8_t i) {
    return enemyArray[i].alive;
}

struct point_t getEnemyLocation(uint8_t i){
    struct point_t enemyLocation;
    enemyLocation.x = enemyArray[i].x / 512; // scale down
    enemyLocation.y = enemyArray[i].y / 1024; // scale down
    return enemyLocation;
}

//void broadcastInfo(void) 
//{
//   BROADCAST_BUFFER.status = TN_FLAG + AD_Flag + WF_Flag;
//   return;
//}


/*------------------------------- Footnotes -------------------------------*/

/*------------------------------ End of file ------------------------------*/


