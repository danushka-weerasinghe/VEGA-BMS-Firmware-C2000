/*
 * spi_430.h - common function declarations for different SPI implementations
 *
 * Copyright (c) 2012 by Rick Kimball <rick@kimballsoftware.com>
 * spi abstraction api for msp430
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * 2017 Copyright (c) VEGA INNOVATIONS  All Rights Reserved.
 * Modified for TI TMS320F28069 by Murad Mohamed Jabir <muradjabir@yahoo.com>
 * Added support for dual SPI channels
 *
 */

#ifndef _C2000_SPI_H_
#define _C2000_SPI_H_

#include "DSP28x_Project.h"

//SPIB
void spib_initialize(void);
void spib_disable(void);
unsigned char spib_send(const unsigned char);
void spib_set_bitorder(const unsigned char);
void spib_set_datamode(const unsigned char);
void spib_set_divisor(unsigned int clkdiv);

#endif /*_C2000_SPI_H_*/

/*********************************************************************************************************
    END FILE
 *********************************************************************************************************/
