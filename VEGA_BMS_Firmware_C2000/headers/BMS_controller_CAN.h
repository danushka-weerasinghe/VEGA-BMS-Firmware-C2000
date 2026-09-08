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
    CAN_OBJ_0,
    CAN_OBJ_1,
    CAN_OBJ_2,
    CAN_OBJ_3, //ota_flag
    CAN_OBJ_4, //ota_response
    CAN_OBJ_5,
    CAN_OBJ_6,
    CAN_OBJ_7,
    CAN_OBJ_8,
    CAN_OBJ_9,
    CAN_OBJ_10,
    CAN_OBJ_11, //evcc
    CAN_OBJ_12, //bms_cmd
    CAN_OBJ_13,
    CAN_OBJ_14, //bms_debug_69
    CAN_OBJ_15, //bms_can_data
    CAN_OBJ_16, //evcu
    CAN_OBJ_17, //get_slave_rx
    CAN_OBJ_18, //set_slave_tx
    CAN_OBJ_19, //chiller_rx
    CAN_OBJ_20, //chiller_tx
    CAN_OBJ_21,
    CAN_OBJ_22,
    CAN_OBJ_23,
    CAN_OBJ_24,
    CAN_OBJ_25,
    CAN_OBJ_26,
    CAN_OBJ_27,
    CAN_OBJ_28,
    CAN_OBJ_29,
    CAN_OBJ_30,
    CAN_OBJ_31
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
