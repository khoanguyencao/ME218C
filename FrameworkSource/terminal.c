/****************************************************************************
 Module
   terminal.c

 Revision
   1.0.1

 Description
  File holds functions for printing and receiving characters to/from the serial
  emulator through a UART-USB bridge interface.
 Notes
  For the PIC32 port, we are using UART 1

 History
 When           Who     What/Why
 -------------- ---     --------
 08/29/20 14:46 ram     first pass
 10/05/20 19:38 ram     starting work on PIC32 port
 ***************************************************************************/

/*----------------------------- Include Files -----------------------------*/

// Hardware
#include <xc.h>
#include "ES_Port.h"

#include <stdio.h>

//this module
#include "terminal.h"
/*----------------------------- Module Defines ----------------------------*/
#define BAUD_CONST 42 // sets up baud rate for 115200

//#define TEST
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/

/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/
/*******************************************************************************
 * Function: TerminalInit
 * Arguments: None
 * Returns nothing
 * 
 * Created by: R. Merchant
 * Description: Sets up UART1 to serve as the terminal interface. Sets the baud
 * rate and maps the needed pins
 ******************************************************************************/
void Terminal_HWInit(void)
{
#define USE_RB2_3
#ifdef USE_RB2_3
  // Set up RA2 as RX and RB4 as TX
  // clear analog control
  ANSELA &= BIT2LO;
  ANSELB &= BIT4LO;
  // start with TX high
  LATBbits.LATB4 = 1;
  // set RB3 as output
  TRISBbits.TRISB4 = 0;
  // set RA2 as input
  TRISAbits.TRISA2 = 1;
  // Remap the pins
  RPB4R = 0b0001; // U1TX -> RB4
  U1RXR = 0b0000; // U1RX <- RA2
#else
  ANSELB &= (BIT6LO & BIT7LO);
  LATBbits.LATB7 = 1;
  TRISBbits.TRISB7 = 0;
  TRISBbits.TRISB6 = 1;
  RPB7R = 0b0001; // U1TX -> RB3
  U1RXR = 0b0000; // U1RX <- RB2

#endif  
  // diable the UART to be safe
  U1MODEbits.ON = 0;
  // Setup high-speed mode, data = 8 bit, no parity, 1 stop bit
  // Disable: loop-back, autobaud, wake, inversion
  U1MODEbits.BRGH = 1;
  // Diable TX inversion, everything else we don't care about
  U1STA = 0;
  // Set the baud rate based on the constant
  U1BRG = BAUD_CONST;
  
  // redirect printf to UART1 using X32 built in cross over
  __XC_UART = 1; 
  
  // enable the UART module
  U1STAbits.UTXEN = 1; // enable transmit 
  U1STAbits.URXEN = 1; // enable receive
  U1MODEbits.ON = 1; // turn peripheral on
  
  return;
}
/*******************************************************************************
 * Function: Terminal_ReadByte
 * Arguments: None
 * Returns byte
 * 
 * Created by: R. Merchant
 * Description: Read the byte from the receive register
 ******************************************************************************/
uint8_t Terminal_ReadByte(void)
{
  // wait for there to be something
  while(!(U1STAbits.URXDA))
  {}
  // if there was an overrun, clear it to reset the unit
  if(U1STAbits.OERR) U1STAbits.OERR = 0;
  // return the content of the receive register
  return U1RXREG;
}
/*******************************************************************************
 * Function: Terminal_Write
 * Arguments: byte to write
 * Returns nothing
 * 
 * Created by: R. Merchant
 * Description: Writes the byte to the transmit register
 ******************************************************************************/
void Terminal_WriteByte(uint8_t txByte)
{
  // wait for the register to empty
  while(U1STAbits.UTXBF)
  {}
  // write the byte to the register
  U1TXREG = txByte;
  return;
}
/*******************************************************************************
 * Function: Terminal_IsRxData
 * Arguments: none
 * Returns status
 * 
 * Created by: R. Merchant
 * Description: Returns true if there is data in the receive register, or false
 *              if not
 ******************************************************************************/
bool Terminal_IsRxData(void)
{
  // Return Rx Data bit from status register
  return U1STAbits.URXDA;
}

/*******************************************************************************
 * Function: write
 * Arguments: array handle, pointer to array, length of array
 * Returns status
 * 
 * Created by: Microchip
 * Description: Helper function for printf. Fills in the write function that
 *              belongs to the stdio.c file. 
 ******************************************************************************/
#ifdef __XC16__  // DEPRICATED, USE FOR xc16 of xc32 v1.34 or lower
int write(int handle, void *buffer, unsigned int len)
{
    unsigned int i;

    for (i = len; i; --i)
    {
        Terminal_WriteByte(*(char*)buffer++);
    }
    return(len);
}
#endif
/***************************************************************************
 private functions
 ***************************************************************************/
// module test harness:
#ifdef TEST
int main(void)
{
  
  // initialize
  //_HW_Oscillator_Init();
  Terminal_HWInit();
  // test a print
  printf("Hello World\n\r");
  //Terminal_WriteByte('H');
  while(1) //hang out in this loop forever, polling key hits
  {
    // test for tracking keys
    if(IsNewKeyReady())
    {
      // make sure we are getting the correct key
      uint8_t key = GetNewKey();
      printf("\rNew Key Received-> %c <- in test loop\r\n", key);
    }
  }
  return 0;
}
#endif
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
