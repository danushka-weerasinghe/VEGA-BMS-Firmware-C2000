/*
 * GBT_data.h
 *
 *  GB/T 27930 charging data collection and calculation.
 *  Collects BMS data into GBT_Data_i in GB/T units.
 *  CAN frames are built in PDU_matrix.c (PDU_gbt_Data_write).
 *  Battery parameters follow the pack selected in main_data.h.
 *  Only built when GBT_CHARGING = 1 for the selected pack (main_data.h).
 */

#ifndef HEADERS_GBT_DATA_H_
#define HEADERS_GBT_DATA_H_

#include "DSP28x_Project.h"
#include "main_data.h"
#include <stdint.h>

#if (GBT_CHARGING == 1)

/* Battery type codes (GB/T 27930) */
typedef enum
{
    GBT_BATT_LEAD_ACID      = 0x01,
    GBT_BATT_NIMH           = 0x02,
    GBT_BATT_LFP            = 0x03,
    GBT_BATT_LMO            = 0x04,
    GBT_BATT_LCO            = 0x05,
    GBT_BATT_TERNARY        = 0x06,
    GBT_BATT_POLYMER_LI_ION = 0x07,
    GBT_BATT_LI_TITANATE    = 0x08,
    GBT_BATT_OTHER          = 0xFF
} GBT_battery_type;

// ---- Per battery pack values (select pack in main_data.h) ----
//  Rated capacity and max charging current come from eprom_config.c
//  (nominal_capacity, CHARGE_CURRENT_THRESHOLD - EEPROM / CAN configurable).
//  GBT_SERIES_CELLS       - number of cells in series
//  GBT_CELL_UNUSED(ic, c) - true for unused cell positions (same as analyze_rdata)

#if defined(ATV_3_MODULE)

#define GBT_SERIES_CELLS        21      // 3 x 7S
#define GBT_CELL_UNUSED(ic, c)  (((c) > 3) && ((c) < 6))

#elif defined(BIKE_2_MODULE_TENPOWER)

#define GBT_SERIES_CELLS        20      // 2 x 10S
#define GBT_CELL_UNUSED(ic, c)  ((c) == 5)

#elif defined(BIKE_3_MODULE_MOLICELL)

#define GBT_SERIES_CELLS        21      // 3 x 7S
#define GBT_CELL_UNUSED(ic, c)  (((c) > 3) && ((c) < 6))

#elif defined(BIKE_3_MODULE_TENPOWER)

#define GBT_SERIES_CELLS        21      // 3 x 7S
#define GBT_CELL_UNUSED(ic, c)  (((c) > 3) && ((c) < 6))

#elif defined(ETX_10_kWH)

#define GBT_SERIES_CELLS        23
#define GBT_CELL_UNUSED(ic, c)  (((c) == 11) && ((ic) == 1))

#elif defined(ETX_12_kWH)

#define GBT_SERIES_CELLS        22
#define GBT_CELL_UNUSED(ic, c)  (0)

#endif

// ---- Per chemistry values ----
//  GBT_NOMINAL_CELL_V     - nominal cell voltage, 0.01 V/bit

#if defined(CELL_CHEM_LFP)
#define GBT_BATTERY_TYPE        GBT_BATT_LFP
#define GBT_NOMINAL_CELL_V      320
#define GBT_CELL_V_OVER_HIGH    360     // 3.60 V
#define GBT_CELL_V_BELOW_LOW    280     // 2.80 V
#elif defined(CELL_CHEM_NMC)
#define GBT_BATTERY_TYPE        GBT_BATT_TERNARY
#define GBT_NOMINAL_CELL_V      360
#define GBT_CELL_V_OVER_HIGH    420     // 4.20 V
#define GBT_CELL_V_BELOW_LOW    280     // 2.80 V
#endif

#define GBT_BMS_TEMP_LIMIT      85      // Temperature from the Humidity sensor

/* BMS temperature state (temperature = Mcu_Temp(), 1 degC/bit)
 *   temperature > GBT_BMS_TEMP_LIMIT                     -> GBT_STS_OVER_HIGH
 *   temperature outside VALID_MIN .. VALID_MAX           -> GBT_STS_UNTRUSTED */
#define GBT_BMS_TEMP_VALID_MIN  (-40)   // degC, lowest real MCU reading
#define GBT_BMS_TEMP_VALID_MAX  150     // degC, highest real MCU reading

/* SOC state levels, 1 %/bit
 *   soc >  GBT_SOC_OVER_HIGH -> GBT_STS_OVER_HIGH
 *   soc <  GBT_SOC_BELOW_LOW -> GBT_STS_BELOW_LOW
 *   otherwise                -> GBT_STS_NORMAL */
#define GBT_SOC_OVER_HIGH       70      // %
#define GBT_SOC_BELOW_LOW       20      // %


// ---- Derived values, GB/T units (do not edit) ----

#define GBT_MAX_CELL_CHG_VOLTAGE    (HIGHEST_CELL_VOLTAGE_LIMIT / 100)                     // 0.01 V/bit
#define GBT_MAX_CHG_VOLTAGE         ((GBT_MAX_CELL_CHG_VOLTAGE * GBT_SERIES_CELLS) / 10)   // 0.1 V/bit
#define GBT_RATED_VOLTAGE           ((GBT_NOMINAL_CELL_V * GBT_SERIES_CELLS) / 10)         // 0.1 V/bit
#define GBT_MAX_PERMIT_TEMP         (HIGHEST_TEMPERATURE_LIMIT * 10)                       // 0.1 degC/bit

#define GBT_CELL_V_INVALID          0xFFFF  // no cell at this position
#define GBT_TEMP_INVALID            0xFF    // no temperature probe at this position

#define GBT_TEMP_COUNT              (TOTAL_IC * TEMP_PER_IC)    // all BMS temperature probes

#define GBT_STATUS_FRAME_COUNT      4       // index 0..3
#define GBT_CELLS_PER_FRAME         3       // index 4.. = cell voltages
#define GBT_TEMPS_PER_FRAME         1       // index 4.. = temperature, byte 7


/* 2-bit status values (sheet's "0x10" is binary 10 = 2) */
typedef enum
{
    GBT_STS_NORMAL      = 0x0,
    GBT_STS_OVER_HIGH   = 0x1,      // also "abnormal" for insulation / contactor
    GBT_STS_BELOW_LOW   = 0x2,      // also "untrusted" for current / temp / insulation / contactor
    GBT_STS_RESERVED    = 0x3
} GBT_status_val;

#define GBT_STS_ABNORMAL    GBT_STS_OVER_HIGH
#define GBT_STS_UNTRUSTED   GBT_STS_BELOW_LOW

struct GBT_status_A_struct
{
    Uint16 cell_voltage_state   : 2;
    Uint16 soc_state            : 2;
    Uint16 charge_over_current  : 2;
    Uint16 battery_temp_state   : 2;
};

union GBT_status_A_Union
{
    Uint16 all;
    struct GBT_status_A_struct bit;
};

struct GBT_status_B_struct
{
    Uint16 insulation_state     : 2;
    Uint16 single_cell_voltages : 2;
    Uint16 single_cell_temps    : 2;
    Uint16 bms_temp_state       : 2;
};

union GBT_status_B_Union
{
    Uint16 all;
    struct GBT_status_B_struct bit;
};

struct GBT_status_C_struct
{
    Uint16 fc_contactor_state   : 2;
    Uint16 reserved             : 6;
};

union GBT_status_C_Union
{
    Uint16 all;
    struct GBT_status_C_struct bit;
};

/* All values in GB/T units */
struct GBT_Data
{
    /* static parameters */
    Uint16 max_charge_voltage;          // 0.1 V/bit
    Uint16 max_cell_charge_voltage;     // 0.01 V/bit
    Uint16 battery_type;                // GBT_battery_type
    Uint16 rated_voltage;               // 0.1 V/bit
    Uint16 total_energy;                // 0.1 kWh/bit
    int16  max_permit_temp;             // 0.1 degC/bit

    /* live data */

    Uint16 highest_cell_id;             // 1-based
    Uint16 lowest_cell_id;              // 1-based
    Uint16 highest_temp_probe;
    int16  lowest_temp;                 // 0.1 degC/bit
    Uint16 lowest_temp_probe;

    union GBT_status_A_Union status_A;
    union GBT_status_B_Union status_B;
    union GBT_status_C_Union status_C;

    /* all cell voltages, cell 1 = cell_voltage[0] */
    Uint16 cell_count;
    Uint16 cell_voltage[GBT_SERIES_CELLS];  // 0.01 V/bit

    /* all temperatures, probe 1 = temp[0] (IC 0 probe 0, IC 0 probe 1, ... IC 1 probe 0, ...) */
    Uint16 temp_count;
    Uint16 temp[GBT_TEMP_COUNT];            // 1 degC/bit, as BMS_data_IC[].temparature_val[]

};

extern struct GBT_Data GBT_Data_i;

void GBT_Data_init(void);
void GBT_Data_update(void);

#endif /* GBT_CHARGING */

#endif /* HEADERS_GBT_DATA_H_ */
