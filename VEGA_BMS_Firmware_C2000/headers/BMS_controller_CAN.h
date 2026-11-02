/*
 * BMS_controller_CAN.h
 *
 *  Created on: Apr 17, 2025
 *      Author: ashans
 */

#ifndef HEADERS_BMS_CONTROLLER_CAN_H_
#define HEADERS_BMS_CONTROLLER_CAN_H_

#include <stdbool.h>
#include "DSP28x_Project.h"
//#include "main_data.h"
//#include "string.h"

#define NOMINAL_DLC 8
#define NUM_CAN_OBJECTS 32

#ifndef NULL
#define NULL 0
#endif

extern struct ECAN_REGS ECanaShadow;

//CAN Mailbox Object ID
typedef enum
{
    CAN_OBJ_0,      // unused
    CAN_OBJ_1,      // unused
    CAN_OBJ_2,      // unused
    CAN_OBJ_3,      // RX  OTA_FLAG_ADDR          0x17B84 (ext) - OTA handshake / reset command
    CAN_OBJ_4,      // TX  OTA_RESPONSE_ADDR      0x67B84 (ext) - OTA acknowledge
    CAN_OBJ_5,      // unused
    CAN_OBJ_6,      // unused
    CAN_OBJ_7,      // unused
    CAN_OBJ_8,      // unused
    CAN_OBJ_9,      // TX  EEPROM_BROADCAST_ADDR  0x35  - EEPROM config broadcast, pages 1..5
    CAN_OBJ_10,     // unused
    CAN_OBJ_11,     // RX  EVCC_ADDR              0x62  - EVCC flags (charger type, FC contactor, GB/T setup request)
    CAN_OBJ_12,     // RX  BMS_CMD_ADDR           0xDB  - debug / test commands (debug_CMD, SOC reset, chiller test ...)
    CAN_OBJ_13,     // unused
    CAN_OBJ_14,     // TX  BMS_DEBUG_ADDR         0x69  - debug data (debug_CMD set) / charged energy
    CAN_OBJ_15,     // TX  BMS_CAN_DATA_ADDR      0x32  - BMS status to EVCU, pages 0..3
    CAN_OBJ_16,     // RX  EVCU_ADDR              0x31  - EVCU commands (state, contactor on/off)
    CAN_OBJ_17,     // RX  GET_SLAVE_ADDR         0x30  - slave BMS data to master
    CAN_OBJ_18,     // TX  SET_SLAVE_ADDR         0x29  - master commands to slave BMS
    CAN_OBJ_19,     // RX  CHILLER_RX_ADDR        0x248 - chiller feedback
    CAN_OBJ_20,     // TX  CHILLER_TX_ADDR        0x238 - chiller command
    CAN_OBJ_21,     // RX  EVCU_ODO_ADDR          0x41  - EVCU odometer value
    CAN_OBJ_22,     // TX  GBT_PROTOCOL_DATA      0x38  - GB/T charging data to EVCC (GBT_CHARGING == 1)
    CAN_OBJ_23,     // unused
    CAN_OBJ_24,     // TX  PUMP_TX_ADDR           0x258 - pump command, duty + on/off (CAN_PUMP)
    CAN_OBJ_25,     // RX  PUMP_RX_ADDR           0x268 - pump feedback, speed/temp/V/I/fault (CAN_PUMP)
    CAN_OBJ_26,     // RX  SET_CHG_ENERGY         0xEC  - reset charged energy
    CAN_OBJ_27,     // RX  BMS_CONFIG_UPDATE_ADDR 0xED  - EEPROM parameter update (capacity, limits, pack no., date)
    CAN_OBJ_28,     // unused
    CAN_OBJ_29,     // unused
    CAN_OBJ_30,     // unused
    CAN_OBJ_31      // unused
}Mbox_obj_id_val;

//Mailbox direction configuration
typedef enum
{
    TX_DIR,
    RX_DIR
}CAN_msg_dir;

//CAN Address for each mailbox
//typedef enum
//{
//    OTA_FLAG_ADDR = 0x17B84,
//    OTA_RESPONSE_ADDR = 0x67B84,
//    EVCC_ADDR = 0x62,
//    BMS_CMD_ADDR = 0xDB,
//    BMS_DEBUG_ADDR = 0x69,
//    BMS_CAN_DATA_ADDR = 0x32,
//    EVCU_ADDR = 0x31,
//    GET_SLAVE_ADDR = 0x30,
//    SET_SLAVE_ADDR = 0x29,
//    CHILLER_RX_ADDR = 0x248,
//    CHILLER_TX_ADDR = 0x238
//}CAN_msg_addr;

//CAN message Frame type
typedef enum
{
    STD_FRAME,
    EXT_FRAME
}CAN_msg_frame_type;

typedef struct {
    Uint16 readData[NOMINAL_DLC];  // 8-byte buffer for each CAN message
    Uint16 sendData[NOMINAL_DLC];
} CAN_MsgBuffer;

typedef struct {
    CAN_MsgBuffer msg[NUM_CAN_OBJECTS];  // Array of message buffers
} CAN_BufferMap;

// Define CAN message structure
typedef struct CAN_Message_t{
    Mbox_obj_id_val Mbox_Obj_id;  // CAN mailbox
    Uint16 Rx_data[8]; // Receive Data array
    Uint16 Tx_data[8]; // Transmit Data array
    struct CAN_Message_t* CAN_msg_buffer;
} CAN_Message_t;

extern CAN_Message_t* Msg_head; //global head pointer
extern CAN_BufferMap canMap;
//struct ECAN_REGS ECanaShadow;

void AddToBuffer(Mbox_obj_id_val Obj_ID, Uint16* Rx_data); //insert to the linked list node
CAN_Message_t *GetFromBuffer(Mbox_obj_id_val Obj_ID);
void removeBuffer(Mbox_obj_id_val Obj_ID);//delete a node from the linked list
extern void CAN_readMessage(Mbox_obj_id_val Obj_ID, Uint16* msgData); //read data to the linked list buffer from the mailbox regs
extern void read_CANData(Mbox_obj_id_val Obj_ID, Uint16* CANData);
void CAN_sendMessage(Mbox_obj_id_val objID, Uint16 msgLen, Uint16 *msgData);
void CAN_transmission_acknowledge(Mbox_obj_id_val objID);

// CAN Driver Function Prototypes
extern void CAN_Init();
extern void CAN_enableGlobalInterrupt();
extern Uint8 CAN_getInterruptCause(); //get the RMP bit of the mailbox
extern void CLEAR_INTERUPT_FLAGS();
//extern void ECAN_CONFIG();
void CAN_setupMessageObject(Mbox_obj_id_val st_obj_id, Uint32 set_msg_id, Uint8 set_dlc,
                            CAN_msg_dir set_msg_dir, CAN_msg_frame_type set_msg_frame);
//void CAN_Transmit(Uint32 base, Uint32 mailbox, const CAN_Message_t *msg);
//bool CAN_Receive(Uint32 base, Uint32 mailbox, CAN_Message_t *msg);
//bool CAN_MessageAvailable(Uint32 base, Uint32 mailbox);

#endif /* HEADERS_BMS_CONTROLLER_CAN_H_ */
