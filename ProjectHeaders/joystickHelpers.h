/* 
 * File:   joystick.h
 * Author: Ashley B. Nguyen
 *
 * Created on November 3, 2020, 4:41 PM
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef JOYSTICK_H
#define	JOYSTICK_H

// Definition of a Cartesian point object
struct joyInfo_t
{
    int8_t x;
    int8_t y;
};

/* function prototypes */
void initJoystick(void);
struct joyInfo_t retrieveJoystickInfo(void);
bool Check4JoystickToggleY(void);
bool Check4JoystickToggleX(void);
//int16_t getJoystickY(void);
//int16_t getJoystickX(void);

#endif	/* JOYSTICK_H */

