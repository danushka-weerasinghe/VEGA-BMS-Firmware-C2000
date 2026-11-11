/**
 * @file    GBT_data.h
 * @brief   GB/T 27930 charging data: BMS values collected into GBT_Data_i in GB/T units.
 * @details GBT_Data_update() fills GBT_Data_i from Controller, PDU_getData_i and BMS_data_IC.
 *          The CAN frames are built in PDU_matrix.c (PDU_gbt_Data_write()).
 *          Battery parameters follow the pack selected in main_data.h.
 *          Only built when GBT_CHARGING = 1 for the selected pack (main_data.h).
 * @author  Danushka Weerasinghe
 * @date    2026-10-01
 */

#ifndef HEADERS_GBT_DATA_H_
#define HEADERS_GBT_DATA_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>                 /* uint16_t (used by the files that include this header) */
#include "DSP28x_Project.h"         /* Uint16, int16 */
#include "main_data.h"              /* GBT_CHARGING, SERIES_CELLS, TOTAL_IC, TEMP_PER_IC */

#if (GBT_CHARGING == 1)

/* Public defines ------------------------------------------------------------*/
/* Array sizes */
#define GBT_SERIES_CELLS            SERIES_CELLS                /**< Series cells in the pack (cell_voltage[] size) */
#define GBT_TEMP_COUNT              (TOTAL_IC * TEMP_PER_IC)    /**< All BMS temperature probes (temp[] size) */

/* Invalid markers */
#define GBT_CELL_V_INVALID          0xFFFF  /**< cell_voltage[]: no cell at this position */
#define GBT_TEMP_INVALID            0xFF    /**< temp[]: no temperature probe at this position */

/* CAN frame index layout (PDU_matrix.c, Battery_controller.c) */
#define GBT_SETUP_FRAME_COUNT       3       /**< Index 0..2 = setup data, sent once per EVCC request */
#define GBT_STATUS_FRAME_COUNT      5       /**< Index 0..4 = setup 0..2 + live 3..4 */
#define GBT_CELLS_PER_FRAME         3       /**< Index 5.. = cell voltages, 3 per frame */
#define GBT_TEMPS_PER_FRAME         1       /**< Index 5.. = temperature, 1 per frame (byte 7) */

/* Status value aliases */
#define GBT_STS_ABNORMAL            GBT_STS_OVER_HIGH   /**< "Abnormal" for insulation / contactor */
#define GBT_STS_UNTRUSTED           GBT_STS_BELOW_LOW   /**< "Untrusted" for current / temp / insulation / contactor */

/* Public enums --------------------------------------------------------------*/
/**
 * @brief Battery type codes (GB/T 27930), GBT_Data.battery_type.
 */
typedef enum
{
    GBT_BATT_LEAD_ACID      = 0x01, /**< Lead acid */
    GBT_BATT_NIMH           = 0x02, /**< Nickel metal hydride */
    GBT_BATT_LFP            = 0x03, /**< Lithium iron phosphate */
    GBT_BATT_LMO            = 0x04, /**< Lithium manganese oxide */
    GBT_BATT_LCO            = 0x05, /**< Lithium cobalt oxide */
    GBT_BATT_TERNARY        = 0x06, /**< Ternary (NMC) */
    GBT_BATT_POLYMER_LI_ION = 0x07, /**< Lithium polymer */
    GBT_BATT_LI_TITANATE    = 0x08, /**< Lithium titanate */
    GBT_BATT_OTHER          = 0xFF  /**< Other */
} GBT_battery_type;

/**
 * @brief 2-bit status values used in status_A / status_B / status_C.
 * @note  The sheet's "0x10" is binary 10 = 2.
 */
typedef enum
{
    GBT_STS_NORMAL      = 0x0,      /**< Normal */
    GBT_STS_OVER_HIGH   = 0x1,      /**< Over high, also "abnormal" for insulation / contactor */
    GBT_STS_BELOW_LOW   = 0x2,      /**< Below low, also "untrusted" for current / temp / insulation / contactor */
    GBT_STS_RESERVED    = 0x3       /**< Reserved */
} GBT_status_val;

/* Public structs ------------------------------------------------------------*/
/**
 * @brief Status byte A bits, each GBT_status_val.
 */
typedef struct GBT_status_A_struct
{
    Uint16 cell_voltage_state   : 2;    /**< Pack cell voltage: normal / over high / below low */
    Uint16 soc_state            : 2;    /**< SOC: normal / over high / below low */
    Uint16 charge_over_current  : 2;    /**< Charging current: normal / over high / untrusted */
    Uint16 battery_temp_state   : 2;    /**< Battery temperature: normal / over high / untrusted */
} gbt_status_a_bits_t;

/**
 * @brief Status byte A, whole or by bit.
 */
typedef union GBT_status_A_Union
{
    Uint16 all;                         /**< Whole byte */
    struct GBT_status_A_struct bit;     /**< Single states */
} gbt_status_a_t;

/**
 * @brief Status byte B bits, each GBT_status_val.
 */
typedef struct GBT_status_B_struct
{
    Uint16 insulation_state     : 2;    /**< Insulation: normal / abnormal / untrusted (not implemented: normal) */
    Uint16 single_cell_voltages : 2;    /**< Single cell voltages (not set, 0 = normal) */
    Uint16 single_cell_temps    : 2;    /**< Single cell temperatures (not set, 0 = normal) */
    Uint16 bms_temp_state       : 2;    /**< BMS (MCU) temperature: normal / over high / untrusted */
} gbt_status_b_bits_t;

/**
 * @brief Status byte B, whole or by bit.
 */
typedef union GBT_status_B_Union
{
    Uint16 all;                         /**< Whole byte */
    struct GBT_status_B_struct bit;     /**< Single states */
} gbt_status_b_t;

/**
 * @brief Status byte C bits, each GBT_status_val.
 */
typedef struct GBT_status_C_struct
{
    Uint16 fc_contactor_state   : 2;    /**< Charge contactor: normal / abnormal */
    Uint16 reserved             : 6;    /**< Not used */
} gbt_status_c_bits_t;

/**
 * @brief Status byte C, whole or by bit.
 */
typedef union GBT_status_C_Union
{
    Uint16 all;                         /**< Whole byte */
    struct GBT_status_C_struct bit;     /**< Single states */
} gbt_status_c_t;

/**
 * @brief GB/T charging data, all values in GB/T units.
 */
typedef struct GBT_Data
{
    /* static parameters */
    Uint16 max_charge_voltage;          /**< Max pack charge voltage, 0.1 V/bit */
    Uint16 max_cell_charge_voltage;     /**< Max cell charge voltage, 0.01 V/bit */
    Uint16 battery_type;                /**< GBT_battery_type */
    Uint16 rated_voltage;               /**< Rated pack voltage, 0.1 V/bit */
    Uint16 total_energy;                /**< Rated energy, 0.1 kWh/bit */
    int16  max_permit_temp;             /**< Max permitted temperature, 0.1 degC/bit */
    Uint16 rated_capacity;              /**< Rated capacity, 0.1 Ah/bit (nominal_capacity) */
    Uint16 max_charge_current;          /**< Max charge current, 0.1 A/bit, positive (CHARGE_CURRENT_THRESHOLD) */

    /* live data */
    Uint16 highest_cell_id;             /**< Highest voltage cell, 1-based */
    Uint16 lowest_cell_id;              /**< Lowest voltage cell, 1-based */
    Uint16 highest_temp_probe;          /**< Highest temperature probe, 1-based */
    int16  lowest_temp;                 /**< Lowest temperature, 0.1 degC/bit */
    Uint16 lowest_temp_probe;           /**< Lowest temperature probe, 1-based */

    union GBT_status_A_Union status_A;  /**< Status byte A */
    union GBT_status_B_Union status_B;  /**< Status byte B */
    union GBT_status_C_Union status_C;  /**< Status byte C */

    /* all cell voltages, cell 1 = cell_voltage[0] */
    Uint16 cell_count;                      /**< Cells used in cell_voltage[] */
    Uint16 cell_voltage[GBT_SERIES_CELLS];  /**< Cell voltages, 0.01 V/bit */

    /* all temperatures, probe 1 = temp[0] (IC 0 probe 0, IC 0 probe 1, ... IC 1 probe 0, ...) */
    Uint16 temp_count;                      /**< Probes in temp[] */
    Uint16 temp[GBT_TEMP_COUNT];            /**< Temperatures, 1 degC/bit, as BMS_data_IC[].temparature_val[] */
} gbt_data_t;

/* Extern declarations -------------------------------------------------------*/
/** @brief GB/T charging data, filled by GBT_Data_init() / GBT_Data_update(), sent by PDU_matrix.c. */
extern struct GBT_Data GBT_Data_i;

/* Public function prototypes ------------------------------------------------*/
/**
 * @brief   Initialise GBT_Data_i: static parameters, invalid markers and the cell map.
 * @note    Call once at start-up, after init_nominal_capacity() and init_current_limits().
 */
void GBT_Data_init(void);

/**
 * @brief   Refresh the live GB/T data: cell / temperature ids, ratings, status bytes,
 *          cell voltages and temperatures.
 * @note    Call after PDU_getData_i is updated (timer_task_can).
 */
void GBT_Data_update(void);

#endif /* GBT_CHARGING */

#endif /* HEADERS_GBT_DATA_H_ */
