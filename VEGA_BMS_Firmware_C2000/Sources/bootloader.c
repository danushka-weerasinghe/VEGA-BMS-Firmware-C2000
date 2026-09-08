///*
// * bootloader.c
// *
// *  Created on: May 28, 2025
// *      Author: ashans
// */
//
//#include "bootloader.h"
//#include "BMS_controller_CAN.h"
//#include "main_data.h"
//
//#pragma CODE_SECTION(move_to_kernal, "ramfuncs");
//#pragma CODE_SECTION(clear_jump_flag, "ramfuncs");
//#pragma CODE_SECTION(resetBMS, "ramfuncs");
//
////
//// Flash Status Structure
////
//FLASH_ST FlashStatus;
////extern struct ECAN_REGS ECanaShadow;
//
//extern Uint32 Flash_CPUScaleFactor;
//extern void (*Flash_CallbackPtr) (void);
//Uint16 fail;
////Uint16 count = 0;
//
//Uint16 OTA_handshake_connect_reset()
// {
//    Uint16 wordData[4];
//    Uint16 byteData[4];
//    Uint16 canData[8];
//    int i;
//    Uint16 sum;
//    Uint16 status = 0;
//
//    CAN_readMessage(CAN_OBJ_3, canData);
//
//    for(i = 0; i<4; i++){
//        wordData[i] = 0x0000;
//        byteData[i] = 0x0000;
//    }
//
//    for(i = 0; i<4; i++){
// // Fetch the LSB
//        wordData[i] =  canData[2*i];   // LS byte
//
// // Fetch the MSB
//        byteData[i] =  canData[2*i+1];  // MS byte
//
// // form the wordData from the MSB:LSB
//        wordData[i] |= (byteData[i] << 8);
//    }
//
//    sum = 0x0000;
//
//    for(i = 0; i<4; i++){
//        sum = sum + wordData[i];
//    }
//
//    if (canData[0] == 0x01 && sum == 0x01){
//        status = OTA_HANDSHAKE;
//    }
//    else if (canData[0] == 0x11 && sum == 0x11){
//        status = OTA_RESET;
//    }
//
//    return status;
//}
//
//void send_OTA_ACK(Uint8 ack_state) {
//
//    // Configure mailbox 4 for transmission
//    ECanaMboxes.MBOX4.MDL.byte.BYTE0 = 0xFF;
//
//    switch(ack_state) {
//        case INIT_ACK:
//            ECanaMboxes.MBOX4.MDL.byte.BYTE1 = 0;
//            break;
//        case START_ACK:
//            ECanaMboxes.MBOX4.MDL.byte.BYTE1 = 0x23;
//            break;
//    }
//
//    ECanaMboxes.MBOX4.MDL.byte.BYTE2 = 0;
//    ECanaMboxes.MBOX4.MDL.byte.BYTE3 = 0;
//    ECanaMboxes.MBOX4.MDH.byte.BYTE4 = 0;
//    ECanaMboxes.MBOX4.MDH.byte.BYTE5 = 0;
//    ECanaMboxes.MBOX4.MDH.byte.BYTE6 = 0;
//    ECanaMboxes.MBOX4.MDH.byte.BYTE7 = 0;
//
//    EALLOW; // Enable write access to protected registers
//
//    // Set transmission request using shadow register
//    ECanaShadow.CANTRS.bit.TRS4 = 1;
//    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all; // Write back to main register
//
//    // Wait for transmission to complete using shadow register
//    do {
//        ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;
//    } while (ECanaShadow.CANTA.bit.TA4 != 1);
//
//    // Clear the transmission flag using shadow register
//    ECanaShadow.CANTA.bit.TA4 = 1;
//    ECanaRegs.CANTA.all = ECanaShadow.CANTA.all; // Write back to main register
//
//    EDIS; // Disable write access to protected registers
//}
//
//
//void move_to_kernal(Uint8 kernal_state)
//{
////    count = 111;
//    Uint16 status;
//    Uint16 flag_buff[FLASH_FLAG_LENGTH];
//
//    if(kernal_state == BOOTLOADER_STATE)
//    {
//        flag_buff[FLASH_FLAG] = BOOTLOADER;
//        flag_buff[JUMP_FLAG] = NULL_STATE;
//        flag_buff[BACKUP_FLAG] = read_flag(BACKUP_FLAG);
//    }
//    else if(kernal_state == BACKUP_STATE)
//    {
//        InitFlash();
//        DisableDog();
//        XtalOscSel();
//        InitPll(9,2);
//        flag_buff[FLASH_FLAG] = BACKUP;
//        flag_buff[JUMP_FLAG] = read_flag(JUMP_FLAG);
//        flag_buff[BACKUP_FLAG] = read_flag(BACKUP_FLAG);
//    }
//
//    fail = 0;
//    DINT; // Disable CPU interrupts
//    InitPieCtrl(); //PIE control registers to their default state
//    //
//    // Disable CPU interrupts and clear all CPU interrupt flags
//    //
//    IER = 0x0000;
//    IFR = 0x0000;
//
//    CsmUnlock();
//
//    EALLOW;
//    Flash_CPUScaleFactor = SCALE_FACTOR;
//    Flash_CallbackPtr = NULL;
//    EDIS;
//
//    status = Flash_Erase(SECTORE,
//                         &FlashStatus);
//    if(status != STATUS_SUCCESS)
//    {
//        fail++;
//        resetBMS();
//        return;
//    }
//
//    status = Flash_Program((Uint16 *) FLASH_FLAG_ADDRESS,
//             (Uint16 *)flag_buff, FLASH_FLAG_LENGTH, &FlashStatus);
////    count = 112;
//
//    if(status != STATUS_SUCCESS)
//    {
//        fail++;
//        resetBMS();
//        return;
//    }
////    count = 113;
//
//    if(kernal_state == BOOTLOADER_STATE)
//    {
//        send_OTA_ACK(START_ACK);
//    }
//
////    DELAY_US(5000000);
//    resetBMS();
//
//    for(;;);
//
////    ((void(*)())KERNAL_ADDRESS)();
//
//
//}
//
//void clear_jump_flag()
//{
//    InitFlash();
//
//    Uint16 status;
//    Uint16 flag_buff[FLASH_FLAG_LENGTH];
//    fail = 0;
//
//    DINT; // Disable CPU interrupts
//    InitPieCtrl(); //PIE control registers to their default state
//    //
//    // Disable CPU interrupts and clear all CPU interrupt flags
//    //
//    IER = 0x0000;
//    IFR = 0x0000;
//
//    DisableDog();
//    XtalOscSel();
//    InitPll(9,2);
//
//    CsmUnlock();
//
//    EALLOW;
//    Flash_CPUScaleFactor = SCALE_FACTOR;
//    Flash_CallbackPtr = NULL;
//    EDIS;
//
//    flag_buff[FLASH_FLAG] = APPLICATION;
//    flag_buff[JUMP_FLAG] = 0x0001;
//    flag_buff[BACKUP_FLAG] = NULL_STATE;
//
//    status = Flash_Erase(SECTORE,
//                         &FlashStatus);
//    if(status != STATUS_SUCCESS)
//    {
//        fail++;
//        resetBMS();
//        return;
//    }
//
//    status = Flash_Program((Uint16 *) FLASH_FLAG_ADDRESS,
//             (Uint16 *)flag_buff, FLASH_FLAG_LENGTH, &FlashStatus);
//    if(status != STATUS_SUCCESS)
//    {
//        fail++;
//        resetBMS();
//        return;
//    }
//}
//
//Uint16 read_flag(Uint8 flag_status)
//{
//    Uint16 flag_val;
//    if(flag_status == FLASH_FLAG)
//    {
//        flag_val = *(Uint16 *)FLASH_FLAG_ADDRESS;
//    }
//    else if(flag_status == JUMP_FLAG)
//    {
//        flag_val = *(Uint16 *)JUMP_FLAG_ADDRESS;
//    }
//    else if(flag_status == BACKUP_FLAG)
//    {
//        flag_val = *(Uint16 *)BACKUP_FLAG_ADDRESS;
//    }
//
//    return flag_val;
//}
//
//void resetBMS()
//{
//    EALLOW;  // Enable write access to protected registers
//    SysCtrlRegs.WDCR = 0;  // Write an incorrect value to the WDCR register
//    EDIS;    // Disable write access to protected registers
//}
//
//void update_BMS()
//{
//    Uint16 otacmd;
//    otacmd = OTA_handshake_connect_reset();
//
//    switch(otacmd)
//    {
//    case OTA_HANDSHAKE:
//        send_OTA_ACK(INIT_ACK);
//        break;
//    case OTA_RESET:
//        send_OTA_ACK(INIT_ACK);
//        CLEAR_INTERUPT_FLAGS();
//#ifdef CAN_BOOTLOADING
//        move_to_kernal(BOOTLOADER_STATE);
//#endif
//        break;
//    default:
//        send_OTA_ACK(INIT_ACK);
//        break;
//    }
//}
