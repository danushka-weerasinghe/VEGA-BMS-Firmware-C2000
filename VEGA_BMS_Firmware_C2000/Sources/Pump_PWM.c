#include "DSP28x_Project.h"
#include "main_data.h"              // PWM_PUMP / CAN_PUMP (vehicle selection)
#include "BMS_controller_CAN.h"     // canMap, CAN_sendMessage, CAN_OBJ_24 (CAN pump 0x258)
#include "Pump_PWM.h" /*
 * Pump_PWM.c
 *
 *  Created on: May 20, 2024
 *      Author: Danushka Weerasinghe
 */


//    Uint32  ECap1IntCount;
//    Uint32  ECap1PassCount;
    Uint32  EPwm2TimerDirection;

    Uint16 tbprd;
    Uint16 desired_freq = 500; // Desired frequency in Hz (DC45A PWM input: 50-800 Hz)
    Uint16 duty_cycle_percent =20 ; // Desired duty cycle in percentage


//
// InitEPwmTimer -
//
void
InitEPwmTimer(Uint8 pumpSpeed)
{


        EALLOW; // Allow protected register writes
        SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC =0 ; // Disable time-base clock
        EDIS; // Disallow protected register writes


        // Calculate period register (TBPRD) based on desired frequency
        // TBCLK = 90 MHz / (HSPCLKDIV 2 x CLKDIV 2) = 22.5 MHz, up-count: f = TBCLK / (TBPRD + 1)
        // 500 Hz -> 44999. Valid range with this TBCLK: ~344 Hz (TBPRD 65535) and up
        tbprd = (Uint16)((22500000UL / desired_freq) - 1);

        // Time-Base Control Register (TBCTL)
        EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up mode
        EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE; // Disable phase loading
        EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW; // Load the period register from the shadow register
        EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // Sync input selected
        EPwm2Regs.TBCTL.bit.HSPCLKDIV = 1; // High-speed time-base clock prescaler
        EPwm2Regs.TBCTL.bit.CLKDIV = 1; // Time-base clock prescaler /2 (needed so TBPRD fits in 16 bits at 500 Hz)
        EPwm2Regs.TBCTL.bit.FREE_SOFT = 2; // free run: keep the pump PWM running when the debugger halts the CPU
                                           // (reset default 0 = freeze -> output stuck high/low = gaps on the scope)

        // Set the period for the PWM signal
        // Time-Base Period Register (TBPRD)
        EPwm2Regs.TBPRD = tbprd; // Set timer period

        // Time-Base Phase Register (TBPHS)
        EPwm2Regs.TBPHS.all = 0x00000000; // Set phase to zero

        // Time-Base Counter Register (TBCTR)
        EPwm2Regs.TBCTR = 0; // Clear counter

        // Counter-Compare Control Register (CMPCTL)
        EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // Shadow mode for CMPA
        EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW; // Shadow mode for CMPB
        EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // Load on CTR=0
        EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // Load on CTR=0

        // Calculate CMPA value for the desired duty cycle
        EPwm2Regs.CMPA.half.CMPA = (Uint16)(((100-pumpSpeed)/100.0 ) * tbprd);

        // Action Qualifier Control Register for Output A (AQCTLA)
       EPwm2Regs.AQCTLA.all = 0x0000; // Clear all action qualifier bits
       EPwm2Regs.AQCTLA.bit.CAU = AQ_SET; // Set output when counter equals CMPA up
       EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR; // Clear output when counter equals CMPA down
       EPwm2Regs.AQCTLA.bit.PRD = AQ_CLEAR; // Clear output when counter equals PRD


            // Force the PWM output low initially
//            EPwm2Regs.AQSFRC.bit.RLDCSF = AQSFRC_IMMEDIATE; // Load on immediate
       EPwm2Regs.AQSFRC.bit.OTSFA = 1; // One-time software force action A
       EPwm2Regs.AQSFRC.bit.ACTSFA = AQ_CLEAR; // Force PWM output low


       EPwm2TimerDirection = EPWM_TIMER_UP;

        // Enable TBCLK within the ePWM module
       EALLOW;
       SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
       EDIS;
}

/*
 * Pump speed feedback - eCAP2 on GPIO15 (pin 88)
 * Schematic: PUMP_FB -> U2 comparator -> R18 -> C2000_FEEDBACK -> GPIO15/ECAP2
 *
 * eCAP2 counts SYSCLK (90 MHz) in continuous delta mode: each rising edge of the feedback
 * pulse loads CAP1 with the time since the previous rising edge, then resets the counter.
 * DC45A: speed (rpm) = 30 x pulse frequency (Hz)  ->  rpm = 30 * 90 000 000 / period
 */
static volatile Uint32 pump_fb_period = 0;  // SYSCLK counts between two rising edges, 0 = no valid period
static volatile Uint32 pump_fb_tick = 0;    // CpuTimer0 tick of the last ACCEPTED period
static volatile Uint32 pump_fb_raw_tick = 0;// CpuTimer0 tick of the last rising edge (any, accepted or not)
static volatile Uint16 pump_fb_seen = 0;    // 1 = at least one rising edge since power-up
Uint16 pump_rpm = 0;                        // measured pump speed, rpm (watch window / debug)

void InitPumpFeedback(void)
{
    EALLOW;
    SysCtrlRegs.PCLKCR1.bit.ECAP2ENCLK = 1;     // eCAP2 clock ON
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;         // pull-up ON (pin stays defined if R18 is not fitted)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 2;       // 6-sample input qualification (uses QUALPRD1, not changed here)
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 1;        // GPIO15 = ECAP2 (overrides SCIRXDB if InitScibGpio() selected it)
    PieVectTable.ECAP2_INT = &ecap2_isr;
    EDIS;

    ECap2Regs.ECEINT.all = 0x0000;              // disable all capture interrupts
    ECap2Regs.ECCLR.all = 0xFFFF;               // clear all capture flags
    ECap2Regs.ECCTL1.bit.CAPLDEN = 0;           // disable CAP1-CAP4 loads while configuring
    ECap2Regs.ECCTL2.bit.TSCTRSTOP = 0;         // stop the counter

    ECap2Regs.ECCTL1.bit.FREE_SOFT = 2;         // free run: keep counting when the debugger halts the CPU
    ECap2Regs.ECCTL1.bit.PRESCALE = 0;          // no input prescaler, every edge
    ECap2Regs.ECCTL1.bit.CAP1POL = 0;           // CAP1 on rising edge
    ECap2Regs.ECCTL1.bit.CTRRST1 = 1;           // delta mode: reset the counter after each capture
    ECap2Regs.ECCTL2.bit.CAP_APWM = 0;          // capture mode
    ECap2Regs.ECCTL2.bit.CONT_ONESHT = 0;       // continuous
    ECap2Regs.ECCTL2.bit.STOP_WRAP = 0;         // wrap after CAP1, only CAP1 is used
    ECap2Regs.ECCTL2.bit.SYNCI_EN = 0;          // no sync input
    ECap2Regs.ECCTL2.bit.SYNCO_SEL = 2;         // sync output disabled
    ECap2Regs.TSCTR = 0;

    ECap2Regs.ECCTL1.bit.CAPLDEN = 1;           // enable capture loads
    ECap2Regs.ECCTL2.bit.TSCTRSTOP = 1;         // start the counter
    /* capture interrupt stays OFF here: pump_feedback_enable() turns it ON only while the pump is ON */

    PieCtrlRegs.PIEIER4.bit.INTx2 = 1;          // ECAP2_INT = PIE group 4, INT2
    IER |= M_INT4;
}

/*
 * Noise rejection: a real pump gives a steady pulse train, noise (open FB wire, mains hum, PWM coupling)
 * gives periods that jump around. A period is accepted only after PUMP_FB_STABLE_EDGES periods in a row,
 * each within 1/2^PUMP_FB_STABLE_TOL_SHIFT of the previous one. Only accepted periods refresh pump_fb_tick,
 * so with noise only the speed times out to 0 after PUMP_FB_PWM_TIMEOUT_TICKS -> no-feedback fault.
 */
static Uint32 pump_fb_last_raw = 0;         // previous raw period, 0 = none
static Uint16 pump_fb_stable = 0;           // consecutive periods that matched the previous one

__interrupt void ecap2_isr(void)
{
    Uint32 period = ECap2Regs.CAP1;
    Uint32 now = CpuTimer0.InterruptCount;
    Uint32 diff;

    if (!pump_fb_seen || ((now - pump_fb_raw_tick) > PUMP_FB_PWM_TIMEOUT_TICKS))
    {
        /* first edge after power-up or after > 1 s without edges: CAP1 holds the time since an old edge
           (or a wrapped counter), discarded */
        pump_fb_last_raw = 0;
        pump_fb_stable = 0;
    }
    else if (period < PUMP_FB_MIN_PERIOD)
    {
        /* too short = noise spike; the next period is cut short by it as well, so start the match again */
        pump_fb_last_raw = 0;
        pump_fb_stable = 0;
    }
    else
    {
        if (pump_fb_last_raw != 0)
        {
            diff = (period > pump_fb_last_raw) ? (period - pump_fb_last_raw) : (pump_fb_last_raw - period);
            if (diff <= (pump_fb_last_raw >> PUMP_FB_STABLE_TOL_SHIFT))
            {
                if (pump_fb_stable < 0xFFFF)
                {
                    pump_fb_stable++;
                }
            }
            else
            {
                pump_fb_stable = 0;             // period jumped: not a steady pump signal
            }
        }
        pump_fb_last_raw = period;

        if (pump_fb_stable >= PUMP_FB_STABLE_EDGES)
        {
            pump_fb_period = period;            // steady signal: accept
            pump_fb_tick = now;
        }
    }
    pump_fb_seen = 1;
    pump_fb_raw_tick = now;

    ECap2Regs.ECCLR.bit.CEVT1 = 1;
    ECap2Regs.ECCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}

/* measured pump speed in rpm, 0 when no pulse for PUMP_FB_PWM_TIMEOUT_TICKS. Call from the main loop. */
Uint16 pump_speed_rpm(void)
{
    Uint32 period = pump_fb_period;
    Uint32 tick = pump_fb_tick;

    if ((CpuTimer0.InterruptCount - tick) > PUMP_FB_PWM_TIMEOUT_TICKS)
    {
        pump_fb_period = 0;                     // no accepted period for 1 s: stopped / no feedback / only noise
        pump_rpm = 0;
    }
    else if (period == 0)
    {
        pump_rpm = 0;                           // only one edge so far, waiting for the second
    }
    else
    {
        pump_rpm = (Uint16)(PUMP_FB_RPM_FACTOR / period);
    }
    return pump_rpm;
}

/* 1 = PWM pump is turning at PUMP_FB_MIN_RPM or faster (eCAP2 feedback is fresh) and not running dry */
Uint16 pump_pwm_feedback_ok(void)
{
    return ((pump_speed_rpm() >= PUMP_FB_MIN_RPM) && !pump_dry_run) ? 1 : 0;
}

/*
 * Dry-run detection - call every 50 ms (CpuTimer0 tick).
 * pump_dry_run = 1 when pump_rpm has been > PUMP_DRY_RPM for PUMP_DRY_DETECT_TICKS of real time
 * (CpuTimer0 ticks, so it does not depend on how often the main loop calls this).
 * Clears by itself as soon as the speed is back at or below PUMP_DRY_RPM (or the pump is stopped);
 * the cooling state machine latches the fault and stops the pump.
 */
Uint16 pump_dry_run = 0;
static Uint16 pump_dry_active = 0;          // 1 = speed is above PUMP_DRY_RPM
static Uint32 pump_dry_start = 0;           // CpuTimer0 tick when it went above

Uint16 pump_dry_update(void)
{
    if (pump_speed_rpm() > PUMP_DRY_RPM)
    {
        if (!pump_dry_active)
        {
            pump_dry_active = 1;
            pump_dry_start = CpuTimer0.InterruptCount;
        }
        pump_dry_run = ((CpuTimer0.InterruptCount - pump_dry_start) >= PUMP_DRY_DETECT_TICKS) ? 1 : 0;
    }
    else
    {
        pump_dry_active = 0;
        pump_dry_run = 0;
    }
    return pump_dry_run;
}

/*
 * No-feedback detection - call every 50 ms after pump_dry_update() (uses the pump_rpm it refreshed).
 * pump_no_feedback = 1 when the pump has been commanded ON with pump_rpm < PUMP_FB_MIN_RPM for
 * PUMP_NOFB_DETECT_TICKS of real time (CpuTimer0 ticks). Clears when the pump is OFF or the speed comes back.
 */
Uint16 pump_no_feedback = 0;
static Uint16 pump_nofb_active = 0;         // 1 = pump ON and too slow
static Uint32 pump_nofb_start = 0;          // CpuTimer0 tick when that started

Uint16 pump_nofb_update(Uint16 pump_on)
{
    if (pump_on && (pump_rpm < PUMP_FB_MIN_RPM))
    {
        if (!pump_nofb_active)
        {
            pump_nofb_active = 1;
            pump_nofb_start = CpuTimer0.InterruptCount;
        }
        pump_no_feedback = ((CpuTimer0.InterruptCount - pump_nofb_start) >= PUMP_NOFB_DETECT_TICKS) ? 1 : 0;
    }
    else
    {
        pump_nofb_active = 0;
        pump_no_feedback = 0;
    }
    return pump_no_feedback;
}

/*
 * Turn the eCAP2 speed feedback ON / OFF together with the pump. Call when the pump duty changes.
 *   ON : start fresh (first edge discarded, no old speed), capture interrupt enabled
 *   OFF: capture interrupt disabled, speed and dry-run / no-feedback flags cleared -> GPIO15 is not read
 */
void pump_feedback_enable(Uint16 on)
{
    ECap2Regs.ECEINT.bit.CEVT1 = 0;             // no interrupt while resetting the state
    ECap2Regs.ECCLR.all = 0xFFFF;               // drop any edge captured while OFF

    pump_fb_seen = 0;
    pump_fb_period = 0;
    pump_fb_last_raw = 0;
    pump_fb_stable = 0;
    pump_rpm = 0;
    pump_dry_active = 0;
    pump_dry_run = 0;
    pump_nofb_active = 0;
    pump_no_feedback = 0;

    if (on)
    {
        ECap2Regs.ECEINT.bit.CEVT1 = 1;         // interrupt on every rising edge
    }
}

struct pumpTx pump_cntrl_cmd;

void pumpData_write(Uint16* pump_data_write, enum pump_state pump_status)
{
    pump_cntrl_cmd.pump_power = pump_status;
    pump_cntrl_cmd.target_duty.all = 150;//originally 1000

    pump_data_write[0] = pump_cntrl_cmd.target_duty.byte.DUTY_BYTE1;
    pump_data_write[1] = pump_cntrl_cmd.target_duty.byte.DUTY_BYTE2;
    pump_data_write[2] = pump_cntrl_cmd.pump_power;
    pump_data_write[3] = 0;
    pump_data_write[4] = 0;
    pump_data_write[5] = 0;
    pump_data_write[6] = 0;
    pump_data_write[7] = 0;
}

struct pumpRx pump_debug_cmd;

static volatile Uint16 pump_can_fb_enabled = 0;    // 1 = pump ON: 0x268 frames are used (see pump_can_feedback_enable)

void pumpData_read(Uint16* pump_data_read)
{
    if (!pump_can_fb_enabled)
    {
        return;         // pump OFF: 0x268 not used (mailbox already read by the caller)
    }
    pump_debug_cmd.target_speed_fb = pump_data_read[0];
    pump_debug_cmd.target_speed_fb |= pump_data_read[1] << 8;
    pump_debug_cmd.pump_power_fb = pump_data_read[2];
    pump_debug_cmd.pump_temp = pump_data_read[3];
    pump_debug_cmd.bus_v = pump_data_read[4];
    pump_debug_cmd.pump_current = pump_data_read[5];
    pump_debug_cmd.pump_error_code.all = pump_data_read[6];
    pump_debug_cmd.rx_tick = CpuTimer0.InterruptCount;
    pump_debug_cmd.rx_ok = 1;
}

/* 1 = pump feedback (0x268) says the pump is running without fault, and the feedback is fresh */
Uint16 pump_feedback_ok(void)
{
    if (!pump_debug_cmd.rx_ok)
    {
        return 0;       // no feedback received yet
    }
    if ((CpuTimer0.InterruptCount - pump_debug_cmd.rx_tick) > PUMP_FB_TIMEOUT_TICKS)
    {
        return 0;       // feedback lost
    }
    if (pump_debug_cmd.pump_power_fb != PUMP_FB_RUNNING)
    {
        return 0;       // pump off or fault
    }
    if (pump_debug_cmd.pump_error_code.all != 0)
    {
        return 0;       // over current / over voltage / under voltage / dry running / block
    }
    if (pump_debug_cmd.target_speed_fb == 0)
    {
        return 0;       // not turning
    }
    return 1;
}

/*
 * CAN pump fault detection - same idea as the PWM pump, but the CAN pump reports its own faults in 0x268.
 * Call from the main loop while the pump is ON. Each condition must last its time (CpuTimer0 real time):
 *   pump_can_dry_run     : 0x268 byte 6 dry_running bit set                           for PUMP_DRY_DETECT_TICKS  (3 s)
 *   pump_can_error       : 0x268 over current / over voltage / under voltage / block,
 *                          or pump state = PUMP_FB_FAULT                              for PUMP_CAN_ERR_DETECT_TICKS (1 s)
 *   pump_can_no_feedback : 0x268 missing (> PUMP_FB_TIMEOUT_TICKS), pump not RUNNING or speed 0,
 *                          without one of the faults above                            for PUMP_NOFB_DETECT_TICKS (5 s)
 * The cooling state machine latches the fault and stops the pump.
 */
Uint16 pump_can_dry_run = 0;
Uint16 pump_can_error = 0;
Uint16 pump_can_no_feedback = 0;
static Uint16 pump_can_dry_active = 0, pump_can_err_active = 0, pump_can_nofb_active = 0;
static Uint32 pump_can_dry_start = 0, pump_can_err_start = 0, pump_can_nofb_start = 0;

/* 1 when cond has been true for ticks of real time (CpuTimer0) */
static Uint16 pump_cond_held(Uint16 cond, Uint16 *active, Uint32 *start, Uint32 ticks)
{
    if (!cond)
    {
        *active = 0;
        return 0;
    }
    if (!*active)
    {
        *active = 1;
        *start = CpuTimer0.InterruptCount;
    }
    return ((CpuTimer0.InterruptCount - *start) >= ticks) ? 1 : 0;
}

void pump_can_update(void)
{
    Uint16 fresh, dry, err;

    fresh = pump_debug_cmd.rx_ok
            && ((CpuTimer0.InterruptCount - pump_debug_cmd.rx_tick) <= PUMP_FB_TIMEOUT_TICKS);

    dry = fresh && pump_debug_cmd.pump_error_code.bit.dry_running;

    err = fresh && !dry
          && (pump_debug_cmd.pump_error_code.bit.over_current
              || pump_debug_cmd.pump_error_code.bit.over_voltage
              || pump_debug_cmd.pump_error_code.bit.under_voltage
              || pump_debug_cmd.pump_error_code.bit.block_protection
              || (pump_debug_cmd.pump_power_fb == PUMP_FB_FAULT));

    pump_can_dry_run     = pump_cond_held(dry, &pump_can_dry_active, &pump_can_dry_start, PUMP_DRY_DETECT_TICKS);
    pump_can_error       = pump_cond_held(err, &pump_can_err_active, &pump_can_err_start, PUMP_CAN_ERR_DETECT_TICKS);
    pump_can_no_feedback = pump_cond_held(!dry && !err && !pump_feedback_ok(),
                                          &pump_can_nofb_active, &pump_can_nofb_start, PUMP_NOFB_DETECT_TICKS);
}

/*
 * Turn the CAN pump feedback ON / OFF together with the pump. Call when the pump is switched ON or OFF.
 *   ON : start fresh - old 0x268 data is not used, waits for new frames
 *   OFF: 0x268 frames ignored, fault flags cleared
 */
void pump_can_feedback_enable(Uint16 on)
{
    pump_can_fb_enabled = 0;
    pump_debug_cmd.rx_ok = 0;
    pump_can_dry_active = 0;
    pump_can_err_active = 0;
    pump_can_nofb_active = 0;
    pump_can_dry_run = 0;
    pump_can_error = 0;
    pump_can_no_feedback = 0;
    pump_can_fb_enabled = on ? 1 : 0;
}

/*
 * ---------- chiller pump control (moved from Battery_controller.c) ----------
 * Called from chillerCtrl() in Battery_controller.c:
 *   pump_cmd(on)        every pass: switch the pump ON / OFF and run the fault detection
 *   pump_ok()           1 = pump feedback OK -> chiller may run
 *   pump_fault_latch()  1 = pump fault -> chillerCtrl() goes to COOL_PUMP_FAULT (latched until BMS reset)
 */
#if defined(CAN_PUMP)
#define PUMP_DRY()              pump_can_dry_run        // 0x268 dry_running bit for 3 s = no water
#define PUMP_NOFB()             pump_can_no_feedback    // 0x268 missing / not running / speed 0 for 5 s = no feedback
#define PUMP_ERR()              pump_can_error          // 0x268 over current / voltage / block for 1 s = pump error
#elif defined(PWM_PUMP)
#define PUMP_DRY()              pump_dry_run            // eCAP2 speed > PUMP_DRY_RPM for 3 s = no water
#define PUMP_NOFB()             pump_no_feedback        // pump ON, speed < PUMP_FB_MIN_RPM for 5 s = no feedback
#define PUMP_ERR()              0                       // PWM pump has no error report
#else
#define PUMP_DRY()              0
#define PUMP_NOFB()             0
#define PUMP_ERR()              0
#endif

Uint16 pump_fault_code = PUMP_FAULT_NONE;       // PUMP_FAULT_xxx (watch window), sent on CAN_OBJ_9 (0x35) mux page 6 byte 1
Uint16 pump_dry_fault_count = 0;                // dry-run stops since power-up (watch window)
Uint16 pump_nofb_fault_count = 0;               // no-feedback stops since power-up (watch window)
Uint16 pump_err_fault_count = 0;                // CAN pump error stops since power-up (watch window)
Uint16 pump_speed = 0;                          // PWM pump duty now, %

/* 1 = pump feedback OK (running, no fault, feedback fresh) */
Uint16 pump_ok(void)
{
#if defined(CAN_PUMP)
    return pump_feedback_ok();          // 0x268 feedback: running, no fault, not lost
#elif defined(PWM_PUMP)
    return pump_pwm_feedback_ok();      // eCAP2 (GPIO15) speed >= PUMP_FB_MIN_RPM, not dry
#else
    return 1;                           // no pump feedback in this build
#endif
}

/* 1 = a pump fault is present now: pump_fault_code and its counter are set (dry run > pump error > no feedback) */
Uint16 pump_fault_latch(void)
{
    if (PUMP_DRY())
    {
        pump_fault_code = PUMP_FAULT_DRY_RUN;
        if (pump_dry_fault_count < 0xFFFF)
        {
            pump_dry_fault_count++;
        }
    }
    else if (PUMP_ERR())
    {
        pump_fault_code = PUMP_FAULT_PUMP_ERROR;
        if (pump_err_fault_count < 0xFFFF)
        {
            pump_err_fault_count++;
        }
    }
    else if (PUMP_NOFB())
    {
        pump_fault_code = PUMP_FAULT_NO_FEEDBACK;
        if (pump_nofb_fault_count < 0xFFFF)
        {
            pump_nofb_fault_count++;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

/* pump command
 *   PWM pump: reconfigure only when the duty changes
 *   CAN pump: 0x258 sent every call, ON or OFF (pump runs full speed without a command)
 */
void pump_cmd(Uint16 on)
{
#if defined(PWM_PUMP)
    Uint16 duty = on ? PUMP_PWM_DUTY : 0;
    static Uint16 pwm_init = 0;
    if ((duty != pump_speed) || !pwm_init)
    {
        pump_speed = duty;
        InitEPwmTimer(pump_speed);
        pump_feedback_enable(on);   // eCAP2 speed feedback read only while the pump is ON
        pwm_init = 1;
    }
    if (on)
    {
        pump_dry_update();      // refresh pump_rpm (0 after 1 s without an accepted pulse) and pump_dry_run
        pump_nofb_update(1);    // pump_no_feedback: ON but no speed for 5 s
    }
#elif defined(CAN_PUMP)
    static Uint16 last_on = 0;

    if (on != last_on)
    {
        pump_can_feedback_enable(on);           // 0x268 feedback used only while the pump is ON
    }
    last_on = on;

    /* 0x258 sent on every call, ON or OFF. The pump runs at full speed (~7200 rpm) when it gets
       no 0x258 (seen on the bus log at power-up), so the OFF command must never stop */
    pumpData_write(canMap.msg[CAN_OBJ_24].sendData, on ? ON : OFF);
    CAN_sendMessage(CAN_OBJ_24, NOMINAL_DLC, canMap.msg[CAN_OBJ_24].sendData);
    if (on)
    {
        pump_can_update();                      // dry run / pump error / no feedback from 0x268
    }
#else
    (void)on;                                   // no pump in this build
#endif
}
