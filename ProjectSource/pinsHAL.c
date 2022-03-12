
/* 
 * File:   pins_PIC32_HAL.c
 * Author: maez
 *
 * Created on September 29, 2020, 09:57 PM
 */


#include "pinsHAL.h"
#include <xc.h>



/*------------------------- Function Implementation --------------------------*/


void pinMode(uint8_t pin, uint8_t mode)
{
    switch(pin)
    {
        case 2:

            ANSELAbits.ANSA0 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISAbits.TRISA0 = 1;
                    break;
                    
                case OUTPUT:
                    TRISAbits.TRISA0 = 0;
                    break;
              
            }
            break;

        case 3:

            ANSELAbits.ANSA1 = 0;

            switch(mode)
            {
               case INPUT:
                    TRISAbits.TRISA1 = 1;
                    break;
                    
                case OUTPUT:
                    TRISAbits.TRISA1 = 0;
                    break;
             
            }
            break;
        
        case 9:
            switch(mode)
            {
                case INPUT:
                    TRISAbits.TRISA2 = 1;
                    break;
                    
                case OUTPUT:
                    TRISAbits.TRISA2 = 0;
                    break;
                
               
            }
            break;
        
        case 10:
            switch(mode)
            {
                case INPUT:
                    TRISAbits.TRISA3 = 1;
                    break;
                    
                case OUTPUT:
                    TRISAbits.TRISA3 = 0;
                    break;
                
            }
            break;
        
        case 12:
            switch(mode)
            {
                case INPUT:
                    TRISAbits.TRISA4 = 1;
                    break;
                    
                case OUTPUT:
                    TRISAbits.TRISA4 = 0;
                    break;
                
            }
            break;
        
        case 4:

            ANSELBbits.ANSB0 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB0 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB0 = 0;
                    break;
            }
            break;
        
        case 5:

            ANSELBbits.ANSB1 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB1 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB1 = 0;
                    break;
                
            }
            break;
        
        case 6:

            ANSELBbits.ANSB2 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB2 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB2 = 0;
                    break;
                
            }
            break;
        
        case 7:

            ANSELBbits.ANSB3 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB3 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB3 = 0;
                    break;
                
            }
            break;
        
        case 11:

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB4 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB4 = 0;
                    break;
                
            }
            break;
        
        case 14:


            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB5 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB5 = 0;
                    break;
                
            }
            break;
        
        case 15:

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB6 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB6 = 0;
                    break;
                
            }
            break;
        
        case 16:

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB7 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB7 = 0;
                    break;
                
            }
            break;
        
        case 17:


            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB8 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB8 = 0;
                    break;
                
            }
            break;
        
        case 18:

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB9 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB9 = 0;
                    break;
                
            }
            break;
        
        case 21:


            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB10 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB10 = 0;
                    break;
                
            }
            break;
        
        case 22:

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB11 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB11 = 0;
                    break;
                
            }
            break;
        
        case 23:

            ANSELBbits.ANSB12 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB12 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB12 = 0;
                    break;
                
            }
            break;
        
        case 24:

            ANSELBbits.ANSB13 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB13 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB13 = 0;
                    break;
                
            }
            break;
        
        case 25:

            ANSELBbits.ANSB14 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB14 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB14 = 0;
                    break;
                
            }
            break;
        
        case 26:

            ANSELBbits.ANSB15 = 0;

            switch(mode)
            {
                case INPUT:
                    TRISBbits.TRISB15 = 1;
                    break;
                    
                case OUTPUT:
                    TRISBbits.TRISB15 = 0;
                    break;
                
            }
            break;
        
    }
}

void digitalWrite(uint8_t pin, uint8_t value)
{
    switch(pin)
    {
        case 2:
            switch(value)
            {
                case HIGH:
                    LATAbits.LATA0 = 1;
                    break;
                    
                case LOW:
                    LATAbits.LATA0 = 0;
                    break;
                
            }
            break;

        case 3:
            switch(value)
            {
               case HIGH:
                    LATAbits.LATA1 = 1;
                    break;
                    
                case LOW:
                    LATAbits.LATA1 = 0;
                    break;
                
            }
            break;
        
        case 9:
            switch(value)
            {
                case HIGH:
                    LATAbits.LATA2 = 1;
                    break;
                    
                case LOW:
                    LATAbits.LATA2 = 0;
                    break;
                
            }
            break;
        
        case 10:
            switch(value)
            {
                case HIGH:
                    LATAbits.LATA3 = 1;
                    break;
                    
                case LOW:
                    LATAbits.LATA3 = 0;
                    break;
                
            }
            break;
        
        case 12:
            switch(value)
            {
                case HIGH:
                    LATAbits.LATA4 = 1;
                    break;
                    
                case LOW:
                    LATAbits.LATA4 = 0;
                    break;
                
            }
            break;
        
        case 4:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB0 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB0 = 0;
                    break;
                
            }
            break;
        
        case 5:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB1 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB1 = 0;
                    break;
                
            }
            break;
        
        case 6:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB2 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB2 = 0;
                    break;
                
            }
            break;
        
        case 7:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB3 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB3 = 0;
                    break;
                
            }
            break;
        
        case 11:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB4 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB4 = 0;
                    break;
                
            }
            break;
        
        case 14:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB5 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB5 = 0;
                    break;
                
            }
            break;
        
        case 15:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB6 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB6 = 0;
                    break;
                
            }
            break;
        
        case 16:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB7 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB7 = 0;
                    break;
                
            }
            break;
        
        case 17:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB8 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB8 = 0;
                    break;
                
            }
            break;
        
        case 18:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB9 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB9 = 0;
                    break;
                
            }
            break;
        
        case 21:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB10 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB10 = 0;
                    break;
                
            }
            break;
        
        case 22:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB11 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB11 = 0;
                    break;
                
            }
            break;
        
        case 23:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB12 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB12 = 0;
                    break;
                
            }
            break;
        
        case 24:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB13 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB13 = 0;
                    break;
                
            }
            break;
        
        case 25:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB14 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB14 = 0;
                    break;
                
            }
            break;
        
        case 26:
            switch(value)
            {
                case HIGH:
                    LATBbits.LATB15 = 1;
                    break;
                    
                case LOW:
                    LATBbits.LATB15 = 0;
                    break;
                
            }
            break;
        
    }
}

uint8_t digitalRead(uint8_t pin)
{
    uint8_t pinVal;

    switch(pin)
    {
        case 2:
            pinVal = PORTAbits.RA0;
            break;

        case 3:
            pinVal = PORTAbits.RA1;
            break;

        case 9:
            pinVal = PORTAbits.RA2;
            break;

        case 10:
            pinVal = PORTAbits.RA3;
            break;

        case 12:
            pinVal = PORTAbits.RA4;
            break;

        case 4:
            pinVal = PORTBbits.RB0;
            break;

        case 5:
            pinVal = PORTBbits.RB1;
            break;

        case 6:
            pinVal = PORTBbits.RB2;
            break;

        case 7:
            pinVal = PORTBbits.RB3;
            break;

        case 11:
            pinVal = PORTBbits.RB4;
            break;

        case 14:
            pinVal = PORTBbits.RB5;
            break;

        case 15:
            pinVal = PORTBbits.RB6;
            break;

        case 16:
            pinVal = PORTBbits.RB7;
            break;

        case 17:
            pinVal = PORTBbits.RB8;
            break;

        case 18:
            pinVal = PORTBbits.RB9;
            break;

        case 21:
            pinVal = PORTBbits.RB10;
            break;

        case 22:
            pinVal = PORTBbits.RB11;
            break;

        case 23:
            pinVal = PORTBbits.RB12;
            break;

        case 24:
            pinVal = PORTBbits.RB13;
            break;

        case 25:
            pinVal = PORTBbits.RB14;
            break;

        case 26:
            pinVal = PORTBbits.RB15;
            break;

    }

    return pinVal;
}

