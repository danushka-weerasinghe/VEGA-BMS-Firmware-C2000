/*
 * PCA2129RTC.c
 *
 *  Created on: Jun 18, 2020
 *      Author: VEGA_1010
 */

#include "DSP28x_Project.h"
#include <BMS_I2C.h>

extern Uint8 t50_ms_timer;

#define I2C_timeout     2   /*Timeout in multiples of 50ms*/

/*
 * Encode time
 */
void InitDateTime(DateTime *real_time_x,unsigned char msg[]){
    //msg->SlaveAddress = PCA2129_SLAVE_ADDRESS;
    msg[0] = decToBcd(real_time_x->ss)+0x80;
    msg[1] = decToBcd(real_time_x->mm);
    msg[2] = decToBcd(real_time_x->hh);
    msg[3] = decToBcd(real_time_x->d);
    msg[4] = 0x00;
    msg[5] = decToBcd(real_time_x->m);
    msg[6] = decToBcd((unsigned char)(real_time_x->y-2000));
}

/*
 * decode time
 */
void decord_time(DateTime *real_time_x,unsigned char buffer[]){
    real_time_x->ss = bcdToDec(buffer[3]);
    real_time_x->mm = bcdToDec(buffer[4]);
    real_time_x->hh = bcdToDec(buffer[5]);
    real_time_x->d = bcdToDec(buffer[6]);
    real_time_x->m = bcdToDec(buffer[8]);
    real_time_x->y = 2000+bcdToDec(buffer[9]);
}

/*
 * BCD value to  decimal
 */
unsigned char bcdToDec(unsigned char value) {
  return (unsigned char) ( (value/16*10) + (value%16) );
}

/*
 * Decimal to BCD value
 */
unsigned char decToBcd(unsigned char value) {
  return (unsigned char) ( (value/10*16) + (value%10) );
}

//
// I2CA_WriteToReg - This function writes data bytes to a slave
//                   device's register.
//
// INPUTS:
//      - slave_addr ==> Address of slave device being written to
//      - reg[] ==> Slave device register being written to
//      - data[] ==> data buffer of bytes to be written to the slave
//                      device register
//      - data_size ==> # of data bytes being written, size of data buffer
//
//Uint16 I2CA_WriteToReg(Uint16 slave_addr, Uint8 reg,
//                         Uint8 data[], Uint16 data_size)
//{
//    Uint16 i, Status;
//    Uint8 waitcount;
//
//    Status = I2C_SUCCESS;
//
//    //
//    // Wait until the STP bit is cleared from any previous master communication
//    // Clearing of this bit by the module is delayed until after the SCD bit is
//    // set. If this bit is not checked prior to initiating a new message, the
//    // I2C could get confused.
//    //
//    waitcount = t50_ms_timer;
//
//    while (I2caRegs.I2CMDR.bit.STP == 1)
//    {
//        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//        {
//            Status = I2C_STP_NOT_READY_ERROR;
//            return Status;
//        }
//        //return I2C_STP_NOT_READY_ERROR;
//    }
//
//    //
//    // Setup slave address
//    //
//    I2caRegs.I2CSAR = slave_addr;
//
//    //
//    // Check if bus busy
//    //
//    if (I2caRegs.I2CSTR.bit.BB == 1)
//    {
//        return I2C_BUS_BUSY_ERROR;
//    }
//
//    //
//    // Set up as master transmitter
//    // FREE + MST + TRX + IRS
//    //
//    I2caRegs.I2CMDR.all = 0x4620;
//
//    //
//    // Setup number of bytes to send
//    // == register byte + (# of data[] buffer bytes)
//    //
//    I2caRegs.I2CCNT = 1 + data_size;
//
//    I2caRegs.I2CMDR.bit.STT = 0x1; // Send START condition
//    I2caRegs.I2CMDR.bit.STP = 0x1; // STOP condition will be
//                                   // generated when I2CCNT is zero
//
//    //
//    // I2C module will send the following:
//    // register byte ==> data bytes ==> STOP condition
//    //
//    waitcount = t50_ms_timer;
//    while(!I2caRegs.I2CSTR.bit.XRDY){
//        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//        {
//            Status = 0x1111;
//            return Status;
//        }
//    } // Make sure data
//                                       // is ready to be written
//    I2caRegs.I2CDXR = reg;
//
//    #if NACK_CHECK // check if NACK was received
//        if(I2caRegs.I2CSTR.bit.NACK == 1)
//        {
//            I2caRegs.I2CMDR.bit.STP = 1;
//            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
//
//            Status = I2C_ERROR;
//            return Status;
//        }
//    #endif
//
//    // Transmit Data Bytes followed by STOP condition
//    for (i=0; i< data_size; i++)
//    {
//        waitcount = t50_ms_timer;
//        while(!I2caRegs.I2CSTR.bit.XRDY){
//            if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//            {
//                Status = 0x1111;
//                return Status;
//            }
//        } // Make sure data
//                                           // is ready to be written
//        I2caRegs.I2CDXR = data[i];
//
//        #if NACK_CHECK // check if NACK was received
//            if(I2caRegs.I2CSTR.bit.NACK == 1)
//            {
//                I2caRegs.I2CMDR.bit.STP = 1;
//                I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
//
//                Status = I2C_ERROR;
//                break;
//            }
//        #endif
//    }
//
//
//    // Data successfully written
//    return Status;
//}

//
// I2CA_WriteToReg - This function writes data bytes to a slave
//                   device's register.
//
// INPUTS:
//      - slave_addr ==> Address of slave device being written to
//      - reg[] ==> Slave device registers being written to
//      - reg_size ==> # of register bytes being written, size of
//                          reg_size buffer
//      - data[] ==> data buffer of bytes to be written to the slave
//                      device register
//      - data_size ==> # of data bytes being written, size of data buffer
//
Uint16 I2CA_WriteToManyRegs(Uint16 slave_addr, Uint8 reg[],
               Uint16 reg_size, Uint8 data[], Uint16 data_size)
{
    Uint16 i, Status;
    Uint8 waitcount;

    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
    waitcount = t50_ms_timer;
    while (I2caRegs.I2CMDR.bit.STP == 1)
    {
        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
        {
            Status = I2C_STP_NOT_READY_ERROR;
            return Status;
        }
        //return I2C_STP_NOT_READY_ERROR;
    }

    //
    // Setup slave address
    //
    I2caRegs.I2CSAR = slave_addr;

    //
    // Check if bus busy
    //
    if (I2caRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2caRegs.I2CMDR.all = 0x4620;

    //
    // Setup number of bytes to send
    // == (# of register bytes) + (# of data[] buffer bytes)
    //
    I2caRegs.I2CCNT = reg_size + data_size;

    I2caRegs.I2CMDR.bit.STT = 0x1; // Send START condition
    I2caRegs.I2CMDR.bit.STP = 0x1; // STOP condition will be
                                   // generated when I2CCNT is zero

    //
    // I2C module will send the following:
    // register bytes ==> data bytes ==> STOP condition
    //

    // Transmit Register Bytes
    for (i=0; i< reg_size; i++)
    {
        waitcount = t50_ms_timer;
        while(!I2caRegs.I2CSTR.bit.XRDY){
            if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
            {
                Status = 0x1111;
                return Status;
            }
        } // Make sure data
                                           // is ready to be written
        I2caRegs.I2CDXR = reg[i];

        #if NACK_CHECK // check if NACK was received
            if(I2caRegs.I2CSTR.bit.NACK == 1)
            {
                I2caRegs.I2CMDR.bit.STP = 1;
                I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

                Status = I2C_ERROR;
                return Status;
            }
        #endif
    }

    // Transmit Data Bytes followed by STOP condition
    for (i=0; i< data_size; i++)
    {
        waitcount = t50_ms_timer;
        while(!I2caRegs.I2CSTR.bit.XRDY){
            if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
            {
                Status = 0x1111;
                return Status;
            }
        } // Make sure data
                                           // is ready to be written
        I2caRegs.I2CDXR = data[i];

        #if NACK_CHECK // check if NACK was received
            if(I2caRegs.I2CSTR.bit.NACK == 1)
            {
                I2caRegs.I2CMDR.bit.STP = 1;
                I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

                Status = I2C_ERROR;
                return Status;
            }
        #endif
    }


    // Data successfully written
    return Status;
}

//
// I2CA_ReadFromReg - This function reads data bytes from a slave
//                   device's register.
//
// INPUTS:
//      - slave_addr ==> Address of slave device being read from
//      - reg ==> Slave device register being read from
//      - data[] ==> data buffer to store the bytes read from the
//                      slave device register
//      - data_size ==> # of data bytes being received, size of data buffer
//
//Uint16 I2CA_ReadFromReg(Uint16 slave_addr, Uint8 reg,
//                  Uint8 data[], Uint16 data_size)
//{
//    Uint16 i, Status;
//    Uint8 waitcount;
//
//    Status = I2C_SUCCESS;
//
//    //
//    // Wait until the STP bit is cleared from any previous master communication
//    // Clearing of this bit by the module is delayed until after the SCD bit is
//    // set. If this bit is not checked prior to initiating a new message, the
//    // I2C could get confused.
//    //
//    waitcount = t50_ms_timer;
//    while (I2caRegs.I2CMDR.bit.STP == 1)
//    {
//        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//        {
//            Status = I2C_STP_NOT_READY_ERROR;
//            return Status;
//        }
//        //return I2C_STP_NOT_READY_ERROR;
//    }
//
//    //
//    // Setup slave address
//    //
//    I2caRegs.I2CSAR = slave_addr;
//
//    //
//    // Check if bus busy
//    //
//    if (I2caRegs.I2CSTR.bit.BB == 1)
//    {
//        return I2C_BUS_BUSY_ERROR;
//    }
//
//    //
//    // 1. Transmit the slave address followed by the register
//    //      bytes being read from
//    //
//
//    //
//    // Setup number of bytes to send
//    // == # of register bytes
//    //
//    I2caRegs.I2CCNT = 1;
//
//    //
//    // Set up as master transmitter
//    // FREE + MST + TRX + IRS
//    //
//    I2caRegs.I2CMDR.all = 0x4620;
//
//    I2caRegs.I2CMDR.bit.STT = 0x1; // Send START condition
//
//    waitcount = t50_ms_timer;
//    while(!I2caRegs.I2CSTR.bit.ARDY) // Wait for slave address to be sent
//    {
//        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//        {
//            Status = 0x1111;
//            return Status;
//        }
//        if(I2caRegs.I2CSTR.bit.XSMT == 0)
//        {
//            break;
//        }
//    }
//
//    #if NACK_CHECK // check if NACK was received
//        if(I2caRegs.I2CSTR.bit.NACK == 1)
//        {
//            I2caRegs.I2CMDR.bit.STP = 1;
//            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
//
//            Status = I2C_ERROR;
//            return Status;
//        }
//    #endif
//
//
//
//    // Transmit Register Byte
//    waitcount = t50_ms_timer;
//    while(!I2caRegs.I2CSTR.bit.XRDY){
//        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//        {
//            Status = 0x1111;
//            return Status;
//        }
//    } // Make sure data
//                                       // is ready to be written
//    I2caRegs.I2CDXR = reg;
//
//    #if NACK_CHECK // check if NACK was received
//        if(I2caRegs.I2CSTR.bit.NACK == 1)
//        {
//            I2caRegs.I2CMDR.bit.STP = 1;
//            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
//
//            Status = I2C_ERROR;
//            return Status;
//        }
//    #endif
//
//
//    // Wait for previous communication to complete
//    waitcount = t50_ms_timer;
//    while(!I2caRegs.I2CSTR.bit.ARDY){
//        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//        {
//            Status = 0x1111;
//            return Status;
//        }
//    }
//
//    //
//    // 2. Receive data bytes from slave device
//    //
//
//    //
//    // Set up as master receiver
//    // FREE + MST + IRS ==> (Master Receiver)
//    //
//    I2caRegs.I2CMDR.all = 0x4420;
//
//    //
//    // Setup number of bytes to receive
//    // == # of data bytes
//    //
//    I2caRegs.I2CCNT = data_size;
//
//    I2caRegs.I2CMDR.bit.STT = 0x1; // Send repeated START condition & Slave Addr
//    I2caRegs.I2CMDR.bit.STP = 0x1; // set STOP condition to be
//                                   // generated when I2CCNT is zero
//
//    #if NACK_CHECK // check if NACK was received
//        if(I2caRegs.I2CSTR.bit.NACK == 1)
//        {
//            I2caRegs.I2CMDR.bit.STP = 1;
//            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
//
//            Status = I2C_ERROR;
//            return Status;
//        }
//    #endif
//
//    for (i=0; i< data_size; i++)
//    {
//        waitcount = t50_ms_timer;
//        while(!I2caRegs.I2CSTR.bit.RRDY){
//            if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
//            {
//                Status = 0x1111;
//                return Status;
//            }
//        } // Make sure data
//                                           // is ready to be received
//
//        data[i] = I2caRegs.I2CDRR; // Buffer the received byte
//
//    }
//
//    // Data successfully read
//    return Status;
//}

//
// I2CA_ReadFromManyRegs - This function reads data bytes from a slave
//                  device's registers. For transmitting more than one
//                  register during the initial writes
//
// INPUTS:
//      - slave_addr ==> Address of slave device being read from
//      - reg[] ==> Slave device registers being read from
//      - reg_size ==> # of register bytes being written, size of
//                          reg_size buffer
//      - data[] ==> data buffer to store the bytes read from the
//                      slave device register
//      - data_size ==> # of data bytes being received, size of data buffer
//
Uint16 I2CA_ReadFromManyRegs(Uint16 slave_addr, Uint8 reg[],
                  Uint16 reg_size, Uint8 data[], Uint16 data_size)
{
    Uint16 i, Status;
    Uint8 waitcount;

    Status = I2C_SUCCESS;

    //
    // Wait until the STP bit is cleared from any previous master communication
    // Clearing of this bit by the module is delayed until after the SCD bit is
    // set. If this bit is not checked prior to initiating a new message, the
    // I2C could get confused.
    //
    waitcount = t50_ms_timer;
    while (I2caRegs.I2CMDR.bit.STP == 1)
    {
        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
        {
            Status = I2C_STP_NOT_READY_ERROR;
            return Status;
        }//return I2C_STP_NOT_READY_ERROR;
    }

    //
    // Setup slave address
    //
    I2caRegs.I2CSAR = slave_addr;

    //
    // Check if bus busy
    //
    if (I2caRegs.I2CSTR.bit.BB == 1)
    {
        return I2C_BUS_BUSY_ERROR;
    }

    //
    // 1. Transmit the slave address followed by the register
    //      bytes being read from
    //

    //
    // Setup number of bytes to send
    // == # of register bytes
    //
    I2caRegs.I2CCNT = reg_size;

    //
    // Set up as master transmitter
    // FREE + MST + TRX + IRS
    //
    I2caRegs.I2CMDR.all = 0x4620;

    I2caRegs.I2CMDR.bit.STT = 0x1; // Send START condition
    waitcount = t50_ms_timer;
    while(!I2caRegs.I2CSTR.bit.ARDY) // Wait for slave address to be sent
    {
        if(I2caRegs.I2CSTR.bit.XSMT == 0)
        {
            break; // ACK received on slave address
        }
        else if((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
        {
            Status = 0x1111;
            return Status;
        }
    }

    #if NACK_CHECK // check if NACK was received
        if(I2caRegs.I2CSTR.bit.NACK == 1)
        {
            I2caRegs.I2CMDR.bit.STP = 1;
            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

            Status = I2C_ERROR;
            return Status;
        }
    #endif



    // Transmit Register Bytes
    for (i=0; i< reg_size; i++)
    {
        waitcount = t50_ms_timer;
        while(!I2caRegs.I2CSTR.bit.XRDY){
            if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
            {
                Status = 0x1112;
                return Status;
            }
        } // Make sure data
                                           // is ready to be written
        I2caRegs.I2CDXR = reg[i];

        #if NACK_CHECK // check if NACK was received
            if(I2caRegs.I2CSTR.bit.NACK == 1)
            {
                I2caRegs.I2CMDR.bit.STP = 1;
                I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

                Status = I2C_ERROR;
                return Status;
            }
        #endif
    }

    // Wait for previous communication to complete
    waitcount = t50_ms_timer;
    while(!I2caRegs.I2CSTR.bit.ARDY){
        if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
        {
            Status = 0x1113;
            return Status;
        }
    }

    //
    // 2. Receive data bytes from slave device
    //

    //
    // Set up as master receiver
    // FREE + MST + IRS ==> (Master Receiver)
    //
    I2caRegs.I2CMDR.all = 0x4420;

    //
    // Setup number of bytes to receive
    // == # of data bytes
    //
    I2caRegs.I2CCNT = data_size;

    I2caRegs.I2CMDR.bit.STT = 0x1; // Send repeated START condition & Slave Addr
    I2caRegs.I2CMDR.bit.STP = 0x1; // set STOP condition to be
                                   // generated when I2CCNT is zero

    #if NACK_CHECK // check if NACK was received
        if(I2caRegs.I2CSTR.bit.NACK == 1)
        {
            I2caRegs.I2CMDR.bit.STP = 1;
            I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;

            Status = I2C_ERROR;
            return Status;
        }
    #endif

    for (i=0; i< data_size; i++)
    {
        waitcount = t50_ms_timer;
        while(!I2caRegs.I2CSTR.bit.RRDY)  // Make sure data is ready to be received
        {
            if ((t50_ms_timer >= (waitcount + I2C_timeout)) || (t50_ms_timer < waitcount))
            {
                Status = 0x1114;
                return Status;
            }
        }

        data[i] = I2caRegs.I2CDRR; // Buffer the received byte

    }

    // Data successfully read
    return Status;
}


