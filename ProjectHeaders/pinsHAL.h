/* 
 * File:   pinsHAL.h
 * Author: maez
 *
 * Created on September 28, 2020, 10:05 PM
 */

#include <stdint.h>

#ifndef PINSHAL_H
#define	PINSHAL_H

/*------------------------------ Header Defines ------------------------------*/

#define HIGH                1
#define LOW                 0
#define INPUT               1
#define OUTPUT              0

/*--------------------------- Function Prototypes ----------------------------*/

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
uint8_t digitalRead(uint8_t pin);

#endif	/* pinsHAL_H */

