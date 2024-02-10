/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for driver for .
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

#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

/**
  Section: Included Files
*/

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set RA0 procedures
#define RA0_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define RA0_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define RA0_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define RA0_GetValue()              PORTAbits.RA0
#define RA0_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define RA0_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define RA0_SetPullup()             do { WPUAbits.WPUA0 = 1; } while(0)
#define RA0_ResetPullup()           do { WPUAbits.WPUA0 = 0; } while(0)
#define RA0_SetAnalogMode()         do { ANSELAbits.ANSELA0 = 1; } while(0)
#define RA0_SetDigitalMode()        do { ANSELAbits.ANSELA0 = 0; } while(0)

// get/set RA1 procedures
#define RA1_SetHigh()            do { LATAbits.LATA1 = 1; } while(0)
#define RA1_SetLow()             do { LATAbits.LATA1 = 0; } while(0)
#define RA1_Toggle()             do { LATAbits.LATA1 = ~LATAbits.LATA1; } while(0)
#define RA1_GetValue()              PORTAbits.RA1
#define RA1_SetDigitalInput()    do { TRISAbits.TRISA1 = 1; } while(0)
#define RA1_SetDigitalOutput()   do { TRISAbits.TRISA1 = 0; } while(0)
#define RA1_SetPullup()             do { WPUAbits.WPUA1 = 1; } while(0)
#define RA1_ResetPullup()           do { WPUAbits.WPUA1 = 0; } while(0)
#define RA1_SetAnalogMode()         do { ANSELAbits.ANSELA1 = 1; } while(0)
#define RA1_SetDigitalMode()        do { ANSELAbits.ANSELA1 = 0; } while(0)

// get/set RA2 procedures
#define RA2_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define RA2_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define RA2_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define RA2_GetValue()              PORTAbits.RA2
#define RA2_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define RA2_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define RA2_SetPullup()             do { WPUAbits.WPUA2 = 1; } while(0)
#define RA2_ResetPullup()           do { WPUAbits.WPUA2 = 0; } while(0)
#define RA2_SetAnalogMode()         do { ANSELAbits.ANSELA2 = 1; } while(0)
#define RA2_SetDigitalMode()        do { ANSELAbits.ANSELA2 = 0; } while(0)

// get/set IO_MIC_HF_DETECT aliases
#define IO_MIC_HF_DETECT_TRIS                 TRISAbits.TRISA3
#define IO_MIC_HF_DETECT_LAT                  LATAbits.LATA3
#define IO_MIC_HF_DETECT_PORT                 PORTAbits.RA3
#define IO_MIC_HF_DETECT_WPU                  WPUAbits.WPUA3
#define IO_MIC_HF_DETECT_OD                   ODCONAbits.ODCA3
#define IO_MIC_HF_DETECT_ANS                  ANSELAbits.ANSELA3
#define IO_MIC_HF_DETECT_SetHigh()            do { LATAbits.LATA3 = 1; } while(0)
#define IO_MIC_HF_DETECT_SetLow()             do { LATAbits.LATA3 = 0; } while(0)
#define IO_MIC_HF_DETECT_Toggle()             do { LATAbits.LATA3 = ~LATAbits.LATA3; } while(0)
#define IO_MIC_HF_DETECT_GetValue()           PORTAbits.RA3
#define IO_MIC_HF_DETECT_SetDigitalInput()    do { TRISAbits.TRISA3 = 1; } while(0)
#define IO_MIC_HF_DETECT_SetDigitalOutput()   do { TRISAbits.TRISA3 = 0; } while(0)
#define IO_MIC_HF_DETECT_SetPullup()          do { WPUAbits.WPUA3 = 1; } while(0)
#define IO_MIC_HF_DETECT_ResetPullup()        do { WPUAbits.WPUA3 = 0; } while(0)
#define IO_MIC_HF_DETECT_SetPushPull()        do { ODCONAbits.ODCA3 = 0; } while(0)
#define IO_MIC_HF_DETECT_SetOpenDrain()       do { ODCONAbits.ODCA3 = 1; } while(0)
#define IO_MIC_HF_DETECT_SetAnalogMode()      do { ANSELAbits.ANSELA3 = 1; } while(0)
#define IO_MIC_HF_DETECT_SetDigitalMode()     do { ANSELAbits.ANSELA3 = 0; } while(0)

// get/set IO_BATT_VOLTAGE aliases
#define IO_BATT_VOLTAGE_TRIS                 TRISAbits.TRISA4
#define IO_BATT_VOLTAGE_LAT                  LATAbits.LATA4
#define IO_BATT_VOLTAGE_PORT                 PORTAbits.RA4
#define IO_BATT_VOLTAGE_WPU                  WPUAbits.WPUA4
#define IO_BATT_VOLTAGE_OD                   ODCONAbits.ODCA4
#define IO_BATT_VOLTAGE_ANS                  ANSELAbits.ANSELA4
#define IO_BATT_VOLTAGE_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define IO_BATT_VOLTAGE_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define IO_BATT_VOLTAGE_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define IO_BATT_VOLTAGE_GetValue()           PORTAbits.RA4
#define IO_BATT_VOLTAGE_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define IO_BATT_VOLTAGE_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define IO_BATT_VOLTAGE_SetPullup()          do { WPUAbits.WPUA4 = 1; } while(0)
#define IO_BATT_VOLTAGE_ResetPullup()        do { WPUAbits.WPUA4 = 0; } while(0)
#define IO_BATT_VOLTAGE_SetPushPull()        do { ODCONAbits.ODCA4 = 0; } while(0)
#define IO_BATT_VOLTAGE_SetOpenDrain()       do { ODCONAbits.ODCA4 = 1; } while(0)
#define IO_BATT_VOLTAGE_SetAnalogMode()      do { ANSELAbits.ANSELA4 = 1; } while(0)
#define IO_BATT_VOLTAGE_SetDigitalMode()     do { ANSELAbits.ANSELA4 = 0; } while(0)

// get/set IO_MIC_OUT_DISABLE aliases
#define IO_MIC_OUT_DISABLE_TRIS                 TRISAbits.TRISA5
#define IO_MIC_OUT_DISABLE_LAT                  LATAbits.LATA5
#define IO_MIC_OUT_DISABLE_PORT                 PORTAbits.RA5
#define IO_MIC_OUT_DISABLE_WPU                  WPUAbits.WPUA5
#define IO_MIC_OUT_DISABLE_OD                   ODCONAbits.ODCA5
#define IO_MIC_OUT_DISABLE_ANS                  ANSELAbits.ANSELA5
#define IO_MIC_OUT_DISABLE_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define IO_MIC_OUT_DISABLE_GetValue()           PORTAbits.RA5
#define IO_MIC_OUT_DISABLE_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_SetPullup()          do { WPUAbits.WPUA5 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_ResetPullup()        do { WPUAbits.WPUA5 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_SetPushPull()        do { ODCONAbits.ODCA5 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_SetOpenDrain()       do { ODCONAbits.ODCA5 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetAnalogMode()      do { ANSELAbits.ANSELA5 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetDigitalMode()     do { ANSELAbits.ANSELA5 = 0; } while(0)

// get/set IO_VOICE_IN aliases
#define IO_VOICE_IN_TRIS                 TRISAbits.TRISA6
#define IO_VOICE_IN_LAT                  LATAbits.LATA6
#define IO_VOICE_IN_PORT                 PORTAbits.RA6
#define IO_VOICE_IN_WPU                  WPUAbits.WPUA6
#define IO_VOICE_IN_OD                   ODCONAbits.ODCA6
#define IO_VOICE_IN_ANS                  ANSELAbits.ANSELA6
#define IO_VOICE_IN_SetHigh()            do { LATAbits.LATA6 = 1; } while(0)
#define IO_VOICE_IN_SetLow()             do { LATAbits.LATA6 = 0; } while(0)
#define IO_VOICE_IN_Toggle()             do { LATAbits.LATA6 = ~LATAbits.LATA6; } while(0)
#define IO_VOICE_IN_GetValue()           PORTAbits.RA6
#define IO_VOICE_IN_SetDigitalInput()    do { TRISAbits.TRISA6 = 1; } while(0)
#define IO_VOICE_IN_SetDigitalOutput()   do { TRISAbits.TRISA6 = 0; } while(0)
#define IO_VOICE_IN_SetPullup()          do { WPUAbits.WPUA6 = 1; } while(0)
#define IO_VOICE_IN_ResetPullup()        do { WPUAbits.WPUA6 = 0; } while(0)
#define IO_VOICE_IN_SetPushPull()        do { ODCONAbits.ODCA6 = 0; } while(0)
#define IO_VOICE_IN_SetOpenDrain()       do { ODCONAbits.ODCA6 = 1; } while(0)
#define IO_VOICE_IN_SetAnalogMode()      do { ANSELAbits.ANSELA6 = 1; } while(0)
#define IO_VOICE_IN_SetDigitalMode()     do { ANSELAbits.ANSELA6 = 0; } while(0)

// get/set IO_MIC_HF_SELECT aliases
#define IO_MIC_HF_SELECT_TRIS                 TRISAbits.TRISA7
#define IO_MIC_HF_SELECT_LAT                  LATAbits.LATA7
#define IO_MIC_HF_SELECT_PORT                 PORTAbits.RA7
#define IO_MIC_HF_SELECT_WPU                  WPUAbits.WPUA7
#define IO_MIC_HF_SELECT_OD                   ODCONAbits.ODCA7
#define IO_MIC_HF_SELECT_ANS                  ANSELAbits.ANSELA7
#define IO_MIC_HF_SELECT_SetHigh()            do { LATAbits.LATA7 = 1; } while(0)
#define IO_MIC_HF_SELECT_SetLow()             do { LATAbits.LATA7 = 0; } while(0)
#define IO_MIC_HF_SELECT_Toggle()             do { LATAbits.LATA7 = ~LATAbits.LATA7; } while(0)
#define IO_MIC_HF_SELECT_GetValue()           PORTAbits.RA7
#define IO_MIC_HF_SELECT_SetDigitalInput()    do { TRISAbits.TRISA7 = 1; } while(0)
#define IO_MIC_HF_SELECT_SetDigitalOutput()   do { TRISAbits.TRISA7 = 0; } while(0)
#define IO_MIC_HF_SELECT_SetPullup()          do { WPUAbits.WPUA7 = 1; } while(0)
#define IO_MIC_HF_SELECT_ResetPullup()        do { WPUAbits.WPUA7 = 0; } while(0)
#define IO_MIC_HF_SELECT_SetPushPull()        do { ODCONAbits.ODCA7 = 0; } while(0)
#define IO_MIC_HF_SELECT_SetOpenDrain()       do { ODCONAbits.ODCA7 = 1; } while(0)
#define IO_MIC_HF_SELECT_SetAnalogMode()      do { ANSELAbits.ANSELA7 = 1; } while(0)
#define IO_MIC_HF_SELECT_SetDigitalMode()     do { ANSELAbits.ANSELA7 = 0; } while(0)

// get/set TX1 aliases
#define TX1_TRIS                 TRISBbits.TRISB0
#define TX1_LAT                  LATBbits.LATB0
#define TX1_PORT                 PORTBbits.RB0
#define TX1_WPU                  WPUBbits.WPUB0
#define TX1_OD                   ODCONBbits.ODCB0
#define TX1_ANS                  ANSELBbits.ANSELB0
#define TX1_SetHigh()            do { LATBbits.LATB0 = 1; } while(0)
#define TX1_SetLow()             do { LATBbits.LATB0 = 0; } while(0)
#define TX1_Toggle()             do { LATBbits.LATB0 = ~LATBbits.LATB0; } while(0)
#define TX1_GetValue()           PORTBbits.RB0
#define TX1_SetDigitalInput()    do { TRISBbits.TRISB0 = 1; } while(0)
#define TX1_SetDigitalOutput()   do { TRISBbits.TRISB0 = 0; } while(0)
#define TX1_SetPullup()          do { WPUBbits.WPUB0 = 1; } while(0)
#define TX1_ResetPullup()        do { WPUBbits.WPUB0 = 0; } while(0)
#define TX1_SetPushPull()        do { ODCONBbits.ODCB0 = 0; } while(0)
#define TX1_SetOpenDrain()       do { ODCONBbits.ODCB0 = 1; } while(0)
#define TX1_SetAnalogMode()      do { ANSELBbits.ANSELB0 = 1; } while(0)
#define TX1_SetDigitalMode()     do { ANSELBbits.ANSELB0 = 0; } while(0)

// get/set RX1 aliases
#define RX1_TRIS                 TRISBbits.TRISB1
#define RX1_LAT                  LATBbits.LATB1
#define RX1_PORT                 PORTBbits.RB1
#define RX1_WPU                  WPUBbits.WPUB1
#define RX1_OD                   ODCONBbits.ODCB1
#define RX1_ANS                  ANSELBbits.ANSELB1
#define RX1_SetHigh()            do { LATBbits.LATB1 = 1; } while(0)
#define RX1_SetLow()             do { LATBbits.LATB1 = 0; } while(0)
#define RX1_Toggle()             do { LATBbits.LATB1 = ~LATBbits.LATB1; } while(0)
#define RX1_GetValue()           PORTBbits.RB1
#define RX1_SetDigitalInput()    do { TRISBbits.TRISB1 = 1; } while(0)
#define RX1_SetDigitalOutput()   do { TRISBbits.TRISB1 = 0; } while(0)
#define RX1_SetPullup()          do { WPUBbits.WPUB1 = 1; } while(0)
#define RX1_ResetPullup()        do { WPUBbits.WPUB1 = 0; } while(0)
#define RX1_SetPushPull()        do { ODCONBbits.ODCB1 = 0; } while(0)
#define RX1_SetOpenDrain()       do { ODCONBbits.ODCB1 = 1; } while(0)
#define RX1_SetAnalogMode()      do { ANSELBbits.ANSELB1 = 1; } while(0)
#define RX1_SetDigitalMode()     do { ANSELBbits.ANSELB1 = 0; } while(0)

// get/set IO_SWITCHED_PWR_ENABLE aliases
#define IO_SWITCHED_PWR_ENABLE_TRIS                 TRISBbits.TRISB2
#define IO_SWITCHED_PWR_ENABLE_LAT                  LATBbits.LATB2
#define IO_SWITCHED_PWR_ENABLE_PORT                 PORTBbits.RB2
#define IO_SWITCHED_PWR_ENABLE_WPU                  WPUBbits.WPUB2
#define IO_SWITCHED_PWR_ENABLE_OD                   ODCONBbits.ODCB2
#define IO_SWITCHED_PWR_ENABLE_ANS                  ANSELBbits.ANSELB2
#define IO_SWITCHED_PWR_ENABLE_SetHigh()            do { LATBbits.LATB2 = 1; } while(0)
#define IO_SWITCHED_PWR_ENABLE_SetLow()             do { LATBbits.LATB2 = 0; } while(0)
#define IO_SWITCHED_PWR_ENABLE_Toggle()             do { LATBbits.LATB2 = ~LATBbits.LATB2; } while(0)
#define IO_SWITCHED_PWR_ENABLE_GetValue()           PORTBbits.RB2
#define IO_SWITCHED_PWR_ENABLE_SetDigitalInput()    do { TRISBbits.TRISB2 = 1; } while(0)
#define IO_SWITCHED_PWR_ENABLE_SetDigitalOutput()   do { TRISBbits.TRISB2 = 0; } while(0)
#define IO_SWITCHED_PWR_ENABLE_SetPullup()          do { WPUBbits.WPUB2 = 1; } while(0)
#define IO_SWITCHED_PWR_ENABLE_ResetPullup()        do { WPUBbits.WPUB2 = 0; } while(0)
#define IO_SWITCHED_PWR_ENABLE_SetPushPull()        do { ODCONBbits.ODCB2 = 0; } while(0)
#define IO_SWITCHED_PWR_ENABLE_SetOpenDrain()       do { ODCONBbits.ODCB2 = 1; } while(0)
#define IO_SWITCHED_PWR_ENABLE_SetAnalogMode()      do { ANSELBbits.ANSELB2 = 1; } while(0)
#define IO_SWITCHED_PWR_ENABLE_SetDigitalMode()     do { ANSELBbits.ANSELB2 = 0; } while(0)

// get/set IO_PWR_BTN aliases
#define IO_PWR_BTN_TRIS                 TRISBbits.TRISB3
#define IO_PWR_BTN_LAT                  LATBbits.LATB3
#define IO_PWR_BTN_PORT                 PORTBbits.RB3
#define IO_PWR_BTN_WPU                  WPUBbits.WPUB3
#define IO_PWR_BTN_OD                   ODCONBbits.ODCB3
#define IO_PWR_BTN_ANS                  ANSELBbits.ANSELB3
#define IO_PWR_BTN_SetHigh()            do { LATBbits.LATB3 = 1; } while(0)
#define IO_PWR_BTN_SetLow()             do { LATBbits.LATB3 = 0; } while(0)
#define IO_PWR_BTN_Toggle()             do { LATBbits.LATB3 = ~LATBbits.LATB3; } while(0)
#define IO_PWR_BTN_GetValue()           PORTBbits.RB3
#define IO_PWR_BTN_SetDigitalInput()    do { TRISBbits.TRISB3 = 1; } while(0)
#define IO_PWR_BTN_SetDigitalOutput()   do { TRISBbits.TRISB3 = 0; } while(0)
#define IO_PWR_BTN_SetPullup()          do { WPUBbits.WPUB3 = 1; } while(0)
#define IO_PWR_BTN_ResetPullup()        do { WPUBbits.WPUB3 = 0; } while(0)
#define IO_PWR_BTN_SetPushPull()        do { ODCONBbits.ODCB3 = 0; } while(0)
#define IO_PWR_BTN_SetOpenDrain()       do { ODCONBbits.ODCB3 = 1; } while(0)
#define IO_PWR_BTN_SetAnalogMode()      do { ANSELBbits.ANSELB3 = 1; } while(0)
#define IO_PWR_BTN_SetDigitalMode()     do { ANSELBbits.ANSELB3 = 0; } while(0)

// get/set IO_EXT_PWR_DETECT aliases
#define IO_EXT_PWR_DETECT_TRIS                 TRISBbits.TRISB4
#define IO_EXT_PWR_DETECT_LAT                  LATBbits.LATB4
#define IO_EXT_PWR_DETECT_PORT                 PORTBbits.RB4
#define IO_EXT_PWR_DETECT_WPU                  WPUBbits.WPUB4
#define IO_EXT_PWR_DETECT_OD                   ODCONBbits.ODCB4
#define IO_EXT_PWR_DETECT_ANS                  ANSELBbits.ANSELB4
#define IO_EXT_PWR_DETECT_SetHigh()            do { LATBbits.LATB4 = 1; } while(0)
#define IO_EXT_PWR_DETECT_SetLow()             do { LATBbits.LATB4 = 0; } while(0)
#define IO_EXT_PWR_DETECT_Toggle()             do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define IO_EXT_PWR_DETECT_GetValue()           PORTBbits.RB4
#define IO_EXT_PWR_DETECT_SetDigitalInput()    do { TRISBbits.TRISB4 = 1; } while(0)
#define IO_EXT_PWR_DETECT_SetDigitalOutput()   do { TRISBbits.TRISB4 = 0; } while(0)
#define IO_EXT_PWR_DETECT_SetPullup()          do { WPUBbits.WPUB4 = 1; } while(0)
#define IO_EXT_PWR_DETECT_ResetPullup()        do { WPUBbits.WPUB4 = 0; } while(0)
#define IO_EXT_PWR_DETECT_SetPushPull()        do { ODCONBbits.ODCB4 = 0; } while(0)
#define IO_EXT_PWR_DETECT_SetOpenDrain()       do { ODCONBbits.ODCB4 = 1; } while(0)
#define IO_EXT_PWR_DETECT_SetAnalogMode()      do { ANSELBbits.ANSELB4 = 1; } while(0)
#define IO_EXT_PWR_DETECT_SetDigitalMode()     do { ANSELBbits.ANSELB4 = 0; } while(0)

// get/set IO_IGN_DETECT aliases
#define IO_IGN_DETECT_TRIS                 TRISBbits.TRISB5
#define IO_IGN_DETECT_LAT                  LATBbits.LATB5
#define IO_IGN_DETECT_PORT                 PORTBbits.RB5
#define IO_IGN_DETECT_WPU                  WPUBbits.WPUB5
#define IO_IGN_DETECT_OD                   ODCONBbits.ODCB5
#define IO_IGN_DETECT_ANS                  ANSELBbits.ANSELB5
#define IO_IGN_DETECT_SetHigh()            do { LATBbits.LATB5 = 1; } while(0)
#define IO_IGN_DETECT_SetLow()             do { LATBbits.LATB5 = 0; } while(0)
#define IO_IGN_DETECT_Toggle()             do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define IO_IGN_DETECT_GetValue()           PORTBbits.RB5
#define IO_IGN_DETECT_SetDigitalInput()    do { TRISBbits.TRISB5 = 1; } while(0)
#define IO_IGN_DETECT_SetDigitalOutput()   do { TRISBbits.TRISB5 = 0; } while(0)
#define IO_IGN_DETECT_SetPullup()          do { WPUBbits.WPUB5 = 1; } while(0)
#define IO_IGN_DETECT_ResetPullup()        do { WPUBbits.WPUB5 = 0; } while(0)
#define IO_IGN_DETECT_SetPushPull()        do { ODCONBbits.ODCB5 = 0; } while(0)
#define IO_IGN_DETECT_SetOpenDrain()       do { ODCONBbits.ODCB5 = 1; } while(0)
#define IO_IGN_DETECT_SetAnalogMode()      do { ANSELBbits.ANSELB5 = 1; } while(0)
#define IO_IGN_DETECT_SetDigitalMode()     do { ANSELBbits.ANSELB5 = 0; } while(0)

// get/set SDI1 aliases
#define SDI1_TRIS                 TRISCbits.TRISC0
#define SDI1_LAT                  LATCbits.LATC0
#define SDI1_PORT                 PORTCbits.RC0
#define SDI1_WPU                  WPUCbits.WPUC0
#define SDI1_OD                   ODCONCbits.ODCC0
#define SDI1_ANS                  ANSELCbits.ANSELC0
#define SDI1_SetHigh()            do { LATCbits.LATC0 = 1; } while(0)
#define SDI1_SetLow()             do { LATCbits.LATC0 = 0; } while(0)
#define SDI1_Toggle()             do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define SDI1_GetValue()           PORTCbits.RC0
#define SDI1_SetDigitalInput()    do { TRISCbits.TRISC0 = 1; } while(0)
#define SDI1_SetDigitalOutput()   do { TRISCbits.TRISC0 = 0; } while(0)
#define SDI1_SetPullup()          do { WPUCbits.WPUC0 = 1; } while(0)
#define SDI1_ResetPullup()        do { WPUCbits.WPUC0 = 0; } while(0)
#define SDI1_SetPushPull()        do { ODCONCbits.ODCC0 = 0; } while(0)
#define SDI1_SetOpenDrain()       do { ODCONCbits.ODCC0 = 1; } while(0)
#define SDI1_SetAnalogMode()      do { ANSELCbits.ANSELC0 = 1; } while(0)
#define SDI1_SetDigitalMode()     do { ANSELCbits.ANSELC0 = 0; } while(0)

// get/set RC1 procedures
#define RC1_SetHigh()            do { LATCbits.LATC1 = 1; } while(0)
#define RC1_SetLow()             do { LATCbits.LATC1 = 0; } while(0)
#define RC1_Toggle()             do { LATCbits.LATC1 = ~LATCbits.LATC1; } while(0)
#define RC1_GetValue()              PORTCbits.RC1
#define RC1_SetDigitalInput()    do { TRISCbits.TRISC1 = 1; } while(0)
#define RC1_SetDigitalOutput()   do { TRISCbits.TRISC1 = 0; } while(0)
#define RC1_SetPullup()             do { WPUCbits.WPUC1 = 1; } while(0)
#define RC1_ResetPullup()           do { WPUCbits.WPUC1 = 0; } while(0)
#define RC1_SetAnalogMode()         do { ANSELCbits.ANSELC1 = 1; } while(0)
#define RC1_SetDigitalMode()        do { ANSELCbits.ANSELC1 = 0; } while(0)

// get/set RC2 procedures
#define RC2_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define RC2_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define RC2_Toggle()             do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define RC2_GetValue()              PORTCbits.RC2
#define RC2_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define RC2_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)
#define RC2_SetPullup()             do { WPUCbits.WPUC2 = 1; } while(0)
#define RC2_ResetPullup()           do { WPUCbits.WPUC2 = 0; } while(0)
#define RC2_SetAnalogMode()         do { ANSELCbits.ANSELC2 = 1; } while(0)
#define RC2_SetDigitalMode()        do { ANSELCbits.ANSELC2 = 0; } while(0)

// get/set SPI1_CS_DPOT aliases
#define SPI1_CS_DPOT_TRIS                 TRISCbits.TRISC3
#define SPI1_CS_DPOT_LAT                  LATCbits.LATC3
#define SPI1_CS_DPOT_PORT                 PORTCbits.RC3
#define SPI1_CS_DPOT_WPU                  WPUCbits.WPUC3
#define SPI1_CS_DPOT_OD                   ODCONCbits.ODCC3
#define SPI1_CS_DPOT_ANS                  ANSELCbits.ANSELC3
#define SPI1_CS_DPOT_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define SPI1_CS_DPOT_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define SPI1_CS_DPOT_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define SPI1_CS_DPOT_GetValue()           PORTCbits.RC3
#define SPI1_CS_DPOT_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define SPI1_CS_DPOT_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define SPI1_CS_DPOT_SetPullup()          do { WPUCbits.WPUC3 = 1; } while(0)
#define SPI1_CS_DPOT_ResetPullup()        do { WPUCbits.WPUC3 = 0; } while(0)
#define SPI1_CS_DPOT_SetPushPull()        do { ODCONCbits.ODCC3 = 0; } while(0)
#define SPI1_CS_DPOT_SetOpenDrain()       do { ODCONCbits.ODCC3 = 1; } while(0)
#define SPI1_CS_DPOT_SetAnalogMode()      do { ANSELCbits.ANSELC3 = 1; } while(0)
#define SPI1_CS_DPOT_SetDigitalMode()     do { ANSELCbits.ANSELC3 = 0; } while(0)

// get/set IO_BT_RESET aliases
#define IO_BT_RESET_TRIS                 TRISCbits.TRISC4
#define IO_BT_RESET_LAT                  LATCbits.LATC4
#define IO_BT_RESET_PORT                 PORTCbits.RC4
#define IO_BT_RESET_WPU                  WPUCbits.WPUC4
#define IO_BT_RESET_OD                   ODCONCbits.ODCC4
#define IO_BT_RESET_ANS                  ANSELCbits.ANSELC4
#define IO_BT_RESET_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define IO_BT_RESET_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define IO_BT_RESET_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define IO_BT_RESET_GetValue()           PORTCbits.RC4
#define IO_BT_RESET_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define IO_BT_RESET_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define IO_BT_RESET_SetPullup()          do { WPUCbits.WPUC4 = 1; } while(0)
#define IO_BT_RESET_ResetPullup()        do { WPUCbits.WPUC4 = 0; } while(0)
#define IO_BT_RESET_SetPushPull()        do { ODCONCbits.ODCC4 = 0; } while(0)
#define IO_BT_RESET_SetOpenDrain()       do { ODCONCbits.ODCC4 = 1; } while(0)
#define IO_BT_RESET_SetAnalogMode()      do { ANSELCbits.ANSELC4 = 1; } while(0)
#define IO_BT_RESET_SetDigitalMode()     do { ANSELCbits.ANSELC4 = 0; } while(0)

// get/set IO_BT_MFB aliases
#define IO_BT_MFB_TRIS                 TRISCbits.TRISC5
#define IO_BT_MFB_LAT                  LATCbits.LATC5
#define IO_BT_MFB_PORT                 PORTCbits.RC5
#define IO_BT_MFB_WPU                  WPUCbits.WPUC5
#define IO_BT_MFB_OD                   ODCONCbits.ODCC5
#define IO_BT_MFB_ANS                  ANSELCbits.ANSELC5
#define IO_BT_MFB_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define IO_BT_MFB_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define IO_BT_MFB_Toggle()             do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define IO_BT_MFB_GetValue()           PORTCbits.RC5
#define IO_BT_MFB_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define IO_BT_MFB_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)
#define IO_BT_MFB_SetPullup()          do { WPUCbits.WPUC5 = 1; } while(0)
#define IO_BT_MFB_ResetPullup()        do { WPUCbits.WPUC5 = 0; } while(0)
#define IO_BT_MFB_SetPushPull()        do { ODCONCbits.ODCC5 = 0; } while(0)
#define IO_BT_MFB_SetOpenDrain()       do { ODCONCbits.ODCC5 = 1; } while(0)
#define IO_BT_MFB_SetAnalogMode()      do { ANSELCbits.ANSELC5 = 1; } while(0)
#define IO_BT_MFB_SetDigitalMode()     do { ANSELCbits.ANSELC5 = 0; } while(0)

// get/set RC6 procedures
#define RC6_SetHigh()            do { LATCbits.LATC6 = 1; } while(0)
#define RC6_SetLow()             do { LATCbits.LATC6 = 0; } while(0)
#define RC6_Toggle()             do { LATCbits.LATC6 = ~LATCbits.LATC6; } while(0)
#define RC6_GetValue()              PORTCbits.RC6
#define RC6_SetDigitalInput()    do { TRISCbits.TRISC6 = 1; } while(0)
#define RC6_SetDigitalOutput()   do { TRISCbits.TRISC6 = 0; } while(0)
#define RC6_SetPullup()             do { WPUCbits.WPUC6 = 1; } while(0)
#define RC6_ResetPullup()           do { WPUCbits.WPUC6 = 0; } while(0)
#define RC6_SetAnalogMode()         do { ANSELCbits.ANSELC6 = 1; } while(0)
#define RC6_SetDigitalMode()        do { ANSELCbits.ANSELC6 = 0; } while(0)

// get/set TX2 aliases
#define TX2_TRIS                 TRISCbits.TRISC7
#define TX2_LAT                  LATCbits.LATC7
#define TX2_PORT                 PORTCbits.RC7
#define TX2_WPU                  WPUCbits.WPUC7
#define TX2_OD                   ODCONCbits.ODCC7
#define TX2_ANS                  ANSELCbits.ANSELC7
#define TX2_SetHigh()            do { LATCbits.LATC7 = 1; } while(0)
#define TX2_SetLow()             do { LATCbits.LATC7 = 0; } while(0)
#define TX2_Toggle()             do { LATCbits.LATC7 = ~LATCbits.LATC7; } while(0)
#define TX2_GetValue()           PORTCbits.RC7
#define TX2_SetDigitalInput()    do { TRISCbits.TRISC7 = 1; } while(0)
#define TX2_SetDigitalOutput()   do { TRISCbits.TRISC7 = 0; } while(0)
#define TX2_SetPullup()          do { WPUCbits.WPUC7 = 1; } while(0)
#define TX2_ResetPullup()        do { WPUCbits.WPUC7 = 0; } while(0)
#define TX2_SetPushPull()        do { ODCONCbits.ODCC7 = 0; } while(0)
#define TX2_SetOpenDrain()       do { ODCONCbits.ODCC7 = 1; } while(0)
#define TX2_SetAnalogMode()      do { ANSELCbits.ANSELC7 = 1; } while(0)
#define TX2_SetDigitalMode()     do { ANSELCbits.ANSELC7 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);



/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handler for the IOCAF3 pin functionality
 * @Example
    IOCAF3_ISR();
 */
void IOCAF3_ISR(void);

/**
  @Summary
    Interrupt Handler Setter for IOCAF3 pin interrupt-on-change functionality

  @Description
    Allows selecting an interrupt handler for IOCAF3 at application runtime
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    InterruptHandler function pointer.

  @Example
    PIN_MANAGER_Initialize();
    IOCAF3_SetInterruptHandler(MyInterruptHandler);

*/
void IOCAF3_SetInterruptHandler(void (* InterruptHandler)(void));

/**
  @Summary
    Dynamic Interrupt Handler for IOCAF3 pin

  @Description
    This is a dynamic interrupt handler to be used together with the IOCAF3_SetInterruptHandler() method.
    This handler is called every time the IOCAF3 ISR is executed and allows any function to be registered at runtime.
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCAF3_SetInterruptHandler(IOCAF3_InterruptHandler);

*/
extern void (*IOCAF3_InterruptHandler)(void);

/**
  @Summary
    Default Interrupt Handler for IOCAF3 pin

  @Description
    This is a predefined interrupt handler to be used together with the IOCAF3_SetInterruptHandler() method.
    This handler is called every time the IOCAF3 ISR is executed. 
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCAF3_SetInterruptHandler(IOCAF3_DefaultInterruptHandler);

*/
void IOCAF3_DefaultInterruptHandler(void);


/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handler for the IOCBF3 pin functionality
 * @Example
    IOCBF3_ISR();
 */
void IOCBF3_ISR(void);

/**
  @Summary
    Interrupt Handler Setter for IOCBF3 pin interrupt-on-change functionality

  @Description
    Allows selecting an interrupt handler for IOCBF3 at application runtime
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    InterruptHandler function pointer.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF3_SetInterruptHandler(MyInterruptHandler);

*/
void IOCBF3_SetInterruptHandler(void (* InterruptHandler)(void));

/**
  @Summary
    Dynamic Interrupt Handler for IOCBF3 pin

  @Description
    This is a dynamic interrupt handler to be used together with the IOCBF3_SetInterruptHandler() method.
    This handler is called every time the IOCBF3 ISR is executed and allows any function to be registered at runtime.
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF3_SetInterruptHandler(IOCBF3_InterruptHandler);

*/
extern void (*IOCBF3_InterruptHandler)(void);

/**
  @Summary
    Default Interrupt Handler for IOCBF3 pin

  @Description
    This is a predefined interrupt handler to be used together with the IOCBF3_SetInterruptHandler() method.
    This handler is called every time the IOCBF3 ISR is executed. 
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF3_SetInterruptHandler(IOCBF3_DefaultInterruptHandler);

*/
void IOCBF3_DefaultInterruptHandler(void);


/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handler for the IOCBF4 pin functionality
 * @Example
    IOCBF4_ISR();
 */
void IOCBF4_ISR(void);

/**
  @Summary
    Interrupt Handler Setter for IOCBF4 pin interrupt-on-change functionality

  @Description
    Allows selecting an interrupt handler for IOCBF4 at application runtime
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    InterruptHandler function pointer.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF4_SetInterruptHandler(MyInterruptHandler);

*/
void IOCBF4_SetInterruptHandler(void (* InterruptHandler)(void));

/**
  @Summary
    Dynamic Interrupt Handler for IOCBF4 pin

  @Description
    This is a dynamic interrupt handler to be used together with the IOCBF4_SetInterruptHandler() method.
    This handler is called every time the IOCBF4 ISR is executed and allows any function to be registered at runtime.
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF4_SetInterruptHandler(IOCBF4_InterruptHandler);

*/
extern void (*IOCBF4_InterruptHandler)(void);

/**
  @Summary
    Default Interrupt Handler for IOCBF4 pin

  @Description
    This is a predefined interrupt handler to be used together with the IOCBF4_SetInterruptHandler() method.
    This handler is called every time the IOCBF4 ISR is executed. 
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF4_SetInterruptHandler(IOCBF4_DefaultInterruptHandler);

*/
void IOCBF4_DefaultInterruptHandler(void);


/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handler for the IOCBF5 pin functionality
 * @Example
    IOCBF5_ISR();
 */
void IOCBF5_ISR(void);

/**
  @Summary
    Interrupt Handler Setter for IOCBF5 pin interrupt-on-change functionality

  @Description
    Allows selecting an interrupt handler for IOCBF5 at application runtime
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    InterruptHandler function pointer.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF5_SetInterruptHandler(MyInterruptHandler);

*/
void IOCBF5_SetInterruptHandler(void (* InterruptHandler)(void));

/**
  @Summary
    Dynamic Interrupt Handler for IOCBF5 pin

  @Description
    This is a dynamic interrupt handler to be used together with the IOCBF5_SetInterruptHandler() method.
    This handler is called every time the IOCBF5 ISR is executed and allows any function to be registered at runtime.
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF5_SetInterruptHandler(IOCBF5_InterruptHandler);

*/
extern void (*IOCBF5_InterruptHandler)(void);

/**
  @Summary
    Default Interrupt Handler for IOCBF5 pin

  @Description
    This is a predefined interrupt handler to be used together with the IOCBF5_SetInterruptHandler() method.
    This handler is called every time the IOCBF5 ISR is executed. 
    
  @Preconditions
    Pin Manager intializer called

  @Returns
    None.

  @Param
    None.

  @Example
    PIN_MANAGER_Initialize();
    IOCBF5_SetInterruptHandler(IOCBF5_DefaultInterruptHandler);

*/
void IOCBF5_DefaultInterruptHandler(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/