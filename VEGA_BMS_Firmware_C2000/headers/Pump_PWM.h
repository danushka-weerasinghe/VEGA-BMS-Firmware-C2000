#include "DSP28x_Project.h"

/*
 * Pump_PWM.h
 *
 *  Created on: May 20, 2024
 *      Author: Danushka Weerasinghe
 */

#ifndef PUMP_PWM_H_
#define PUMP_PWM_H_

//
// Defines that configure the start/end period for the timer
//
#define PWM3_TIMER_MIN     10
#define PWM3_TIMER_MAX     8000

//
// Defines to keep track of which way the timer value is moving
//
#define EPWM_TIMER_UP   1
#define EPWM_TIMER_DOWN 0


/* pump state, 0x268 byte 2 */
#define PUMP_FB_OFF             0
#define PUMP_FB_RUNNING         1
#define PUMP_FB_FAULT           2

#define PUMP_FB_TIMEOUT_TICKS   40      // 2 s without 0x268 = feedback lost (CpuTimer0 tick = 50 ms)

extern struct pumpRx pump_debug_cmd;



//

//
// Globals
//

//    Uint32  ECap1IntCount;
//    Uint32  ECap1PassCount;
//    Uint32  EPwm3TimerDirection;
//
//    Uint16 tbprd;
//    Uint16 desired_freq ; // Desired frequency in Hz
//    Uint16 duty_cycle_percent ; // Desired duty cycle in percentage


struct pumpFaultCodes
{
    Uint8 over_current:1;
    Uint8 over_voltage:1;
    Uint8 under_voltage:1;
    Uint8 dry_running:1;
    Uint8 block_protection:1;
    Uint8 reserved:3;
};

union pump_error
{
    Uint8 all;
    struct pumpFaultCodes bit;
};

struct pump_duty_bytes
{
    Uint16 DUTY_BYTE1 :8;
    Uint16 DUTY_BYTE2 :8;

};

union pump_duty_value
{
    Uint16 all;
    struct pump_duty_bytes byte;
};

struct pumpRx
{
    union pump_error pump_error_code;   // byte 6, fault bit flags (0 = no fault)
    Uint16 target_speed_fb;             // bytes 0-1, actual speed, RPM
    Uint16 pump_power_fb;               // byte 2, pump state: PUMP_FB_OFF / PUMP_FB_RUNNING / PUMP_FB_FAULT
    Uint16 pump_temp;                   // byte 3, raw, degC = value - 40
    Uint16 bus_v;                       // byte 4, 0.1 V/bit
    Uint16 pump_current;                // byte 5, 0.1 A/bit
    Uint32 rx_tick;                     // CpuTimer0 tick of the last 0x268 frame
    Uint16 rx_ok;                       // 1 = at least one 0x268 frame received
};

struct pumpTx
{
    union pump_duty_value target_duty;
    Uint16 pump_power;
};

enum pump_state {OFF,ON};


//
// Function Prototypes
//
__interrupt void ecap1_isr(void);
void InitECapture(void);
void InitEPwmTimer(Uint8 pumpStatus);

void pumpData_write(Uint16* pump_data_write, enum pump_state pump_status);
void pumpData_read(Uint16* pump_data_read);

Uint16 pump_feedback_ok(void);


#endif /* PUMP_PWM_H_ */
