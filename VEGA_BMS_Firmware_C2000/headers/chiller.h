/*
 * Chiller.h
 *
 *  Created on: Nov 2, 2023
 *      Author: Danushka Weerasinghe
 */

#ifndef CHILLER_H_
#define CHILLER_H_

#include "DSP28x_Project.h"
#include "main_data.h"
#include "BMS_controller_CAN.h"

/* chiller_test command (0xDB byte 2) */
#define CHILLER_TEST_AUTO   0   // normal: chiller controlled by pack temperature
#define CHILLER_TEST_ON     1   // force chiller + pump ON (any other value = auto)

void SCIb_SEND(Uint8 length_sci_rev);
void chller_operation();
void chillerData_write(Uint16* chiller_data_write, Uint8 enable);
void chillerData_read(Uint16* chiller_data_read);

struct chillerCtrl_data_struct

{

    Uint8 comp_status;
    Uint16 comp_speed;
    Uint16 comp_power_limit;

};

extern struct chillerCtrl_data_struct  chillerCtrl_data;

struct chillerDiag_data_struct

{

    Uint16 comp_speed_actual;
    enum chiller_comp_state_enum chiller_comp_state;
    enum chiller_error_enum chiller_error;
    Uint16 current;
    Uint16 voltage;
    Uint32 rx_tick;                 // CpuTimer0 tick of the last 0x248 frame
    Uint16 rx_ok;                   // 1 = 0x248 received since the chiller was switched ON
};

extern struct chillerDiag_data_struct  chillerDiag_data;

/* chiller_fault_code: chiller unit error, sent on CAN_OBJ_9 (0x35) mux page 6 byte 2 (eprom_config.c),
   DBC signal EEP_ChillerFaultCode. A chiller fault stops cooling: relay (chiller_enable) OFF, pump OFF, chiller OFF,
   latched until BMS reset (COOL_CHILLER_FAULT in chillerCtrl, Battery_controller.c).
   1..7 = error reported by the chiller in 0x248 byte 4 (chiller_error_enum), 8..9 = found by the BMS */
#define CHILLER_FAULT_NONE          0
#define CHILLER_FAULT_OVER_CURRENT  1       // over_current
#define CHILLER_FAULT_RUNNING_OV    2       // running_ov
#define CHILLER_FAULT_RUNNING_UV    3       // running_uv
#define CHILLER_FAULT_STANDBY_OV    4       // standby_ov
#define CHILLER_FAULT_STANDBY_UV    5       // standby_uv
#define CHILLER_FAULT_COMM_FAILURE  6       // comm_failure (chiller lost the 0x238 command)
#define CHILLER_FAULT_SPEED_ERROR   7       // speed_error
#define CHILLER_FAULT_NO_REPLY      8       // chiller ON but no 0x248 for CHILLER_FB_TIMEOUT_TICKS
#define CHILLER_FAULT_COMPRESSOR    9       // 0x248 byte 3 = compressor_error without an error code

#define CHILLER_FB_TIMEOUT_TICKS    40      // 2 s without 0x248 while ON = no reply (CpuTimer0 tick = 50 ms)
#define CHILLER_ERR_DETECT_TICKS    20      // chiller error / compressor error in 0x248 for 1 s = fault (CpuTimer0 tick = 50 ms)

extern Uint16 chiller_fault_code;           // CHILLER_FAULT_xxx, first fault, kept until BMS reset
extern Uint16 chiller_fault_now;            // 1 = chiller fault confirmed now -> chillerCtrl() stops cooling
void chiller_fault_update(Uint16 on);       // call on every chiller_cmd() pass, on = chiller commanded ON

#endif /* CHILLER_H_ */
