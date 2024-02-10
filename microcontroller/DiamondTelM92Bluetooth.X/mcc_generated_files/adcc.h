/**
  ADCC Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    adcc.h

  @Summary
    This is the generated header file for the ADCC driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs 

  @Description
    This header file provides APIs for driver for ADCC.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F27Q43
        Driver Version    :  2.1.5
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

#ifndef ADCC_H
#define ADCC_H

/**
  Section: Included Files
*/

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: Data Types Definitions
*/

/**
 *  result size of an A/D conversion
 */

typedef uint16_t adc_result_t;
#ifndef uint24_t
typedef __uint24 uint24_t;
#endif

/** ADCC Channel Definition

 @Summary
   Defines the channels available for conversion.

 @Description
   This routine defines the channels that are available for the module to use.

 Remarks:
   None
 */

typedef enum
{
    IO_BATT_VOLTAGE =  0x4,
    channel_VSS =  0x3B,
    channel_Temp =  0x3C,
    channel_DAC1 =  0x3D,
    channel_FVR_Buffer1 =  0x3E,
    channel_FVR_Buffer2 =  0x3F
} adcc_channel_t;

/**
  Section: ADCC Module APIs
*/

/**
  @Summary
    Initializes the ADCC.

  @Description
    This routine initializes the ADCC and must be called before any other ADCC routine.
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
    adc_result_t convertedValue;    

    ADCC_Initialize();
    convertedValue = ADCC_GetSingleConversion(channel_ANA0);
    </code>
*/
void ADCC_Initialize(void);

/**
  @Summary
    Starts A/D conversion on selected analog channel.

  @Description
    This routine is used to trigger A/D conversion on selected analog channel.
    
  @Preconditions
    ADCC_Initialize() function should have been called before calling this function.

  @Returns
    None

  @Param
    channel: Analog channel number on which A/D conversion has to be applied.
             For available analog channels refer adcc_channel_t enum from adcc.h file

  @Example
    <code>
    adc_result_t convertedValue; 

    ADCC_Initialize();   
    ADCC_StartConversion(channel_ANA0);
    while(!ADCC_IsConversionDone());
    convertedValue = ADCC_GetConversionResult();
    </code>
*/
void ADCC_StartConversion(adcc_channel_t channel);

/**
  @Summary
    Determine if A/D conversion is completed.

  @Description
    This routine is used to determine if A/D conversion is completed.

  @Preconditions
    ADCC_Initialize() and ADCC_StartConversion(adcc_channel_t channel)
    functions should have been called before calling this function.

  @Returns
    true  - If conversion is completed
    false - If conversion is not completed

  @Param
    None

  @Example
    <code>
    adc_result_t convertedValue;    

    ADCC_Initialize();    
    ADCC_StartConversion(channel_ANA0);
    while(!ADCC_IsConversionDone());
    convertedValue = ADCC_GetConversionResult();
    </code>
 */
bool ADCC_IsConversionDone(void);

/**
  @Summary
    Returns result of latest A/D conversion.

  @Description
    This routine is used to retrieve the result of latest A/D conversion.
    This routine returns the conversion value only after the conversion is complete.
    

  @Preconditions
    ADCC_Initialize(), ADCC_StartConversion() functions should have been called
    before calling this function.
    Completion status should be checked using ADCC_IsConversionDone() routine.

  @Returns
    Returns the result of A/D conversion.

  @Param
    None

  @Example
    <code>
    adc_result_t convertedValue;

    ADCC_Initialize();    
    ADCC_StartConversion(channel_ANA0);
    while(!ADCC_IsConversionDone());
    convertedValue = ADCC_GetConversionResult();
    </code>
 */
adc_result_t ADCC_GetConversionResult(void);

/**
  @Summary
    Returns the result of A/D conversion for requested analog channel.

  @Description
    This routine is used to retrieve the result of A/D conversion for requested 
    analog channel.

  @Preconditions
    ADCC_Initialize() and ADCC_DisableContinuousConversion() functions should have 
    been called before calling this function.

  @Returns
    Returns the result of A/D conversion.

  @Param
    channel: Analog channel number for which A/D conversion has to be applied
             For available analog channels refer adcc_channel_t enum from adcc.h file

  @Example
    <code>
    adcc_channel_t convertedValue;

    ADCC_Initialize();
    ADCC_DisableContinuousConversion();
    
    convertedValue = ADCC_GetSingleConversion(channel_ANA0);
    </code>
*/
adc_result_t ADCC_GetSingleConversion(adcc_channel_t channel);

/**
  @Summary
    Stops the ongoing continuous A/D conversion.

  @Description
    This routine is used to stop ongoing continuous A/D conversion.

  @Preconditions
    ADCC_Initialize() and ADCC_StartConversion() functions should have been called before calling this function.

  @Returns
    None

  @Param
    None

  @Example
    <code>
    ADCC_Initialize();
    ADCC_StartConversion(channel_ANA0);
    ADCC_StopConversion();
    </code>
*/
void ADCC_StopConversion(void);

/**
  @Summary
    Stops the ADCC from re-triggering A/D conversion cycle 
    upon completion of each conversion.

  @Description
    In continuous mode, stops the ADCC from re-triggering A/D conversion cycle 
    upon completion of each conversion.

  @Preconditions
    ADCC_Initialize() and ADCC_EnableContinuousConversion() function should have been called before calling this function.

  @Returns
    None

  @Param
    None

  @Example
    <code>
    ADCC_Initialize();
    ADCC_EnableContinuousConversion();
    ADCC_SetStopOnInterrupt();
    </code>
*/
void ADCC_SetStopOnInterrupt(void);

/**
  @Summary
    Discharges the input sample capacitor by setting the channel to AVss.

  @Description
    This routine is used to discharge input sample capacitor by selecting analog
    ground (AVss) channel.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    None

  @Example
    <code>
    ADCC_Initialize();
    ADCC_DischargeSampleCapacitor();
    </code>
*/
void ADCC_DischargeSampleCapacitor(void); 

/**
  @Summary
    Loads the Acquisition Time Control register.

  @Description
    This routine is used to load 13-bit ADCC Acquisition Time Control register by
    a value provided by user.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    13-bit value to be set in the acquisition register.

  @Example
    <code>
    uint16_t acquisitionValue = 98;
    ADCC_Initialize();
    ADCC_LoadAcquisitionRegister(acquisitionValue);
    </code>
*/
void ADCC_LoadAcquisitionRegister(uint16_t);

/**
  @Summary
    Loads the Precharge Time Control register.

  @Description
    This routine is used to load 13-bit ADCC Precharge Time Control register by
    a value provided by user.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    13-bit value to be set in the precharge register.

  @Example
    <code>
    uint16_t prechargeTime = 98;
    ADCC_Initialize();
    ADCC_SetPrechargeTime(prechargeTime);
    </code>
*/
void ADCC_SetPrechargeTime(uint16_t);

/**
  @Summary
    Loads the Repeat Setting register.

  @Description
    This routine loads ADCC Repeat Setting register with 8-bit value provided by user.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    8-bit value to be set in the Repeat Setting register.

  @Example
    <code>
    uint8_t repeat = 98;
    ADCC_Initialize();
    ADCC_SetRepeatCount(repeat);
    </code>
*/
void ADCC_SetRepeatCount(uint8_t);

/**
  @Summary
    Returns the current value of Repeat Count register.

  @Description
    This routine retrieves the current value of ADCC Repeat Count register.

  @Preconditions
    ADCC_Initialize(), ADCC_StartConversion() should have been called before calling
    this function.

  @Returns
    Value of ADCC Repeat Count register

  @Param
    None.

  @Example
    <code>
    adc_result_t convertedValue;
    uint8_t count;
    ADCC_Initialize();
    ADCC_StartConversion(channel_ANA0);
    count = ADCC_GetCurrentCountofConversions();
    </code>
*/
uint8_t ADCC_GetCurrentCountofConversions(void);

/**
  @Summary
    Clears the A/D Accumulator.

  @Description
    This routine is used to clear A/D Accumulator

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    None.

  @Example
    <code>
    ADCC_Initialize();
    ADCC_ClearAccumulator();
    </code>
*/
void ADCC_ClearAccumulator(void);

/**
  @Summary
   Returns the value of ADCC Accumulator.

  @Description
    This routine is is to retrieve the 17-bit value of ADCC accumulator.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    17-bit value obtained from ADCC Accumulator register.

  @Param
    None.

  @Example
    <code>
    uint24_t accumulatorValue;
    ADCC_Initialize();
    accumulatorValue = ADCC_GetAccumulatorValue();
    </code>
*/
uint24_t ADCC_GetAccumulatorValue(void);

/**
  @Summary
   Determines if ADCC accumulator has overflowed.

  @Description
    This routine is used to determine whether ADCC accumulator has overflowed.

  @Preconditions  
    ADCC_Initialize(), ADCC_StartConversion() should have been called before calling
    this function.

  @Returns
    1: ADCC accumulator or ERR calculation have overflowed
    0: ADCC accumulator and ERR calculation have not overflowed

  @Param
    None.

  @Example
    <code>
    bool accumulatorOverflow;    
    ADCC_Initialize();
    ADCC_StartConversion();
    accumulatorOverflow = ADCC_HasAccumulatorOverflowed();
    </code>
*/
bool ADCC_HasAccumulatorOverflowed(void);

/**
  @Summary
   Returns the value of ADCC Filter register.

  @Description
    This routine is used to retrieve value of ADCC Filter register.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    16-bit value obtained from ADFLTRH and ADFLTRL registers.

  @Param
    None.

  @Example
    <code>
    uint16_t filterValue;
    ADCC_Initialize();
    filterValue = ADCC_GetFilterValue();
    </code>
*/
uint16_t ADCC_GetFilterValue(void);

/**
  @Summary
   Returns the value of ADCC Previous Result register.

  @Description
    This routine is used to retrieve value of ADCC Previous register.

  @Preconditions
    ADCC_Initialize() and ADCC_StartConversion() should have been called before
    calling this function.

  @Returns
    16-bit value obtained from ADPREVH and ADPREVL registers.

  @Param
    None.

  @Example
    <code>
    uint16_t prevResult, convertedValue;
    ADCC_Initialize();
    ADCC_StartConversion(channel_ANA0);
    convertedValue = ADCC_GetConversionResult();
    prevResult = ADCC_GetPreviousResult();
    </code>
*/
uint16_t ADCC_GetPreviousResult(void);

/**
  @Summary
   Sets the ADCC Threshold Set-point.

  @Description
    This routine is used to set value of ADCC Threshold Set-point.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    16-bit value for set point.

  @Example
    <code>
    uint16_t setPoint = 90;
    ADCC_Initialize();
    ADCC_DefineSetPoint(setPoint);
    ADCC_StartConversion(channel_ANA0);
    </code>
*/
void ADCC_DefineSetPoint(uint16_t);

/**
  @Summary
   Sets the value of ADCC Upper Threshold.

  @Description
    This routine is used to set value of ADCC Upper Threshold register.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    16-bit value for upper threshold.

  @Example
    <code>
        uint16_t upperThreshold = 90;
        ADCC_Initialize();
        ADCC_SetUpperThreshold(upperThreshold);
        ADCC_StartConversion(channel_ANA0);
    </code>
*/
void ADCC_SetUpperThreshold(uint16_t);

/**
  @Summary
   Sets the value of ADCC Lower Threshold.

  @Description
    This routine is used to set value of ADCC Lower Threshold register.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    16- bit value for lower threshold.

  @Example
    <code>
    uint16_t lowerThreshold = 90;
    ADCC_Initialize();
    ADCC_SetLowerThreshold(lowerThreshold);
    ADCC_StartConversion(channel_ANA0);    
    </code>
*/
void ADCC_SetLowerThreshold(uint16_t);

/**
  @Summary
   Returns the value of ADCC Set-point Error.

  @Description
    This routine retrieves the value of ADCC Set-point Error register.

  @Preconditions
    ADCC_Initialize(), ADCC_StartConversion() should have been called before calling
    this function.

  @Returns
    16-bit value obtained from ADERRH and ADERRL registers.

  @Param
    None.

  @Example
    <code>
    uint16_t error;
    ADCC_Initialize();
    ADCC_StartConversion(channel_ANA0);
    error = ADCC_GetErrorCalculation(void);
    </code>
*/
uint16_t ADCC_GetErrorCalculation(void);

/**
  @Summary
   Enables Double-Sampling.

  @Description
    This routine is used to enable double-sampling bit.
    Two conversions are performed on each trigger. Data from the first conversion 
    appears in PREV.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    None.

  @Example
    <code>
    ADCC_Initialize();
    ADCC_EnableDoubleSampling();    
    ADCC_StartConversion(channel_ANA0);
    </code>
*/
void ADCC_EnableDoubleSampling(void);

/**
  @Summary
   Enables continuous A/D conversion.

  @Description
    This routine is used to enable continuous A/D conversion.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    None.

  @Example
    <code>
    ADCC_Initialize();
    ADCC_EnableContinuousConversion();
    </code>
*/
void ADCC_EnableContinuousConversion(void);

/**
  @Summary
   Disables continuous A/D conversion.

  @Description
    This routine is used to disable continuous A/D conversion.

  @Preconditions
    ADCC_Initialize() should have been called before calling this function.

  @Returns
    None

  @Param
    None.

  @Example
    <code>
    ADCC_Initialize();
    ADCC_DisableContinuousConversion();
    </code>
*/
void ADCC_DisableContinuousConversion(void);

/**
  @Summary
   Determines if ADCC ERR crosses upper threshold.

  @Description
    This routine is used to determine if ADCC ERR has crossed the upper threshold.

  @Preconditions
    ADCC_Initialize() and ADCC_StartConversion() should have been called 
    before calling this function.

  @Returns
    1: if ERR > UTH
    0: if ERR <= UTH

  @Param
    None.

  @Example
    <code>
    bool uThr;
    ADCC_Initialize();
    ADCC_StartConversion(channel_ANA0);
    uThr = ADCC_HasErrorCrossedUpperThreshold();
    </code>
*/
bool ADCC_HasErrorCrossedUpperThreshold(void);

/**
  @Summary
   Determines if ADCC ERR is less than lower threshold.

  @Description
    This routine is used to determine if ADCC ERR is less than the lower threshold.

  @Preconditions
    ADCC_Initialize() and ADCC_StartConversion() should have been called 
    before calling this function.

  @Returns
    1: if ERR < LTH
    0: if ERR >= LTH

  @Param
    None.

  @Example
    <code>
    bool lThr;
    ADCC_Initialize();
    ADCC_StartConversion(channel_ANA0);
    lThr = ADCC_HasErrorCrossedLowerThreshold();
    </code>
*/
bool ADCC_HasErrorCrossedLowerThreshold(void);

/**
  @Summary
   Returns Status of ADCC

  @Description
    This routine is used to retrieve contents of ADCC status register.

  @Preconditions
    ADCC_Initialize() and ADCC_StartConversion() should have been called 
    before calling this function.

  @Returns
    Returns the contents of ADCC STATUS register

  @Param
    None.

  @Example
    <code>
    uint8_t adccStatus;
    ADCC_Initialize();
    ADCC_StartConversion(channel_ANA0);
    adccStatus = ADCC_GetConversionStageStatus();
    </code>
*/
uint8_t ADCC_GetConversionStageStatus(void);




#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif    //ADCC_H
/**
 End of File
*/

