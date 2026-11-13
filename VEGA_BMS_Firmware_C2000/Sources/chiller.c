/**
 * @file    chiller.c
 * @brief   Chiller unit interface: CAN command 0x238, CAN feedback 0x248, SCI command, chiller fault.
 * @author  Danushka Weerasinghe
 * @date    2024-06-06
 */

/* Hardware includes ---------------------------------------------------------*/
#include "DSP28x_Project.h"         /* ScibRegs, CpuTimer0 */

/* Own includes --------------------------------------------------------------*/
#include "chiller.h"

/* Private defines -----------------------------------------------------------*/
#define COMPRESSOR_SPEED            3200    /**< SCI chiller compressor speed command, rpm */
#define SCI_FRAME_PERIOD_CALLS      20      /**< SCI frame sent every 20 calls of chller_operation() (1 s at 50 ms) */
#define SCI_FRAME_LEN               16      /**< SCI frame length, bytes */

#define CHILLER_CAN_SPEED           3400    /**< CAN chiller compressor speed in 0x238 bytes 0-1, rpm */
#define CHILLER_POWER_LIMIT         5      /**< CAN chiller power limit in 0x238 byte 2, 0.1 kW/bit (5 = 0.5 kW) */

#define CHILLER_FB_TIMEOUT_TICKS    40      /**< 2 s without 0x248 while ON = no reply, CpuTimer0 ticks */
#define CHILLER_ERR_DETECT_TICKS    20      /**< Error / compressor error in 0x248 for 1 s = fault, CpuTimer0 ticks */

/* Global variables ----------------------------------------------------------*/
chiller_ctrl_data_t chillerCtrl_data;                   /**< See chiller.h */
chiller_diag_data_t chillerDiag_data;                   /**< See chiller.h */
Uint16 chiller_fault_code = CHILLER_FAULT_NONE;         /**< See chiller.h */
Uint16 chiller_fault_now = 0;                           /**< See chiller.h */

/* Private variables ---------------------------------------------------------*/
static Uint16 txb_data_buffer[25];          /**< SCI transmit frame */
static Uint16 uart = 0;                     /**< SCI frames sent since power-up (watch window) */
static Uint16 uart_1s_timmer_count = 0;     /**< Calls since the last SCI frame */
static int txb_data_count = 0;              /**< Index of the next byte to send */

static Uint16 chiller_fault_on_last = 0;    /**< chiller ON state at the previous call */
static Uint32 chiller_fault_on_tick = 0;    /**< CpuTimer0 tick when the chiller was switched ON */
static Uint16 chiller_err_active = 0;       /**< 1 = chiller reports an error now */
static Uint32 chiller_err_start = 0;        /**< CpuTimer0 tick when that started */

/* Public functions ----------------------------------------------------------*/
/**
 * @brief  SCI chiller: send the compressor command frame once per second.
 * @details Frame: 0xAA, 0x00, 0x01, speed low, speed high, 9 x 0x00, checksum
 *          (two's complement of the sum of bytes 1..13), 0x55.
 * @see    chiller.h
 */
void chller_operation(void)
{
    Uint16 checksum = 0;
    Uint16 i;
    unsigned short count_txb_length;

    uart_1s_timmer_count++;
    if (uart_1s_timmer_count > SCI_FRAME_PERIOD_CALLS)
    {
        txb_data_buffer[0] = 0xAA;
        txb_data_buffer[1] = 0x00;
        txb_data_buffer[2] = 0x01;
        txb_data_buffer[3] = COMPRESSOR_SPEED & 0xFF;
        txb_data_buffer[4] = (COMPRESSOR_SPEED >> 8) & 0xFF;
        txb_data_buffer[5] = 0x00;
        txb_data_buffer[6] = 0x00;
        txb_data_buffer[7] = 0x00;
        txb_data_buffer[8] = 0x00;
        txb_data_buffer[9] = 0x00;
        txb_data_buffer[10] = 0x00;
        txb_data_buffer[11] = 0x00;
        txb_data_buffer[12] = 0x00;
        txb_data_buffer[13] = 0x00;

        for (i = 1; i <= 13; i++)
        {
            checksum += txb_data_buffer[i];
        }

        txb_data_buffer[14] = ~checksum + 1;
        txb_data_buffer[15] = 0x55;

        ScibRegs.SCICTL1.bit.TXENA = 1;
        txb_data_count = 0;
        for (count_txb_length = 0; count_txb_length < SCI_FRAME_LEN; count_txb_length++)
        {
            ScibRegs.SCITXBUF = txb_data_buffer[txb_data_count];
            while (ScibRegs.SCICTL2.bit.TXEMPTY != 1)
            {
            }
            txb_data_count++;
        }
        uart++;

        uart_1s_timmer_count = 0;
    }
}

/**
 * @brief  Fill the 0x238 chiller command bytes.
 * @see    chiller.h
 */
void chillerData_write(Uint16 *chiller_data_write, Uint8 enable)
{
    chillerCtrl_data.comp_status = enable ? 1 : 0;
    chillerCtrl_data.comp_power_limit = enable ? CHILLER_POWER_LIMIT : 0;  // power limit x 0.1 kW
    chillerCtrl_data.comp_speed = enable ? CHILLER_CAN_SPEED : 0;  // compressor speed, rpm

    chiller_data_write[0] = chillerCtrl_data.comp_speed;
    chiller_data_write[1] = chillerCtrl_data.comp_speed >> 8;
    chiller_data_write[2] = chillerCtrl_data.comp_power_limit;
    chiller_data_write[3] = chillerCtrl_data.comp_status;
    chiller_data_write[4] = 0;
    chiller_data_write[5] = 0;
    chiller_data_write[6] = 0;
    chiller_data_write[7] = 0;
}

/**
 * @brief  Store a received 0x248 chiller feedback frame.
 * @see    chiller.h
 */
void chillerData_read(Uint16 *chiller_data_read)
{
    chillerDiag_data.comp_speed_actual = chiller_data_read[0];
    chillerDiag_data.comp_speed_actual |= chiller_data_read[1] << 8;
    chillerDiag_data.chiller_comp_state = (enum chiller_comp_state_enum)chiller_data_read[3];
    chillerDiag_data.chiller_error = (enum chiller_error_enum)chiller_data_read[4];
    chillerDiag_data.current = chiller_data_read[6];
    chillerDiag_data.voltage = chiller_data_read[7] * 4;
    chillerDiag_data.rx_tick = CpuTimer0.InterruptCount;
    chillerDiag_data.rx_ok = 1;
}

/**
 * @brief  Detect chiller faults. Call on every chiller_cmd() pass.
 * @details chiller_fault_now = 1 when a fault is confirmed: chillerCtrl() goes to
 *          COOL_CHILLER_FAULT (relay chiller_enable = 0, pump OFF, chiller OFF), latched until
 *          BMS reset. chiller_fault_code keeps the first confirmed fault (0x35 page 6 byte 2).
 * @see    chiller.h
 */
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

/* end of file */
