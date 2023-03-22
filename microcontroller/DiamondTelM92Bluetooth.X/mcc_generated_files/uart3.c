/**
  UART3 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    uart3.c

  @Summary
    This is the generated driver implementation file for the UART3 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This source file provides APIs for UART3.
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
#include "uart3.h"
#include "interrupt_manager.h"

/**
  Section: Macro Declarations
*/
#define UART3_TX_BUFFER_SIZE 64
#define UART3_RX_BUFFER_SIZE 8

/**
  Section: Global Variables
*/

static volatile uint8_t uart3TxHead = 0;
static volatile uint8_t uart3TxTail = 0;
static volatile uint8_t uart3TxBuffer[UART3_TX_BUFFER_SIZE];
volatile uint8_t uart3TxBufferRemaining;

static volatile uint8_t uart3RxHead = 0;
static volatile uint8_t uart3RxTail = 0;
static volatile uint8_t uart3RxBuffer[UART3_RX_BUFFER_SIZE];
static volatile uart3_status_t uart3RxStatusBuffer[UART3_RX_BUFFER_SIZE];
volatile uint8_t uart3RxCount;
static volatile uart3_status_t uart3RxLastError;

/**
  Section: UART3 APIs
*/
void (*UART3_FramingErrorHandler)(void);
void (*UART3_OverrunErrorHandler)(void);
void (*UART3_ErrorHandler)(void);

void UART3_DefaultFramingErrorHandler(void);
void UART3_DefaultOverrunErrorHandler(void);
void UART3_DefaultErrorHandler(void);

void UART3_Initialize(void)
{
    // Disable interrupts before changing states
    PIE9bits.U3RXIE = 0;
    UART3_SetRxInterruptHandler(UART3_Receive_ISR);
    PIE9bits.U3TXIE = 0;
    UART3_SetTxInterruptHandler(UART3_Transmit_ISR);

    // Set the UART3 module to the options selected in the user interface.

    // P1L 0; 
    U3P1L = 0x00;

    // P2L 0; 
    U3P2L = 0x00;

    // P3L 0; 
    U3P3L = 0x00;

    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN enabled; ABDEN disabled; 
    U3CON0 = 0xB0;

    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U3CON1 = 0x80;

    // TXPOL not inverted; FLO off; RXPOL inverted; RUNOVF RX input shifter stops all activity; STP Transmit 1Stop bit, receiver verifies first Stop bit; 
    U3CON2 = 0x40;

    // BRGL 15; 
    U3BRGL = 0x0F;

    // BRGH 39; 
    U3BRGH = 0x27;

    // STPMD in middle of first Stop bit; TXWRE No error; 
    U3FIFO = 0x00;

    // ABDIF Auto-baud not enabled or not complete; WUIF WUE not enabled by software; ABDIE disabled; 
    U3UIR = 0x00;

    // ABDOVF Not overflowed; TXCIF 0; RXBKIF No Break detected; RXFOIF not overflowed; CERIF No Checksum error; 
    U3ERRIR = 0x00;

    // TXCIE disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; RXFOIE disabled; PERIE disabled; RXBKIE disabled; 
    U3ERRIE = 0x00;


    UART3_SetFramingErrorHandler(UART3_DefaultFramingErrorHandler);
    UART3_SetOverrunErrorHandler(UART3_DefaultOverrunErrorHandler);
    UART3_SetErrorHandler(UART3_DefaultErrorHandler);

    uart3RxLastError.status = 0;

    // initializing the driver state
    uart3TxHead = 0;
    uart3TxTail = 0;
    uart3TxBufferRemaining = sizeof(uart3TxBuffer);
    uart3RxHead = 0;
    uart3RxTail = 0;
    uart3RxCount = 0;

    // enable receive interrupt
    PIE9bits.U3RXIE = 1;
}

bool UART3_is_rx_ready(void)
{
    return (uart3RxCount ? true : false);
}

bool UART3_is_tx_ready(void)
{
    return (uart3TxBufferRemaining ? true : false);
}

bool UART3_is_tx_done(void)
{
    return U3ERRIRbits.TXMTIF;
}

uart3_status_t UART3_get_last_status(void){
    return uart3RxLastError;
}

uint8_t UART3_Read(void)
{
    uint8_t readValue  = 0;
    
    while(0 == uart3RxCount)
    {
    }

    uart3RxLastError = uart3RxStatusBuffer[uart3RxTail];

    readValue = uart3RxBuffer[uart3RxTail++];
   	if(sizeof(uart3RxBuffer) <= uart3RxTail)
    {
        uart3RxTail = 0;
    }
    PIE9bits.U3RXIE = 0;
    uart3RxCount--;
    PIE9bits.U3RXIE = 1;

    return readValue;
}

void UART3_Write(uint8_t txData)
{
    while(0 == uart3TxBufferRemaining)
    {
    }

    if(0 == PIE9bits.U3TXIE)
    {
        U3TXB = txData;
    }
    else
    {
        PIE9bits.U3TXIE = 0;
        uart3TxBuffer[uart3TxHead++] = txData;
        if(sizeof(uart3TxBuffer) <= uart3TxHead)
        {
            uart3TxHead = 0;
        }
        uart3TxBufferRemaining--;
    }
    PIE9bits.U3TXIE = 1;
}

void UART3_WriteImmediately(uint8_t txData) {
    while(0 == uart3TxBufferRemaining)
    {
    }

    if(0 == PIE9bits.U3TXIE)
    {
        U3TXB = txData;
    }
    else
    {
        PIE9bits.U3TXIE = 0;
        if (uart3TxTail == 0) {
          uart3TxTail = sizeof(uart3TxBuffer) - 1;
        } else {
          --uart3TxTail;
        }
        
        uart3TxBuffer[uart3TxTail] = txData;
        uart3TxBufferRemaining--;
    }
    PIE9bits.U3TXIE = 1;
}

void __interrupt(irq(U3TX),base(8),low_priority) UART3_tx_vect_isr()
{   
    if(UART3_TxInterruptHandler)
    {
        UART3_TxInterruptHandler();
    }
}

void __interrupt(irq(U3RX),base(8),low_priority) UART3_rx_vect_isr()
{
    if(UART3_RxInterruptHandler)
    {
        UART3_RxInterruptHandler();
    }
}



void UART3_Transmit_ISR(void)
{
    // use this default transmit interrupt handler code
    if(sizeof(uart3TxBuffer) > uart3TxBufferRemaining)
    {
        U3TXB = uart3TxBuffer[uart3TxTail++];
       if(sizeof(uart3TxBuffer) <= uart3TxTail)
        {
            uart3TxTail = 0;
        }
        uart3TxBufferRemaining++;
    }
    else
    {
        PIE9bits.U3TXIE = 0;
    }
    
    // or set custom function using UART3_SetTxInterruptHandler()
}

void UART3_Receive_ISR(void)
{
    // use this default receive interrupt handler code
    uart3RxStatusBuffer[uart3RxHead].status = 0;

    if(U3ERRIRbits.FERIF){
        uart3RxStatusBuffer[uart3RxHead].ferr = 1;
        UART3_FramingErrorHandler();
    }
    
    if(U3ERRIRbits.RXFOIF){
        uart3RxStatusBuffer[uart3RxHead].oerr = 1;
        UART3_OverrunErrorHandler();
    }
    
    if(uart3RxStatusBuffer[uart3RxHead].status){
        UART3_ErrorHandler();
    } else {
        UART3_RxDataHandler();
    }

    // or set custom function using UART3_SetRxInterruptHandler()
}

void UART3_RxDataHandler(void){
    // use this default receive interrupt handler code
    uart3RxBuffer[uart3RxHead++] = U3RXB;
    if(sizeof(uart3RxBuffer) <= uart3RxHead)
    {
        uart3RxHead = 0;
    }
    uart3RxCount++;
}

void UART3_DefaultFramingErrorHandler(void){}

void UART3_DefaultOverrunErrorHandler(void){}

void UART3_DefaultErrorHandler(void){
    UART3_RxDataHandler();
}

void UART3_SetFramingErrorHandler(void (* interruptHandler)(void)){
    UART3_FramingErrorHandler = interruptHandler;
}

void UART3_SetOverrunErrorHandler(void (* interruptHandler)(void)){
    UART3_OverrunErrorHandler = interruptHandler;
}

void UART3_SetErrorHandler(void (* interruptHandler)(void)){
    UART3_ErrorHandler = interruptHandler;
}



void UART3_SetRxInterruptHandler(void (* InterruptHandler)(void)){
    UART3_RxInterruptHandler = InterruptHandler;
}

void UART3_SetTxInterruptHandler(void (* InterruptHandler)(void)){
    UART3_TxInterruptHandler = InterruptHandler;
}


/**
  End of File
*/
