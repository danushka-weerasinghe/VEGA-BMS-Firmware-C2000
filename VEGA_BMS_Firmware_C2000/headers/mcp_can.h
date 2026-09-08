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
** Descriptions:               mcp2515 main .h file
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
#ifndef _MCP2515_H_
#define _MCP2515_H_

#include "mcp_can_dfs.h"

//void mcp2515_MCP_CAN(INT8U _CS);
void mcp2515_reset(void);
INT8U mcp2515_readRegister(const INT8U address);
void mcp2515_readRegisterS(const INT8U address, INT8U values[], const INT8U n);
void mcp2515_setRegister(const INT8U address, const INT8U value);
void mcp2515_setRegisterS(const INT8U address, const INT8U values[], const INT8U n);
void mcp2515_modifyRegister(const INT8U address, const INT8U mask, const INT8U data);
INT8U mcp2515_readStatus(void);
INT8U mcp2515_setMode(const INT8U opMode);
INT8U mcp2515_setCANCTRL_Mode(const INT8U newmode);
INT8U mcp2515_configRate(const INT8U canSpeed, const INT8U canClock);
void mcp2515_write_mf(const INT8U mcp_addr, const INT8U ext, const INT32U id);
void mcp2515_initCANBuffers(void);
void mcp2515_read_id(const INT8U mcp_addr, INT8U* ext, INT32U* id);
void mcp2515_write_canMsg(const INT8U buffer_sidh_addr);
void mcp2515_read_canMsg(const INT8U buffer_sidh_addr);
INT8U mcp2515_getNextFreeTXBuf(INT8U *txbuf_n);
INT8U mcp2515_init(const INT8U canIDMode, const INT8U canSpeed, const INT8U canClock);
void mcp2515_MCP_CAN(INT8U _CS);
INT8U mcp2515_begin(INT8U idmodeset, INT8U speedset, INT8U clockset);
INT8U mcp2515_init_Mask(INT8U num, INT8U ext, INT32U ulData);
INT8U mcp2515_init_Filt(INT8U num, INT8U ext, INT32U ulData);
//INT8U mcp2515_init_Filt(INT8U num, INT32U ulData);
INT8U mcp2515_setMsg(INT32U id, INT8U rtr, INT8U ext, INT8U len, INT8U *pData);
INT8U mcp2515_clearMsg();
INT8U mcp2515_sendMsg();
INT8U mcp2515_sendMsgBuf_ext(INT32U id, INT8U ext, INT8U len, INT8U *buf);
INT8U mcp2515_sendMsgBuf(INT32U id, INT8U len, INT8U *buf);
INT8U mcp2515_readMsg();
INT8U mcp2515_readMsgBuf_ext(INT32U *id, INT8U *ext, INT8U *len, INT8U buf[]);
INT8U mcp2515_readMsgBuf(INT32U *id, INT8U *len, INT8U buf[]);
INT8U mcp2515_checkReceive(void);
INT8U mcp2515_checkError(void);
INT8U mcp2515_getError(void);
uint32_t MCP2515_getCanId(/*MCP2515_Handle handle*/);
//void MCP2515_setCanId(/*MCP2515_Handle handle,*/ uint32_t id);
INT8U mcp2515_errorCountRX(void);
INT8U mcp2515_errorCountTX(void);
INT8U enOneShotTX(void);                                            // Enable one-shot transmission
INT8U disOneShotTX(void);                                           // Disable one-shot transmission
INT8U abortTX(void);                                                // Abort queued transmission(s)
INT8U setGPO(INT8U data);                                           // Sets GPO
INT8U getGPI(void); 
// void setRcvFlag(MCP2515_Handle handle, bool state);
// bool getRcvFlag(MCP2515_Handle handle);

#ifdef __cplusplus
}
#endif

#endif
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
