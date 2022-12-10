/**
  DAC1 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    dac1.h

  @Summary
    This is the generated header file for the DAC1 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for DAC1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F27Q43
        Driver Version    :  2.10
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

#ifndef DAC1_H
#define DAC1_H

/**
  Section: Included Files
*/

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: DAC1 APIs
*/

/**
  @Summary
    Initializes the DAC1

  @Description
    This routine initializes the DAC1.
    This routine must be called before any other DAC1 routine is called.
    This routine should only be called once during system initialization.

  @Preconditions
    None

  @Param
    None

  @Returns
    None

  @Comment
    

  @Example
    <code>
    DAC1_Initialize();
    </code>
*/
void DAC1_Initialize(void);

/**
  @Summary
    Set Input data into DAC1.

  @Description
    This routine pass the digital input data into
    DAC1 voltage reference control register.

  @Preconditions
    The DAC1_Initialize() routine should be called
    prior to use this routine.

  @Param
    inputData - 8bit digital data to DAC1.

  @Returns
    None

  @Example
    <code>
    uint8_t count=0;

    DAC1_Initialize();

    for(count=0; count<=30; count++)
    {
        DAC1_SetOutput(count);
    }

    while(1)
    {
    }
    </code>
*/
void DAC1_SetOutput(uint8_t inputData);

/**
  @Summary
    Read input data fed to DAC1.

  @Description
    This routine reads the digital input data fed to
    DAC1 voltage reference control register.

  @Preconditions
    The DAC1_Initialize() routine should be called
    prior to use this routine.

  @Param
    None

  @Returns
    uint8_t inputData - digital data fed to DAC1

  @Example
    <code>
    uint8_t count=0;
    uint8_t inputData;

    DAC1_Initialize();

    inputData = DAC1_GetOutput();

    while(1)
    {
    }
    </code>
*/
uint8_t DAC1_GetOutput(void);

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif // DAC1_H
/**
 End of File
*/
