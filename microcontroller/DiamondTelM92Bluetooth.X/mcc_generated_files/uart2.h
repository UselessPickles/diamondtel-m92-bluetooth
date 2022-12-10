/**
  UART2 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    uart2.h

  @Summary
    This is the generated header file for the UART2 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for UART2.
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

#ifndef UART2_H
#define UART2_H

/**
  Section: Included Files
*/

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: Macro Declarations
*/

#define UART2_DataReady  (UART2_is_rx_ready())

/**
  Section: Data Type Definitions
*/

typedef union {
    struct {
        unsigned perr : 1;
        unsigned ferr : 1;
        unsigned oerr : 1;
        unsigned reserved : 5;
    };
    uint8_t status;
}uart2_status_t;

/**
 Section: Global variables
 */
extern volatile uint8_t uart2TxBufferRemaining;
extern volatile uint8_t uart2RxCount;

/**
  Section: UART2 APIs
*/

/**
  @Summary
    Initialization routine that takes inputs from the UART2 GUI.

  @Description
    This routine initializes the UART2 driver.
    This routine must be called before any other UART2 routine is called.

  @Preconditions
    None

  @Param
    None

  @Returns
    None

  @Comment

  @Example
*/
void UART2_Initialize(void);

/**
  @Summary
    Checks if the UART2 receiver ready for reading

  @Description
    This routine checks if UART2 receiver has received data 
    and ready to be read

  @Preconditions
    UART2_Initialize() function should be called
    before calling this function
    UART2 receiver should be enabled before calling this 
    function

  @Param
    None

  @Returns
    Status of UART2 receiver
    TRUE: UART2 receiver is ready for reading
    FALSE: UART2 receiver is not ready for reading
    
  @Example
    <code>
    void main(void)
    {
        volatile uint8_t rxData;
        
        // Initialize the device
        SYSTEM_Initialize();
        
        while(1)
        {
            // Logic to echo received data
            if(UART2_is_rx_ready())
            {
                rxData = UART2_Read();
                if(UART2_is_tx_ready())
                {
                    UART2_Write(rxData);
                }
            }
        }
    }
    </code>
*/
bool UART2_is_rx_ready(void);

/**
  @Summary
    Checks if the UART2 transmitter is ready to transmit data

  @Description
    This routine checks if UART2 transmitter is ready 
    to accept and transmit data byte

  @Preconditions
    UART2_Initialize() function should have been called
    before calling this function.
    UART2 transmitter should be enabled before calling 
    this function

  @Param
    None

  @Returns
    Status of UART2 transmitter
    TRUE: UART2 transmitter is ready
    FALSE: UART2 transmitter is not ready
    
  @Example
    <code>
    void main(void)
    {
        volatile uint8_t rxData;
        
        // Initialize the device
        SYSTEM_Initialize();
        
        while(1)
        {
            // Logic to echo received data
            if(UART2_is_rx_ready())
            {
                rxData = UART2_Read();
                if(UART2_is_tx_ready())
                {
                    UART2_Write(rxData);
                }
            }
        }
    }
    </code>
*/
bool UART2_is_tx_ready(void);

/**
  @Summary
    Checks if UART2 data is transmitted

  @Description
    This function return the status of transmit shift register

  @Preconditions
    UART2_Initialize() function should be called
    before calling this function
    UART2 transmitter should be enabled and UART2_Write
    should be called before calling this function

  @Param
    None

  @Returns
    Status of UART2 transmit shift register
    TRUE: Data completely shifted out if the UART shift register
    FALSE: Data is not completely shifted out of the shift register
    
  @Example
    <code>
    void main(void)
    {
        volatile uint8_t rxData;
        
        // Initialize the device
        SYSTEM_Initialize();
        
        while(1)
        {
            if(UART2_is_tx_ready())
            {
                LED_0_SetHigh();
                UART2Write(rxData);
            }
            if(UART2_is_tx_done()
            {
                LED_0_SetLow();
            }
        }
    }
    </code>
*/
bool UART2_is_tx_done(void);

/**
  @Summary
    Gets the error status of the last read byte.

  @Description
    This routine gets the error status of the last read byte.

  @Preconditions
    UART2_Initialize() function should have been called
    before calling this function. The returned value is only
    updated after a read is called.

  @Param
    None

  @Returns
    the status of the last read byte

  @Example
	<code>
    void main(void)
    {
        volatile uint8_t rxData;
        volatile uart2_status_t rxStatus;
        
        // Initialize the device
        SYSTEM_Initialize();
        
        // Enable the Global Interrupts
        INTERRUPT_GlobalInterruptEnable();
        
        while(1)
        {
            // Logic to echo received data
            if(UART2_is_rx_ready())
            {
                rxData = UART2_Read();
                rxStatus = UART2_get_last_status();
                if(rxStatus.ferr){
                    LED_0_SetHigh();
                }
            }
        }
    }
    </code>
 */
uart2_status_t UART2_get_last_status(void);

/**
  @Summary
    Read a byte of data from the UART2.

  @Description
    This routine reads a byte of data from the UART2.

  @Preconditions
    UART2_Initialize() function should have been called
    before calling this function. The transfer status should be checked to see
    if the receiver is not empty before calling this function.
	
	UART2_DataReady is a macro which checks if any byte is received.
	Call this macro before using this function.

  @Param
    None

  @Returns
    A data byte received by the driver.
	
  @Example
	<code>
            void main(void) {
                            // initialize the device
                            SYSTEM_Initialize();
                            uint8_t data;

                            // Enable the Global Interrupts
                            INTERRUPT_GlobalInterruptEnable();

                            // Enable the Peripheral Interrupts
                            INTERRUPT_PeripheralInterruptEnable();

                            printf("\t\tTEST CODE\n\r");		//Enable redirect STDIO to USART before using printf statements
                            printf("\t\t---- ----\n\r");
                            printf("\t\tECHO TEST\n\r");
                            printf("\t\t---- ----\n\n\r");
                            printf("Enter any string: ");
                            do{
                            data = UART2_Read();		// Read data received
                            UART2_Write(data);			// Echo back the data received
                            }while(!UART2_DataReady);		//check if any data is received

                    }
    </code>
*/
uint8_t UART2_Read(void);

 /**
  @Summary
    Writes a byte of data to the UART2.

  @Description
    This routine writes a byte of data to the UART2.

  @Preconditions
    UART2_Initialize() function should have been called
    before calling this function. The transfer status should be checked to see
    if transmitter is not busy before calling this function.

  @Param
    txData  - Data byte to write to the UART2

  @Returns
    None
  
  @Example
      <code>
          Refer to UART2_Read() for an example	
      </code>
*/
void UART2_Write(uint8_t txData);

/**
  @Summary
    Maintains the driver's transmitter state machine and implements its ISR.

  @Description
    This routine is used to maintain the driver's internal transmitter state
    machine.This interrupt service routine is called when the state of the
    transmitter needs to be maintained in a non polled manner.

  @Preconditions
    UART2_Initialize() function should have been called
    for the ISR to execute correctly.

  @Param
    None

  @Returns
    None
*/     
void UART2_Transmit_ISR(void);

/**
  @Summary
    Maintains the driver's receiver state machine and implements its ISR

  @Description
    This routine is used to maintain the driver's internal receiver state
    machine.This interrupt service routine is called when the state of the
    receiver needs to be maintained in a non polled manner.

  @Preconditions
    UART2_Initialize() function should have been called
    for the ISR to execute correctly.

  @Param
    None

  @Returns
    None
*/       
void UART2_Receive_ISR(void);

/**
  @Summary
    Maintains the driver's receiver state machine

  @Description
    This routine is called by the receive state routine and is used to maintain 
    the driver's internal receiver state machine. It should be called by a custom
    ISR to maintain normal behavior

  @Preconditions
    UART2_Initialize() function should have been called
    for the ISR to execute correctly.

  @Param
    None

  @Returns
    None
*/
void UART2_RxDataHandler(void);

/**
  @Summary
    Set UART2 Framing Error Handler

  @Description
    This API sets the function to be called upon UART2 framing error

  @Preconditions
    Initialize  the UART2 before calling this API

  @Param
    Address of function to be set as framing error handler

  @Returns
    None
*/
void UART2_SetFramingErrorHandler(void (* interruptHandler)(void));

/**
  @Summary
    Set UART2 Overrun Error Handler

  @Description
    This API sets the function to be called upon UART2 overrun error

  @Preconditions
    Initialize  the UART2 module before calling this API

  @Param
    Address of function to be set as overrun error handler

  @Returns
    None
*/
void UART2_SetOverrunErrorHandler(void (* interruptHandler)(void));

/**
  @Summary
    Set UART2 Error Handler

  @Description
    This API sets the function to be called upon UART2 error

  @Preconditions
    Initialize  the UART2 module before calling this API

  @Param
    Address of function to be set as error handler

  @Returns
    None
*/
void UART2_SetErrorHandler(void (* interruptHandler)(void));



/**
  @Summary
    UART2 Receive Interrupt Handler

  @Description
    This is a pointer to the function that will be called upon UART2 receive interrupt

  @Preconditions
    Initialize  the UART2 module with receive interrupt enabled

  @Param
    None

  @Returns
    None
*/
void (*UART2_RxInterruptHandler)(void);

/**
  @Summary
    UART2 Transmit Interrupt Handler

  @Description
    This is a pointer to the function that will be called upon UART2 transmit interrupt

  @Preconditions
    Initialize  the UART2 module with transmit interrupt enabled

  @Param
    None

  @Returns
    None
*/
void (*UART2_TxInterruptHandler)(void);



/**
  @Summary
    Set UART2 Receive Interrupt Handler

  @Description
    This API sets the function to be called upon UART2 receive interrupt

  @Preconditions
    Initialize  the UART2 module with receive interrupt enabled before calling this API

  @Param
    Address of function to be set as receive interrupt handler

  @Returns
    None
*/
void UART2_SetRxInterruptHandler(void (* InterruptHandler)(void));

/**
  @Summary
    Set UART2 Transmit Interrupt Handler

  @Description
    This API sets the function to be called upon UART2 transmit interrupt

  @Preconditions
    Initialize  the UART2 module with transmit interrupt enabled before calling this API

  @Param
    Address of function to be set as transmit interrupt handler

  @Returns
    None
*/
void UART2_SetTxInterruptHandler(void (* InterruptHandler)(void));



#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif  // UART2_H
/**
 End of File
*/
