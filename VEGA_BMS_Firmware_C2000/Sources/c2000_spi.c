/**
 * File: usi_spi.c - msp430 USI SPI implementation
 *
 * Copyright (c) 2012 by Rick Kimball <rick@kimballsoftware.com>
 * spi abstraction api for msp430
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * 07-14-2012 - rick@kimballsoftware.com 
 *    Fixed MODE2/MODE3 phase problems. Added logic to deal with USI5 errata.
 *
 * 2017 Copyright (c) VEGA INNOVATIONS  All Rights Reserved.
 * Modified for TI TMS320F28069 by Murad Mohamed Jabir <muradjabir@yahoo.com>
 * Added support for dual SPI channels
 *
 */

//typedef unsigned char _Bool;
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File


void spib_initialize(void)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.SPIBENCLK = 1;      // SPI-B

   	SpibRegs.SPICCR.all =0x0007;	             // Reset on, rising edge, 16-bit char bits
	SpibRegs.SPICTL.all =0x0006;    		     // Enable master mode, normal phase,
                                                 // enable talk, and SPI int disabled.
	SpibRegs.SPIBRR =0x00E;
    SpibRegs.SPICCR.all =0x0087;		         // Relinquish SPI from Reset
    SpibRegs.SPIPRI.bit.FREE = 1;                // Set so breakpoints don't disturb xmission
}

void spib_disable(void) {
//    USICTL0 |= USISWRST;        // put USI in reset mode

    SpibRegs.SPICCR.bit.SPISWRESET = 0;
}

/**
 * spi_send() - send a byte and recv response
 */
unsigned char spib_send(const unsigned char data)
{
   while(SpibRegs.SPISTS.bit.BUFFULL_FLAG == 1)
   {
   }
   SpibRegs.SPITXBUF = data << 8;
   while(SpibRegs.SPISTS.bit.INT_FLAG != 1)
   {
   }
   return(SpibRegs.SPIRXBUF & 0xFF);

}

/**
 * spi_set_divisor() - set new clock divider for USI
 *
 * There are a fixed set of valid values for clock divisors
 * see the slau144 for details. DIV by 2/4/8 .. 128
 */

void spib_set_divisor(unsigned int clkdiv)
{
 //   USICTL0 |= USISWRST;        // put USI in reset mode
   // USICKCTL = (USICKCTL & ~SPI_DIV_MASK) | clkdiv;
    //USICTL0 &= ~USISWRST;        // release for operation

    SpibRegs.SPICCR.bit.SPISWRESET = 0;
	SpibRegs.SPIBRR = clkdiv-1;
    SpibRegs.SPICCR.bit.SPISWRESET = 1;

}

/**
 * spi_set_bitorder (enum LSBFIRST=0|MSBFIRST=1)
 *
 * Note: this should use the LSBFIRST/MSBFIRST defines however
 * it doesn't to allow this code to compile without Energia.
 *
 */
void spib_set_bitorder(const unsigned char order)
{
//    USICTL0 |= USISWRST;        // put USI in reset mode
//    USICTL0 = (USICTL0 & ~SPI_LSBMSB_MASK) | ((order == 1 /*MSBFIRST*/) ? 0 : USILSB); /* MSBFIRST = 1 */
//    USICTL0 &= ~USISWRST;       // release for operation
}

/**
 * spi_set_datamode() - Motorola SPI Mode 0 ... 3
 *
 * mode is really an enum SPI_MODE0 ... SPI_MODE3 as defined in
 * the Energia header file.
 */
void spib_set_datamode(const unsigned char mode)
{
    //USICTL0 |= USISWRST;        // put USI in reset mode while we make changes
	EALLOW;
    SpibRegs.SPICCR.bit.SPISWRESET = 0;
    switch(mode) {
    case 0:                   /* SPI_MODE0 */
       // USICKCTL &= ~USICKPL; /* CPOL=0 */
        //USICTL1  |= USICKPH;  /* CPHA=0 */

		SpibRegs.SPICCR.bit.CLKPOLARITY = 0;
		SpibRegs.SPICTL.bit.CLK_PHASE = 0;
        break;

    case 1:                   /* SPI_MODE1 */
       // USICKCTL &= ~USICKPL; /* CPOL=0 */
        //USICTL1  &= ~USICKPH; /* CPHA=1 */

		SpibRegs.SPICCR.bit.CLKPOLARITY = 0;
		SpibRegs.SPICTL.bit.CLK_PHASE = 1;
        break;

    case 2:                   /* SPI_MODE2 */
        //USICKCTL |= USICKPL;  /* CPOL=1 */
        //USICTL1  |= USICKPH;  /* CPHA=0 */

		SpibRegs.SPICCR.bit.CLKPOLARITY = 1;
		SpibRegs.SPICTL.bit.CLK_PHASE = 0;
        break;

    case 4:                   /* SPI_MODE3 */
        //USICKCTL |= USICKPL;  /* CPOL=1 */
        //USICTL1  &= ~USICKPH; /* CPHA=1 */

		SpibRegs.SPICCR.bit.CLKPOLARITY = 1;
		SpibRegs.SPICTL.bit.CLK_PHASE = 1;
        break;

    default:
        break;
    }
    //USICTL0 &= ~USISWRST;       // release for operation
	SpibRegs.SPICCR.bit.SPISWRESET = 1;
    EDIS;
}

/*********************************************************************************************************
    END FILE
 *********************************************************************************************************/

