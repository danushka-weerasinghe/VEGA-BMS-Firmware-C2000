/*
 * BMS_controller_CAN.c
 *
 *  Created on: Apr 17, 2025
 *      Author: ashans
 */
#include "BMS_controller_CAN.h"
//#include "main_data.h"
#include <stdlib.h>

//CAN_Message_t Set_msg;
Uint16 CANerrorCnt = 0;
//CAN_BufferMap canMap;


void CAN_Init() {

    InitECanGpio();     //initialize CAN GPIO pins
    InitECana();    //initialize CAN module

//    // Initialize CAN module
//    CAN_initModule(base);
//
//    // Set CAN bit timing based on system clock and desired baudrate
//    CAN_setBitRate(base, DEVICE_SYSCLK_FREQ, baudrate, 16);
//
//    // Enable the CAN module
//    CAN_enableModule(base);
//
//    // Enable interrupts if required (optional)
//    CAN_enableInterrupt(base, CAN_INT_IE0 | CAN_INT_ERROR | CAN_INT_STATUS);
}

//void CAN_Transmit(uint32_t base, uint32_t mailbox, const CAN_Message_t *msg) {
//    uint32_t flags = 0;
//    if (msg->is_extended) {
//        flags |= CAN_MSG_FRAME_EXT;
//    } else {
//        flags |= CAN_MSG_FRAME_STD;
//    }
//CANge
//    CAN_setupMessageObject(base, mailbox, msg->id, flags,
//                           CAN_MSG_OBJ_TYPE_TX, 0,
//                           CAN_MSG_OBJ_NO_FLAGS, msg->dlc);
//    CAN_sendMessage(base, mailbox, msg->dlc, msg->data);
//}
//
//bool CAN_Receive(uint32_t base, uint32_t mailbox, CAN_Message_t *msg) {
//    if (!CAN_isMessageReceived(base, mailbox)) {
//        return false;
//    }
//
//    uint32_t flags = 0;
//    CAN_readMessage(base, mailbox, msg->data);
//    CAN_getMessageObject(base, mailbox, &msg->id, &flags, &msg->dlc);
//
//    msg->is_extended = (flags & CAN_MSG_FRAME_EXT) != 0;
//    return true;
//}
//
//bool CAN_MessageAvailable(uint32_t base, uint32_t mailbox) {
//    return CAN_isMessageReceived(base, mailbox);
//}


void CAN_enableGlobalInterrupt()
{
    EALLOW;
    ECanaShadow.CANGIM.all = ECanaRegs.CANGIM.all;
    ECanaShadow.CANGIM.all = 0;
    ECanaShadow.CANGIM.bit.I1EN = 1;  // enable I1EN
    ECanaRegs.CANGIM.all = ECanaShadow.CANGIM.all;
    EDIS;
}

Uint8 CAN_getInterruptCause()
{
    Uint8 status = 0;
    if (ECanaRegs.CANRMP.bit.RMP3)
    {
        status = CAN_OBJ_3;
    }
    if (ECanaRegs.CANRMP.bit.RMP11)
    {
        status = CAN_OBJ_11;
    }
    if (ECanaRegs.CANRMP.bit.RMP12)
    {
        status = CAN_OBJ_12;
    }
    if (ECanaRegs.CANRMP.bit.RMP16)
    {
        status = CAN_OBJ_16;
    }
    if (ECanaRegs.CANRMP.bit.RMP17)
    {
        status = CAN_OBJ_17;
    }
    if (ECanaRegs.CANRMP.bit.RMP19)
    {
        status = CAN_OBJ_19;
    }
    if (ECanaRegs.CANRMP.bit.RMP21)
    {
        status = CAN_OBJ_21;
    }
    if (ECanaRegs.CANRMP.bit.RMP26)
    {
        status = CAN_OBJ_26;
    }
    if (ECanaRegs.CANRMP.bit.RMP27)
    {
        status = CAN_OBJ_27;
    }
    return status;
}

void CLEAR_INTERUPT_FLAGS()
{
    ECanaShadow.CANRMP.all = ECanaRegs.CANRMP.all;
//  ECanaShadow.CANRMP.bit.RMP0 = 1;
//  ECanaShadow.CANRMP.bit.RMP1 = 1;
//  ECanaShadow.CANRMP.bit.RMP2 = 1;
    ECanaShadow.CANRMP.bit.RMP3 = 1; //OTA message
//   ECanaShadow.CANRMP.bit.RMP4 = 1;
//  ECanaShadow.CANRMP.bit.RMP5 = 1;
//  ECanaShadow.CANRMP.bit.RMP6 = 1;
//  ECanaShadow.CANRMP.bit.RMP7 = 1;
//  ECanaShadow.CANRMP.bit.RMP8 = 1;
//  ECanaShadow.CANRMP.bit.RMP9 = 1;
    ECanaShadow.CANRMP.bit.RMP10 = 1;
//  ECanaShadow.CANRMP.bit.RMP11 = 1;
    ECanaShadow.CANRMP.bit.RMP12 = 1;
//  ECanaShadow.CANRMP.bit.RMP13 = 1;
    ECanaShadow.CANRMP.bit.RMP14 = 1;
//  ECanaShadow.CANRMP.bit.RMP15 = 1;
    ECanaShadow.CANRMP.bit.RMP16 = 1;
//  ECanaShadow.CANRMP.bit.RMP17 = 1;

    ECanaShadow.CANRMP.bit.RMP18 = 1;
    ECanaShadow.CANRMP.bit.RMP19 = 1;
    ECanaShadow.CANRMP.bit.RMP20 = 1;

    ECanaShadow.CANRMP.bit.RMP21 = 1;
    ECanaShadow.CANRMP.bit.RMP22 = 1;
    ECanaShadow.CANRMP.bit.RMP23 = 1;
    ECanaShadow.CANRMP.bit.RMP24 = 1;
    ECanaShadow.CANRMP.bit.RMP25 = 1;
    ECanaShadow.CANRMP.bit.RMP26 = 1;
    ECanaShadow.CANRMP.bit.RMP27 = 1;
    ECanaShadow.CANRMP.bit.RMP28 = 1;
    ECanaShadow.CANRMP.bit.RMP29 = 1;
    ECanaShadow.CANRMP.bit.RMP30 = 1;
    ECanaShadow.CANRMP.bit.RMP31 = 1;

    ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all;
//    ecana_interrupt_counter++;
    PieCtrlRegs.PIEACK.bit.ACK9 = 1;
}

void CAN_setupMessageObject(Mbox_obj_id_val st_obj_id, Uint32 set_msg_id, Uint8 set_dlc,
                            CAN_msg_dir set_msg_dir, CAN_msg_frame_type set_msg_frame)
{
    switch(st_obj_id)
    {
        case CAN_OBJ_0:
            ECanaMboxes.MBOX0.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX0.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX0.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX0.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX0.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD0 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME0 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM0 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL0 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX0.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX0.MDL.all = 0x00000000;
            ECanaMboxes.MBOX0.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_1:
            ECanaMboxes.MBOX1.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX1.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX1.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX1.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX1.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD1 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME1 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM1 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL1 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX1.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX1.MDL.all = 0x00000000;
            ECanaMboxes.MBOX1.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_2:
            ECanaMboxes.MBOX2.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX2.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX2.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX2.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX2.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD2 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME2 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM2 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL2 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX2.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX2.MDL.all = 0x00000000;
            ECanaMboxes.MBOX2.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_3://OTA_FLAG:
            ECanaMboxes.MBOX3.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX3.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX3.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX3.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX3.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX3.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD3 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME3 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM3 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL3 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX3.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX3.MDL.all = 0x00000000;
            ECanaMboxes.MBOX3.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_4: //OTA_RESPONSE:
            ECanaMboxes.MBOX4.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX4.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX4.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX4.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX4.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX4.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD4 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME4 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM4 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL4 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX4.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX4.MDL.all = 0x00000000;
            ECanaMboxes.MBOX4.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_5:
            ECanaMboxes.MBOX5.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX5.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX5.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX5.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX5.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX5.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD5 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME5 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM5 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL5 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX5.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX5.MDL.all = 0x00000000;
            ECanaMboxes.MBOX5.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_6:
            ECanaMboxes.MBOX6.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX6.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX6.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX6.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX6.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX6.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD6 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME6 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM6 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL6 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX6.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX6.MDL.all = 0x00000000;
            ECanaMboxes.MBOX6.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_7:
            ECanaMboxes.MBOX7.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX7.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX7.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX7.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX7.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX7.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD7 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME7 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM7 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL7 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX7.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX7.MDL.all = 0x00000000;
            ECanaMboxes.MBOX7.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_8:
            ECanaMboxes.MBOX8.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX8.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX8.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX8.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX8.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX8.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD8 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME8 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM8 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL8 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX8.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX8.MDL.all = 0x00000000;
            ECanaMboxes.MBOX8.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_9:
            ECanaMboxes.MBOX9.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX9.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX9.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX9.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX9.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX9.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD9 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME9 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM9 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL9 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX9.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX9.MDL.all = 0x00000000;
            ECanaMboxes.MBOX9.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_10:
            ECanaMboxes.MBOX10.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX10.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX10.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX10.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX10.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX10.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD10 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME10 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM10 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL10 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX10.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX10.MDL.all = 0x00000000;
            ECanaMboxes.MBOX10.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_11: //EVCC:
            ECanaMboxes.MBOX11.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX11.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX11.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX11.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX11.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX11.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD11 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME11 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM11 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL11 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX11.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX11.MDL.all = 0x00000000;
            ECanaMboxes.MBOX11.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_12://BMS_CMD:
            ECanaMboxes.MBOX12.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX12.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX12.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX12.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX12.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX12.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD12 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME12 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM12 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL12 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX12.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX12.MDL.all = 0x00000000;
            ECanaMboxes.MBOX12.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_13:
            ECanaMboxes.MBOX13.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX13.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX13.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX13.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX13.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX13.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD13 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME13 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM13 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL13 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX13.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX13.MDL.all = 0x00000000;
            ECanaMboxes.MBOX13.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_14: //BMS_DEBUG:
            ECanaMboxes.MBOX14.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX14.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX14.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX14.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX14.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX14.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD14 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME14 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM14 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL14 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX14.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX14.MDL.all = 0x00000000;
            ECanaMboxes.MBOX14.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_15://BMS_CAN_DATA:
            ECanaMboxes.MBOX15.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX15.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX15.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX15.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX15.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX15.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD15 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME15 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM15 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL15 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX15.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX15.MDL.all = 0x00000000;
            ECanaMboxes.MBOX15.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_16://EVCU:
            ECanaMboxes.MBOX16.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX16.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX16.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX16.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX16.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX16.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD16 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME16 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM16 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL16 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX16.MDL.all = 0x00000000;
            ECanaMboxes.MBOX16.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_17://GET_SLAVE:
            ECanaMboxes.MBOX17.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX17.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX17.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX17.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX17.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX17.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD17 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME17 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM17 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL17 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX17.MDL.all = 0x00000000;
            ECanaMboxes.MBOX17.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_18://SET_SLAVE:
            ECanaMboxes.MBOX18.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX18.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX18.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX18.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX18.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX18.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD18 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME18 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM18 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL18 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX18.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX18.MDL.all = 0x00000000;
            ECanaMboxes.MBOX18.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_19://CHILLER_RX:
            ECanaMboxes.MBOX19.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX19.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX19.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX19.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX19.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX19.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD19 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME19 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM19 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL19 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX19.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX19.MDL.all = 0x00000000;
            ECanaMboxes.MBOX19.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_20://CHILLER_TX:
            ECanaMboxes.MBOX20.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX20.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX20.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX20.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX20.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX20.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD20 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME20 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM20 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL20 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX20.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX20.MDL.all = 0x00000000;
            ECanaMboxes.MBOX20.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_21:
            ECanaMboxes.MBOX21.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX21.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX21.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX21.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX21.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX21.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD21 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME21 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM21 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL21 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX21.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX21.MDL.all = 0x00000000;
            ECanaMboxes.MBOX21.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_22:
            ECanaMboxes.MBOX22.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX22.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX22.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX22.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX22.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX22.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD20 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME20 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM20 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL20 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX22.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX22.MDL.all = 0x00000000;
            ECanaMboxes.MBOX22.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_23:
            ECanaMboxes.MBOX23.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX23.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX23.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX23.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX23.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX23.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD23 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME23 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM23 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL23 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX23.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX23.MDL.all = 0x00000000;
            ECanaMboxes.MBOX23.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_24:
            ECanaMboxes.MBOX24.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX24.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX24.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX24.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX24.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX24.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD24 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME24 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM24 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL24 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX24.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX24.MDL.all = 0x00000000;
            ECanaMboxes.MBOX24.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_25:
            ECanaMboxes.MBOX25.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX25.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX25.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX25.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX25.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX25.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD25 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME25 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM25 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL25 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX25.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX25.MDL.all = 0x00000000;
            ECanaMboxes.MBOX25.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_26:
            ECanaMboxes.MBOX26.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX26.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX26.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX26.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX26.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX26.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD26 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME26 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM26 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL26 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX26.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX26.MDL.all = 0x00000000;
            ECanaMboxes.MBOX26.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_27:
            ECanaMboxes.MBOX27.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX27.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX27.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX27.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX27.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX27.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD27 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME27 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM27 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL27 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX27.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX27.MDL.all = 0x00000000;
            ECanaMboxes.MBOX27.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_28:
            ECanaMboxes.MBOX28.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX28.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX28.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX28.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX28.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX28.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD28 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME28 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM28 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL28 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX28.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX28.MDL.all = 0x00000000;
            ECanaMboxes.MBOX28.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_29:
            ECanaMboxes.MBOX29.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX29.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX29.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX29.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX29.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX29.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD29 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME29 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM29 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL29 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX29.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX29.MDL.all = 0x00000000;
            ECanaMboxes.MBOX29.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_30:
            ECanaMboxes.MBOX30.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX30.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX30.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX30.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX30.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX30.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD30 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME30 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM30 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL30 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX30.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX30.MDL.all = 0x00000000;
            ECanaMboxes.MBOX30.MDH.all = 0x00000000;
            break;
        case CAN_OBJ_31:
            ECanaMboxes.MBOX31.MSGID.all = 0x00000000;
            switch(set_msg_frame)
            {
                case STD_FRAME:
                    ECanaMboxes.MBOX31.MSGID.bit.STDMSGID = set_msg_id;
                    break;
                case EXT_FRAME:
                    ECanaMboxes.MBOX31.MSGID.bit.IDE = 0x1;
                    ECanaMboxes.MBOX31.MSGID.bit.STDMSGID = (set_msg_id >> 18) & 0x7FF;
                    ECanaMboxes.MBOX31.MSGID.bit.EXTMSGID_H = (set_msg_id >> 16) & 0x3;
                    ECanaMboxes.MBOX31.MSGID.bit.EXTMSGID_L = set_msg_id & 0xFFFF;
                    break;
                default:
                    break;
            }
            EALLOW;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANMD.bit.MD31 = set_msg_dir;
            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;
            ECanaShadow.CANME.bit.ME31 = 1;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
            ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
            ECanaShadow.CANMIM.bit.MIM31 = set_msg_dir;
            ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;
            ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
//            ECanaShadow.CANMIL.all = 0;
            ECanaShadow.CANMIL.bit.MIL31 = set_msg_dir;
            ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;
            EDIS;
            ECanaMboxes.MBOX31.MSGCTRL.bit.DLC = set_dlc;
            ECanaMboxes.MBOX31.MDL.all = 0x00000000;
            ECanaMboxes.MBOX31.MDH.all = 0x00000000;
            break;
        default:
            break;
    }
    ECanaShadow.CANTRS.all = 0;
}



//void ECAN_CONFIG()
//{
////    ECanaShadow.CANMIL.all = 0;
//    CAN_setupMessageObject(CAN_OBJ_3, OTA_FLAG_ADDR, NOMINAL_DLC,
//                           RX_DIR, EXT_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_4, OTA_RESPONSE_ADDR, NOMINAL_DLC,
//                           TX_DIR, EXT_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_11, EVCC_ADDR, NOMINAL_DLC,
//                           RX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_12, BMS_CMD_ADDR, NOMINAL_DLC,
//                           RX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_14, BMS_DEBUG_ADDR, NOMINAL_DLC,
//                           TX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_15, BMS_CAN_DATA_ADDR, NOMINAL_DLC,
//                           TX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_16, EVCU_ADDR, NOMINAL_DLC,
//                           RX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_17, GET_SLAVE_ADDR, NOMINAL_DLC,
//                           RX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_18, SET_SLAVE_ADDR, NOMINAL_DLC,
//                           TX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_19, CHILLER_RX_ADDR, NOMINAL_DLC,
//                           RX_DIR, STD_FRAME);
//    CAN_setupMessageObject(CAN_OBJ_20, CHILLER_TX_ADDR, NOMINAL_DLC,
//                           TX_DIR, STD_FRAME);
//}

void CAN_sendMessage(Mbox_obj_id_val objID, Uint16 msgLen, Uint16 *msgData)
{
    EALLOW;
    ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;

    switch(objID)
    {
    case CAN_OBJ_0:
        ECanaMboxes.MBOX0.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX0.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX0.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX0.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX0.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX0.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX0.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX0.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS0 = 1;
        break;
    case CAN_OBJ_1:
        ECanaMboxes.MBOX1.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX1.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX1.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX1.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX1.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX1.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX1.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX1.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS1 = 1;
        break;
    case CAN_OBJ_2:
        ECanaMboxes.MBOX2.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX2.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX2.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX2.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX2.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX2.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX2.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX2.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS2 = 1;
        break;
    case CAN_OBJ_3:
        ECanaMboxes.MBOX3.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX3.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX3.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX3.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX3.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX3.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX3.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX3.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS3 = 1;
        break;
    case CAN_OBJ_4:
        ECanaMboxes.MBOX4.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX4.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX4.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX4.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX4.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX4.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX4.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX4.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS4 = 1;
        break;
    case CAN_OBJ_5:
        ECanaMboxes.MBOX5.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX5.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX5.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX5.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX5.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX5.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX5.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX5.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS5 = 1;
        break;
    case CAN_OBJ_6:
        ECanaMboxes.MBOX6.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX6.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX6.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX6.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX6.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX6.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX6.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX6.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS6 = 1;
        break;
    case CAN_OBJ_7:
        ECanaMboxes.MBOX7.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX7.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX7.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX7.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX7.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX7.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX7.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX7.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS7 = 1;
        break;
    case CAN_OBJ_8:
        ECanaMboxes.MBOX8.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX8.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX8.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX8.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX8.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX8.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX8.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX8.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS8 = 1;
        break;
    case CAN_OBJ_9:
        ECanaMboxes.MBOX9.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX9.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX9.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX9.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX9.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX9.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX9.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX9.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS9 = 1;
        break;
    case CAN_OBJ_10:
        ECanaMboxes.MBOX10.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX10.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX10.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX10.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX10.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX10.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX10.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX10.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS10 = 1;
        break;
    case CAN_OBJ_11:
        ECanaMboxes.MBOX11.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX11.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX11.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX11.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX11.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX11.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX11.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX11.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS11 = 1;
        break;
    case CAN_OBJ_12:
        ECanaMboxes.MBOX12.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX12.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX12.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX12.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX12.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX12.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX12.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX12.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS12 = 1;
        break;
    case CAN_OBJ_13:
        ECanaMboxes.MBOX13.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX13.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX13.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX13.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX13.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX13.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX13.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX13.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS11 = 1;
        break;
    case CAN_OBJ_14:
        ECanaMboxes.MBOX14.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX14.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX14.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX14.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX14.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX14.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX14.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX14.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS14 = 1;
        break;
    case CAN_OBJ_15:
        ECanaMboxes.MBOX15.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX15.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX15.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX15.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX15.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX15.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX15.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX15.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS15 = 1;
        break;
    case CAN_OBJ_16:
        ECanaMboxes.MBOX16.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX16.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX16.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX16.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX16.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX16.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX16.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX16.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS16 = 1;
        break;
    case CAN_OBJ_17:
        ECanaMboxes.MBOX17.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX17.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX17.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX17.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX17.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX17.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX17.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX17.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS17 = 1;
        break;
    case CAN_OBJ_18:
        ECanaMboxes.MBOX18.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX18.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX18.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX18.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX18.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX18.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX18.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX18.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS18 = 1;
        break;
    case CAN_OBJ_19:
        ECanaMboxes.MBOX19.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX19.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX19.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX19.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX19.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX19.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX19.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX19.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS19 = 1;
        break;
    case CAN_OBJ_20:
        ECanaMboxes.MBOX20.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX20.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX20.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX20.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX20.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX20.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX20.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX20.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS20 = 1;
        break;
    case CAN_OBJ_21:
        ECanaMboxes.MBOX21.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX21.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX21.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX21.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX21.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX21.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX21.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX21.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS21 = 1;
        break;
    case CAN_OBJ_22:
        ECanaMboxes.MBOX22.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX22.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX22.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX22.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX22.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX22.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX22.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX22.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS22 = 1;
        break;
    case CAN_OBJ_23:
        ECanaMboxes.MBOX23.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX23.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX23.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX23.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX23.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX23.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX23.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX23.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS21 = 1;
        break;
    case CAN_OBJ_24:
        ECanaMboxes.MBOX24.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX24.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX24.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX24.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX24.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX24.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX24.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX24.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS24 = 1;
        break;
    case CAN_OBJ_25:
        ECanaMboxes.MBOX25.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX25.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX25.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX25.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX25.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX25.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX25.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX25.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS25 = 1;
        break;
    case CAN_OBJ_26:
        ECanaMboxes.MBOX26.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX26.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX26.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX26.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX26.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX26.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX26.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX26.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS26 = 1;
        break;
    case CAN_OBJ_27:
        ECanaMboxes.MBOX27.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX27.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX27.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX27.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX27.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX27.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX27.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX27.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS27 = 1;
        break;
    case CAN_OBJ_28:
        ECanaMboxes.MBOX28.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX28.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX28.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX28.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX28.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX28.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX28.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX28.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS28 = 1;
        break;
    case CAN_OBJ_29:
        ECanaMboxes.MBOX29.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX29.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX29.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX29.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX29.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX29.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX29.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX29.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS29 = 1;
        break;
    case CAN_OBJ_30:
        ECanaMboxes.MBOX30.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX30.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX30.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX30.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX30.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX30.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX30.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX30.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS30 = 1;
        break;
    case CAN_OBJ_31:
        ECanaMboxes.MBOX31.MDL.byte.BYTE0 = msgData[0];
        ECanaMboxes.MBOX31.MDL.byte.BYTE1 = msgData[1];
        ECanaMboxes.MBOX31.MDL.byte.BYTE2 = msgData[2];
        ECanaMboxes.MBOX31.MDL.byte.BYTE3 = msgData[3];
        ECanaMboxes.MBOX31.MDH.byte.BYTE4 = msgData[4];
        ECanaMboxes.MBOX31.MDH.byte.BYTE5 = msgData[5];
        ECanaMboxes.MBOX31.MDH.byte.BYTE6 = msgData[6];
        ECanaMboxes.MBOX31.MDH.byte.BYTE7 = msgData[7];
        ECanaShadow.CANTRS.all = 0;
        ECanaShadow.CANTRS.bit.TRS31 = 1;
        break;
    default:
        break;
    }

    ECanaRegs.CANTRR.all = 0x00000000;
    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
    EDIS;
    ECanaShadow.CANTRS.all = 0x00000000;
}

void CAN_readMessage(Mbox_obj_id_val objID, Uint16* msgData)
{
    switch(objID)
    {
    case CAN_OBJ_0:
        msgData[0] = (Uint16)ECanaMboxes.MBOX0.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX0.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX0.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX0.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX0.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX0.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX0.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX0.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_1:
        msgData[0] = (Uint16)ECanaMboxes.MBOX1.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX1.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX1.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX1.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX1.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX1.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX1.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX1.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_2:
        msgData[0] = (Uint16)ECanaMboxes.MBOX2.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX2.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX2.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX2.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX2.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX2.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX2.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX2.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_3:
        msgData[0] = (Uint16)ECanaMboxes.MBOX3.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX3.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX3.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX3.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX3.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX3.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX3.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX3.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_4:
        msgData[0] = (Uint16)ECanaMboxes.MBOX4.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX4.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX4.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX4.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX4.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX4.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX4.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX4.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_5:
        msgData[0] = (Uint16)ECanaMboxes.MBOX5.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX5.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX5.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX5.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX5.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX5.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX5.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX5.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_6:
        msgData[0] = (Uint16)ECanaMboxes.MBOX6.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX6.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX6.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX6.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX6.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX6.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX6.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX6.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_7:
        msgData[0] = (Uint16)ECanaMboxes.MBOX7.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX7.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX7.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX7.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX7.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX7.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX7.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX7.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_8:
        msgData[0] = (Uint16)ECanaMboxes.MBOX8.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX8.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX8.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX8.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX8.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX8.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX8.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX8.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_9:
        msgData[0] = (Uint16)ECanaMboxes.MBOX9.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX9.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX9.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX9.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX9.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX9.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX9.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX9.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_10:
        msgData[0] = (Uint16)ECanaMboxes.MBOX10.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX10.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX10.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX10.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX10.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX10.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX10.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX10.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_11:
        msgData[0] = (Uint16)ECanaMboxes.MBOX11.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX11.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX11.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX11.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX11.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX11.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX11.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX11.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_12:
        msgData[0] = (Uint16)ECanaMboxes.MBOX12.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX12.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX12.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX12.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX12.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX12.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX12.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX12.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_13:
        msgData[0] = (Uint16)ECanaMboxes.MBOX13.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX13.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX13.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX13.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX13.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX13.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX13.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX13.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_14:
        msgData[0] = (Uint16)ECanaMboxes.MBOX14.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX14.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX14.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX14.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX14.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX14.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX14.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX14.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_15:
        msgData[0] = (Uint16)ECanaMboxes.MBOX15.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX15.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX15.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX15.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX15.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX15.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX15.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX15.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_16:
        msgData[0] = (Uint16)ECanaMboxes.MBOX16.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX16.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX16.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX16.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX16.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX16.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX16.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX16.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_17:
        msgData[0] = (Uint16)ECanaMboxes.MBOX17.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX17.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX17.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX17.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX17.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX17.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX17.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX17.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_18:
        msgData[0] = (Uint16)ECanaMboxes.MBOX18.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX18.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX18.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX18.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX18.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX18.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX18.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX18.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_19:
        msgData[0] = (Uint16)ECanaMboxes.MBOX19.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX19.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX19.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX19.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX19.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX19.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX19.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX19.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_20:
        msgData[0] = (Uint16)ECanaMboxes.MBOX20.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX20.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX20.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX20.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX20.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX20.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX20.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX20.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_21:
        msgData[0] = (Uint16)ECanaMboxes.MBOX21.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX21.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX21.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX21.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX21.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX21.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX21.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX21.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_22:
        msgData[0] = (Uint16)ECanaMboxes.MBOX22.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX22.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX22.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX22.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX22.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX22.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX22.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX22.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_23:
        msgData[0] = (Uint16)ECanaMboxes.MBOX23.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX23.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX23.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX23.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX23.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX23.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX23.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX23.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_24:
        msgData[0] = (Uint16)ECanaMboxes.MBOX24.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX24.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX24.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX24.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX24.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX24.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX24.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX24.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_25:
        msgData[0] = (Uint16)ECanaMboxes.MBOX25.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX25.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX25.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX25.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX25.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX25.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX25.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX25.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_26:
        msgData[0] = (Uint16)ECanaMboxes.MBOX26.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX26.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX26.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX26.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX26.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX26.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX26.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX26.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_27:
        msgData[0] = (Uint16)ECanaMboxes.MBOX27.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX27.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX27.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX27.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX27.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX27.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX27.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX27.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_28:
        msgData[0] = (Uint16)ECanaMboxes.MBOX28.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX28.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX28.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX28.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX28.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX28.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX28.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX28.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_29:
        msgData[0] = (Uint16)ECanaMboxes.MBOX29.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX29.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX29.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX29.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX29.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX29.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX29.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX29.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_30:
        msgData[0] = (Uint16)ECanaMboxes.MBOX30.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX30.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX30.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX30.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX30.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX30.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX30.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX30.MDH.byte.BYTE7;
        break;
    case CAN_OBJ_31:
        msgData[0] = (Uint16)ECanaMboxes.MBOX31.MDL.byte.BYTE0;
        msgData[1] = (Uint16)ECanaMboxes.MBOX31.MDL.byte.BYTE1;
        msgData[2] = (Uint16)ECanaMboxes.MBOX31.MDL.byte.BYTE2;
        msgData[3] = (Uint16)ECanaMboxes.MBOX31.MDL.byte.BYTE3;
        msgData[4] = (Uint16)ECanaMboxes.MBOX31.MDH.byte.BYTE4;
        msgData[5] = (Uint16)ECanaMboxes.MBOX31.MDH.byte.BYTE5;
        msgData[6] = (Uint16)ECanaMboxes.MBOX31.MDH.byte.BYTE6;
        msgData[7] = (Uint16)ECanaMboxes.MBOX31.MDH.byte.BYTE7;
        break;
    default:
        break;
    }

//    AddToBuffer(Obj_ID, msgData);
}

void CAN_transmission_acknowledge(Mbox_obj_id_val objID)
{
    EALLOW; // Enable write access to protected registers

    switch(objID)
    {
    case CAN_OBJ_4:
        // Wait for transmission to complete using shadow register
        do {
            ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;
        } while (ECanaShadow.CANTA.bit.TA4 != 1);

        // Clear the transmission flag using shadow register
        ECanaShadow.CANTA.bit.TA4 = 1;
        ECanaRegs.CANTA.all = ECanaShadow.CANTA.all; // Write back to main register
        break;
    default:
        break;
    }

    EDIS; // Disable write access to protected registers

}


CAN_Message_t* Msg_head = NULL;

void AddToBuffer(Mbox_obj_id_val Obj_ID, Uint16* Rx_data)
{
    Uint8 i;
    CAN_Message_t* newMsg = (CAN_Message_t*)malloc(sizeof(CAN_Message_t));
    if(!newMsg)
    {
        CANerrorCnt++;
    }
    newMsg->Mbox_Obj_id = Obj_ID;

    for(i = 0; i < NOMINAL_DLC; i++)
    {
        newMsg->Rx_data[i] = Rx_data[i];
    }
    newMsg->CAN_msg_buffer = Msg_head;
    Msg_head = newMsg;
}

void removeBuffer(Mbox_obj_id_val Obj_ID)
{
    CAN_Message_t *tempbuf = Msg_head, *Prev_msg = NULL;

    while(tempbuf && tempbuf->Mbox_Obj_id != Obj_ID)
    {
        Prev_msg = tempbuf;
        tempbuf = tempbuf->CAN_msg_buffer;
    }
    if(!tempbuf) return;

    if(!Prev_msg)
    {
        Msg_head = tempbuf->CAN_msg_buffer;
    }
    else
    {
        Prev_msg->CAN_msg_buffer = tempbuf->CAN_msg_buffer;
    }
    free(tempbuf);
}

CAN_Message_t *GetFromBuffer(Mbox_obj_id_val Obj_ID)
{
    CAN_Message_t *tempbuf = Msg_head;
    while(tempbuf)
    {
        if(tempbuf->Mbox_Obj_id == Obj_ID)
        {
            return tempbuf;
        }
        tempbuf = tempbuf->CAN_msg_buffer;
    }
    return NULL;
}

void read_CANData(Mbox_obj_id_val Obj_ID, Uint16* CANData)
{
    Uint8 i;

    CAN_readMessage(Obj_ID, CANData);
    CAN_Message_t *get_data_buff = GetFromBuffer(Obj_ID);
    if(get_data_buff)
    {
        for(i = 0; i < NOMINAL_DLC; i++)
        {
            CANData[i] = get_data_buff->Rx_data[i];
        }
    }
    else
    {
        CANerrorCnt++;
    }
    removeBuffer(Obj_ID);
}

