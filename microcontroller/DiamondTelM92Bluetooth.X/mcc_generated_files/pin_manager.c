/**
  Generated Pin Manager File

  Company:
    Microchip Technology Inc.

  File Name:
    pin_manager.c

  Summary:
    This is the Pin Manager file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F27Q43
        Driver Version    :  2.11
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.36 and above
        MPLAB             :  MPLAB X 6.00

    Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "pin_manager.h"
#include "interrupt_manager.h"




void (*IOCAF3_InterruptHandler)(void);
void (*IOCBF3_InterruptHandler)(void);
void (*IOCBF4_InterruptHandler)(void);
void (*IOCBF5_InterruptHandler)(void);


void PIN_MANAGER_Initialize(void)
{
    /**
    LATx registers
    */
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x08;

    /**
    TRISx registers
    */
    TRISA = 0x0D;
    TRISB = 0xFE;
    TRISC = 0x41;

    /**
    ANSELx registers
    */
    ANSELC = 0x00;
    ANSELB = 0xC4;
    ANSELA = 0x04;

    /**
    WPUx registers
    */
    WPUE = 0x00;
    WPUB = 0x22;
    WPUA = 0x08;
    WPUC = 0x00;

    /**
    RxyI2C registers
    */
    RB1I2C = 0x00;
    RB2I2C = 0x00;
    RC3I2C = 0x00;
    RC4I2C = 0x00;

    /**
    ODx registers
    */
    ODCONE = 0x00;
    ODCONA = 0x00;
    ODCONB = 0x00;
    ODCONC = 0x10;

    /**
    SLRCONx registers
    */
    SLRCONA = 0xFF;
    SLRCONB = 0xFF;
    SLRCONC = 0xFF;

    /**
    INLVLx registers
    */
    INLVLA = 0xFF;
    INLVLB = 0xFF;
    INLVLC = 0xFF;
    INLVLE = 0x08;


    /**
    IOCx registers 
    */
    //interrupt on change for group IOCAF - flag
    IOCAFbits.IOCAF3 = 0;
    //interrupt on change for group IOCAN - negative
    IOCANbits.IOCAN3 = 1;
    //interrupt on change for group IOCAP - positive
    IOCAPbits.IOCAP3 = 1;
    //interrupt on change for group IOCBF - flag
    IOCBFbits.IOCBF3 = 0;
    //interrupt on change for group IOCBF - flag
    IOCBFbits.IOCBF4 = 0;
    //interrupt on change for group IOCBF - flag
    IOCBFbits.IOCBF5 = 0;
    //interrupt on change for group IOCBN - negative
    IOCBNbits.IOCBN3 = 1;
    //interrupt on change for group IOCBN - negative
    IOCBNbits.IOCBN4 = 1;
    //interrupt on change for group IOCBN - negative
    IOCBNbits.IOCBN5 = 1;
    //interrupt on change for group IOCBP - positive
    IOCBPbits.IOCBP3 = 1;
    //interrupt on change for group IOCBP - positive
    IOCBPbits.IOCBP4 = 1;
    //interrupt on change for group IOCBP - positive
    IOCBPbits.IOCBP5 = 1;



    // register default IOC callback functions at runtime; use these methods to register a custom function
    IOCAF3_SetInterruptHandler(IOCAF3_DefaultInterruptHandler);
    IOCBF3_SetInterruptHandler(IOCBF3_DefaultInterruptHandler);
    IOCBF4_SetInterruptHandler(IOCBF4_DefaultInterruptHandler);
    IOCBF5_SetInterruptHandler(IOCBF5_DefaultInterruptHandler);
   
    // Enable IOCI interrupt 
    PIE0bits.IOCIE = 1; 
    
	
    U2RXPPS = 0x16;   //RC6->UART2:RX2;    
    SPI1SCKPPS = 0x12;   //RC2->SPI1:SCK1;    
    RB0PPS = 0x20;   //RB0->UART1:TX1;    
    RC1PPS = 0x32;   //RC1->SPI1:SDO1;    
    RC2PPS = 0x31;   //RC2->SPI1:SCK1;    
    RA1PPS = 0x26;   //RA1->UART3:TX3;    
    U3RXPPS = 0x00;   //RA0->UART3:RX3;    
    U1RXPPS = 0x09;   //RB1->UART1:RX1;    
    RC7PPS = 0x23;   //RC7->UART2:TX2;    
    SPI1SDIPPS = 0x10;   //RC0->SPI1:SDI1;    
}
  
void __interrupt(irq(IOC),base(8),low_priority) PIN_MANAGER_IOC()
{   
	// interrupt on change for pin IOCAF3
    if(IOCAFbits.IOCAF3 == 1)
    {
        IOCAF3_ISR();  
    }	
	// interrupt on change for pin IOCBF3
    if(IOCBFbits.IOCBF3 == 1)
    {
        IOCBF3_ISR();  
    }	
	// interrupt on change for pin IOCBF4
    if(IOCBFbits.IOCBF4 == 1)
    {
        IOCBF4_ISR();  
    }	
	// interrupt on change for pin IOCBF5
    if(IOCBFbits.IOCBF5 == 1)
    {
        IOCBF5_ISR();  
    }	
}

/**
   IOCAF3 Interrupt Service Routine
*/
void IOCAF3_ISR(void) {

    // Add custom IOCAF3 code

    // Call the interrupt handler for the callback registered at runtime
    if(IOCAF3_InterruptHandler)
    {
        IOCAF3_InterruptHandler();
    }
    IOCAFbits.IOCAF3 = 0;
}

/**
  Allows selecting an interrupt handler for IOCAF3 at application runtime
*/
void IOCAF3_SetInterruptHandler(void (* InterruptHandler)(void)){
    IOCAF3_InterruptHandler = InterruptHandler;
}

/**
  Default interrupt handler for IOCAF3
*/
void IOCAF3_DefaultInterruptHandler(void){
    // add your IOCAF3 interrupt custom code
    // or set custom function using IOCAF3_SetInterruptHandler()
}

/**
   IOCBF3 Interrupt Service Routine
*/
void IOCBF3_ISR(void) {

    // Add custom IOCBF3 code

    // Call the interrupt handler for the callback registered at runtime
    if(IOCBF3_InterruptHandler)
    {
        IOCBF3_InterruptHandler();
    }
    IOCBFbits.IOCBF3 = 0;
}

/**
  Allows selecting an interrupt handler for IOCBF3 at application runtime
*/
void IOCBF3_SetInterruptHandler(void (* InterruptHandler)(void)){
    IOCBF3_InterruptHandler = InterruptHandler;
}

/**
  Default interrupt handler for IOCBF3
*/
void IOCBF3_DefaultInterruptHandler(void){
    // add your IOCBF3 interrupt custom code
    // or set custom function using IOCBF3_SetInterruptHandler()
}

/**
   IOCBF4 Interrupt Service Routine
*/
void IOCBF4_ISR(void) {

    // Add custom IOCBF4 code

    // Call the interrupt handler for the callback registered at runtime
    if(IOCBF4_InterruptHandler)
    {
        IOCBF4_InterruptHandler();
    }
    IOCBFbits.IOCBF4 = 0;
}

/**
  Allows selecting an interrupt handler for IOCBF4 at application runtime
*/
void IOCBF4_SetInterruptHandler(void (* InterruptHandler)(void)){
    IOCBF4_InterruptHandler = InterruptHandler;
}

/**
  Default interrupt handler for IOCBF4
*/
void IOCBF4_DefaultInterruptHandler(void){
    // add your IOCBF4 interrupt custom code
    // or set custom function using IOCBF4_SetInterruptHandler()
}

/**
   IOCBF5 Interrupt Service Routine
*/
void IOCBF5_ISR(void) {

    // Add custom IOCBF5 code

    // Call the interrupt handler for the callback registered at runtime
    if(IOCBF5_InterruptHandler)
    {
        IOCBF5_InterruptHandler();
    }
    IOCBFbits.IOCBF5 = 0;
}

/**
  Allows selecting an interrupt handler for IOCBF5 at application runtime
*/
void IOCBF5_SetInterruptHandler(void (* InterruptHandler)(void)){
    IOCBF5_InterruptHandler = InterruptHandler;
}

/**
  Default interrupt handler for IOCBF5
*/
void IOCBF5_DefaultInterruptHandler(void){
    // add your IOCBF5 interrupt custom code
    // or set custom function using IOCBF5_SetInterruptHandler()
}

/**
 End of File
*/