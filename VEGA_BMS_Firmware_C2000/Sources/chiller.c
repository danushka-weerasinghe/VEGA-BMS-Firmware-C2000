/*
 * chiller.c
 *
 *  Created on: Jun 6, 2024
 *      Author: Danushka Weerasinghe
 */

#include "chiller.h"
#include <stdint.h>

extern CAN_BufferMap canMap;

uint16_t txb_data_buffer[25];

uint16_t uart = 0;

uint16_t uart_1s_timmer_count = 0;
int txb_data_count = 0;
int rxb_data_count = 0;

#define compressor_speed 3200
#define pump_speed

void chller_operation()
{
    uart_1s_timmer_count++;
    if (uart_1s_timmer_count > 20)
    {
        txb_data_buffer[0] = 0xAA;
        txb_data_buffer[1] = 0x00;
        txb_data_buffer[2] = 0x01;
//    txb_data_buffer[3]   = 0x14;
//    txb_data_buffer[4]   = 0x05;
        txb_data_buffer[3] = compressor_speed & 0xFF;
        txb_data_buffer[4] = (compressor_speed >> 8) & 0xFF;
        txb_data_buffer[5] = 0x00;
        txb_data_buffer[6] = 0x00;
        txb_data_buffer[7] = 0x00;
        txb_data_buffer[8] = 0x00;
        txb_data_buffer[9] = 0x00;
        txb_data_buffer[10] = 0x00;
        txb_data_buffer[11] = 0x00;
        txb_data_buffer[12] = 0x00;
        txb_data_buffer[13] = 0x00;

        uint16_t checksum = 0;
        uint16_t i;
        for (i = 1; i <= 13; i++)
        {
            checksum += txb_data_buffer[i];
        }

        txb_data_buffer[14] = ~checksum + 1;
        txb_data_buffer[15] = 0x55;

        ScibRegs.SCICTL1.bit.TXENA = 1;
        txb_data_count = 0;
        unsigned short count_txb_length;
        for (count_txb_length = 0; count_txb_length < 16; count_txb_length++)
        {
            ScibRegs.SCITXBUF = txb_data_buffer[txb_data_count];
            while (ScibRegs.SCICTL2.bit.TXEMPTY != 1)
            {
            }
            txb_data_count++;
            //PieCtrlRegs.PIEACK.all |= 0x100;      // Issue PIE ACK
        }
        uart++;

        uart_1s_timmer_count = 0;
    }
}

struct chillerCtrl_data_struct chillerCtrl_data;
void chillerData_write(Uint16* chiller_data_write, Uint8 enable)
{
    chillerCtrl_data.comp_status = enable ? 1 : 0;
    chillerCtrl_data.comp_power_limit = enable ? 3 : 0;  // MAX Power 20*0.1 = 2KW
    chillerCtrl_data.comp_speed = enable ? 3200 : 0;     //

    chiller_data_write[0] = chillerCtrl_data.comp_speed;
    chiller_data_write[1] = chillerCtrl_data.comp_speed >> 8;
    chiller_data_write[2] = chillerCtrl_data.comp_power_limit;
    chiller_data_write[3] = chillerCtrl_data.comp_status;
    chiller_data_write[4] = 0;
    chiller_data_write[5] = 0;
    chiller_data_write[6] = 0;
    chiller_data_write[7] = 0;

//    CAN_sendMessage(CAN_OBJ_20, NOMINAL_DLC, canMap.msg[CAN_OBJ_20].sendData);
//    ECanaShadow.CANTRS.bit.TRS20 = 1;

}

struct chillerDiag_data_struct chillerDiag_data;
void chillerData_read(Uint16* chiller_data_read)
{
//    CAN_readMessage(CAN_OBJ_19, canMap.msg[CAN_OBJ_19].readData);

    chillerDiag_data.comp_speed_actual = chiller_data_read[0];
    chillerDiag_data.comp_speed_actual |= chiller_data_read[1] << 8;
//      chillerCtrl_data.compressor_speed_actual = ECanaMboxes.MBOX17.MDL.byte.BYTE3;
    chillerDiag_data.chiller_comp_state = (enum chiller_comp_state_enum)chiller_data_read[3];
    chillerDiag_data.chiller_error = (enum chiller_error_enum)chiller_data_read[4];
//      chillerCtrl_data.compressor_speed_actual = ECanaMboxes.MBOX19.MDH.byte.BYTE6;
    chillerDiag_data.current = chiller_data_read[6];
    chillerDiag_data.voltage = chiller_data_read[7] * 4;
    chillerDiag_data.rx_tick = CpuTimer0.InterruptCount;
    chillerDiag_data.rx_ok = 1;

//      Chiller_compressor_state = {if(ECanaMboxes.MBOX19.MDL.byte.BYTE3 == 0x00){off}} ;

}

/*
 * Chiller unit fault - call on every chiller_cmd() pass (on = chiller commanded ON).
 * While ON: 0x248 missing for CHILLER_FB_TIMEOUT_TICKS (2 s)                    -> CHILLER_FAULT_NO_REPLY
 *           0x248 byte 4 (chiller_error) not no_fault for CHILLER_ERR_DETECT_TICKS (1 s) -> that code (1..7)
 *           0x248 byte 3 = compressor_error (byte 4 = 0) for CHILLER_ERR_DETECT_TICKS  -> CHILLER_FAULT_COMPRESSOR
 * chiller_fault_now = 1 when a fault is confirmed: chillerCtrl() goes to COOL_CHILLER_FAULT
 * (relay chiller_enable = 0, pump OFF, chiller OFF), latched until BMS reset.
 * chiller_fault_code keeps the first confirmed fault until BMS reset (0x35 page 6 byte 2).
 */
Uint16 chiller_fault_code = CHILLER_FAULT_NONE;
Uint16 chiller_fault_now = 0;
static Uint16 chiller_fault_on_last = 0;
static Uint32 chiller_fault_on_tick = 0;    // CpuTimer0 tick when the chiller was switched ON
static Uint16 chiller_err_active = 0;       // 1 = chiller reports an error now
static Uint32 chiller_err_start = 0;        // CpuTimer0 tick when that started

void chiller_fault_update(Uint16 on)
{
    Uint32 now = CpuTimer0.InterruptCount;
    Uint16 code = CHILLER_FAULT_NONE;
    Uint16 confirmed = 0;

    if (on && !chiller_fault_on_last)
    {
        chiller_fault_on_tick = now;        // just switched ON: old 0x248 data not used, wait for new frames
        chillerDiag_data.rx_ok = 0;
    }
    chiller_fault_on_last = on;
    chiller_fault_now = 0;

    if (!on)
    {
        chiller_err_active = 0;             // chiller OFF: not checked
        return;
    }

    if (!chillerDiag_data.rx_ok || ((now - chillerDiag_data.rx_tick) > CHILLER_FB_TIMEOUT_TICKS))
    {
        if ((now - chiller_fault_on_tick) > CHILLER_FB_TIMEOUT_TICKS)
        {
            code = CHILLER_FAULT_NO_REPLY;  // ON for 2 s, no (fresh) 0x248
            confirmed = 1;                  // already timed by CHILLER_FB_TIMEOUT_TICKS
        }
        chiller_err_active = 0;
    }
    else
    {
        if (chillerDiag_data.chiller_error != no_fault)
        {
            code = (Uint16)chillerDiag_data.chiller_error & 0xFF;  // error reported by the chiller (1..7)
        }
        else if (chillerDiag_data.chiller_comp_state == compressor_error)
        {
            code = CHILLER_FAULT_COMPRESSOR;
        }

        if (code == CHILLER_FAULT_NONE)
        {
            chiller_err_active = 0;
        }
        else
        {
            if (!chiller_err_active)
            {
                chiller_err_active = 1;
                chiller_err_start = now;
            }
            confirmed = ((now - chiller_err_start) >= CHILLER_ERR_DETECT_TICKS) ? 1 : 0;
        }
    }

    if (confirmed)
    {
        chiller_fault_now = 1;
        if (chiller_fault_code == CHILLER_FAULT_NONE)
        {
            chiller_fault_code = code;      // first fault kept until BMS reset
        }
    }
}
