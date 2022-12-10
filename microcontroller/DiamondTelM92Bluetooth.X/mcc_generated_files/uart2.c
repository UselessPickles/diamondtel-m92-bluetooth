/**
  UART2 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    uart2.c

  @Summary
    This is the generated driver implementation file for the UART2 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This source file provides APIs for UART2.
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
#include "uart2.h"
#include "interrupt_manager.h"

/**
  Section: Macro Declarations
*/
#define UART2_TX_BUFFER_SIZE 16
#define UART2_RX_BUFFER_SIZE 255

/**
  Section: Global Variables
*/

static volatile uint8_t uart2TxHead = 0;
static volatile uint8_t uart2TxTail = 0;
static volatile uint8_t uart2TxBuffer[UART2_TX_BUFFER_SIZE];
volatile uint8_t uart2TxBufferRemaining;

static volatile uint8_t uart2RxHead = 0;
static volatile uint8_t uart2RxTail = 0;
static volatile uint8_t uart2RxBuffer[UART2_RX_BUFFER_SIZE];
static volatile uart2_status_t uart2RxStatusBuffer[UART2_RX_BUFFER_SIZE];
volatile uint8_t uart2RxCount;
static volatile uart2_status_t uart2RxLastError;

/**
  Section: UART2 APIs
*/
void (*UART2_FramingErrorHandler)(void);
void (*UART2_OverrunErrorHandler)(void);
void (*UART2_ErrorHandler)(void);

void UART2_DefaultFramingErrorHandler(void);
void UART2_DefaultOverrunErrorHandler(void);
void UART2_DefaultErrorHandler(void);

void UART2_Initialize(void)
{
    // Disable interrupts before changing states
    PIE8bits.U2RXIE = 0;
    UART2_SetRxInterruptHandler(UART2_Receive_ISR);
    PIE8bits.U2TXIE = 0;
    UART2_SetTxInterruptHandler(UART2_Transmit_ISR);

    // Set the UART2 module to the options selected in the user interface.

    // P1L 0; 
    U2P1L = 0x00;

    // P2L 0; 
    U2P2L = 0x00;

    // P3L 0; 
    U2P3L = 0x00;

    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN enabled; ABDEN disabled; 
    U2CON0 = 0xB0;

    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U2CON1 = 0x80;

    // TXPOL not inverted; FLO off; RXPOL not inverted; RUNOVF RX input shifter stops all activity; STP Transmit 1Stop bit, receiver verifies first Stop bit; 
    U2CON2 = 0x00;

    // BRGL 68; 
    U2BRGL = 0x44;

    // BRGH 0; 
    U2BRGH = 0x00;

    // STPMD in middle of first Stop bit; TXWRE No error; 
    U2FIFO = 0x00;

    // ABDIF Auto-baud not enabled or not complete; WUIF WUE not enabled by software; ABDIE disabled; 
    U2UIR = 0x00;

    // ABDOVF Not overflowed; TXCIF 0; RXBKIF No Break detected; RXFOIF not overflowed; CERIF No Checksum error; 
    U2ERRIR = 0x00;

    // TXCIE disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; RXFOIE disabled; PERIE disabled; RXBKIE disabled; 
    U2ERRIE = 0x00;


    UART2_SetFramingErrorHandler(UART2_DefaultFramingErrorHandler);
    UART2_SetOverrunErrorHandler(UART2_DefaultOverrunErrorHandler);
    UART2_SetErrorHandler(UART2_DefaultErrorHandler);

    uart2RxLastError.status = 0;

    // initializing the driver state
    uart2TxHead = 0;
    uart2TxTail = 0;
    uart2TxBufferRemaining = sizeof(uart2TxBuffer);
    uart2RxHead = 0;
    uart2RxTail = 0;
    uart2RxCount = 0;

    // enable receive interrupt
    PIE8bits.U2RXIE = 1;
}

bool UART2_is_rx_ready(void)
{
    return (uart2RxCount ? true : false);
}

bool UART2_is_tx_ready(void)
{
    return (uart2TxBufferRemaining ? true : false);
}

bool UART2_is_tx_done(void)
{
    return U2ERRIRbits.TXMTIF;
}

uart2_status_t UART2_get_last_status(void){
    return uart2RxLastError;
}

uint8_t UART2_Read(void)
{
    uint8_t readValue  = 0;
    
    while(0 == uart2RxCount)
    {
    }

    uart2RxLastError = uart2RxStatusBuffer[uart2RxTail];

    readValue = uart2RxBuffer[uart2RxTail++];
   	if(sizeof(uart2RxBuffer) <= uart2RxTail)
    {
        uart2RxTail = 0;
    }
    PIE8bits.U2RXIE = 0;
    uart2RxCount--;
    PIE8bits.U2RXIE = 1;

    return readValue;
}

void UART2_Write(uint8_t txData)
{
    while(0 == uart2TxBufferRemaining)
    {
    }

    if(0 == PIE8bits.U2TXIE)
    {
        U2TXB = txData;
    }
    else
    {
        PIE8bits.U2TXIE = 0;
        uart2TxBuffer[uart2TxHead++] = txData;
        if(sizeof(uart2TxBuffer) <= uart2TxHead)
        {
            uart2TxHead = 0;
        }
        uart2TxBufferRemaining--;
    }
    PIE8bits.U2TXIE = 1;
}

void __interrupt(irq(U2TX),base(8),low_priority) UART2_tx_vect_isr()
{   
    if(UART2_TxInterruptHandler)
    {
        UART2_TxInterruptHandler();
    }
}

void __interrupt(irq(U2RX),base(8),low_priority) UART2_rx_vect_isr()
{
    if(UART2_RxInterruptHandler)
    {
        UART2_RxInterruptHandler();
    }
}


extern void UART_TransferNextByte( void );

void UART2_Transmit_ISR(void)
{
    // use this default transmit interrupt handler code
    if(sizeof(uart2TxBuffer) > uart2TxBufferRemaining)
    {
        U2TXB = uart2TxBuffer[uart2TxTail++];
       if(sizeof(uart2TxBuffer) <= uart2TxTail)
        {
            uart2TxTail = 0;
        }
        uart2TxBufferRemaining++;
        UART_TransferNextByte();
    }
    else
    {
        PIE8bits.U2TXIE = 0;
        UART_TransferNextByte();
    }
    
    // or set custom function using UART2_SetTxInterruptHandler()
}

void UART2_Receive_ISR(void)
{
    // use this default receive interrupt handler code
    uart2RxStatusBuffer[uart2RxHead].status = 0;

    if(U2ERRIRbits.FERIF){
        uart2RxStatusBuffer[uart2RxHead].ferr = 1;
        UART2_FramingErrorHandler();
    }
    
    if(U2ERRIRbits.RXFOIF){
        uart2RxStatusBuffer[uart2RxHead].oerr = 1;
        UART2_OverrunErrorHandler();
    }
    
    if(uart2RxStatusBuffer[uart2RxHead].status){
        UART2_ErrorHandler();
    } else {
        UART2_RxDataHandler();
    }

    // or set custom function using UART2_SetRxInterruptHandler()
}

void UART2_RxDataHandler(void){
    // use this default receive interrupt handler code
    uart2RxBuffer[uart2RxHead++] = U2RXB;
    if(sizeof(uart2RxBuffer) <= uart2RxHead)
    {
        uart2RxHead = 0;
    }
    uart2RxCount++;
}

void UART2_DefaultFramingErrorHandler(void){}

void UART2_DefaultOverrunErrorHandler(void){}

void UART2_DefaultErrorHandler(void){
    UART2_RxDataHandler();
}

void UART2_SetFramingErrorHandler(void (* interruptHandler)(void)){
    UART2_FramingErrorHandler = interruptHandler;
}

void UART2_SetOverrunErrorHandler(void (* interruptHandler)(void)){
    UART2_OverrunErrorHandler = interruptHandler;
}

void UART2_SetErrorHandler(void (* interruptHandler)(void)){
    UART2_ErrorHandler = interruptHandler;
}



void UART2_SetRxInterruptHandler(void (* InterruptHandler)(void)){
    UART2_RxInterruptHandler = InterruptHandler;
}

void UART2_SetTxInterruptHandler(void (* InterruptHandler)(void)){
    UART2_TxInterruptHandler = InterruptHandler;
}


/**
  End of File
*/
