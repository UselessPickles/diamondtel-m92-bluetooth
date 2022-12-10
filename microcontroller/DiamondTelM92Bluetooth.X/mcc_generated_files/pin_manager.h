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

// get/set IO_PWR aliases
#define IO_PWR_TRIS                 TRISAbits.TRISA0
#define IO_PWR_LAT                  LATAbits.LATA0
#define IO_PWR_PORT                 PORTAbits.RA0
#define IO_PWR_WPU                  WPUAbits.WPUA0
#define IO_PWR_OD                   ODCONAbits.ODCA0
#define IO_PWR_ANS                  ANSELAbits.ANSELA0
#define IO_PWR_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define IO_PWR_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define IO_PWR_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define IO_PWR_GetValue()           PORTAbits.RA0
#define IO_PWR_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define IO_PWR_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define IO_PWR_SetPullup()          do { WPUAbits.WPUA0 = 1; } while(0)
#define IO_PWR_ResetPullup()        do { WPUAbits.WPUA0 = 0; } while(0)
#define IO_PWR_SetPushPull()        do { ODCONAbits.ODCA0 = 0; } while(0)
#define IO_PWR_SetOpenDrain()       do { ODCONAbits.ODCA0 = 1; } while(0)
#define IO_PWR_SetAnalogMode()      do { ANSELAbits.ANSELA0 = 1; } while(0)
#define IO_PWR_SetDigitalMode()     do { ANSELAbits.ANSELA0 = 0; } while(0)

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

// get/set RA6 procedures
#define RA6_SetHigh()            do { LATAbits.LATA6 = 1; } while(0)
#define RA6_SetLow()             do { LATAbits.LATA6 = 0; } while(0)
#define RA6_Toggle()             do { LATAbits.LATA6 = ~LATAbits.LATA6; } while(0)
#define RA6_GetValue()              PORTAbits.RA6
#define RA6_SetDigitalInput()    do { TRISAbits.TRISA6 = 1; } while(0)
#define RA6_SetDigitalOutput()   do { TRISAbits.TRISA6 = 0; } while(0)
#define RA6_SetPullup()             do { WPUAbits.WPUA6 = 1; } while(0)
#define RA6_ResetPullup()           do { WPUAbits.WPUA6 = 0; } while(0)
#define RA6_SetAnalogMode()         do { ANSELAbits.ANSELA6 = 1; } while(0)
#define RA6_SetDigitalMode()        do { ANSELAbits.ANSELA6 = 0; } while(0)

// get/set RA7 procedures
#define RA7_SetHigh()            do { LATAbits.LATA7 = 1; } while(0)
#define RA7_SetLow()             do { LATAbits.LATA7 = 0; } while(0)
#define RA7_Toggle()             do { LATAbits.LATA7 = ~LATAbits.LATA7; } while(0)
#define RA7_GetValue()              PORTAbits.RA7
#define RA7_SetDigitalInput()    do { TRISAbits.TRISA7 = 1; } while(0)
#define RA7_SetDigitalOutput()   do { TRISAbits.TRISA7 = 0; } while(0)
#define RA7_SetPullup()             do { WPUAbits.WPUA7 = 1; } while(0)
#define RA7_ResetPullup()           do { WPUAbits.WPUA7 = 0; } while(0)
#define RA7_SetAnalogMode()         do { ANSELAbits.ANSELA7 = 1; } while(0)
#define RA7_SetDigitalMode()        do { ANSELAbits.ANSELA7 = 0; } while(0)

// get/set IO_VOICE_IN aliases
#define IO_VOICE_IN_TRIS                 TRISBbits.TRISB0
#define IO_VOICE_IN_LAT                  LATBbits.LATB0
#define IO_VOICE_IN_PORT                 PORTBbits.RB0
#define IO_VOICE_IN_WPU                  WPUBbits.WPUB0
#define IO_VOICE_IN_OD                   ODCONBbits.ODCB0
#define IO_VOICE_IN_ANS                  ANSELBbits.ANSELB0
#define IO_VOICE_IN_SetHigh()            do { LATBbits.LATB0 = 1; } while(0)
#define IO_VOICE_IN_SetLow()             do { LATBbits.LATB0 = 0; } while(0)
#define IO_VOICE_IN_Toggle()             do { LATBbits.LATB0 = ~LATBbits.LATB0; } while(0)
#define IO_VOICE_IN_GetValue()           PORTBbits.RB0
#define IO_VOICE_IN_SetDigitalInput()    do { TRISBbits.TRISB0 = 1; } while(0)
#define IO_VOICE_IN_SetDigitalOutput()   do { TRISBbits.TRISB0 = 0; } while(0)
#define IO_VOICE_IN_SetPullup()          do { WPUBbits.WPUB0 = 1; } while(0)
#define IO_VOICE_IN_ResetPullup()        do { WPUBbits.WPUB0 = 0; } while(0)
#define IO_VOICE_IN_SetPushPull()        do { ODCONBbits.ODCB0 = 0; } while(0)
#define IO_VOICE_IN_SetOpenDrain()       do { ODCONBbits.ODCB0 = 1; } while(0)
#define IO_VOICE_IN_SetAnalogMode()      do { ANSELBbits.ANSELB0 = 1; } while(0)
#define IO_VOICE_IN_SetDigitalMode()     do { ANSELBbits.ANSELB0 = 0; } while(0)

// get/set IO_MIC_OUT_DISABLE aliases
#define IO_MIC_OUT_DISABLE_TRIS                 TRISBbits.TRISB1
#define IO_MIC_OUT_DISABLE_LAT                  LATBbits.LATB1
#define IO_MIC_OUT_DISABLE_PORT                 PORTBbits.RB1
#define IO_MIC_OUT_DISABLE_WPU                  WPUBbits.WPUB1
#define IO_MIC_OUT_DISABLE_OD                   ODCONBbits.ODCB1
#define IO_MIC_OUT_DISABLE_ANS                  ANSELBbits.ANSELB1
#define IO_MIC_OUT_DISABLE_SetHigh()            do { LATBbits.LATB1 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetLow()             do { LATBbits.LATB1 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_Toggle()             do { LATBbits.LATB1 = ~LATBbits.LATB1; } while(0)
#define IO_MIC_OUT_DISABLE_GetValue()           PORTBbits.RB1
#define IO_MIC_OUT_DISABLE_SetDigitalInput()    do { TRISBbits.TRISB1 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetDigitalOutput()   do { TRISBbits.TRISB1 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_SetPullup()          do { WPUBbits.WPUB1 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_ResetPullup()        do { WPUBbits.WPUB1 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_SetPushPull()        do { ODCONBbits.ODCB1 = 0; } while(0)
#define IO_MIC_OUT_DISABLE_SetOpenDrain()       do { ODCONBbits.ODCB1 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetAnalogMode()      do { ANSELBbits.ANSELB1 = 1; } while(0)
#define IO_MIC_OUT_DISABLE_SetDigitalMode()     do { ANSELBbits.ANSELB1 = 0; } while(0)

// get/set RB2 procedures
#define RB2_SetHigh()            do { LATBbits.LATB2 = 1; } while(0)
#define RB2_SetLow()             do { LATBbits.LATB2 = 0; } while(0)
#define RB2_Toggle()             do { LATBbits.LATB2 = ~LATBbits.LATB2; } while(0)
#define RB2_GetValue()              PORTBbits.RB2
#define RB2_SetDigitalInput()    do { TRISBbits.TRISB2 = 1; } while(0)
#define RB2_SetDigitalOutput()   do { TRISBbits.TRISB2 = 0; } while(0)
#define RB2_SetPullup()             do { WPUBbits.WPUB2 = 1; } while(0)
#define RB2_ResetPullup()           do { WPUBbits.WPUB2 = 0; } while(0)
#define RB2_SetAnalogMode()         do { ANSELBbits.ANSELB2 = 1; } while(0)
#define RB2_SetDigitalMode()        do { ANSELBbits.ANSELB2 = 0; } while(0)

// get/set RB3 procedures
#define RB3_SetHigh()            do { LATBbits.LATB3 = 1; } while(0)
#define RB3_SetLow()             do { LATBbits.LATB3 = 0; } while(0)
#define RB3_Toggle()             do { LATBbits.LATB3 = ~LATBbits.LATB3; } while(0)
#define RB3_GetValue()              PORTBbits.RB3
#define RB3_SetDigitalInput()    do { TRISBbits.TRISB3 = 1; } while(0)
#define RB3_SetDigitalOutput()   do { TRISBbits.TRISB3 = 0; } while(0)
#define RB3_SetPullup()             do { WPUBbits.WPUB3 = 1; } while(0)
#define RB3_ResetPullup()           do { WPUBbits.WPUB3 = 0; } while(0)
#define RB3_SetAnalogMode()         do { ANSELBbits.ANSELB3 = 1; } while(0)
#define RB3_SetDigitalMode()        do { ANSELBbits.ANSELB3 = 0; } while(0)

// get/set RB4 procedures
#define RB4_SetHigh()            do { LATBbits.LATB4 = 1; } while(0)
#define RB4_SetLow()             do { LATBbits.LATB4 = 0; } while(0)
#define RB4_Toggle()             do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define RB4_GetValue()              PORTBbits.RB4
#define RB4_SetDigitalInput()    do { TRISBbits.TRISB4 = 1; } while(0)
#define RB4_SetDigitalOutput()   do { TRISBbits.TRISB4 = 0; } while(0)
#define RB4_SetPullup()             do { WPUBbits.WPUB4 = 1; } while(0)
#define RB4_ResetPullup()           do { WPUBbits.WPUB4 = 0; } while(0)
#define RB4_SetAnalogMode()         do { ANSELBbits.ANSELB4 = 1; } while(0)
#define RB4_SetDigitalMode()        do { ANSELBbits.ANSELB4 = 0; } while(0)

// get/set RB5 procedures
#define RB5_SetHigh()            do { LATBbits.LATB5 = 1; } while(0)
#define RB5_SetLow()             do { LATBbits.LATB5 = 0; } while(0)
#define RB5_Toggle()             do { LATBbits.LATB5 = ~LATBbits.LATB5; } while(0)
#define RB5_GetValue()              PORTBbits.RB5
#define RB5_SetDigitalInput()    do { TRISBbits.TRISB5 = 1; } while(0)
#define RB5_SetDigitalOutput()   do { TRISBbits.TRISB5 = 0; } while(0)
#define RB5_SetPullup()             do { WPUBbits.WPUB5 = 1; } while(0)
#define RB5_ResetPullup()           do { WPUBbits.WPUB5 = 0; } while(0)
#define RB5_SetAnalogMode()         do { ANSELBbits.ANSELB5 = 1; } while(0)
#define RB5_SetDigitalMode()        do { ANSELBbits.ANSELB5 = 0; } while(0)

// get/set SPI1_CS_DPOT aliases
#define SPI1_CS_DPOT_TRIS                 TRISCbits.TRISC0
#define SPI1_CS_DPOT_LAT                  LATCbits.LATC0
#define SPI1_CS_DPOT_PORT                 PORTCbits.RC0
#define SPI1_CS_DPOT_WPU                  WPUCbits.WPUC0
#define SPI1_CS_DPOT_OD                   ODCONCbits.ODCC0
#define SPI1_CS_DPOT_ANS                  ANSELCbits.ANSELC0
#define SPI1_CS_DPOT_SetHigh()            do { LATCbits.LATC0 = 1; } while(0)
#define SPI1_CS_DPOT_SetLow()             do { LATCbits.LATC0 = 0; } while(0)
#define SPI1_CS_DPOT_Toggle()             do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define SPI1_CS_DPOT_GetValue()           PORTCbits.RC0
#define SPI1_CS_DPOT_SetDigitalInput()    do { TRISCbits.TRISC0 = 1; } while(0)
#define SPI1_CS_DPOT_SetDigitalOutput()   do { TRISCbits.TRISC0 = 0; } while(0)
#define SPI1_CS_DPOT_SetPullup()          do { WPUCbits.WPUC0 = 1; } while(0)
#define SPI1_CS_DPOT_ResetPullup()        do { WPUCbits.WPUC0 = 0; } while(0)
#define SPI1_CS_DPOT_SetPushPull()        do { ODCONCbits.ODCC0 = 0; } while(0)
#define SPI1_CS_DPOT_SetOpenDrain()       do { ODCONCbits.ODCC0 = 1; } while(0)
#define SPI1_CS_DPOT_SetAnalogMode()      do { ANSELCbits.ANSELC0 = 1; } while(0)
#define SPI1_CS_DPOT_SetDigitalMode()     do { ANSELCbits.ANSELC0 = 0; } while(0)

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

// get/set RC3 procedures
#define RC3_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define RC3_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define RC3_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define RC3_GetValue()              PORTCbits.RC3
#define RC3_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define RC3_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define RC3_SetPullup()             do { WPUCbits.WPUC3 = 1; } while(0)
#define RC3_ResetPullup()           do { WPUCbits.WPUC3 = 0; } while(0)
#define RC3_SetAnalogMode()         do { ANSELCbits.ANSELC3 = 1; } while(0)
#define RC3_SetDigitalMode()        do { ANSELCbits.ANSELC3 = 0; } while(0)

// get/set IO_BT_ON aliases
#define IO_BT_ON_TRIS                 TRISCbits.TRISC4
#define IO_BT_ON_LAT                  LATCbits.LATC4
#define IO_BT_ON_PORT                 PORTCbits.RC4
#define IO_BT_ON_WPU                  WPUCbits.WPUC4
#define IO_BT_ON_OD                   ODCONCbits.ODCC4
#define IO_BT_ON_ANS                  ANSELCbits.ANSELC4
#define IO_BT_ON_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define IO_BT_ON_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define IO_BT_ON_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define IO_BT_ON_GetValue()           PORTCbits.RC4
#define IO_BT_ON_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define IO_BT_ON_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define IO_BT_ON_SetPullup()          do { WPUCbits.WPUC4 = 1; } while(0)
#define IO_BT_ON_ResetPullup()        do { WPUCbits.WPUC4 = 0; } while(0)
#define IO_BT_ON_SetPushPull()        do { ODCONCbits.ODCC4 = 0; } while(0)
#define IO_BT_ON_SetOpenDrain()       do { ODCONCbits.ODCC4 = 1; } while(0)
#define IO_BT_ON_SetAnalogMode()      do { ANSELCbits.ANSELC4 = 1; } while(0)
#define IO_BT_ON_SetDigitalMode()     do { ANSELCbits.ANSELC4 = 0; } while(0)

// get/set RC5 procedures
#define RC5_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define RC5_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define RC5_Toggle()             do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define RC5_GetValue()              PORTCbits.RC5
#define RC5_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define RC5_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)
#define RC5_SetPullup()             do { WPUCbits.WPUC5 = 1; } while(0)
#define RC5_ResetPullup()           do { WPUCbits.WPUC5 = 0; } while(0)
#define RC5_SetAnalogMode()         do { ANSELCbits.ANSELC5 = 1; } while(0)
#define RC5_SetDigitalMode()        do { ANSELCbits.ANSELC5 = 0; } while(0)

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

// get/set IO_BT_MFB aliases
#define IO_BT_MFB_TRIS                 TRISCbits.TRISC7
#define IO_BT_MFB_LAT                  LATCbits.LATC7
#define IO_BT_MFB_PORT                 PORTCbits.RC7
#define IO_BT_MFB_WPU                  WPUCbits.WPUC7
#define IO_BT_MFB_OD                   ODCONCbits.ODCC7
#define IO_BT_MFB_ANS                  ANSELCbits.ANSELC7
#define IO_BT_MFB_SetHigh()            do { LATCbits.LATC7 = 1; } while(0)
#define IO_BT_MFB_SetLow()             do { LATCbits.LATC7 = 0; } while(0)
#define IO_BT_MFB_Toggle()             do { LATCbits.LATC7 = ~LATCbits.LATC7; } while(0)
#define IO_BT_MFB_GetValue()           PORTCbits.RC7
#define IO_BT_MFB_SetDigitalInput()    do { TRISCbits.TRISC7 = 1; } while(0)
#define IO_BT_MFB_SetDigitalOutput()   do { TRISCbits.TRISC7 = 0; } while(0)
#define IO_BT_MFB_SetPullup()          do { WPUCbits.WPUC7 = 1; } while(0)
#define IO_BT_MFB_ResetPullup()        do { WPUCbits.WPUC7 = 0; } while(0)
#define IO_BT_MFB_SetPushPull()        do { ODCONCbits.ODCC7 = 0; } while(0)
#define IO_BT_MFB_SetOpenDrain()       do { ODCONCbits.ODCC7 = 1; } while(0)
#define IO_BT_MFB_SetAnalogMode()      do { ANSELCbits.ANSELC7 = 1; } while(0)
#define IO_BT_MFB_SetDigitalMode()     do { ANSELCbits.ANSELC7 = 0; } while(0)

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




#endif // PIN_MANAGER_H
/**
 End of File
*/