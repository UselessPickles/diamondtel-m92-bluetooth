/**
  SPI1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    spi1.c

  @Summary
    This is the generated driver implementation file for the SPI1 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides implementations for driver APIs for SPI1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F27Q43
        Driver Version    :  1.0.0
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

#include "spi1.h"
#include <xc.h>

typedef struct { 
    uint8_t con0; 
    uint8_t con1; 
    uint8_t con2; 
    uint8_t baud; 
    uint8_t operation;
} spi1_configuration_t;


//con0 == SPIxCON0, con1 == SPIxCON1, con2 == SPIxCON2, baud == SPIxBAUD, operation == Master/Slave
static const spi1_configuration_t spi1_configuration[] = {   
    { 0x2, 0xc0, 0x0, 0x1, 0 }
};

void SPI1_Initialize(void)
{
    //EN disabled; LSBF MSb first; MST bus slave; BMODE last byte; 
    SPI1CON0 = 0x02;
    //SMP End; CKE Active to idle; CKP Idle:Low, Active:High; FST disabled; SSP active high; SDIP active high; SDOP active high; 
    SPI1CON1 = 0xC0;
    //SSET disabled; TXR not required for a transfer; RXR data is not stored in the FIFO; 
    SPI1CON2 = 0x00;
    //CLKSEL FOSC; 
    SPI1CLK = 0x00;
    //BAUD 1; 
    SPI1BAUD = 0x01;
    TRISCbits.TRISC2 = 0;
}

bool SPI1_Open(spi1_modes_t spi1UniqueConfiguration)
{
    if(!SPI1CON0bits.EN)
    {
        SPI1CON0 = spi1_configuration[spi1UniqueConfiguration].con0;
        SPI1CON1 = spi1_configuration[spi1UniqueConfiguration].con1;
        SPI1CON2 = spi1_configuration[spi1UniqueConfiguration].con2 | (_SPI1CON2_SPI1RXR_MASK | _SPI1CON2_SPI1TXR_MASK);
        SPI1BAUD = spi1_configuration[spi1UniqueConfiguration].baud;        
        TRISCbits.TRISC2 = spi1_configuration[spi1UniqueConfiguration].operation;
        SPI1CON0bits.EN = 1;
        return true;
    }
    return false;
}

void SPI1_Close(void)
{
    SPI1CON0bits.EN = 0;
}

uint8_t SPI1_ExchangeByte(uint8_t data)
{
    SPI1TCNTL = 1;
    SPI1TXB = data;
    while(!PIR3bits.SPI1RXIF);
    return SPI1RXB;
}

void SPI1_ExchangeBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        SPI1TCNTL = 1;
        SPI1TXB = *data;
        while(!PIR3bits.SPI1RXIF);
        *data++ = SPI1RXB;
    }
}

// Half Duplex SPI Functions
void SPI1_WriteBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        SPI1_ExchangeByte(*data++);
    }
}

void SPI1_ReadBlock(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        *data++ = SPI1_ExchangeByte(0);
    }
}

void SPI1_WriteByte(uint8_t byte)
{
    SPI1TXB = byte;
}

uint8_t SPI1_ReadByte(void)
{
    return SPI1RXB;
}