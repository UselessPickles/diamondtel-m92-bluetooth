/**
  TMR4 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    tmr4.h

  @Summary
    This is the generated header file for the TMR4 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for TMR4.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F27Q43
        Driver Version    :  2.11
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.36 and above 
        MPLAB 	          :  MPLAB X 6.00
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

#ifndef TMR4_H
#define TMR4_H

/**
  Section: Included Files
*/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
 Section: Data Type Definitions
*/

/**
  HLT Mode Setting Enumeration

  @Summary
    Defines the different modes of the HLT.

  @Description
    This defines the several modes of operation of the Timer with
	HLT extension. The modes can be set in a control register associated
	with the timer
*/

typedef enum
{

	/* Roll-over Pulse mode clears the TMRx upon TMRx = PRx, then continue running.
	ON bit must be set and is not affected by Resets
	*/

   /* Roll-over Pulse mode indicates that Timer starts
   immediately upon ON = 1 (Software Control)
   */
   TMR4_ROP_STARTS_TMRON,

   /* Roll-over Pulse mode indicates that the Timer starts
       when ON = 1 and TMRx_ers = 1. Stops when TMRx_ers = 0
     */
   TMR4_ROP_STARTS_TMRON_ERSHIGH,

   /* Roll-over Pulse mode indicates that the Timer starts
      when ON = 1 and TMRx_ers = 0. Stops when TMRx_ers = 1
     */
   TMR4_ROP_STARTS_TMRON_ERSLOW,

   /* Roll-over Pulse mode indicates that the Timer resets
   upon rising or falling edge of TMRx_ers
     */
   TMR4_ROP_RESETS_ERSBOTHEDGE,

   /* Roll-over Pulse mode indicates that the Timer resets
    upon rising edge of TMRx_ers
     */
   TMR4_ROP_RESETS_ERSRISINGEDGE,

   /* Roll-over Pulse mode indicates that the Timer resets
   upon falling edge of TMRx_ers
     */
   TMR4_ROP_RESETS_ERSFALLINGEDGE,

   /* Roll-over Pulse mode indicates that the Timer resets
   upon TMRx_ers = 0
     */
   TMR4_ROP_RESETS_ERSLOW,

   /* Roll-over Pulse mode indicates that the Timer resets
   upon TMRx_ers = 1
     */
   TMR4_ROP_RESETS_ERSHIGH,

    /*In all One-Shot mode the timer resets and the ON bit is
	cleared when the timer value matches the PRx period
	value. The ON bit must be set by software to start
	another timer cycle.
	*/

   /* One shot mode indicates that the Timer starts
    immediately upon ON = 1 (Software Control)
     */
   TMR4_OS_STARTS_TMRON,

   /* One shot mode indicates that the Timer starts
    when a rising edge is detected on the TMRx_ers
     */
   TMR4_OS_STARTS_ERSRISINGEDGE ,

   /* One shot mode indicates that the Timer starts
    when a falling edge is detected on the TMRx_ers
     */
   TMR4_OS_STARTS_ERSFALLINGEDGE ,

   /* One shot mode indicates that the Timer starts
    when either a rising or falling edge is detected on TMRx_ers
     */
   TMR4_OS_STARTS_ERSBOTHEDGE,

   /* One shot mode indicates that the Timer starts
    upon first TMRx_ers rising edge and resets on all
	subsequent TMRx_ers rising edges
     */
   TMR4_OS_STARTS_ERSFIRSTRISINGEDGE,

   /* One shot mode indicates that the Timer starts
    upon first TMRx_ers falling edge and restarts on all
	subsequent TMRx_ers falling edges
     */
   TMR4_OS_STARTS_ERSFIRSTFALLINGEDGE,

   /* One shot mode indicates that the Timer starts
    when a rising edge is detected on the TMRx_ers,
	resets upon TMRx_ers = 0
     */
   TMR4_OS_STARTS_ERSRISINGEDGEDETECT,
     /* One shot mode indicates that the Timer starts
    when a falling edge is detected on the TMRx_ers,
	resets upon TMRx_ers = 1
     */
   TMR4_OS_STARTS_ERSFALLINGEDGEDETECT,
   
   /* One shot mode indicates that the Timer starts
    when a TMRx_ers = 1,ON =1 and resets upon TMRx_ers =0
    */
   TMR4_OS_STARTS_TMRON_ERSHIGH = 0x16,
           
   /* One shot mode indicates that the Timer starts
     when a TMRx_ers = 0,ON = 1 and resets upon TMRx_ers =1 
    */
   TMR4_OS_STARTS_TMRON_ERSLOW = 0x17,
        
   /*In all Mono-Stable mode the ON bit must be initially set,but
     not cleared upon the TMRx = PRx, and the timer will start upon
     an TMRx_ers start event following TMRx = PRx.*/
               
   /* Mono Stable mode indicates that the Timer starts
      when a rising edge is detected on the TMRx_ers and ON = 1
    */
   TMR4_MS_STARTS_TMRON_ERSRISINGEDGEDETECT = 0x11,
           
   /* Mono Stable mode indicates that the Timer starts
      when a falling edge is detected on the TMRx_ers and ON = 1
    */
   TMR4_MS_STARTS_TMRON_ERSFALLINGEDGEDETECT = 0x12,
           
   /* Mono Stable mode indicates that the Timer starts
      when  either a rising or falling edge is detected on TMRx_ers 
      and ON = 1
    */
   TMR4_MS_STARTS_TMRON_ERSBOTHEDGE = 0x13
           
} TMR4_HLT_MODE;

/**
  HLT Reset Source Enumeration

  @Summary
    Defines the different reset source of the HLT.

  @Description
    This source can control starting and stopping of the
	timer, as well as resetting the timer, depending on
	which mode the timer is in. The mode of the timer is
	controlled by the HLT_MODE
*/

typedef enum
{
     /* T4INPPS is the Timer external reset source
     */
    TMR4_T4INPPS,

    /* Timer2 Postscale is the Timer external reset source 
     */
    TMR4_T2POSTSCALED,
    
    /* Reserved enum cannot be used 
     */
    TMR4_RESERVED,
    
    /* Timer6 Postscale is the Timer external reset source 
     */
    TMR4_T6POSTSCALED,

    /* CCP1_OUT is the Timer external reset source 
     */
    TMR4_CCP1_OUT,

    /* CCP2_OUT is the Timer external reset source 
     */
    TMR4_CCP2_OUT,

    /* CCP3_OUT is the Timer external reset source 
     */
    TMR4_CCP3_OUT,

    /* PWM1S1P1_out is the Timer external reset source 
     */
    TMR4_PWM1S1P1_OUT,

    /* PWM1S1P2_out is the Timer external reset source 
     */
    TMR4_PWM1S1P2_OUT,

    /* PWM2S1P1_out is the Timer external reset source 
     */
    TMR4_PWM2S1P1_OUT,

    /* PWM2S1P2_out is the Timer external reset source 
     */
    TMR4_PWM2S1P2_OUT,

    /* PWM3S1P1_out is the Timer external reset source 
     */
    TMR4_PWM3S1P1_OUT,

    /* PWM3S1P2_out is the Timer external reset source 
     */
    TMR4_PWM3S1P2_OUT,

    /* Reserved enum cannot be used 
    */
    TMR4_RESERVED_2,

    /* Reserved enum cannot be used 
    */
    TMR4_RESERVED_3,

    /* CMP1_OUT is the Timer external reset source 
     */
    TMR4_CMP1_OUT,

    /* CMP2_OUT is the Timer external reset source 
     */
    TMR4_CMP2_OUT,

    /* ZCD_Output is the Timer external reset source 
     */
    TMR4_ZCD_OUTPUT,

    /* CLC1_out is the Timer external reset source 
     */
    TMR4_CLC1_OUT,
         
    /* CLC2_out is the Timer external reset source 
     */
    TMR4_CLC2_OUT,
            
    /* CLC3_out is the Timer external reset source 
     */
    TMR4_CLC3_OUT,

    /* CLC4_out is the Timer external reset source 
     */
    TMR4_CLC4_OUT,  

    /* CLC5_out is the Timer external reset source 
     */
    TMR4_CLC5_OUT,
         
    /* CLC6_out is the Timer external reset source 
     */
    TMR4_CLC6_OUT,
            
    /* CLC7_out is the Timer external reset source 
     */
    TMR4_CLC7_OUT,
    
    /* CLC8_out is the Timer external reset source 
     */
    TMR4_CLC8_OUT,

    /* UART1_rx_edge is the Timer external reset source 
     */
    TMR4_UART1_RX_EDGE,

    /* UART1_tx_edge is the Timer external reset source 
     */
    TMR4_UART1_TX_EDGE,

    /* UART2_rx_edge is the Timer external reset source 
     */
    TMR4_UART2_RX_EDGE,

    /* UART2_tx_edge is the Timer external reset source 
     */
    TMR4_UART2_TX_EDGE,

    /* UART3_rx_edge is the Timer external reset source 
     */
    TMR4_UART3_RX_EDGE,

    /* UART3_tx_edge is the Timer external reset source 
     */
    TMR4_UART3_TX_EDGE,

    /* UART4_rx_edge is the Timer external reset source 
     */
    TMR4_UART4_RX_EDGE,

    /* UART4_tx_edge is the Timer external reset source 
     */
    TMR4_UART4_TX_EDGE,

    /* UART5_rx_edge is the Timer external reset source 
     */
    TMR4_UART5_RX_EDGE,

    /* UART5_tx_edge is the Timer external reset source 
     */
    TMR4_UART5_TX_EDGE,

    /* Reserved enum cannot be used 
    */
    TMR4_RESERVED_4


} TMR4_HLT_EXT_RESET_SOURCE;


/**
  Section: Macro Declarations
*/

/**
  Section: TMR4 APIs
*/

/**
  @Summary
    Initializes the TMR4 module.

  @Description
    This function initializes the TMR4 Registers.
    This function must be called before any other TMR4 function is called.

  @Preconditions
    None

  @Param
    None

  @Returns
    None

  @Comment
    

  @Example
    <code>
    main()
    {
        // Initialize TMR4 module
        TMR4_Initialize();

        // Do something else...
    }
    </code>
*/
void TMR4_Initialize(void);

/**
  @Summary
    Configures the Hardware Limit Timer mode.

  @Description
    Writes the T4HLTbits.MODE bits.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    mode - Value to write into T4HLTbits.MODE bits.

  @Returns
    None

  @Example
    <code>
	main()
    {

	    TMR4_HLT_MODE hltmode;
		hltmode = TMR4_ROP_STARTS_TMRON_EN;

		// Initialize TMR4 module
		 TMR4.Initialize();

		// Set the HLT mode
		TMR4_ModeSet (hltmode);

		// Do something else...
    }
    </code>
*/
void TMR4_ModeSet(TMR4_HLT_MODE mode);

/**
  @Summary
    Configures the HLT external reset source.

  @Description
    Writes the T4RSTbits.RSEL bits.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    reset - Value to write into T4RSTbits.RSEL bits.

  @Returns
    None

  @Example
    <code>
	main()
    {

	    TMR4_HLT_EXT_RESET_SOURCE hltresetsrc;
		hltresetsrc = T2IN;

        // Initialize TMR4 module

		// Set the HLT mode
		TMR4_ExtResetSourceSet(hltresetsrc);

		// Do something else...
    }
    </code>
*/
void TMR4_ExtResetSourceSet(TMR4_HLT_EXT_RESET_SOURCE reset);

/**
  @Summary
    This function starts the TMR4.

  @Description
    This function starts the TMR4 operation.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    None

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_Start();

    // Do something else...
    </code>
*/
void TMR4_Start(void);

/**
  @Summary
    This function starts the TMR4.

  @Description
    This function starts the TMR4 operation.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    None

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_StartTimer();

    // Do something else...
    </code>
*/
void TMR4_StartTimer(void);

/**
  @Summary
    This function stops the TMR4.

  @Description
    This function stops the TMR4 operation.
    This function must be called after the start of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    None

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_Start();

    // Do something else...

    // Stop TMR4;
    TMR4_Stop();
    </code>
*/
void TMR4_Stop(void);

/**
  @Summary
    This function stops the TMR4.

  @Description
    This function stops the TMR4 operation.
    This function must be called after the start of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    None

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_StartTimer();

    // Do something else...

    // Stop TMR4;
    TMR4_StopTimer();
    </code>
*/
void TMR4_StopTimer(void);

/**
  @Summary
    Reads the TMR4 register.

  @Description
    This function reads the TMR4 register value and return it.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    This function returns the current value of TMR4 register.

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_Start();

    // Read the current value of TMR4
    if(0 == TMR4_Counter8BitGet())
    {
        // Do something else...

        // Reload the TMR value
        TMR4_Period8BitSet();
    }
    </code>
*/
uint8_t TMR4_Counter8BitGet(void);

/**
  @Summary
    Reads the TMR4 register.

  @Description
    This function reads the TMR4 register value and return it.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    None

  @Returns
    This function returns the current value of TMR4 register.

  @Example
    <code>
    // Initialize TMR4 module

    // Start TMR4
    TMR4_StartTimer();

    // Read the current value of TMR4
    if(0 == TMR4_ReadTimer())
    {
        // Do something else...

        // Reload the TMR value
        TMR4_LoadPeriodRegister();
    }
    </code>
*/
uint8_t TMR4_ReadTimer(void);

/**
  @Summary
    Writes the TMR4 register.

  @Description
    This function writes the TMR4 register.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    timerVal - Value to write into TMR4 register.

  @Returns
    None

  @Example
    <code>
    #define PERIOD 0x80
    #define ZERO   0x00

    while(1)
    {
        // Read the TMR4 register
        if(ZERO == TMR4_Counter8BitGet())
        {
            // Do something else...

            // Write the TMR4 register
            TMR4_Counter8BitSet(PERIOD);
        }

        // Do something else...
    }
    </code>
*/
void TMR4_Counter8BitSet(uint8_t timerVal);

/**
  @Summary
    Writes the TMR4 register.

  @Description
    This function writes the TMR4 register.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    timerVal - Value to write into TMR4 register.

  @Returns
    None

  @Example
    <code>
    #define PERIOD 0x80
    #define ZERO   0x00

    while(1)
    {
        // Read the TMR4 register
        if(ZERO == TMR4_ReadTimer())
        {
            // Do something else...

            // Write the TMR4 register
            TMR4_WriteTimer(PERIOD);
        }

        // Do something else...
    }
    </code>
*/
void TMR4_WriteTimer(uint8_t timerVal);

/**
  @Summary
    Load value to Period Register.

  @Description
    This function writes the value to PR4 register.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    periodVal - Value to load into TMR4 register.

  @Returns
    None

  @Example
    <code>
    #define PERIOD1 0x80
    #define PERIOD2 0x40
    #define ZERO    0x00

    while(1)
    {
        // Read the TMR4 register
        if(ZERO == TMR4_Counter8BitGet())
        {
            // Do something else...

            if(flag)
            {
                flag = 0;

                // Load Period 1 value
                TMR4_Period8BitSet(PERIOD1);
            }
            else
            {
                 flag = 1;

                // Load Period 2 value
                TMR4_Period8BitSet(PERIOD2);
            }
        }

        // Do something else...
    }
    </code>
*/
void TMR4_Period8BitSet(uint8_t periodVal);

/**
  @Summary
    Load value to Period Register.

  @Description
    This function writes the value to PR4 register.
    This function must be called after the initialization of TMR4.

  @Preconditions
    Initialize  the TMR4 before calling this function.

  @Param
    periodVal - Value to load into TMR4 register.

  @Returns
    None

  @Example
    <code>
    #define PERIOD1 0x80
    #define PERIOD2 0x40
    #define ZERO    0x00

    while(1)
    {
        // Read the TMR4 register
        if(ZERO == TMR4_ReadTimer())
        {
            // Do something else...

            if(flag)
            {
                flag = 0;

                // Load Period 1 value
                TMR4_LoadPeriodRegister(PERIOD1);
            }
            else
            {
                 flag = 1;

                // Load Period 2 value
                TMR4_LoadPeriodRegister(PERIOD2);
            }
        }

        // Do something else...
    }
    </code>
*/
void TMR4_LoadPeriodRegister(uint8_t periodVal);


/**
  @Summary
    CallBack function

  @Description
    This function is called from the timer ISR. User can write your code in this function.

  @Preconditions
    Initialize  the TMR4 module with interrupt before calling this function.

  @Param
    None

  @Returns
    None
*/
 void TMR4_CallBack(void);
/**
  @Summary
    Set Timer Interrupt Handler

  @Description
    This sets the function to be called during the ISR

  @Preconditions
    Initialize  the TMR4 module with interrupt before calling this.

  @Param
    Address of function to be set

  @Returns
    None
*/
 void TMR4_SetInterruptHandler(void (* InterruptHandler)(void));

/**
  @Summary
    Timer Interrupt Handler

  @Description
    This is a function pointer to the function that will be called during the ISR

  @Preconditions
    Initialize  the TMR4 module with interrupt before calling this isr.

  @Param
    None

  @Returns
    None
*/
extern void (*TMR4_InterruptHandler)(void);

/**
  @Summary
    Default Timer Interrupt Handler

  @Description
    This is the default Interrupt Handler function

  @Preconditions
    Initialize  the TMR4 module with interrupt before calling this isr.

  @Param
    None

  @Returns
    None
*/
void TMR4_DefaultInterruptHandler(void);


 #ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif // TMR4_H
/**
 End of File
*/