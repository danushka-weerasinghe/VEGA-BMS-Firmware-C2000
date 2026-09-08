///*
// * bootloader.h
// *
// *  Created on: May 28, 2025
// *      Author: ashans
// */
//
//#ifndef HEADERS_BOOTLOADER_H_
//#define HEADERS_BOOTLOADER_H_
//
//#include "DSP28x_Project.h"
//#include "Flash2806x_API_Library.h"
//
////MEMORY ADDRESSES
//#define FLASH_FLAG_ADDRESS 0x3E4000
//#define JUMP_FLAG_ADDRESS 0x3E4001
//#define BACKUP_FLAG_ADDRESS 0x3E4002
//#define KERNAL_ADDRESS 0x3F7FF6
//#define FLASH_FLAG_LENGTH 3
//
////STATE FLAGS
//#define BOOTLOADER 0xAAAA
//#define BOOT_CONFIG 0x5555
//#define APPLICATION 0x4444
//#define BACKUP 0xBBBB
//
//typedef enum
//{
//    NULL_STATE,
//    FULL_STATE,
//    OTA_HANDSHAKE,
//    OTA_RESET,
//    BOOTLOADER_STATE,
//    BACKUP_STATE
//}kernal_state;
//
////#define INIT_ACK 1
////#define START_ACK 2
//
//typedef enum
//{
//    INIT_ACK,
//    START_ACK
//}acknowlagements;
//
//typedef enum
//{
//    FLASH_FLAG,
//    JUMP_FLAG,
//    BACKUP_FLAG
//}flags;
//
//void update_BMS(void);
//Uint16 OTA_handshake_connect_reset(void);
//void move_to_kernal(Uint8 kernal_state);
//void send_OTA_ACK(Uint8 ack_state);
//void clear_jump_flag();
//Uint16 read_flag(Uint8 flag_status);
//void resetBMS(void);
//
//#endif /* HEADERS_BOOTLOADER_H_ */
