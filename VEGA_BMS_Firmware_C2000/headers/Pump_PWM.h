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
    union pump_error pump_error_code;
    Uint16 target_speed_fb;
    Uint16 pump_power_fb;
    Uint16 pump_temp;
    Uint16 bus_v;
    Uint16 pump_current;
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

#endif /* PUMP_PWM_H_ */
