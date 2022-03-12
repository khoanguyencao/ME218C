/* 
 * File:   playerFSM.h
 * Author: Kyle Denton
 *
 */

#ifndef ZENERFSM_H
#define	ZENERFSM_H

/*------------------------------ Header Include ------------------------------*/
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

#include "ES_Events.h"
#include "ES_Port.h"

/*------------------------------ Header Defines ------------------------------*/


/*------------------------------ Header Objects ------------------------------*/

// List of states for the player
typedef enum
{
    initPlayerState, ActiveZener, DeactivatedZener, DeadZener

}playerState_t;

// Definition of a Cartesian point object
struct point_t
{
    uint16_t x;
    uint16_t y;
};

struct enemy_t
{
    uint8_t team;
    uint16_t x;
    uint16_t y;
    bool alive;
};

struct acceleration_t
{
    int32_t idealx;
    int32_t idealy;
    int32_t limitedx;
    int32_t limitedy;
};

struct velocity_t
{
    int32_t idealx;
    int32_t idealy;
    int32_t limitedx;
    int32_t limitedy;
};

struct position_t
{
    uint32_t literalx;
    uint32_t literaly;
    uint16_t scaledx;
    uint16_t scaledy;
};

// Definition of the player's ship object
struct ship_t
{
    struct acceleration_t thrust;
    struct velocity_t v;
    struct position_t pos;
    uint16_t x;
    int32_t ideal_dx;
    int32_t limited_dx;
    int8_t ideal_ddx;
    int8_t limited_ddx;
    uint16_t y;
    int32_t ideal_dy;
    int32_t limited_dy;
    int8_t ideal_ddy;
    int8_t limited_ddy;
};





/*------------------------ Public Function Prototypes ------------------------*/

bool InitZENER(uint8_t Priority);
bool PostZENER(ES_Event_t ThisEvent);
ES_Event_t RunZENER(ES_Event_t ThisEvent);
playerState_t QueryZENER(void);

uint8_t getPlayerX();
uint8_t getPlayerY();

uint8_t getNumberOfEnemies(void);
bool enemyIsAlive(uint8_t i);
struct point_t getEnemyLocation(uint8_t i);

bool check4Damage(uint16_t enemyX, uint16_t enemyY, int16_t laserDx, int16_t laserDy);


#endif	/* ZENERFSM_H */

