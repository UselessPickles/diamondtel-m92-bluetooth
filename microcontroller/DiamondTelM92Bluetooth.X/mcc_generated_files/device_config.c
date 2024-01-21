/**
  @Generated PIC10 / PIC12 / PIC16 / PIC18 MCUs Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    mcc.c

  @Summary:
    This is the device_config.c file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F27Q43
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.36 and above or later
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

// Configuration bits: selected in the GUI

// CONFIG1
#pragma config FEXTOSC = OFF    // External Oscillator Selection->Oscillator not enabled
#pragma config RSTOSC = HFINTOSC_64MHZ    // Reset Oscillator Selection->HFINTOSC with HFFRQ = 64 MHz and CDIV = 1:1

// CONFIG2
#pragma config CLKOUTEN = OFF    // Clock out Enable bit->CLKOUT function is disabled
#pragma config PR1WAY = ON    // PRLOCKED One-Way Set Enable bit->PRLOCKED bit can be cleared and set only once
#pragma config CSWEN = ON    // Clock Switch Enable bit->Writing to NOSC and NDIV is allowed
#pragma config FCMEN = ON    // Fail-Safe Clock Monitor Enable bit->Fail-Safe Clock Monitor enabled

// CONFIG3
#pragma config MCLRE = EXTMCLR    // MCLR Enable bit->If LVP = 0, MCLR pin is MCLR; If LVP = 1, RE3 pin function is MCLR 
#pragma config PWRTS = PWRT_16    // Power-up timer selection bits->PWRT set at 16ms
#pragma config MVECEN = ON    // Multi-vector enable bit->Multi-vector enabled, Vector table used for interrupts
#pragma config IVT1WAY = ON    // IVTLOCK bit One-way set enable bit->IVTLOCKED bit can be cleared and set only once
#pragma config LPBOREN = OFF    // Low Power BOR Enable bit->Low-Power BOR disabled
#pragma config BOREN = SBORDIS    // Brown-out Reset Enable bits->Brown-out Reset enabled , SBOREN bit is ignored

// CONFIG4
#pragma config BORV = VBOR_2P45    // Brown-out Reset Voltage Selection bits->Brown-out Reset Voltage (VBOR) set to 2.45V
#pragma config ZCD = OFF    // ZCD Disable bit->ZCD module is disabled. ZCD can be enabled by setting the ZCDSEN bit of ZCDCON
#pragma config PPS1WAY = ON    // PPSLOCK bit One-Way Set Enable bit->PPSLOCKED bit can be cleared and set only once; PPS registers remain locked after one clear/set cycle
#pragma config STVREN = ON    // Stack Full/Underflow Reset Enable bit->Stack full/underflow will cause Reset
#pragma config LVP = OFF    // Low Voltage Programming Enable bit->HV on MCLR/VPP must be used for programming
#pragma config XINST = OFF    // Extended Instruction Set Enable bit->Extended Instruction Set and Indexed Addressing Mode disabled

// CONFIG5
#pragma config WDTCPS = WDTCPS_31    // WDT Period selection bits->Divider ratio 1:65536; software control of WDTPS
#pragma config WDTE = OFF    // WDT operating mode->WDT Disabled; SWDTEN is ignored

// CONFIG6
#pragma config WDTCWS = WDTCWS_7    // WDT Window Select bits->window always open (100%); software control; keyed access not required
#pragma config WDTCCS = SC    // WDT input clock selector->Software Control

// CONFIG7
#pragma config BBSIZE = BBSIZE_512    // Boot Block Size selection bits->Boot Block size is 512 words
#pragma config BBEN = OFF    // Boot Block enable bit->Boot block disabled
#pragma config SAFEN = OFF    // Storage Area Flash enable bit->SAF disabled
#pragma config DEBUG = OFF    // Background Debugger->Background Debugger disabled

// CONFIG8
#pragma config WRTB = OFF    // Boot Block Write Protection bit->Boot Block not Write protected
#pragma config WRTC = OFF    // Configuration Register Write Protection bit->Configuration registers not Write protected
#pragma config WRTD = OFF    // Data EEPROM Write Protection bit->Data EEPROM not Write protected
#pragma config WRTSAF = OFF    // SAF Write protection bit->SAF not Write Protected
#pragma config WRTAPP = OFF    // Application Block write protection bit->Application Block not write protected

// CONFIG10
#pragma config CP = OFF    // PFM and Data EEPROM Code Protection bit->PFM and Data EEPROM code protection disabled
