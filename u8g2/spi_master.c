#include "../u8g2Headers/spi_master.h"
 //tweaked for PIC32MX170F256B, uses SPI1
         
/****************************************************************************
 Function
    SPI_Init
 Parameters
    void
 Returns
    void
 Description
    set up the SPI system for use. Set the clock phase and polarity, master mode
    clock source, baud rate, SS control, transfer width (8-bits)
 Notes
    don't forget to map the pins & set up the TRIS not only for the SPI pins
    but also for the DC (RB13) & Reset (RB12) pins 
 
****************************************************************************/

void SPI_Init(void){
    //Clear all ANSEL bits to turn off all analog function
    ANSELA = 0;
    ANSELB = 0;
    
    
    RPA0R = 0b0011;                 //Map A0 to SS
    RPA1R = 0b0011;                 // Map A1 to SDO
    
    //Make necessary pins outputs
    TRISAbits.TRISA0 = 0; 
    TRISAbits.TRISA1 = 0;
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB12 = 0;
    TRISBbits.TRISB13 = 0;
    
    
    SPI1CONbits.ON = 0;             // Clear ON bit of SPI1CON
    SPI1BUF;                        // Clear receive buffer
    SPI1BRG = 0;                    // 10 MHz
    SPI1CONbits.ENHBUF = 0;         // Set ENHBUF low
 
    SPI1STATbits.SPIROV = 0;        // Clear SPIROV bit in STAT register
    
    SPI1CONbits.MSTEN = 1;          // Set as master
    SPI1CON2bits.AUDEN = 0;
    SPI1CONbits.MODE16 = 0;         // 8 bit width
    SPI1CONbits.MODE32 = 0;         // 8 bit width
    
    // Clock Setup:
    SPI1CONbits.MCLKSEL = 0;
    SPI1CONbits.CKP = 1;            //Set clock polarity to idle high
    SPI1CONbits.CKE = 0;            //Set CKE to low
    
    // SS Setup:
    SPI1CONbits.SSEN = 1;
    SPI1CONbits.FRMPOL = 0;         //Set FRMPOL low for active low
    SPI1CONbits.MSSEN = 1;          //Set MSSEN high
    
    SPI1CONbits.ON = 1;             //Set ON bit of SPI1CON
    
}
/****************************************************************************
 Function
    SPI_Tx
 Parameters
   uint8_t data   the 8-bit value to be sent out through the SPI
 Returns
    void
 Description
    write the data to the SPIxBUF and then wait for it to go out (SPITBF)
 Notes
    don't forget to read the buffer after the transfer to prevent over-runs
 
****************************************************************************/
void SPI_Tx(uint8_t data){
    while(SPI1STATbits.SPITBF) {}       // Wait until Tx buffer isn't full
    SPI1BUF = data;                     // send data to the buffer
    SPI1BUF;                            // clear the buffer 
}

/****************************************************************************
 Function
    SPI_TxBuffer
 Parameters
   uint8_t *buffer, a pointer to the buffer to be transmitted 
   uint8_t length   the number of bytes in the buffer to transmit
 Returns
    void
 Description
   loop through buffer calling SPI_Tx for each element in the buffer
 Notes

 
****************************************************************************/
void SPI_TxBuffer(uint8_t *buffer, uint8_t length){
    
    uint32_t i = 0;
    // For each element in the buffer
    for (i; i < length; i++) {
        // Send it to the Tx function 
        SPI_Tx(*(buffer + i));
    }
    
}
