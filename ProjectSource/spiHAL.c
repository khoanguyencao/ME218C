
/* 
 * File:   pins_PIC32_HAL.c
 * Author: maez
 *
 * Created on October 4, 2020, 10:13 PM
 */

#include "spiHAL.h"
#include "pinsHAL.h"
#include <xc.h>


/*------------------------- Function Implementation --------------------------*/

unsigned char spi_Init(uint8_t bitWidth, uint32_t bitRate)
{
    uint32_t PERIPHERAL_BUS_CLOCK = 20000000; // 20MHz
    uint32_t FASTEST = 10000000;  // 10MHz
    uint32_t SLOWEST = 1220;  // 1,220Hz
    
    unsigned char success = 1;  // Start as true, becomes false upon failure
    
    // 0. Set pins as OUTPUTS and disable analog function (happens within pinMode)
    pinMode(26, OUTPUT);  // SCK pin (RB15)
    pinMode(10, OUTPUT);   // SS2 pin (RA3)
    pinMode(14, OUTPUT);  // SDO2 pin (RB5)
            
    // 1. Map SDO, SCK, and SS outputs
    RPA3Rbits.RPA3R = 0b0100;   // SS2 to RA3 pin
    RPB5Rbits.RPB5R = 0b0100;   // SDO2 to RB5 pin
    
    // 2. No SDI for now
    
    // 3. Stop and reset SPI
    SPI2CONbits.ON = 0;
    
    // 4. Clear receive buffer
    uint32_t dummy;
    dummy = SPI2BUF;
    
    //5. Enable enhance mode
    SPI2CONbits.ENHBUF = 1;
    
    //6. Write Baud Rate
    
    if ( (bitRate < SLOWEST) || (bitRate > FASTEST))
    {
        success = 0;
    }
    else
    {
        uint32_t baud = PERIPHERAL_BUS_CLOCK / (2UL * bitRate) - 1UL;
    
        SPI2BRG = baud;
    }
    
    // 7. Clear SPIROV bit
    SPI2STATbits.SPIROV = 0;
    
    // 8. SPIxCON settings
    SPI2CONbits.MSSEN = 1;      // Automatic Slave Select
    SPI2CONbits.MCLKSEL = 0;    // Select PBCLK for Baud Rate Generator
    SPI2CONbits.DISSDO = 0;     // Enable SDO
    SPI2CONbits.SMP = 0;        // Sample at middle of data output time
    SPI2CONbits.CKE = 0;        // Data changes on transition from idle to active clock
    SPI2CONbits.CKP = 1;        // Clock is idle HIGH (active LOW)
    SPI2CONbits.MSTEN = 1;      // Master mode
    SPI2CONbits.DISSDI = 1;     // Disable SDI
    
    switch(bitWidth)
    {
        case 8:
            SPI2CONbits.MODE16 = 0;     // Set bit width to 8-bits
            SPI2CONbits.MODE32 = 0;     // ^^
            break;
        
        case 16:
            SPI2CONbits.MODE16 = 1;     // Set bit width to 16-bits
            SPI2CONbits.MODE32 = 0;     // ^^
            break;
            
        case 24:
            SPI2CONbits.MODE16 = 1;     // Set bit width to 24-bits
            SPI2CONbits.MODE32 = 1;     // ^^
            break;
            
        case 32:
            SPI2CONbits.MODE16 = 0;     // Set bit width to 32-bits
            SPI2CONbits.MODE32 = 1;     // ^^
            break;
            
        default:
            success = 0;
            break;
    }
    
    // 9. Enable SPI
    SPI2CONbits.ON = 1;
    
    // Empty for loop to allow SS1 line to go high
    uint16_t i;
    for (i = 0; i < 400; i = i + 1){}
    
}


unsigned char spi_Write(uint32_t my32bits)
{
    unsigned char success = 1;
    
    if (SPI2STATbits.SPITBF)
    {
        success = 0;    // Transmit buffer is FULL
    }
    else
    {
        SPI2BUF = my32bits;
    }
    
    return success;
}


unsigned char spi_HasTransferCompleted(void)
{
    unsigned char complete = 1;
    
    if (!SPI2STATbits.SRMT)
    {
        complete = 0;
    }
    
    return complete;
}


unsigned char spi_HasXmitBufferSpaceOpened(void)
{
    unsigned char open = 1;
    
    if (SPI2STATbits.SPITBF)
    {
        open = 0;
    }
    
    return open;
}


uint8_t spi_GetNumOpenXmitSpaces(void)
{
    return SPI2STATbits.TXBUFELM;
}