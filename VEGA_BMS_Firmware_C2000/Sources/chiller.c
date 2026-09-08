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
void chillerData_write(Uint16* chiller_data_write)
{
    chillerCtrl_data.comp_status = 1;
    chillerCtrl_data.comp_power_limit = 3;  // MAX Power 20*0.1 = 2KW
    chillerCtrl_data.comp_speed = 2500;     //

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

//      Chiller_compressor_state = {if(ECanaMboxes.MBOX19.MDL.byte.BYTE3 == 0x00){off}} ;

}

