/****************************************Copyright (c)****************************************************
**                            Shenzhen SeeedStudio Co.,LTD.
**
**                                 http://www.seeedstudio.com
**
**
**
**--------------File Info---------------------------------------------------------------------------------
** File name:                  mcp_can.h
** Latest modified Date:       2015-10-22
** Latest Version:             v0.1
** Descriptions:               mcp2515 main .c file
**
**--------------------------------------------------------------------------------------------------------
** Created by:                 loovee
** Created date:               2012-4-24
** Version:                    v0.1
** Descriptions:
**
**--------------Port Info---------------------------------------------------------------------------------
** Ported by:                  Dmitri Ranfft, Andrew Buckin
** Date of first release:      2015-11-20
** Version:                    v0.1
** Descriptions:               This library was ported to C2000 microcontrollers. Originally it was
**                             written in C++ for Arduino. In case of questions about the original
**                             arduino library, refer to SeedStudio (link is above).
*********************************************************************************************************/
#include "DSP28x_Project.h"
#include "c2000_spi.h"
#include "mcp_can.h"

#define spi_readwrite spib_send
#define spi_read() spi_readwrite(0x00)

#define MAX_CHAR_IN_MESSAGE 8

INT8U   m_nExtFlg;                                                  // Identifier Type
                                                                        // Extended (29 bit) or Standard (11 bit)
INT32U  m_nID;                                                      // CAN ID
INT8U   m_nDlc;                                                     // Data Length Code
INT8U   m_nDta[MAX_CHAR_IN_MESSAGE];                                // Data array
INT8U   m_nRtr;                                                     // Remote request flag
INT8U   m_nfilhit;                                                  // The number of the filter that matched the message
INT8U   MCPCS;                                                      // Chip Select pin number
INT8U   mcpMode;                                                    // Mode to return to after configurations are performed.


//--------------------------------------------------------------------------------------------------------------------------------------------------------
/*********************************************************************************************************
** Function name:           mcp2515_reset
** Descriptions:            reset the device
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void mcp2515_reset(void)
{
    MCP2515_SELECT();
    spi_readwrite(MCP_RESET);
    MCP2515_UNSELECT();
    // SPI.endTransaction();
    DELAY_US(100000);
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegister
** Descriptions:            read register
** input parameters:        address :   register address
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_readRegister(const INT8U address)
{
	INT8U ret;
    MCP2515_SELECT();
    spi_readwrite(MCP_READ);
    spi_readwrite(address);
    ret = spi_read();
	MCP2515_UNSELECT();;
	return ret;
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegisterS
** Descriptions:            read registerS
** input parameters:        address :   register address
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
void mcp2515_readRegisterS(const INT8U address, INT8U values[], const INT8U n)
{
	INT8U i;
	MCP2515_SELECT();
    spi_readwrite(MCP_READ);
    spi_readwrite(address);
	for (i=0; i<n; i++) {
        values[i] = spi_read();
    }
	MCP2515_UNSELECT();;
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegister
** Descriptions:            set register
** input parameters:        address :   register address
                            value   :   register value
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void mcp2515_setRegister(const INT8U address, const INT8U value)
{
	MCP2515_SELECT();
    spi_readwrite(MCP_WRITE);
    spi_readwrite(address);
    spi_readwrite(value);
    MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegisterS
** Descriptions:            set registerS
** input parameters:        address : first register 's address
                            values[]: value
                            n       : data length
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void mcp2515_setRegisterS(const INT8U address, const INT8U values[], const INT8U n)
{
	INT8U i;
    MCP2515_SELECT();
    spi_readwrite(MCP_WRITE);
    spi_readwrite(address);  
    for (i=0; i<n; i++) {
        spi_readwrite(values[i]);
    }
    MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_modifyRegister
** Descriptions:            set bit of one register
** input parameters:        address : register address
                            mask    : bit address
                            data    : data
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
/* set bit of one register      */
void mcp2515_modifyRegister(const INT8U address, const INT8U mask, const INT8U data)
{
	MCP2515_SELECT();
    spi_readwrite(MCP_BITMOD);
    spi_readwrite(address);
    spi_readwrite(mask);
    spi_readwrite(data);
    MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_readStatus
** Descriptions:            read MCP2515's Status
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          the device's status
*********************************************************************************************************/
INT8U mcp2515_readStatus(void)
{
   INT8U i;
    MCP2515_SELECT();
    spi_readwrite(MCP_READ_STATUS);
    i = spi_read();
    MCP2515_UNSELECT();
    return i;
}
/*********************************************************************************************************
** Function name:           mcp2515_setCANCTRL_Mode
** Descriptions:            Set control mode
*********************************************************************************************************/
INT8U mcp2515_setCANCTRL_Mode(const INT8U newmode)
{
    INT8U i;
    mcp2515_modifyRegister(MCP_CANCTRL, MODE_MASK, newmode);
    i = mcp2515_readRegister(MCP_CANCTRL);
    i &= MODE_MASK;
    if ( i == newmode ) {
        return MCP2515_OK;
    }
    return MCP2515_FAIL;
}
/*********************************************************************************************************
** Function name:           mcp2515_setCANCTRL_Mode
** Descriptions:            set control mode
** input parameters:        newmode : mode
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_setMode(const INT8U opMode)
{
    mcpMode = opMode;
    return mcp2515_setCANCTRL_Mode(mcpMode);
}

/*********************************************************************************************************
** Function name:           mcp2515_configRate
** Descriptions:            set boadrate
** input parameters:        canSpeed    : boadrate
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_configRate(const INT8U canSpeed, const INT8U canClock)
{
    INT8U set, cfg1, cfg2, cfg3;
    set = 1;
    switch (canClock)
    {
        case (MCP_8MHZ):
        switch (canSpeed) 
        {
            case (CAN_5KBPS):                                               //   5KBPS                  
            cfg1 = MCP_8MHz_5kBPS_CFG1;
            cfg2 = MCP_8MHz_5kBPS_CFG2;
            cfg3 = MCP_8MHz_5kBPS_CFG3;
            break;

            case (CAN_10KBPS):                                              //  10KBPS                  
            cfg1 = MCP_8MHz_10kBPS_CFG1;
            cfg2 = MCP_8MHz_10kBPS_CFG2;
            cfg3 = MCP_8MHz_10kBPS_CFG3;
            break;

            case (CAN_20KBPS):                                              //  20KBPS                  
            cfg1 = MCP_8MHz_20kBPS_CFG1;
            cfg2 = MCP_8MHz_20kBPS_CFG2;
            cfg3 = MCP_8MHz_20kBPS_CFG3;
            break;

            case (CAN_31K25BPS):                                            //  31.25KBPS                  
            cfg1 = MCP_8MHz_31k25BPS_CFG1;
            cfg2 = MCP_8MHz_31k25BPS_CFG2;
            cfg3 = MCP_8MHz_31k25BPS_CFG3;
            break;

            case (CAN_33K3BPS):                                             //  33.33KBPS                  
            cfg1 = MCP_8MHz_33k3BPS_CFG1;
            cfg2 = MCP_8MHz_33k3BPS_CFG2;
            cfg3 = MCP_8MHz_33k3BPS_CFG3;
            break;

            case (CAN_40KBPS):                                              //  40Kbps
            cfg1 = MCP_8MHz_40kBPS_CFG1;
            cfg2 = MCP_8MHz_40kBPS_CFG2;
            cfg3 = MCP_8MHz_40kBPS_CFG3;
            break;

            case (CAN_50KBPS):                                              //  50Kbps
            cfg1 = MCP_8MHz_50kBPS_CFG1;
            cfg2 = MCP_8MHz_50kBPS_CFG2;
            cfg3 = MCP_8MHz_50kBPS_CFG3;
            break;

            case (CAN_80KBPS):                                              //  80Kbps
            cfg1 = MCP_8MHz_80kBPS_CFG1;
            cfg2 = MCP_8MHz_80kBPS_CFG2;
            cfg3 = MCP_8MHz_80kBPS_CFG3;
            break;

            case (CAN_100KBPS):                                             // 100Kbps
            cfg1 = MCP_8MHz_100kBPS_CFG1;
            cfg2 = MCP_8MHz_100kBPS_CFG2;
            cfg3 = MCP_8MHz_100kBPS_CFG3;
            break;

            case (CAN_125KBPS):                                             // 125Kbps
            cfg1 = MCP_8MHz_125kBPS_CFG1;
            cfg2 = MCP_8MHz_125kBPS_CFG2;
            cfg3 = MCP_8MHz_125kBPS_CFG3;
            break;

            case (CAN_200KBPS):                                             // 200Kbps
            cfg1 = MCP_8MHz_200kBPS_CFG1;
            cfg2 = MCP_8MHz_200kBPS_CFG2;
            cfg3 = MCP_8MHz_200kBPS_CFG3;
            break;

            case (CAN_250KBPS):                                             // 250Kbps
            cfg1 = MCP_8MHz_250kBPS_CFG1;
            cfg2 = MCP_8MHz_250kBPS_CFG2;
            cfg3 = MCP_8MHz_250kBPS_CFG3;
            break;

            case (CAN_500KBPS):                                             // 500Kbps
            cfg1 = MCP_8MHz_500kBPS_CFG1;
            cfg2 = MCP_8MHz_500kBPS_CFG2;
            cfg3 = MCP_8MHz_500kBPS_CFG3;
            break;
        
            case (CAN_1000KBPS):                                            //   1Mbps
            cfg1 = MCP_8MHz_1000kBPS_CFG1;
            cfg2 = MCP_8MHz_1000kBPS_CFG2;
            cfg3 = MCP_8MHz_1000kBPS_CFG3;
            break;  

            default:
            set = 0;
	    return MCP2515_FAIL;
        }
        break;

        case (MCP_16MHZ):
        switch (canSpeed) 
        {
            case (CAN_5KBPS):                                               //   5Kbps
            cfg1 = MCP_16MHz_5kBPS_CFG1;
            cfg2 = MCP_16MHz_5kBPS_CFG2;
            cfg3 = MCP_16MHz_5kBPS_CFG3;
            break;

            case (CAN_10KBPS):                                              //  10Kbps
            cfg1 = MCP_16MHz_10kBPS_CFG1;
            cfg2 = MCP_16MHz_10kBPS_CFG2;
            cfg3 = MCP_16MHz_10kBPS_CFG3;
            break;

            case (CAN_20KBPS):                                              //  20Kbps
            cfg1 = MCP_16MHz_20kBPS_CFG1;
            cfg2 = MCP_16MHz_20kBPS_CFG2;
            cfg3 = MCP_16MHz_20kBPS_CFG3;
            break;

            case (CAN_33K3BPS):                                              //  20Kbps
            cfg1 = MCP_16MHz_33k3BPS_CFG1;
            cfg2 = MCP_16MHz_33k3BPS_CFG2;
            cfg3 = MCP_16MHz_33k3BPS_CFG3;
            break;

            case (CAN_40KBPS):                                              //  40Kbps
            cfg1 = MCP_16MHz_40kBPS_CFG1;
            cfg2 = MCP_16MHz_40kBPS_CFG2;
            cfg3 = MCP_16MHz_40kBPS_CFG3;
            break;

            case (CAN_50KBPS):                                              //  50Kbps
            cfg2 = MCP_16MHz_50kBPS_CFG2;
            cfg3 = MCP_16MHz_50kBPS_CFG3;
            break;

            case (CAN_80KBPS):                                              //  80Kbps
            cfg1 = MCP_16MHz_80kBPS_CFG1;
            cfg2 = MCP_16MHz_80kBPS_CFG2;
            cfg3 = MCP_16MHz_80kBPS_CFG3;
            break;

            case (CAN_100KBPS):                                             // 100Kbps
            cfg1 = MCP_16MHz_100kBPS_CFG1;
            cfg2 = MCP_16MHz_100kBPS_CFG2;
            cfg3 = MCP_16MHz_100kBPS_CFG3;
            break;

            case (CAN_125KBPS):                                             // 125Kbps
            cfg1 = MCP_16MHz_125kBPS_CFG1;
            cfg2 = MCP_16MHz_125kBPS_CFG2;
            cfg3 = MCP_16MHz_125kBPS_CFG3;
            break;

            case (CAN_200KBPS):                                             // 200Kbps
            cfg1 = MCP_16MHz_200kBPS_CFG1;
            cfg2 = MCP_16MHz_200kBPS_CFG2;
            cfg3 = MCP_16MHz_200kBPS_CFG3;
            break;

            case (CAN_250KBPS):                                             // 250Kbps
            cfg1 = MCP_16MHz_250kBPS_CFG1;
            cfg2 = MCP_16MHz_250kBPS_CFG2;
            cfg3 = MCP_16MHz_250kBPS_CFG3;
            break;

            case (CAN_500KBPS):                                             // 500Kbps
            cfg1 = MCP_16MHz_500kBPS_CFG1;
            cfg2 = MCP_16MHz_500kBPS_CFG2;
            cfg3 = MCP_16MHz_500kBPS_CFG3;
            break;
        
            case (CAN_1000KBPS):                                            //   1Mbps
            cfg1 = MCP_16MHz_1000kBPS_CFG1;
            cfg2 = MCP_16MHz_1000kBPS_CFG2;
            cfg3 = MCP_16MHz_1000kBPS_CFG3;
            break;  

            default:
            set = 0;
	    return MCP2515_FAIL;
        }
        break;
        
        case (MCP_20MHZ):
        switch (canSpeed) 
        {
            case (CAN_40KBPS):                                              //  40Kbps
            cfg1 = MCP_20MHz_40kBPS_CFG1;
            cfg2 = MCP_20MHz_40kBPS_CFG2;
            cfg3 = MCP_20MHz_40kBPS_CFG3;
            break;

            case (CAN_50KBPS):                                              //  50Kbps
            cfg1 = MCP_20MHz_50kBPS_CFG1;
            cfg2 = MCP_20MHz_50kBPS_CFG2;
            cfg3 = MCP_20MHz_50kBPS_CFG3;
            break;

            case (CAN_80KBPS):                                              //  80Kbps
            cfg1 = MCP_20MHz_80kBPS_CFG1;
            cfg2 = MCP_20MHz_80kBPS_CFG2;
            cfg3 = MCP_20MHz_80kBPS_CFG3;
            break;

            case (CAN_100KBPS):                                             // 100Kbps
            cfg1 = MCP_20MHz_100kBPS_CFG1;
            cfg2 = MCP_20MHz_100kBPS_CFG2;
            cfg3 = MCP_20MHz_100kBPS_CFG3;
            break;

            case (CAN_125KBPS):                                             // 125Kbps
            cfg1 = MCP_20MHz_125kBPS_CFG1;
            cfg2 = MCP_20MHz_125kBPS_CFG2;
            cfg3 = MCP_20MHz_125kBPS_CFG3;
            break;

            case (CAN_200KBPS):                                             // 200Kbps
            cfg1 = MCP_20MHz_200kBPS_CFG1;
            cfg2 = MCP_20MHz_200kBPS_CFG2;
            cfg3 = MCP_20MHz_200kBPS_CFG3;
            break;

            case (CAN_250KBPS):                                             // 250Kbps
            cfg1 = MCP_20MHz_250kBPS_CFG1;
            cfg2 = MCP_20MHz_250kBPS_CFG2;
            cfg3 = MCP_20MHz_250kBPS_CFG3;
            break;

            case (CAN_500KBPS):                                             // 500Kbps
            cfg1 = MCP_20MHz_500kBPS_CFG1;
            cfg2 = MCP_20MHz_500kBPS_CFG2;
            cfg3 = MCP_20MHz_500kBPS_CFG3;
            break;
        
            case (CAN_1000KBPS):                                            //   1Mbps
            cfg1 = MCP_20MHz_1000kBPS_CFG1;
            cfg2 = MCP_20MHz_1000kBPS_CFG2;
            cfg3 = MCP_20MHz_1000kBPS_CFG3;
            break;  

            default:
            set = 0;
            return MCP2515_FAIL;
        }
        break;
        
        default:
        set = 0;
	return MCP2515_FAIL;
    }

    if (set) {
        mcp2515_setRegister(MCP_CNF1, cfg1);
        mcp2515_setRegister(MCP_CNF2, cfg2);
        mcp2515_setRegister(MCP_CNF3, cfg3);
        return MCP2515_OK;
    }
     
    return MCP2515_FAIL;
}

/*********************************************************************************************************
** Function name:           mcp2515_write_id
** Descriptions:            write can id
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void mcp2515_write_id(const INT8U mcp_addr, const INT8U ext, const INT32U id)
{
    uint16_t canid;
    INT8U tbufdata[4];

    canid = (uint16_t)(id & 0x0FFFF);

    if ( ext == 1) 
    {
        tbufdata[MCP_EID0] = (INT8U) (canid & 0xFF);
        tbufdata[MCP_EID8] = (INT8U) (canid >> 8);
        canid = (uint16_t)(id >> 16);
        tbufdata[MCP_SIDL] = (INT8U) (canid & 0x03);
        tbufdata[MCP_SIDL] += (INT8U) ((canid & 0x1C) << 3);
        tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        tbufdata[MCP_SIDH] = (INT8U) (canid >> 5 );
    }
    else 
    {
        tbufdata[MCP_SIDH] = (INT8U) (canid >> 3 );
        tbufdata[MCP_SIDL] = (INT8U) ((canid & 0x07 ) << 5);
        tbufdata[MCP_EID0] = 0;
        tbufdata[MCP_EID8] = 0;
    }
    
    mcp2515_setRegisterS( mcp_addr, tbufdata, 4 );
}
/*********************************************************************************************************
** Function name:           mcp2515_write_mf
** Descriptions:            Write Masks and Filters
*********************************************************************************************************/
void mcp2515_write_mf(const INT8U mcp_addr, const INT8U ext, const INT32U id)
{
    uint16_t canid;
    INT8U tbufdata[4];

    canid = (uint16_t)(id & 0x0FFFF);

    if ( ext == 1) 
    {
        tbufdata[MCP_EID0] = (INT8U) (canid & 0xFF);
        tbufdata[MCP_EID8] = (INT8U) (canid >> 8);
        canid = (uint16_t)(id >> 16);
        tbufdata[MCP_SIDL] = (INT8U) (canid & 0x03);
        tbufdata[MCP_SIDL] += (INT8U) ((canid & 0x1C) << 3);
        tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        tbufdata[MCP_SIDH] = (INT8U) (canid >> 5 );
    }
    else 
    {
        tbufdata[MCP_EID0] = (INT8U) (canid & 0xFF);
        tbufdata[MCP_EID8] = (INT8U) (canid >> 8);
        canid = (uint16_t)(id >> 16);
        tbufdata[MCP_SIDL] = (INT8U) ((canid & 0x07) << 5);
        tbufdata[MCP_SIDH] = (INT8U) (canid >> 3 );
    }
    
    mcp2515_setRegisterS( mcp_addr, tbufdata, 4 );
}

/*********************************************************************************************************
** Function name:           mcp2515_initCANBuffers
** Descriptions:            init canbuffers
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
void mcp2515_initCANBuffers(void)
{
    INT8U i, a1, a2, a3;
    
    INT8U std = 0;               
    INT8U ext = 1;
    INT32U ulMask = 0x00, ulFilt = 0x00;

    mcp2515_write_mf(MCP_RXM0SIDH, ext, ulMask);			/*Set both masks to 0           */
    mcp2515_write_mf(MCP_RXM1SIDH, ext, ulMask);			/*Mask register ignores ext bit */
    
                                                                        /* Set all filters to 0         */
    mcp2515_write_mf(MCP_RXF0SIDH, ext, ulFilt);			/* RXB0: extended               */
    mcp2515_write_mf(MCP_RXF1SIDH, std, ulFilt);			/* RXB1: standard               */
    mcp2515_write_mf(MCP_RXF2SIDH, ext, ulFilt);			/* RXB2: extended               */
    mcp2515_write_mf(MCP_RXF3SIDH, std, ulFilt);			/* RXB3: standard               */
    mcp2515_write_mf(MCP_RXF4SIDH, ext, ulFilt);
    mcp2515_write_mf(MCP_RXF5SIDH, std, ulFilt);

                                                                        /* Clear, deactivate the three  */
                                                                        /* transmit buffers             */
                                                                        /* TXBnCTRL -> TXBnD7           */
    a1 = MCP_TXB0CTRL;
    a2 = MCP_TXB1CTRL;
    a3 = MCP_TXB2CTRL;
    for (i = 0; i < 14; i++) {                                          /* in-buffer loop               */
        mcp2515_setRegister(a1, 0);
        mcp2515_setRegister(a2, 0);
        mcp2515_setRegister(a3, 0);
        a1++;
        a2++;
        a3++;
    }
    mcp2515_setRegister(MCP_RXB0CTRL, 0);
    mcp2515_setRegister(MCP_RXB1CTRL, 0);
}

/*********************************************************************************************************
** Function name:           mcp2515_read_id
** Descriptions:            read can id
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
void mcp2515_read_id(const INT8U mcp_addr, INT8U* ext, INT32U* id)
{
    INT8U tbufdata[4];

    *ext = 0;
    *id = 0;

    mcp2515_readRegisterS( mcp_addr, tbufdata, 4 );

    *id = (tbufdata[MCP_SIDH]<<3) + (tbufdata[MCP_SIDL]>>5);

    if ( (tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M ) 
    {
                                                                        /* extended id                  */
        *id = (*id<<2) + (tbufdata[MCP_SIDL] & 0x03);
        *id = (*id<<8) + tbufdata[MCP_EID8];
        *id = (*id<<8) + tbufdata[MCP_EID0];
        *ext = 1;
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_write_canMsg
** Descriptions:            write msg
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void mcp2515_write_canMsg(const INT8U buffer_sidh_addr)
{
    INT8U mcp_addr;
    mcp_addr = buffer_sidh_addr;
    mcp2515_setRegisterS(mcp_addr+5, m_nDta, m_nDlc );                  /* write data unsigned chars             */
	
    if ( m_nRtr == 1)                                                   /* if RTR set bit in unsigned char       */
        m_nDlc |= MCP_RTR_MASK;  

    mcp2515_setRegister((mcp_addr+4), m_nDlc );                         /* write the RTR and DLC        */
    mcp2515_write_id(mcp_addr, m_nExtFlg, m_nID );                /* write CAN id                 */
}

/*********************************************************************************************************
** Function name:           mcp2515_read_canMsg
** Descriptions:            read message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
void mcp2515_read_canMsg(const INT8U buffer_sidh_addr)        /* read can msg                 */
{
    INT8U mcp_addr, ctrl;

    mcp_addr = buffer_sidh_addr;

    mcp2515_read_id( mcp_addr, &m_nExtFlg,&m_nID );

    ctrl = mcp2515_readRegister( mcp_addr-1 );
    m_nDlc = mcp2515_readRegister( mcp_addr+4 );

    if (ctrl & 0x08)
        m_nRtr = 1;
    else
        m_nRtr = 0;

    m_nDlc &= MCP_DLC_MASK;
    mcp2515_readRegisterS( mcp_addr+5, &(m_nDta[0]), m_nDlc );
}

/*********************************************************************************************************
** Function name:           mcp2515_sendMsg
** Descriptions:            send message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_getNextFreeTXBuf(INT8U *txbuf_n)                 /* get Next free txbuf          */
{
    INT8U res, i, ctrlval;
    INT8U ctrlregs[MCP_N_TXBUFFERS] = { MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL };

    res = MCP_ALLTXBUSY;
    *txbuf_n = 0x00;

                                                                        /* check all 3 TX-Buffers       */
    for (i=0; i<MCP_N_TXBUFFERS; i++) {
        ctrlval = mcp2515_readRegister( ctrlregs[i] );
        if ( (ctrlval & MCP_TXB_TXREQ_M) == 0 ) {
            *txbuf_n = ctrlregs[i]+1;                                   /* return SIDH-address of Buffer*/
            
            res = MCP2515_OK;
            return res;                                                 /* ! function exit              */
        }
    }
    return res;
}

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            init the device
** input parameters:        canSpeed    : boadrate
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_init(const INT8U canIDMode, const INT8U canSpeed, const INT8U canClock)                       /* MCP2515init                  */
{

	INT8U res;

    mcp2515_reset();
    
    mcpMode = MCP_LOOPBACK;

    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if(res > 0)
    {
      return res;
    }

    // Set Baudrate
    if(mcp2515_configRate(canSpeed, canClock))
    {
      return res;
    }

    if ( res == MCP2515_OK ) {

                                                                        /* init canbuffers              */
        mcp2515_initCANBuffers();

                                                                        /* interrupt mode               */
        mcp2515_setRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

        //Sets BF pins as GPO
        mcp2515_setRegister(MCP_BFPCTRL,MCP_BxBFS_MASK | MCP_BxBFE_MASK);
//        mcp2515_setRegister(MCP_BFPCTRL, MCP_BxBFE_MASK | MCP_BxBFM_MASK);      //changed to activate the RX buffer full registers
        //Sets RTS pins as GPI
        mcp2515_setRegister(MCP_TXRTSCTRL, 0x00);

        switch(canIDMode)
        {
            case (MCP_ANY):
            mcp2515_modifyRegister(MCP_RXB0CTRL,
            MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
            MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
            mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
            MCP_RXB_RX_ANY);
            break;
/*          The followingn two functions of the MCP2515 do not work, there is a bug in the silicon.
            case (MCP_STD): 
            mcp2515_modifyRegister(MCP_RXB0CTRL,
            MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
            MCP_RXB_RX_STD | MCP_RXB_BUKT_MASK );
            mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
            MCP_RXB_RX_STD);
            break;

            case (MCP_EXT): 
            mcp2515_modifyRegister(MCP_RXB0CTRL,
            MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
            MCP_RXB_RX_EXT | MCP_RXB_BUKT_MASK );
            mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
            MCP_RXB_RX_EXT);
            break;
*/
            case (MCP_STDEXT): 
            mcp2515_modifyRegister(MCP_RXB0CTRL,
            MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
            MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK );
            mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
            MCP_RXB_RX_STDEXT);
            break;
    
            default:
   
            return MCP2515_FAIL;
}    


        res = mcp2515_setCANCTRL_Mode(mcpMode);                                                                
        if(res)
        {     
          return res;
        }

    }
    return res;

}
/*********************************************************************************************************
** Function name:           MCP_CAN
** Descriptions:            Public function to declare CAN class and the /CS pin.
*********************************************************************************************************/
void mcp2515_MCP_CAN(INT8U _CS)
{
    MCPCS = _CS;
    // MCP2515_UNSELECT();
  //  pinMode(MCPCS, OUTPUT);
}

/*********************************************************************************************************
** Function name:           init
** Descriptions:            init can and set speed
** input parameters:        speedset : can boadrate
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_begin(INT8U idmodeset, INT8U speedset, INT8U clockset)
{
    INT8U res;

    InitSpibGpio();
    spib_initialize();
    spib_set_datamode(1);
    res = mcp2515_init(idmodeset, speedset, clockset);
    if (res == MCP2515_OK)
        return CAN_OK;
    
    return CAN_FAILINIT;
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            init canid Masks
** input parameters:        num :   Mask number
                            ext :   if ext id
                            ulData: data
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_init_Mask(INT8U num, INT8U ext, INT32U ulData)
{
    INT8U res = MCP2515_OK;

    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if(res > 0){
	    return res;
    }
    
    if (num == 0){
        mcp2515_write_mf(MCP_RXM0SIDH, ext, ulData);

    }
    else if(num == 1){
        mcp2515_write_mf(MCP_RXM1SIDH, ext, ulData);
    }
    else res =  MCP2515_FAIL;
    
    res = mcp2515_setCANCTRL_Mode(mcpMode);
    if(res > 0){
	    return res;
    }
    return res;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            init canid filters
** input parameters:        num  :  Filters number
                            ext  :  if ext id
                            ulData: data
** Output parameters:       NONE
** Returned value:          ifsuccess
*********************************************************************************************************/
//INT8U mcp2515_init_Filt(INT8U num, INT32U ulData)
//{
//    INT8U res = MCP2515_OK;
//    INT8U ext = 0;
//
//    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
//    if(res > 0)
//    {
//      return res;
//    }
//
//    if((num & 0x80000000) == 0x80000000)
//        ext = 1;
//
//    switch( num )
//    {
//        case 0:
//        mcp2515_write_mf(MCP_RXF0SIDH, ext, ulData);
//        break;
//
//        case 1:
//        mcp2515_write_mf(MCP_RXF1SIDH, ext, ulData);
//        break;
//
//        case 2:
//        mcp2515_write_mf(MCP_RXF2SIDH, ext, ulData);
//        break;
//
//        case 3:
//        mcp2515_write_mf(MCP_RXF3SIDH, ext, ulData);
//        break;
//
//        case 4:
//        mcp2515_write_mf(MCP_RXF4SIDH, ext, ulData);
//        break;
//
//        case 5:
//        mcp2515_write_mf(MCP_RXF5SIDH, ext, ulData);
//        break;
//
//        default:
//        res = MCP2515_FAIL;
//    }
//
//    res = mcp2515_setCANCTRL_Mode(mcpMode);
//    if(res > 0)
//    {
//      return res;
//    }
//
//    return res;
//}

INT8U mcp2515_init_Filt(INT8U num, INT8U ext, INT32U ulData)
{
    INT8U res = MCP2515_OK;

    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if(res > 0)
    {
      return res;
    }
    
    switch( num )
    {
        case 0:
        mcp2515_write_mf(MCP_RXF0SIDH, ext, ulData);
        break;

        case 1:
        mcp2515_write_mf(MCP_RXF1SIDH, ext, ulData);
        break;

        case 2:
        mcp2515_write_mf(MCP_RXF2SIDH, ext, ulData);
        break;

        case 3:
        mcp2515_write_mf(MCP_RXF3SIDH, ext, ulData);
        break;

        case 4:
        mcp2515_write_mf(MCP_RXF4SIDH, ext, ulData);
        break;

        case 5:
        mcp2515_write_mf(MCP_RXF5SIDH, ext, ulData);
        break;

        default:
        res = MCP2515_FAIL;
    }
    
    res = mcp2515_setCANCTRL_Mode(mcpMode);
    if(res > 0)
    {
      return res;
    }
    
    return res;
}
/*********************************************************************************************************
** Function name:           mcp2515_setMsg
** Descriptions:            set can message, such as dlc, id, dta[] and so on
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_setMsg(INT32U id, INT8U rtr, INT8U ext, INT8U len, INT8U *pData)
{
    int i = 0;
    m_nID     = id;
    m_nRtr    = rtr;
    m_nExtFlg = ext;
    m_nDlc    = len;
    for(i = 0; i<MAX_CHAR_IN_MESSAGE; i++)
        m_nDta[i] = *(pData+i);
    return MCP2515_OK;
}

/*********************************************************************************************************
** Function name:           clearMsg
** Descriptions:            set all message to zero
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_clearMsg()
{
    int i;
    m_nID       = 0;
    m_nDlc      = 0;
    m_nExtFlg   = 0;
    m_nRtr      = 0;
    m_nfilhit   = 0;
    for(i = 0; i<m_nDlc; i++){
      m_nDta[i] = 0x00;
    }
    return MCP2515_OK;
}

/*********************************************************************************************************
** Function name:           mcp2515_sendMsg
** Descriptions:            send message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_sendMsg()
{
    INT8U res, res1, txbuf_n;
    uint16_t uiTimeOut = 0;

    do {
        res = mcp2515_getNextFreeTXBuf(&txbuf_n);                       /* info = addr.                 */
        uiTimeOut++;
    } while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));

    if(uiTimeOut == TIMEOUTVALUE) 
    {   
        return CAN_GETTXBFTIMEOUT;                                      /* get tx buff time out         */
    }
    uiTimeOut = 0;
    mcp2515_write_canMsg( txbuf_n);
    mcp2515_modifyRegister( txbuf_n-1 , MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M );
    
    do
    {
        uiTimeOut++;        
        res1 = mcp2515_readRegister(txbuf_n-1);                         /* read send buff ctrl reg 	*/
        res1 = res1 & 0x08;                               		
    } while (res1 && (uiTimeOut < TIMEOUTVALUE));   
    
    if(uiTimeOut == TIMEOUTVALUE)                                       /* send msg timeout             */	
        return CAN_SENDMSGTIMEOUT;
    
    return CAN_OK;
}

/*********************************************************************************************************
** Function name:           mcp2515_sendMsgBuf
** Descriptions:            send buf
** input parameters:        id  :   can id
                            ext :   if ext
                            len :   buf length
                            *buf:   data buf
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_sendMsgBuf_ext(INT32U id, INT8U ext, INT8U len, INT8U *buf)
{
	INT8U res;

    mcp2515_setMsg(id, 0, ext, len, buf);
    res = mcp2515_sendMsg();

    return res;
}

INT8U mcp2515_sendMsgBuf(INT32U id, INT8U len, INT8U *buf)
{
    INT8U ext = 0, rtr = 0;
    INT8U res;

    if((id & 0x80000000) == 0x80000000)
        ext = 1;

    if((id & 0x40000000) == 0x40000000)
        rtr = 1;

    mcp2515_setMsg(id, rtr, ext, len, buf);
    res = mcp2515_sendMsg();

    return res;
}

/*********************************************************************************************************
** Function name:           mcp2515_readMsg
** Descriptions:            read message
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_readMsg()
{
    INT8U stat, res;

    stat = mcp2515_readStatus();

    if ( stat & MCP_STAT_RX0IF )                                        /* Msg in Buffer 0              */
    {
        mcp2515_read_canMsg( MCP_RXBUF_0);
        mcp2515_modifyRegister(MCP_CANINTF, MCP_RX0IF, 0);
        res = CAN_OK;
    }
    else if ( stat & MCP_STAT_RX1IF )                                   /* Msg in Buffer 1              */
    {
        mcp2515_read_canMsg( MCP_RXBUF_1);
        mcp2515_modifyRegister(MCP_CANINTF, MCP_RX1IF, 0);
        res = CAN_OK;
    }
    else 
        res = CAN_NOMSG;
    
    return res;
}

/*********************************************************************************************************
** Function name:           mcp2515_readMsgBuf
** Descriptions:            read message buf
** input parameters:        len:    data length
** Output parameters:       buf:    data buf
** Returned value:          if success
*********************************************************************************************************/
INT8U mcp2515_readMsgBuf_ext(INT32U *id, INT8U *ext, INT8U *len, INT8U buf[])
{
    int i;
    if(mcp2515_readMsg() == CAN_NOMSG)
	return CAN_NOMSG;

    *id  = m_nID;
    *len = m_nDlc;
    *ext = m_nExtFlg;
    for(i = 0; i<m_nDlc; i++){
        buf[i] = m_nDta[i];
    }
    return CAN_OK;
}

INT8U mcp2515_readMsgBuf(INT32U *id, INT8U *len, INT8U buf[])
{
    int i;
    if(mcp2515_readMsg() == CAN_NOMSG)
	return CAN_NOMSG;

    if (m_nExtFlg)
        m_nID |= 0x80000000;

    if (m_nRtr)
        m_nID |= 0x40000000;

    *id  = m_nID;
    *len = m_nDlc;

    for(i = 0; i<m_nDlc; i++){
        buf[i] = m_nDta[i];
    }
    return CAN_OK;
}

/*********************************************************************************************************
** Function name:           checkReceive
** Descriptions:            check if got something
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          if something
*********************************************************************************************************/
INT8U mcp2515_checkReceive(void)
{
   INT8U res;
    res = mcp2515_readStatus();                                         /* RXnIF in Bit 1 and 0         */
    if ( res & MCP_STAT_RXIF_MASK )
        return CAN_MSGAVAIL;
    else 
        return CAN_NOMSG;
}

/*********************************************************************************************************
** Function name:           checkError
** Descriptions:            if something error
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
INT8U mcp2515_checkError(void)
{
    INT8U eflg = mcp2515_readRegister(MCP_EFLG);

    if ( eflg & MCP_EFLG_ERRORMASK ) 
        return CAN_CTRLERROR;
    else
        return CAN_OK;
}
/*********************************************************************************************************
** Function name:           getError
** Descriptions:            Returns error register value.
*********************************************************************************************************/
INT8U mcp2515_getError(void)
{
    return mcp2515_readRegister(MCP_EFLG);
}

/*********************************************************************************************************
** Function name:           mcp2515_getCanId
** Descriptions:            when receive something ,u can get the can id!!
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
uint32_t mcp2515_getCanId(/*mcp2515_Handle handle*/)
{
  return m_nID;
}

INT8U mcp2515_errorCountRX(void)                             
{
    return mcp2515_readRegister(MCP_REC);
}

/*********************************************************************************************************
** Function name:           mcp2515_errorCountTX
** Descriptions:            Returns TEC register value
*********************************************************************************************************/
INT8U mcp2515_errorCountTX(void)                             
{
    return mcp2515_readRegister(MCP_TEC);
}

/*********************************************************************************************************
** Function name:           mcp2515_enOneShotTX
** Descriptions:            Enables one shot transmission mode
*********************************************************************************************************/
INT8U mcp2515_enOneShotTX(void)                             
{
    mcp2515_modifyRegister(MCP_CANCTRL, MODE_ONESHOT, MODE_ONESHOT);
    if((mcp2515_readRegister(MCP_CANCTRL) & MODE_ONESHOT) != MODE_ONESHOT)
	    return CAN_FAIL;
    else
	    return CAN_OK;
}

/*********************************************************************************************************
** Function name:           mcp2515_disOneShotTX
** Descriptions:            Disables one shot transmission mode
*********************************************************************************************************/
INT8U mcp2515_disOneShotTX(void)                             
{
    mcp2515_modifyRegister(MCP_CANCTRL, MODE_ONESHOT, 0);
    if((mcp2515_readRegister(MCP_CANCTRL) & MODE_ONESHOT) != 0)
        return CAN_FAIL;
    else
        return CAN_OK;
}

/*********************************************************************************************************
** Function name:           mcp2515_abortTX
** Descriptions:            Aborts any queued transmissions
*********************************************************************************************************/
INT8U mcp2515_abortTX(void)                             
{
    mcp2515_modifyRegister(MCP_CANCTRL, ABORT_TX, ABORT_TX);
	
    // Maybe check to see if the TX buffer transmission request bits are cleared instead?
    if((mcp2515_readRegister(MCP_CANCTRL) & ABORT_TX) != ABORT_TX)
	    return CAN_FAIL;
    else
	    return CAN_OK;
}

/*********************************************************************************************************
** Function name:           setGPO
** Descriptions:            Public function, Checks for r
*********************************************************************************************************/
// INT8U mcp2515_setGPO(INT8U data)
// {
//     mcp2515_modifyRegister(MCP_BFPCTRL, MCP_BxBFS_MASK, (data<<4));
	    
//     return 0;
// }

/*********************************************************************************************************
** Function name:           getGPI
** Descriptions:            Public function, Checks for r
*********************************************************************************************************/
// INT8U mcp2515_getGPI(void)
// {
//     INT8U res;
//     res = mcp2515_readRegister(MCP_TXRTSCTRL) & MCP_BxRTS_MASK;
//     return (res >> 3);
// }


/*********************************************************************************************************
** Function name:           mcp2515_setCanId
** Descriptions:
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
//void mcp2515_setCanId(/*mcp2515_Handle handle,*/ uint32_t id)
//{
//  m_nID = id;
//}

/*********************************************************************************************************
** Function name:           setRcvFlag
** Descriptions:            sets canRcvFlag to the given state (use to set to false after reading the buffer)
** input parameters:        true/false
** Output parameters:       NONE
** Returned value:          NONE
*********************************************************************************************************/
// void setRcvFlag(mcp2515_Handle handle, bool state){
// 	mcp2515_Obj *obj = (mcp2515_Obj *)handle;
// 	obj->canRcvFlag = state;
// }

/*********************************************************************************************************
** Function name:           getRcvFlag
** Descriptions:            returns canRcvFlag state
** input parameters:        NONE
** Output parameters:       NONE
** Returned value:          true/false
*********************************************************************************************************/
// bool getRcvFlag(mcp2515_Handle handle){
// 	mcp2515_Obj *obj = (mcp2515_Obj *)handle;
// 	return obj->canRcvFlag;
// }
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
