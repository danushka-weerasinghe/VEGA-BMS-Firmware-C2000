/**
 * @file    Pump_PWM.c
 * @brief   Chiller coolant pump control: PWM pump (ePWM2 + eCAP2) or CAN pump (0x258 / 0x268).
 * @details PWM pump (DC45A): ePWM2 drives the pump at PUMP_PWM_DUTY, 500 Hz. eCAP2 on GPIO15
 *          measures the speed feedback (rpm = 30 x pulse frequency) with a noise filter.
 *          Faults: dry run (speed too high) and no feedback (speed too low).
 *          CAN pump: 0x258 command, 0x268 feedback with the pump's own fault bits.
 *          Faults: dry run, pump error, no feedback.
 *          All fault times use CpuTimer0 real time (1 tick = 50 ms), so they do not depend on
 *          how often the main loop runs.
 * @author  Danushka Weerasinghe
 * @date    2024-05-20
 */

/* Own header ----------------------------------------------------------------*/
#include "Pump_PWM.h"

/* Hardware / vendor includes ------------------------------------------------*/
#include "DSP28x_Project.h"         /* EPwm2Regs, ECap2Regs, GPIO, PIE, CpuTimer0 */

/* Project includes ----------------------------------------------------------*/
#include "main_data.h"              /* PWM_PUMP / CAN_PUMP (vehicle selection) */
#include "BMS_controller_CAN.h"     /* canMap, CAN_sendMessage, CAN_OBJ_24 (CAN pump 0x258) */

/* Private defines -----------------------------------------------------------*/
/* PWM pump drive */
#define PUMP_PWM_DUTY               60              /**< PWM pump duty while cooling, % */

/* PWM pump speed feedback, eCAP2 on GPIO15. DC45A: rpm = 30 x pulse frequency */
#define PUMP_FB_RPM_FACTOR          2700000000UL    /**< 30 x SYSCLK (90 MHz): rpm = factor / period */
#define PUMP_FB_MAX_RPM             20000UL         /**< Faster than this = noise, ignored (dry run measured 9561 rpm), rpm */
#define PUMP_FB_MIN_PERIOD          (PUMP_FB_RPM_FACTOR / PUMP_FB_MAX_RPM)  /**< Shortest valid period: 135000 SYSCLK counts */
#define PUMP_FB_MIN_RPM             3500            /**< Pump OK at this speed or faster, rpm. At 60 % duty the pump runs
                                                         5250-6550 rpm. Also rejects mains hum on an open FB wire
                                                         (50 Hz = 1500 rpm, 100 Hz = 3000 rpm) */
#define PUMP_FB_PWM_TIMEOUT_TICKS   20              /**< No accepted pulse for 1 s = pump stopped, CpuTimer0 ticks */
#define PUMP_FB_STABLE_EDGES        8               /**< Accept a period only after 8 matching periods in a row */
#define PUMP_FB_STABLE_TOL_SHIFT    4               /**< "Matching" = within 1/16 (6.25 %) of the previous period */

/* PWM pump dry run: the unloaded pump speeds up (with water 5250-6550 rpm, dry 9561 rpm) */
#define PUMP_DRY_RPM                7500            /**< Faster than this = no water, rpm (250 Hz feedback x 30) */
#define PUMP_DRY_DETECT_TICKS       60              /**< 3 s above PUMP_DRY_RPM = dry run, CpuTimer0 ticks */

/* No feedback: pump commanded ON but not turning (blocked / not powered / FB wire open / 0x268 missing) */
#define PUMP_NOFB_DETECT_TICKS      100             /**< 5 s without feedback = no feedback fault, CpuTimer0 ticks */

/* CAN pump feedback 0x268 */
#define PUMP_FB_OFF                 0               /**< 0x268 byte 2: pump off */
#define PUMP_FB_RUNNING             1               /**< 0x268 byte 2: pump running */
#define PUMP_FB_FAULT               2               /**< 0x268 byte 2: pump fault */
#define PUMP_FB_TIMEOUT_TICKS       40              /**< 2 s without 0x268 = feedback lost, CpuTimer0 ticks */
#define PUMP_CAN_ERR_DETECT_TICKS   20              /**< 1 s of over current / voltage / block = pump error, CpuTimer0 ticks */
#define PUMP_CAN_DUTY               150             /**< CAN pump target duty sent in 0x258 bytes 0-1 (originally 1000) */

/* Fault sources for pump_fault_latch(), by pump type */
#if defined(CAN_PUMP)
#define PUMP_DRY()                  pump_can_dry_run        /**< 0x268 dry_running bit for 3 s = no water */
#define PUMP_NOFB()                 pump_can_no_feedback    /**< 0x268 missing / not running / speed 0 for 5 s */
#define PUMP_ERR()                  pump_can_error          /**< 0x268 over current / voltage / block for 1 s */
#elif defined(PWM_PUMP)
#define PUMP_DRY()                  pump_dry_run            /**< eCAP2 speed > PUMP_DRY_RPM for 3 s = no water */
#define PUMP_NOFB()                 pump_no_feedback        /**< Pump ON, speed < PUMP_FB_MIN_RPM for 5 s */
#define PUMP_ERR()                  0                       /**< PWM pump has no error report */
#else
#define PUMP_DRY()                  0
#define PUMP_NOFB()                 0
#define PUMP_ERR()                  0
#endif

/* Private enums -------------------------------------------------------------*/
/**
 * @brief CAN pump power command, 0x258 byte 2.
 */
typedef enum pump_state
{
    PUMP_CMD_OFF = 0,               /**< Pump OFF */
    PUMP_CMD_ON                     /**< Pump ON */
} pump_state_t;

/* Private structs -----------------------------------------------------------*/
/**
 * @brief CAN pump fault bits, 0x268 byte 6.
 */
typedef struct pumpFaultCodes
{
    Uint8 over_current:1;           /**< Over current */
    Uint8 over_voltage:1;           /**< Over voltage */
    Uint8 under_voltage:1;          /**< Under voltage */
    Uint8 dry_running:1;            /**< Dry running (no water) */
    Uint8 block_protection:1;       /**< Rotor blocked */
    Uint8 reserved:3;               /**< Not used */
} pump_fault_bits_t;

/**
 * @brief CAN pump fault byte, whole or by bit.
 */
typedef union pump_error
{
    Uint8 all;                      /**< All fault bits, 0 = no fault */
    struct pumpFaultCodes bit;      /**< Single fault bits */
} pump_error_t;

/**
 * @brief CAN pump target duty split into the two 0x258 bytes.
 */
typedef struct pump_duty_bytes
{
    Uint16 DUTY_BYTE1 :8;           /**< Low byte, 0x258 byte 0 */
    Uint16 DUTY_BYTE2 :8;           /**< High byte, 0x258 byte 1 */
} pump_duty_bytes_t;

/**
 * @brief CAN pump target duty, whole or by byte.
 */
typedef union pump_duty_value
{
    Uint16 all;                     /**< Target duty */
    struct pump_duty_bytes byte;    /**< Target duty by byte */
} pump_duty_value_t;

/**
 * @brief Last CAN pump feedback frame 0x268.
 */
typedef struct pumpRx
{
    union pump_error pump_error_code;   /**< Byte 6: fault bits, 0 = no fault */
    Uint16 target_speed_fb;             /**< Bytes 0-1: actual speed, rpm */
    Uint16 pump_power_fb;               /**< Byte 2: PUMP_FB_OFF / PUMP_FB_RUNNING / PUMP_FB_FAULT */
    Uint16 pump_temp;                   /**< Byte 3: raw, degC = value - 40 */
    Uint16 bus_v;                       /**< Byte 4: supply voltage, 0.1 V/bit */
    Uint16 pump_current;                /**< Byte 5: current, 0.1 A/bit */
    Uint32 rx_tick;                     /**< CpuTimer0 tick of the last 0x268 frame */
    Uint16 rx_ok;                       /**< 1 = at least one 0x268 frame received since pump ON */
} pump_rx_t;

/**
 * @brief CAN pump command 0x258.
 */
typedef struct pumpTx
{
    union pump_duty_value target_duty;  /**< Bytes 0-1: target duty */
    Uint16 pump_power;                  /**< Byte 2: pump_state_t */
} pump_tx_t;

/* Private function prototypes -----------------------------------------------*/
#if defined(PWM_PUMP)
static void   InitEPwmTimer(Uint8 pumpSpeed);
static Uint16 pump_speed_rpm(void);
static Uint16 pump_pwm_feedback_ok(void);
static Uint16 pump_dry_update(void);
static Uint16 pump_nofb_update(Uint16 pump_on);
static void   pump_feedback_enable(Uint16 on);
#endif

#if defined(CAN_PUMP)
static void   pumpData_write(Uint16 *pump_data_write, pump_state_t pump_status);
static Uint16 pump_feedback_ok(void);
static Uint16 pump_cond_held(Uint16 cond, Uint16 *active, Uint32 *start, Uint32 ticks);
static void   pump_can_update(void);
static void   pump_can_feedback_enable(Uint16 on);
#endif

/* Global variables ----------------------------------------------------------*/
Uint16 pump_fault_code = PUMP_FAULT_NONE;       /**< See Pump_PWM.h */

/* Private variables ---------------------------------------------------------*/
/* fault counters (watch window) */
static Uint16 pump_dry_fault_count = 0;         /**< Dry-run stops since power-up */
static Uint16 pump_nofb_fault_count = 0;        /**< No-feedback stops since power-up */
static Uint16 pump_err_fault_count = 0;         /**< CAN pump error stops since power-up */

/* eCAP2 speed feedback (used by ecap2_isr in every build) */
static volatile Uint32 pump_fb_period = 0;      /**< SYSCLK counts between two rising edges, 0 = no valid period */
static volatile Uint32 pump_fb_tick = 0;        /**< CpuTimer0 tick of the last ACCEPTED period */
static volatile Uint32 pump_fb_raw_tick = 0;    /**< CpuTimer0 tick of the last rising edge (accepted or not) */
static volatile Uint16 pump_fb_seen = 0;        /**< 1 = at least one rising edge since pump ON */
static Uint32 pump_fb_last_raw = 0;             /**< Previous raw period, 0 = none */
static Uint16 pump_fb_stable = 0;               /**< Consecutive periods that matched the previous one */

/* CAN pump feedback (pumpData_read is called from the eCAN ISR) */
static pump_rx_t pump_debug_cmd;                    /**< Last 0x268 frame (watch window) */
static volatile Uint16 pump_can_fb_enabled = 0;     /**< 1 = pump ON: 0x268 frames are used */

#if defined(PWM_PUMP)
static Uint16 tbprd;                            /**< ePWM2 period register value */
static Uint16 desired_freq = 500;               /**< PWM frequency, Hz (DC45A input: 50-800 Hz) */
static Uint16 pump_speed = 0;                   /**< PWM pump duty now, % */
static Uint16 pump_rpm = 0;                     /**< Measured pump speed, rpm (watch window) */
static Uint16 pump_dry_run = 0;                 /**< 1 = pump running dry (no water) */
static Uint16 pump_dry_active = 0;              /**< 1 = speed is above PUMP_DRY_RPM */
static Uint32 pump_dry_start = 0;               /**< CpuTimer0 tick when it went above */
static Uint16 pump_no_feedback = 0;             /**< 1 = pump ON but no speed feedback */
static Uint16 pump_nofb_active = 0;             /**< 1 = pump ON and too slow */
static Uint32 pump_nofb_start = 0;              /**< CpuTimer0 tick when that started */
#endif

#if defined(CAN_PUMP)
static pump_tx_t pump_cntrl_cmd;                /**< Last 0x258 command */
static Uint16 pump_can_dry_run = 0;             /**< 1 = pump reports dry running for 3 s */
static Uint16 pump_can_error = 0;               /**< 1 = pump reports over current / voltage / block for 1 s */
static Uint16 pump_can_no_feedback = 0;         /**< 1 = 0x268 missing / not running / speed 0 for 5 s */
static Uint16 pump_can_dry_active = 0;          /**< 1 = dry running bit set now */
static Uint16 pump_can_err_active = 0;          /**< 1 = error bit set now */
static Uint16 pump_can_nofb_active = 0;         /**< 1 = no feedback now */
static Uint32 pump_can_dry_start = 0;           /**< CpuTimer0 tick when the dry running bit was set */
static Uint32 pump_can_err_start = 0;           /**< CpuTimer0 tick when the error bit was set */
static Uint32 pump_can_nofb_start = 0;          /**< CpuTimer0 tick when the feedback was lost */
#endif

/* Public functions ----------------------------------------------------------*/
/**
 * @brief  Set up eCAP2 on GPIO15 to measure the PWM pump speed feedback.
 * @details Schematic: PUMP_FB -> U2 comparator -> R18 -> C2000_FEEDBACK -> GPIO15/ECAP2.
 *          eCAP2 counts SYSCLK (90 MHz) in continuous delta mode: each rising edge loads
 *          CAP1 with the time since the previous rising edge, then resets the counter.
 * @see    Pump_PWM.h
 */
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

/**
 * @brief  Store a received 0x268 CAN pump feedback frame.
 * @see    Pump_PWM.h
 */
void pumpData_read(Uint16 *pump_data_read)
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

/**
 * @brief  Switch the pump ON / OFF and run the pump fault detection.
 * @see    Pump_PWM.h
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
    pumpData_write(canMap.msg[CAN_OBJ_24].sendData, on ? PUMP_CMD_ON : PUMP_CMD_OFF);
    CAN_sendMessage(CAN_OBJ_24, NOMINAL_DLC, canMap.msg[CAN_OBJ_24].sendData);
    if (on)
    {
        pump_can_update();                      // dry run / pump error / no feedback from 0x268
    }
#else
    (void)on;                                   // no pump in this build
#endif
}

/**
 * @brief  Check the pump feedback.
 * @see    Pump_PWM.h
 */
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

/**
 * @brief  Check for a pump fault and record it.
 * @see    Pump_PWM.h
 */
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

/* Interrupt service routines ------------------------------------------------*/
/**
 * @brief  eCAP2 capture interrupt: one rising edge of the PWM pump speed feedback.
 * @details Noise rejection: a real pump gives a steady pulse train, noise (open FB wire, mains
 *          hum, PWM coupling) gives periods that jump around. A period is accepted only after
 *          PUMP_FB_STABLE_EDGES periods in a row, each within 1/2^PUMP_FB_STABLE_TOL_SHIFT of the
 *          previous one. Only accepted periods refresh pump_fb_tick, so with noise only the speed
 *          times out to 0 after PUMP_FB_PWM_TIMEOUT_TICKS -> no-feedback fault.
 * @see    Pump_PWM.h
 */
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

/* Private functions ---------------------------------------------------------*/
#if defined(PWM_PUMP)
/**
 * @brief   Configure ePWM2 for the PWM pump: 500 Hz, up-count, given duty.
 * @param[in] pumpSpeed  Duty cycle, % (0 = pump OFF).
 * @note    TBCLK = 90 MHz / (HSPCLKDIV 2 x CLKDIV 2) = 22.5 MHz, f = TBCLK / (TBPRD + 1).
 *          500 Hz -> TBPRD 44999. Valid frequency with this TBCLK: ~344 Hz and up.
 */
static void InitEPwmTimer(Uint8 pumpSpeed)
{
        EALLOW; // Allow protected register writes
        SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC =0 ; // Disable time-base clock
        EDIS; // Disallow protected register writes

        // Calculate period register (TBPRD) based on desired frequency
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
       EPwm2Regs.AQSFRC.bit.OTSFA = 1; // One-time software force action A
       EPwm2Regs.AQSFRC.bit.ACTSFA = AQ_CLEAR; // Force PWM output low

        // Enable TBCLK within the ePWM module
       EALLOW;
       SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
       EDIS;
}

/**
 * @brief   Measured PWM pump speed.
 * @return  Speed, rpm. 0 when no pulse was accepted for PUMP_FB_PWM_TIMEOUT_TICKS.
 * @note    Call from the main loop. Also refreshes pump_rpm.
 */
static Uint16 pump_speed_rpm(void)
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

/**
 * @brief   Check the PWM pump feedback.
 * @retval  1  Speed >= PUMP_FB_MIN_RPM (eCAP2 feedback fresh) and not running dry.
 * @retval  0  Too slow, no feedback or dry.
 */
static Uint16 pump_pwm_feedback_ok(void)
{
    return ((pump_speed_rpm() >= PUMP_FB_MIN_RPM) && !pump_dry_run) ? 1 : 0;
}

/**
 * @brief   Dry-run detection for the PWM pump.
 * @return  pump_dry_run: 1 when pump_rpm has been > PUMP_DRY_RPM for PUMP_DRY_DETECT_TICKS.
 * @note    Clears by itself as soon as the speed is back at or below PUMP_DRY_RPM.
 *          The cooling state machine latches the fault and stops the pump.
 */
static Uint16 pump_dry_update(void)
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

/**
 * @brief   No-feedback detection for the PWM pump.
 * @param[in] pump_on  1 = pump commanded ON.
 * @return  pump_no_feedback: 1 when the pump has been ON with pump_rpm < PUMP_FB_MIN_RPM
 *          for PUMP_NOFB_DETECT_TICKS.
 * @pre     pump_dry_update() was called first (it refreshes pump_rpm).
 */
static Uint16 pump_nofb_update(Uint16 pump_on)
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

/**
 * @brief   Turn the eCAP2 speed feedback ON / OFF together with the PWM pump.
 * @param[in] on  1 = start fresh (first edge discarded) and enable the capture interrupt,
 *                0 = disable the interrupt and clear speed and fault flags.
 * @note    Call when the pump duty changes.
 */
static void pump_feedback_enable(Uint16 on)
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
#endif /* PWM_PUMP */

#if defined(CAN_PUMP)
/**
 * @brief   Fill the 0x258 CAN pump command bytes.
 * @param[out] pump_data_write  8 data bytes of the frame, one byte per element.
 * @param[in]  pump_status      PUMP_CMD_ON / PUMP_CMD_OFF.
 */
static void pumpData_write(Uint16 *pump_data_write, pump_state_t pump_status)
{
    pump_cntrl_cmd.pump_power = pump_status;
    pump_cntrl_cmd.target_duty.all = PUMP_CAN_DUTY;

    pump_data_write[0] = pump_cntrl_cmd.target_duty.byte.DUTY_BYTE1;
    pump_data_write[1] = pump_cntrl_cmd.target_duty.byte.DUTY_BYTE2;
    pump_data_write[2] = pump_cntrl_cmd.pump_power;
    pump_data_write[3] = 0;
    pump_data_write[4] = 0;
    pump_data_write[5] = 0;
    pump_data_write[6] = 0;
    pump_data_write[7] = 0;
}

/**
 * @brief   Check the CAN pump feedback 0x268.
 * @retval  1  Fresh 0x268 says running, no fault bits, speed > 0.
 * @retval  0  No feedback yet, feedback lost, pump off / fault, or not turning.
 */
static Uint16 pump_feedback_ok(void)
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

/**
 * @brief   Debounce a condition over real time.
 * @param[in]     cond    Condition now.
 * @param[in,out] active  1 while cond is true (internal state).
 * @param[in,out] start   CpuTimer0 tick when cond became true (internal state).
 * @param[in]     ticks   Time cond must hold, CpuTimer0 ticks.
 * @retval  1  cond has been true for ticks.
 * @retval  0  cond false, or not yet true long enough.
 */
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

/**
 * @brief   CAN pump fault detection from 0x268. Call while the pump is ON.
 * @details pump_can_dry_run     : dry_running bit set                               for PUMP_DRY_DETECT_TICKS (3 s)
 *          pump_can_error       : over current / voltage / block, or state = fault  for PUMP_CAN_ERR_DETECT_TICKS (1 s)
 *          pump_can_no_feedback : 0x268 missing, not RUNNING or speed 0             for PUMP_NOFB_DETECT_TICKS (5 s)
 * @note    The cooling state machine latches the fault and stops the pump.
 */
static void pump_can_update(void)
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

/**
 * @brief   Turn the CAN pump feedback ON / OFF together with the pump.
 * @param[in] on  1 = start fresh (old 0x268 data not used, wait for new frames),
 *                0 = ignore 0x268 frames and clear the fault flags.
 * @note    Call when the pump is switched ON or OFF.
 */
static void pump_can_feedback_enable(Uint16 on)
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
#endif /* CAN_PUMP */

/* end of file */
