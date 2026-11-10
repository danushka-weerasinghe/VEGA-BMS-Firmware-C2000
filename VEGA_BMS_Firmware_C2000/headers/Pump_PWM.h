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
#define PUMP_CAN_ERR_DETECT_TICKS 20    // CAN pump: reported over current / voltage / block for 1 s = pump error (CpuTimer0 tick = 50 ms)
                                        // CAN pump dry run uses PUMP_DRY_DETECT_TICKS, no feedback uses PUMP_NOFB_DETECT_TICKS

/* PWM pump speed feedback, eCAP2 on GPIO15. DC45A: rpm = 30 x pulse frequency */
#define PUMP_FB_RPM_FACTOR          2700000000UL    // 30 * SYSCLK (90 MHz): rpm = factor / period
#define PUMP_FB_MAX_RPM             20000UL         // faster than this = noise, ignored (dry run measured 9561 rpm)
#define PUMP_FB_MIN_PERIOD          (PUMP_FB_RPM_FACTOR / PUMP_FB_MAX_RPM)  // 135000 SYSCLK counts
#define PUMP_FB_MIN_RPM             3500            // pump OK when turning at this speed or faster. At PUMP_PWM_DUTY 60 % the pump
                                                    // runs 5250-6550 rpm; below 3500 = no feedback. Also rejects steady mains hum
                                                    // on an open FB wire (50 Hz = 1500 rpm, 100 Hz = 3000 rpm). Lower it if the duty is reduced.
#define PUMP_FB_PWM_TIMEOUT_TICKS   20              // 1 s without an accepted pulse = pump stopped (CpuTimer0 tick = 50 ms)
#define PUMP_FB_STABLE_EDGES        8               // accept a period only after 8 matching periods in a row (noise rejection)
#define PUMP_FB_STABLE_TOL_SHIFT    4               // "matching" = within 1/16 (6.25 %) of the previous period

/* dry run (no water): the unloaded pump speeds up. Measured: with water 5250-6550 rpm, dry 9561 rpm */
#define PUMP_DRY_RPM                7500            // 250 Hz feedback x 30: faster than this = no water
#define PUMP_DRY_DETECT_TICKS       60              // 3 s continuously above PUMP_DRY_RPM = dry run (CpuTimer0 tick = 50 ms)

/* no feedback: pump commanded ON but no speed pulse (pump blocked / not powered / FB wire open) */
#define PUMP_NOFB_DETECT_TICKS      100             // 5 s commanded ON with pump_rpm < PUMP_FB_MIN_RPM = no feedback (CpuTimer0 tick = 50 ms)

extern Uint16 pump_rpm;                             // measured PWM pump speed, rpm
extern Uint16 pump_dry_run;                         // 1 = pump running dry (no water)
extern Uint16 pump_no_feedback;                     // 1 = pump ON but no speed feedback

extern Uint16 pump_can_dry_run;                     // CAN pump: 1 = pump reports dry running (3 s)
extern Uint16 pump_can_error;                       // CAN pump: 1 = pump reports over current / voltage / block (1 s)
extern Uint16 pump_can_no_feedback;                 // CAN pump: 1 = 0x268 missing / not running / speed 0 (5 s)

extern struct pumpRx pump_debug_cmd;

/* ---------- chiller pump control, used by chillerCtrl() (Battery_controller.c) ---------- */
#define PUMP_PWM_DUTY           60      // %, PWM pump duty while cooling

/* pump_fault_code: pump fault that latched cooling OFF (COOL_PUMP_FAULT), 0 until then, cleared only by BMS reset.
   Sent on CAN_OBJ_9 (0x35) mux page 6 byte 1 (eprom_config.c), DBC signal EEP_PumpFaultCode */
#define PUMP_FAULT_NONE         0
#define PUMP_FAULT_DRY_RUN      1       // no water (PWM: speed too high, CAN: pump reports dry running)
#define PUMP_FAULT_NO_FEEDBACK  2       // pump ON but no feedback (PWM: no speed pulse, CAN: 0x268 missing / not running)
#define PUMP_FAULT_PUMP_ERROR   3       // CAN pump reports over current / over voltage / under voltage / block (which one: 0x268 PUMP_FaultCode in the DBC)

extern Uint16 pump_fault_code;                      // PUMP_FAULT_xxx
extern Uint16 pump_dry_fault_count;                 // dry-run stops since power-up (watch window)
extern Uint16 pump_nofb_fault_count;                // no-feedback stops since power-up (watch window)
extern Uint16 pump_err_fault_count;                 // CAN pump error stops since power-up (watch window)
extern Uint16 pump_speed;                           // PWM pump duty now, %



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
__interrupt void ecap2_isr(void);
void InitPumpFeedback(void);
Uint16 pump_speed_rpm(void);
Uint16 pump_pwm_feedback_ok(void);
Uint16 pump_dry_update(void);
Uint16 pump_nofb_update(Uint16 pump_on);
void pump_feedback_enable(Uint16 on);
void InitEPwmTimer(Uint8 pumpStatus);

void pumpData_write(Uint16* pump_data_write, enum pump_state pump_status);
void pumpData_read(Uint16* pump_data_read);

Uint16 pump_feedback_ok(void);
void pump_can_update(void);
void pump_can_feedback_enable(Uint16 on);

void pump_cmd(Uint16 on);           // switch the pump ON / OFF (PWM or CAN), call every chillerCtrl() pass
Uint16 pump_ok(void);               // 1 = pump feedback OK (running, no fault)
Uint16 pump_fault_latch(void);      // 1 = pump fault now: sets pump_fault_code + counter


#endif /* PUMP_PWM_H_ */
