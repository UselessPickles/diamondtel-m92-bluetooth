/**
  UART4 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    uart4.c

  @Summary
    This is the generated driver implementation file for the UART4 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This source file provides APIs for UART4.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F27Q43
        Driver Version    :  2.4.1
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.36 and above
        MPLAB             :  MPLAB X 6.00
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

/**
  Section: Included Files
*/
#include <xc.h>
#include "uart4.h"
#include "interrupt_manager.h"

/**
  Section: Macro Declarations
*/
#define UART4_TX_BUFFER_SIZE 8
#define UART4_RX_BUFFER_SIZE 32

/**
  Section: Global Variables
*/

static volatile uint8_t uart4TxHead = 0;
static volatile uint8_t uart4TxTail = 0;
static volatile uint8_t uart4TxBuffer[UART4_TX_BUFFER_SIZE];
volatile uint8_t uart4TxBufferRemaining;

static volatile uint8_t uart4RxHead = 0;
static volatile uint8_t uart4RxTail = 0;
static volatile uint8_t uart4RxBuffer[UART4_RX_BUFFER_SIZE];
static volatile uart4_status_t uart4RxStatusBuffer[UART4_RX_BUFFER_SIZE];
volatile uint8_t uart4RxCount;
static volatile uart4_status_t uart4RxLastError;

/**
  Section: UART4 APIs
*/
void (*UART4_FramingErrorHandler)(void);
void (*UART4_OverrunErrorHandler)(void);
void (*UART4_ErrorHandler)(void);

void UART4_DefaultFramingErrorHandler(void);
void UART4_DefaultOverrunErrorHandler(void);
void UART4_DefaultErrorHandler(void);

void UART4_Initialize(void)
{
    // Disable interrupts before changing states
    PIE12bits.U4RXIE = 0;
    UART4_SetRxInterruptHandler(UART4_Receive_ISR);
    PIE12bits.U4TXIE = 0;
    UART4_SetTxInterruptHandler(UART4_Transmit_ISR);

    // Set the UART4 module to the options selected in the user interface.

    // P1L 0; 
    U4P1L = 0x00;

    // P2L 0; 
    U4P2L = 0x00;

    // P3L 0; 
    U4P3L = 0x00;

    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN enabled; ABDEN disabled; 
    U4CON0 = 0xB0;

    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U4CON1 = 0x80;

    // TXPOL inverted; FLO off; RXPOL inverted; RUNOVF RX input shifter stops all activity; STP Transmit 1Stop bit, receiver verifies first Stop bit; 
    U4CON2 = 0x44;

    // BRGL 15; 
    U4BRGL = 0x0F;

    // BRGH 39; 
    U4BRGH = 0x27;

    // STPMD in middle of first Stop bit; TXWRE No error; 
    U4FIFO = 0x00;

    // ABDIF Auto-baud not enabled or not complete; WUIF WUE not enabled by software; ABDIE disabled; 
    U4UIR = 0x00;

    // ABDOVF Not overflowed; TXCIF 0; RXBKIF No Break detected; RXFOIF not overflowed; CERIF No Checksum error; 
    U4ERRIR = 0x00;

    // TXCIE disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; RXFOIE disabled; PERIE disabled; RXBKIE disabled; 
    U4ERRIE = 0x00;


    UART4_SetFramingErrorHandler(UART4_DefaultFramingErrorHandler);
    UART4_SetOverrunErrorHandler(UART4_DefaultOverrunErrorHandler);
    UART4_SetErrorHandler(UART4_DefaultErrorHandler);

    uart4RxLastError.status = 0;

    // initializing the driver state
    uart4TxHead = 0;
    uart4TxTail = 0;
    uart4TxBufferRemaining = sizeof(uart4TxBuffer);
    uart4RxHead = 0;
    uart4RxTail = 0;
    uart4RxCount = 0;

    // enable receive interrupt
    PIE12bits.U4RXIE = 1;
}

bool UART4_is_rx_ready(void)
{
    return (uart4RxCount ? true : false);
}

bool UART4_is_tx_ready(void)
{
    return (uart4TxBufferRemaining ? true : false);
}

bool UART4_is_tx_done(void)
{
    return U4ERRIRbits.TXMTIF;
}

uart4_status_t UART4_get_last_status(void){
    return uart4RxLastError;
}

uint8_t UART4_Read(void)
{
    uint8_t readValue  = 0;
    
    while(0 == uart4RxCount)
    {
    }

    uart4RxLastError = uart4RxStatusBuffer[uart4RxTail];

    readValue = uart4RxBuffer[uart4RxTail++];
   	if(sizeof(uart4RxBuffer) <= uart4RxTail)
    {
        uart4RxTail = 0;
    }
    PIE12bits.U4RXIE = 0;
    uart4RxCount--;
    PIE12bits.U4RXIE = 1;

    return readValue;
}

void UART4_Write(uint8_t txData)
{
    while(0 == uart4TxBufferRemaining)
    {
    }

    if(0 == PIE12bits.U4TXIE)
    {
        U4TXB = txData;
    }
    else
    {
        PIE12bits.U4TXIE = 0;
        uart4TxBuffer[uart4TxHead++] = txData;
        if(sizeof(uart4TxBuffer) <= uart4TxHead)
        {
            uart4TxHead = 0;
        }
        uart4TxBufferRemaining--;
    }
    PIE12bits.U4TXIE = 1;
}

void __interrupt(irq(U4TX),base(8),low_priority) UART4_tx_vect_isr()
{   
    if(UART4_TxInterruptHandler)
    {
        UART4_TxInterruptHandler();
    }
}

void __interrupt(irq(U4RX),base(8),low_priority) UART4_rx_vect_isr()
{
    if(UART4_RxInterruptHandler)
    {
        UART4_RxInterruptHandler();
    }
}



void UART4_Transmit_ISR(void)
{
    // use this default transmit interrupt handler code
    if(sizeof(uart4TxBuffer) > uart4TxBufferRemaining)
    {
        U4TXB = uart4TxBuffer[uart4TxTail++];
       if(sizeof(uart4TxBuffer) <= uart4TxTail)
        {
            uart4TxTail = 0;
        }
        uart4TxBufferRemaining++;
    }
    else
    {
        PIE12bits.U4TXIE = 0;
    }
    
    // or set custom function using UART4_SetTxInterruptHandler()
}

void UART4_Receive_ISR(void)
{
    // use this default receive interrupt handler code
    uart4RxStatusBuffer[uart4RxHead].status = 0;

    if(U4ERRIRbits.FERIF){
        uart4RxStatusBuffer[uart4RxHead].ferr = 1;
        UART4_FramingErrorHandler();
    }
    
    if(U4ERRIRbits.RXFOIF){
        uart4RxStatusBuffer[uart4RxHead].oerr = 1;
        UART4_OverrunErrorHandler();
    }
    
    if(uart4RxStatusBuffer[uart4RxHead].status){
        UART4_ErrorHandler();
    } else {
        UART4_RxDataHandler();
    }

    // or set custom function using UART4_SetRxInterruptHandler()
}

void UART4_RxDataHandler(void){
    // use this default receive interrupt handler code
    uart4RxBuffer[uart4RxHead++] = U4RXB;
    if(sizeof(uart4RxBuffer) <= uart4RxHead)
    {
        uart4RxHead = 0;
    }
    uart4RxCount++;
}

void UART4_DefaultFramingErrorHandler(void){}

void UART4_DefaultOverrunErrorHandler(void){}

void UART4_DefaultErrorHandler(void){
    UART4_RxDataHandler();
}

void UART4_SetFramingErrorHandler(void (* interruptHandler)(void)){
    UART4_FramingErrorHandler = interruptHandler;
}

void UART4_SetOverrunErrorHandler(void (* interruptHandler)(void)){
    UART4_OverrunErrorHandler = interruptHandler;
}

void UART4_SetErrorHandler(void (* interruptHandler)(void)){
    UART4_ErrorHandler = interruptHandler;
}



void UART4_SetRxInterruptHandler(void (* InterruptHandler)(void)){
    UART4_RxInterruptHandler = InterruptHandler;
}

void UART4_SetTxInterruptHandler(void (* InterruptHandler)(void)){
    UART4_TxInterruptHandler = InterruptHandler;
}


/**
  End of File
*/
