/*
 * RS485_shunt.c
 *
 *  Created on: Jul 14, 2022
 *      Author: Asith Devinda
 */

#include "F2806x_Device.h"
#include "F2806x_Examples.h"
#include <main_data.h>
#include "stdio.h"
#include <RS485_shunt.h>

#define RS485_timeout   2

Uint16 volatile request_code = 0;
Uint16 volatile INA229_reset_flag = 0;

Uint16 modbus_tx_buf[4];
Uint16 volatile modbus_rx_buf[8];
Sci_data modbus_rx_data;

volatile float INA229_data[5] = {0}; /*{busVoltage, busCurrent, power, energy, charge}*/

/*
 * Initializes SCIA GPIO pins
 * Configure SCI dataframe to be 1 stop bit, No loopback, No parity,8 char bits, async mode
 * Enables SCIA receiver mode
 */

void RS485_Serial_Init(void)
{
    //InitSciaGpio();

    //Select 8 character bits in dataframe//
    SciaRegs.SCICCR.bit.SCICHAR = 0x07;

    //Set baud rate to 115200 (((90Mhz/4)/(115200*8)-1)//
    SciaRegs.SCIHBAUD = 0x00;
    SciaRegs.SCILBAUD = 0x17;

    //Receive enable
    //SciaRegs.SCICTL1.bit.RXENA = 1;

    //Wake up SCIA//
    SciaRegs.SCICTL1.bit.SWRESET = 1;

    //Enable RS485 receiver mode
    RS485_ReceiverEn;
}

/*Initialize timer 2 for MODBUS operation*/
void MODBUS_RTU_Init(void)
{
    ConfigCpuTimer(&CpuTimer2, 90, 750);

    CpuTimer2Regs.TCR.all = 0; //stop timer interrupt
    CpuTimer2Regs.TCR.bit.TSS = 1;  //stop timer
}

Uint16 MODBUS_send(Uint16 function, Uint16 address)
{
    /*Arrange dataframe*/
    modbus_tx_buf[0] = address;
    modbus_tx_buf[1] = function;
    Uint16 crc = ModRTU_CRC(modbus_tx_buf, 2);
    modbus_tx_buf[2] = (crc >> 8);
    modbus_tx_buf[3] = (crc & 0x00FF);

    /*Transmit dataframe*/
    Uint16 Status = RS485_Write(modbus_tx_buf, 4);

    /*Update request code*/
    request_code = function;

    return Status;
}

Uint16 RS485_read(Uint16 *data)
{
    Uint16 Status = 0x0000;
    Uint8 waitcount;

    waitcount = t50_ms_timer;

    SciaRegs.SCICTL1.bit.RXENA = 1;

    while (SciaRegs.SCIRXST.bit.RXRDY != 1)
    {
        if ((t50_ms_timer >= (waitcount + RS485_timeout))
                || (t50_ms_timer < waitcount))
        {
            Status = 0xFFFF; /*SCI read error*/
            SciaRegs.SCICTL1.bit.RXENA = 0;
            return Status;
        }
    }

//    if (SciaRegs.SCIRXST.bit.RXERROR)
//    {
//        /*SWRST*/
//        Status = 0xFFF1;
//
//    }
    if (SciaRegs.SCIRXST.bit.FE)
    {
        /*Frame error - SWRST*/
        Status = 0xFFF2;

    }
    else if (SciaRegs.SCIRXST.bit.OE)
    {
        /*Overflow error - SWRST*/
        Status = 0xFFF3;

    }
    else if (SciaRegs.SCIRXST.bit.PE)
    {
        /*Parity error - SWRST*/
        Status = 0xFFF4;

    }
    else
    {
        *data = SciaRegs.SCIRXBUF.all;
    }

    SciaRegs.SCICTL1.bit.RXENA = 0;

    return Status;
}


/*
 * Puts the transceiver in driver mode
 * Enables transmission
 * Sends the data to transmission buffer
 * Disables transmission and puts the transceiver in receiver mode
 */
Uint16 RS485_Write(Uint16 *data, Uint16 length)
{
    Uint16 Status = 0x0000;
    Uint8 waitcount;

    /*Enable transmit mode*/
    RS485_DriverEn;
    SciaRegs.SCICTL1.bit.TXENA = 1;

    int i=0;
    for (i=0; i<length; i++)
    {
        /*Send data*/
        SciaRegs.SCITXBUF = data[i];

        /*Wait till transmission finished*/
        waitcount = t50_ms_timer;
        while (SciaRegs.SCICTL2.bit.TXRDY != 1)
        {
            if ((t50_ms_timer >= (waitcount + RS485_timeout))
                    || (t50_ms_timer < waitcount))
            {
                Status = 0xFFFF; /*SCI write error*/
                return Status;
            }
        }

    }

    /*Take out of transmit mode*/
    DELAY_US(90);
    RS485_ReceiverEn;
    SciaRegs.SCICTL1.bit.TXENA = 0;

    return i;
}

/*
 * Resets the SCIA
 */
void SCI_reset(void)
{
    SciaRegs.SCICTL1.bit.SWRESET = 0;
    for (int a = 10; a==0; a--){}
    SciaRegs.SCICTL1.bit.SWRESET = 1;

}

double read_MODBUS(void)
{
    for (int i = 0; i < 4; i++)
    {
        while (SciaRegs.SCIRXST.bit.RXRDY == 0)
        {
        }

        modbus_rx_buf[i] = SciaRegs.SCIRXBUF.all;
    }

    modbus_rx_data.d_array.byte_0 = modbus_rx_buf[0];
    modbus_rx_data.d_array.byte_1 = modbus_rx_buf[1];
    modbus_rx_data.d_array.byte_2 = modbus_rx_buf[2];
    modbus_rx_data.d_array.byte_3 = modbus_rx_buf[3];

    return modbus_rx_data.data_d;
}

//
// sciaRxFifoIsr -
//
__interrupt void
sciaRxFifoIsr(void)
{
    if (request_code)
    {
        for(Uint16 i=4;i<8;i++)
        {
            if (i == 6)
            {
                modbus_rx_buf[7]=SciaRegs.SCIRXBUF.all;  // Read data
            }
            else if (i == 7)
            {
                modbus_rx_buf[6]=SciaRegs.SCIRXBUF.all;  // Read data
            }
            else
            {
                modbus_rx_buf[i]=SciaRegs.SCIRXBUF.all;  // Read data
            }

        }
        //check crc
        Uint16 crc = ModRTU_CRC(modbus_rx_buf, 8);

        if (!crc)
        {
            modbus_rx_data.d_array.byte_0 = modbus_rx_buf[2];
            modbus_rx_data.d_array.byte_1 = modbus_rx_buf[3];
            modbus_rx_data.d_array.byte_2 = modbus_rx_buf[4];
            modbus_rx_data.d_array.byte_3 = modbus_rx_buf[5];

            INA229_data[request_code-1] = modbus_rx_data.data_d;
            shuntCS_watchdog = 0;
        }
        else
        {
            shuntCS_watchdog++;
        }
        request_code = 0;

    }
    else
    {
        for (Uint16 i = 0; i < 4; i++)
        {
            modbus_rx_buf[i] = SciaRegs.SCIRXBUF.all;  // Read data
        }
        request_code = ((modbus_rx_buf[1] & 0x00FF) | ((modbus_rx_buf[0] << 8) & 0xFF00));

        if (request_code > 7)
        {
            request_code = 0;
        }
        else
        {
            request_code++;
        }
    }

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE ack
}
