/*
 * soc_error_check.h
 *
 *  SOC estimation error detection (model-compensated weakest-cell voltage)
 *
 *  Created on: Oct 27, 2026
 *      Author: Harshana
 *
 * ---------------------------------------------------------------------------
 *  PURPOSE
 *    Detect when the BMS SOC is clearly too high compared with the real state
 *    of the weakest cell, WITHOUT false alarms during acceleration.
 *
 *  WHY NOT JUST "Vmin < 3.15 V"?
 *    Under load the cell voltage drops by  I*R0 + polarisation(R1,R2), and on
 *    this LFP cell the polarisation recovers slowly (field data: 41 % after
 *    1 min, 79 % after 5 min). At 30 % SOC a 100 A climb pulls the weakest cell
 *    below 3.15 V although its resting voltage is ~3.27 V.
 *
 *  METHOD
 *    1. Estimate the resting (open-circuit) voltage of the weakest cell with
 *       a 2-RC equivalent circuit model:
 *           V_ocv = Vmin + I*R0 + I_f1*R1 + I_f2*R2
 *       I_f1 / I_f2 = cell current low-pass filtered with tau1 / tau2.
 *    2. Mismatch = V_ocv below the limit while BMS SOC is above soc_min.
 *    3. Accumulate mismatch time (up), decay it faster when the condition is
 *       false (down). Error when accumulated time >= trip_time_s.
 *       "Suspect" (debug only) at half the trip time.
 *    4. Error is latched until charging, as in the original logic.
 *
 *  CALIBRATION
 *    SOC_ERR_CFG_ETX10_LFP_REPT135 is calibrated for the ETX 10 kWh pack
 *    (23S1P REPT 135 Ah LFP) from the HPPC test (ecm_parameters_vs_soc.csv)
 *    and 5,648 field drive windows (11 vehicles, Mar-Jul 2026). Replay of the
 *    fleet logs: old logic 85 trips on 8 vehicles, this logic 5 trips, all on
 *    the one vehicle with a confirmed SOC error.
 *    Other packs/chemistries need their own config (R values, OCV limit).
 *
 *  INTEGRATION (example - Battery_controller.c, SOC loop)
 *
 *      #include "soc_error_check.h"
 *      static soc_err_t soc_err;
 *
 *      // once at start-up
 *      soc_err_init(&soc_err, &SOC_ERR_CFG_ETX10_LFP_REPT135);
 *
 *      // every SOC loop pass; dt_s = REAL elapsed time since the last call
 *      soc_err_update(&soc_err,
 *                     (float)Current_value * 0.01f,        // pack current, A, discharge +
 *                     Controller.lowest_cell_volt,          // 0.1 mV units
 *                     Controller.SOC_value,                 // %
 *                     dt_s,
 *                     (PDU_setData_local.fixSetS.bit.EVCU_State == 4)); // charging
 *      PDU_getData_local.fixSetG.bit.soc_estimation_error = soc_err_get_error(&soc_err);
 *
 *    Call it on EVERY pass, also while charging: the current filters must
 *    keep running so the polarisation estimate stays correct.
 *    Optional debug CAN: soc_err_get_v_ocv_mV(), soc_err_get_suspect().
 * ---------------------------------------------------------------------------
 */

#ifndef HEADERS_SOC_ERROR_CHECK_H_
#define HEADERS_SOC_ERROR_CHECK_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- configuration (one per pack type) ---- */
typedef struct
{
    float    r0_ohm;          /* cell ohmic resistance (instant)                  */
    float    r1_ohm;          /* cell fast polarisation resistance                */
    float    tau1_s;          /* fast polarisation time constant                  */
    float    r2_ohm;          /* cell slow polarisation resistance                */
    float    tau2_s;          /* slow polarisation time constant                  */
    float    n_parallel;      /* cells in parallel (cell I = pack I / n_parallel) */
    float    v_ocv_limit_V;   /* mismatch if estimated OCV is below this ...      */
    float    soc_min_pct;     /* ... while BMS SOC is above this                  */
    float    trip_time_s;     /* confirmed mismatch time before error             */
    float    recover_ratio;   /* decay speed when condition is false (x up rate)  */
} soc_err_cfg_t;

/* ---- run-time state (one per BMS) ---- */
typedef struct
{
    const soc_err_cfg_t *cfg;
    float    i_f1_A;          /* cell current, low-pass tau1                      */
    float    i_f2_A;          /* cell current, low-pass tau2                      */
    float    mismatch_s;      /* accumulated mismatch time                        */
    float    v_ocv_V;         /* last estimated OCV of the weakest cell           */
    uint16_t suspect;         /* 1 = mismatch building up (debug only)            */
    uint16_t error;           /* 1 = SOC estimation error (latched)               */
} soc_err_t;

/* ETX 10 kWh - 23S1P REPT 135 Ah LFP (calibrated 27-Oct-2026) */
extern const soc_err_cfg_t SOC_ERR_CFG_ETX10_LFP_REPT135;

void     soc_err_init(soc_err_t *s, const soc_err_cfg_t *cfg);

void     soc_err_update(soc_err_t *s,
                        float    pack_current_A,   /* discharge positive          */
                        uint16_t vmin_01mV,        /* weakest cell, 0.1 mV units  */
                        float    soc_pct,          /* BMS SOC, %                  */
                        float    dt_s,             /* elapsed time since last call*/
                        uint16_t charging);        /* 1 = charging (clears error) */

void     soc_err_clear(soc_err_t *s);              /* clear error + timer only    */

uint16_t soc_err_get_error(const soc_err_t *s);
uint16_t soc_err_get_suspect(const soc_err_t *s);
uint16_t soc_err_get_v_ocv_mV(const soc_err_t *s); /* for debug CAN               */

#ifdef __cplusplus
}
#endif

#endif /* HEADERS_SOC_ERROR_CHECK_H_ */
