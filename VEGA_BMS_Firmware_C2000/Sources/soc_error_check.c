/*
 * soc_error_check.c
 *
 *  SOC estimation error detection (model-compensated weakest-cell voltage)
 *  See soc_error_check.h for the method, calibration and integration.
 *
 *  Created on: Oct 27, 2026
 *      Author: Harshana
 */

#include "soc_error_check.h"

/* Readings outside this window are treated as invalid (sensor/comms fault). */
#define SOC_ERR_VMIN_VALID_LO   20000u   /* 2.000 V */
#define SOC_ERR_VMIN_VALID_HI   45000u   /* 4.500 V */

/*
 * ETX 10 kWh - 23S1P REPT 135 Ah LFP
 *  R0, R1, R2, tau : HPPC 2-RC fit (discharge, 10-40 % SOC), confirmed by field
 *                    fits: R0 0.44-0.49 mOhm, R1 0.63-0.71 mOhm (< 40 % SOC)
 *  3.150 V         : discharge OCV at 4.6 % SOC (20-min rest OCV test, Sep 2026)
 *  60 s / x2       : best trade-off in fleet replay (30 s gave false trips,
 *                    result unchanged for R +/-30 %)
 */
const soc_err_cfg_t SOC_ERR_CFG_ETX10_LFP_REPT135 =
{
    0.00047f,   /* r0_ohm        */
    0.00060f,   /* r1_ohm        */
    60.0f,      /* tau1_s        */
    0.00038f,   /* r2_ohm        */
    900.0f,     /* tau2_s        */
    1.0f,       /* n_parallel    */
    3.150f,     /* v_ocv_limit_V */
    20.0f,      /* soc_min_pct   */
    60.0f,      /* trip_time_s   */
    2.0f        /* recover_ratio */
};

void soc_err_init(soc_err_t *s, const soc_err_cfg_t *cfg)
{
    s->cfg        = cfg;
    s->i_f1_A     = 0.0f;
    s->i_f2_A     = 0.0f;
    s->mismatch_s = 0.0f;
    s->v_ocv_V    = 0.0f;
    s->suspect    = 0u;
    s->error      = 0u;
}

void soc_err_clear(soc_err_t *s)
{
    s->mismatch_s = 0.0f;
    s->suspect    = 0u;
    s->error      = 0u;
}

void soc_err_update(soc_err_t *s, float pack_current_A, uint16_t vmin_01mV,
                    float soc_pct, float dt_s, uint16_t charging)
{
    const soc_err_cfg_t *c = s->cfg;
    float i_cell;
    float a1, a2;

    if ((c == 0) || (dt_s <= 0.0f))
    {
        return;
    }

    /* 1. Polarisation filters - always running (also while charging).
     *    alpha = dt / (tau + dt): stable for any dt; after a long gap the
     *    filter simply settles to the present current (cell has relaxed). */
    i_cell = pack_current_A / c->n_parallel; // find the current for a single cell
    a1 = dt_s / (c->tau1_s + dt_s); // calculate low polarization filter coefficient
    a2 = dt_s / (c->tau2_s + dt_s);
    s->i_f1_A += (i_cell - s->i_f1_A) * a1;
    s->i_f2_A += (i_cell - s->i_f2_A) * a2;

    /* Charging: clear, as the original logic does (EVCU_State == 4). */
    if (charging)
    {
        soc_err_clear(s);
        return;
    }

    /* Invalid cell reading: keep state, do not judge. */
    if ((vmin_01mV < SOC_ERR_VMIN_VALID_LO) || (vmin_01mV > SOC_ERR_VMIN_VALID_HI))
    {
        return;
    }

    /* 2. Estimated resting voltage of the weakest cell (2-RC model). */
    s->v_ocv_V = (float)vmin_01mV * 0.0001f
               + i_cell    * c->r0_ohm
               + s->i_f1_A * c->r1_ohm
               + s->i_f2_A * c->r2_ohm;

    if (s->error)
    {
        return;                                /* latched until charging */
    }

    /* 3. Mismatch time: up while true, down (faster) while false. */
    if ((s->v_ocv_V < c->v_ocv_limit_V) && (soc_pct > c->soc_min_pct))
    {
        s->mismatch_s += dt_s;
    }
    else
    {
        s->mismatch_s -= c->recover_ratio * dt_s;
        if (s->mismatch_s < 0.0f)
        {
            s->mismatch_s = 0.0f;
        }
    }

    s->suspect = (s->mismatch_s >= 0.5f * c->trip_time_s) ? 1u : 0u;

    if (s->mismatch_s >= c->trip_time_s)
    {
        s->error      = 1u;
        s->mismatch_s = c->trip_time_s;
    }
}

uint16_t soc_err_get_error(const soc_err_t *s)
{
    return s->error;
}

uint16_t soc_err_get_suspect(const soc_err_t *s)
{
    return s->suspect;
}

uint16_t soc_err_get_v_ocv_mV(const soc_err_t *s)
{
    float mv = s->v_ocv_V * 1000.0f;
    if (mv < 0.0f)     { mv = 0.0f; }
    if (mv > 65535.0f) { mv = 65535.0f; }
    return (uint16_t)(mv + 0.5f);
}
