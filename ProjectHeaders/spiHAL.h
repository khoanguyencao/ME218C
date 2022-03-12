/* 
 * File:   spiHAL.h
 * Author: maez
 *
 * Created on October 4, 2020, 10:00 PM
 */

#include <stdint.h>

#ifndef SPIHAL_H
#define	SPIHAL_H

/*------------------------------ Header Defines ------------------------------*/

#define SHIFT_8BITS          256
#define SHIFT_16BITS         65536
#define SHIFT_24BITS         16777216

/*--------------------------- Function Prototypes ----------------------------*/

unsigned char spi_Init(uint8_t, uint32_t);
unsigned char spi_Write(uint32_t);
unsigned char spi_HasTransferCompleted(void);
unsigned char spi_HasXmitBufferSpaceOpened(void);
uint8_t spi_GetNumOpenXmitSpaces(void);

#endif	/* spiHAL_H */

