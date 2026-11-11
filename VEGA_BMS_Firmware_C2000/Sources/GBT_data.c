/**
 * @file    GBT_data.c
 * @brief   GB/T 27930 charging data: BMS values collected into GBT_Data_i in GB/T units.
 * @details Fills GBT_Data_i from Controller, PDU_getData_i and BMS_data_IC.
 *          The CAN frames are built in PDU_matrix.c (PDU_gbt_Data_write()).
 * @author  Danushka Weerasinghe
 * @date    2026-10-01
 */

/* Own header ----------------------------------------------------------------*/
#include "GBT_data.h"

/* Hardware / vendor includes ------------------------------------------------*/
#include "DSP28x_Project.h"         /* Uint16, int16, Uint32 */

/* Project includes ----------------------------------------------------------*/
#include "main_data.h"              /* pack / chemistry selection, limits, BMS_data_IC, current_val_A */
#include "PDU_matrix.h"             /* PDU_getData_i, PDU_getData_local */
#include "eprom_config.h"           /* nominal_capacity */

#if (GBT_CHARGING == 1)

/* Private defines -----------------------------------------------------------*/
/* Per battery pack values (select pack in main_data.h). Rated capacity and max charging current
   come from eprom_config.c (nominal_capacity, CHARGE_CURRENT_THRESHOLD - EEPROM / CAN configurable) */
#define GBT_CELL_UNUSED(ic, c)      CELL_UNUSED(ic, c)  /**< 1 = no cell at IC ic, position c */

/* Per chemistry values */
#if defined(CELL_CHEM_LFP)
#define GBT_BATTERY_TYPE            GBT_BATT_LFP        /**< GB/T battery type */
#define GBT_NOMINAL_CELL_V          320                 /**< Nominal cell voltage, 0.01 V/bit */
#define GBT_CELL_V_OVER_HIGH        365                 /**< Cell voltage over high above 3.65 V, 0.01 V/bit */
#define GBT_CELL_V_BELOW_LOW        260                 /**< Cell voltage below low under 2.60 V, 0.01 V/bit */
#elif defined(CELL_CHEM_NMC)
#define GBT_BATTERY_TYPE            GBT_BATT_TERNARY    /**< GB/T battery type */
#define GBT_NOMINAL_CELL_V          360                 /**< Nominal cell voltage, 0.01 V/bit */
#define GBT_CELL_V_OVER_HIGH        425                 /**< Cell voltage over high above 4.25 V, 0.01 V/bit */
#define GBT_CELL_V_BELOW_LOW        260                 /**< Cell voltage below low under 2.60 V, 0.01 V/bit */
#endif

/* BMS temperature state (temperature = Mcu_Temp(), 1 degC/bit)
     temperature > GBT_BMS_TEMP_LIMIT                -> GBT_STS_OVER_HIGH
     temperature outside VALID_MIN .. VALID_MAX      -> GBT_STS_UNTRUSTED */
#define GBT_BMS_TEMP_LIMIT          85      /**< BMS temperature over high, degC */
#define GBT_BMS_TEMP_VALID_MIN      (-40)   /**< Lowest real MCU reading, degC */
#define GBT_BMS_TEMP_VALID_MAX      150     /**< Highest real MCU reading, degC */

/* SOC state levels, 1 %/bit
     soc > GBT_SOC_OVER_HIGH -> GBT_STS_OVER_HIGH
     soc < GBT_SOC_BELOW_LOW -> GBT_STS_BELOW_LOW
     otherwise               -> GBT_STS_NORMAL */
#define GBT_SOC_OVER_HIGH           100     /**< SOC over high, % */
#define GBT_SOC_BELOW_LOW           1       /**< SOC below low, % */

/* Communication lost */
#define GBT_COMM_LOST_COUNT         4000    /**< Watchdog count above this = sensor comm lost (value untrusted) */

/* Derived values, GB/T units (do not edit) */
#define GBT_MAX_CELL_CHG_VOLTAGE    (HIGHEST_CELL_VOLTAGE_LIMIT / 100)                     /**< 0.01 V/bit */
#define GBT_MAX_CHG_VOLTAGE         ((GBT_MAX_CELL_CHG_VOLTAGE * GBT_SERIES_CELLS) / 10)   /**< 0.1 V/bit */
#define GBT_RATED_VOLTAGE           ((GBT_NOMINAL_CELL_V * GBT_SERIES_CELLS) / 10)         /**< 0.1 V/bit */
#define GBT_MAX_PERMIT_TEMP         (HIGHEST_TEMPERATURE_LIMIT * 10)                       /**< 0.1 degC/bit */

/* Private function prototypes -----------------------------------------------*/
static void GBT_cellMap_init(void);
static void GBT_cellVoltage_update(void);
static void GBT_temp_update(void);
static void GBT_status_update(void);
static void GBT_ratings_update(void);

/* Global variables ----------------------------------------------------------*/
struct GBT_Data GBT_Data_i;                 /**< See GBT_data.h */

/* Extern declarations (not yet in their module headers) */
extern CONTROLLER_DATA Controller;                  /**< Battery_controller.c: cell / temperature max, min, ids */
extern int16 temperature;                           /**< Battery_controller.c: BMS MCU temperature (Mcu_Temp), degC */
extern volatile Uint16 shuntCS_watchdog;            /**< Current sensor (shunt) watchdog */
extern volatile Uint16 succed_packet_error_count;   /**< Cell monitor IC (cell voltage / temperature) comm watchdog */

/* Private variables ---------------------------------------------------------*/
static Uint16 GBT_cell_ic[GBT_SERIES_CELLS];    /**< Cell number (0-based) -> BMS_data_IC[] index */
static Uint16 GBT_cell_pos[GBT_SERIES_CELLS];   /**< Cell number (0-based) -> cell_voltages[] index */

/* Public functions ----------------------------------------------------------*/
/**
 * @brief  Initialise GBT_Data_i: static parameters, invalid markers and the cell map.
 * @see    GBT_data.h
 */
void GBT_Data_init(void)
{
    Uint16 i;

    GBT_Data_i.max_charge_voltage = GBT_MAX_CHG_VOLTAGE;
    GBT_Data_i.max_cell_charge_voltage = GBT_MAX_CELL_CHG_VOLTAGE;
    GBT_Data_i.battery_type = GBT_BATTERY_TYPE;
    GBT_Data_i.rated_voltage = GBT_RATED_VOLTAGE;
    GBT_Data_i.max_permit_temp = GBT_MAX_PERMIT_TEMP;

    GBT_ratings_update();

    GBT_Data_i.highest_cell_id = 0;
    GBT_Data_i.lowest_cell_id = 0;
    GBT_Data_i.highest_temp_probe = 0;
    GBT_Data_i.lowest_temp = 0;
    GBT_Data_i.lowest_temp_probe = 0;

    GBT_Data_i.status_A.all = 0;
    GBT_Data_i.status_B.all = 0;
    GBT_Data_i.status_C.all = 0;

    for (i = 0; i < GBT_SERIES_CELLS; i++)
    {
        GBT_Data_i.cell_voltage[i] = GBT_CELL_V_INVALID;
    }

    GBT_Data_i.temp_count = GBT_TEMP_COUNT;
    for (i = 0; i < GBT_TEMP_COUNT; i++)
    {
        GBT_Data_i.temp[i] = GBT_TEMP_INVALID;
    }

    GBT_cellMap_init();
}

/**
 * @brief  Refresh the live GB/T data.
 * @see    GBT_data.h
 */
void GBT_Data_update(void)
{
    GBT_Data_i.highest_cell_id = Controller.highest_cell_id + 1;            // Controller is 0-based -> 1-based
    GBT_Data_i.lowest_cell_id = Controller.lowest_cell_id + 1;              // Controller is 0-based -> 1-based
    GBT_Data_i.highest_temp_probe = Controller.highest_temp_id;             // Controller is already 1-based
    GBT_Data_i.lowest_temp = (int16)Controller.lowest_temp * 10;
    GBT_Data_i.lowest_temp_probe = Controller.lowest_temp_id;               // Controller is already 1-based

    GBT_ratings_update();       // follows capacity / current limit changes over CAN
    GBT_status_update();
    GBT_cellVoltage_update();
    GBT_temp_update();
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief   Build the cell map: GB/T cell number -> BMS IC and position, skipping unused positions.
 * @note    Sets GBT_Data_i.cell_count.
 */
static void GBT_cellMap_init(void)
{
    Uint16 ic, c;

    GBT_Data_i.cell_count = 0;
    for (ic = 0; ic < TOTAL_IC; ic++)
    {
        for (c = 0; c < CELLS_PER_IC; c++)
        {
            if (!GBT_CELL_UNUSED(ic, c) && (GBT_Data_i.cell_count < GBT_SERIES_CELLS))
            {
                GBT_cell_ic[GBT_Data_i.cell_count] = ic;
                GBT_cell_pos[GBT_Data_i.cell_count] = c;
                GBT_Data_i.cell_count++;
            }
        }
    }
}

/**
 * @brief   Copy all cell voltages into GBT_Data_i.cell_voltage[], 0.1 mV -> 0.01 V.
 */
static void GBT_cellVoltage_update(void)
{
    Uint16 i;

    for (i = 0; i < GBT_Data_i.cell_count; i++)
    {
        // 0.1 mV -> 0.01 V
        GBT_Data_i.cell_voltage[i] = BMS_data_IC[GBT_cell_ic[i]].cell_voltages[GBT_cell_pos[i]] / 100;
    }
}

/**
 * @brief   Copy all temperature probes into GBT_Data_i.temp[], 1 degC/bit.
 */
static void GBT_temp_update(void)
{
    Uint16 ic, t;

    for (ic = 0; ic < TOTAL_IC; ic++)
    {
        for (t = 0; t < TEMP_PER_IC; t++)
        {
            // 1 degC/bit, same value as Controller.highest_temp / lowest_temp
            GBT_Data_i.temp[(ic * TEMP_PER_IC) + t] = BMS_data_IC[ic].temparature_val[t];
        }
    }
}

/**
 * @brief   Set the status bytes A, B and C.
 * @details A: cell voltage, SOC, charging over current, battery temperature.
 *          B: insulation (fixed normal), BMS temperature.
 *          C: charge contactor.
 */
static void GBT_status_update(void)
{
    Uint16 cell_v_state;
    Uint16 soc_state;
    Uint16 current_state;
    Uint16 temp_state;
    Uint16 bms_temp_state;
    Uint16 con_state;

    /* pack cell voltage state - max / min cell voltage from Battery_controller.c
     * (PDU_getData_i, 1 mV/bit, master + slave), over high wins */
    if (PDU_getData_i.highest_cell_voltage > (GBT_CELL_V_OVER_HIGH * 10))
    {
        cell_v_state = GBT_STS_OVER_HIGH;
    }
    else if (PDU_getData_i.lowest_cell_voltage < (GBT_CELL_V_BELOW_LOW * 10))
    {
        cell_v_state = GBT_STS_BELOW_LOW;
    }
    else
    {
        cell_v_state = GBT_STS_NORMAL;
    }

    /* cell voltage state */
    GBT_Data_i.status_A.bit.cell_voltage_state = cell_v_state;

    /* pack SOC state - SOC from Battery_controller.c
     * (PDU_getData_i, 1 %/bit) */
    if (PDU_getData_i.display_soc_val > GBT_SOC_OVER_HIGH)
    {
        soc_state = GBT_STS_OVER_HIGH;
    }
    else if (PDU_getData_i.display_soc_val < GBT_SOC_BELOW_LOW)
    {
        soc_state = GBT_STS_BELOW_LOW;
    }
    else
    {
        soc_state = GBT_STS_NORMAL;
    }

    /* SOC state */
    GBT_Data_i.status_A.bit.soc_state = soc_state;

    /* pack charging current state - current from Battery_controller.c
     * (current_val_A, A, charging is negative)
     * charge limit from eprom_config.c (read from EEPROM at start, CAN configurable, 0.01 A/bit)
     *   00 normal    - charging current within CHARGE_CURRENT_THRESHOLD
     *   01 over high - charging current over CHARGE_CURRENT_THRESHOLD, now
     *   10 untrusted - current sensor (shunt) comm lost, value can not be read */
    if (shuntCS_watchdog > GBT_COMM_LOST_COUNT)
    {
        current_state = GBT_STS_UNTRUSTED;
    }
    else if (current_val_A < ((double)CHARGE_CURRENT_THRESHOLD / 100))     // charge limit, 0.01 A -> A, negative
    {
        current_state = GBT_STS_OVER_HIGH;
    }
    else
    {
        current_state = GBT_STS_NORMAL;
    }

    /* charging over current state */
    GBT_Data_i.status_A.bit.charge_over_current = current_state;

    /* pack battery temperature state - highest temperature from Battery_controller.c
     * (Controller.highest_temp, 1 degC/bit)
     * temperature limit from main_data.h (HIGHEST_TEMPERATURE_LIMIT, 1 degC/bit)
     *   00 normal    - highest temperature within HIGHEST_TEMPERATURE_LIMIT
     *   01 over high - highest temperature over HIGHEST_TEMPERATURE_LIMIT, now
     *   10 untrusted - temperature sensor (cell monitor IC) comm lost, value can not be read */
    if (succed_packet_error_count > GBT_COMM_LOST_COUNT)
    {
        temp_state = GBT_STS_UNTRUSTED;
    }
    else if ((int16)Controller.highest_temp > HIGHEST_TEMPERATURE_LIMIT)     // over temperature limit
    {
        temp_state = GBT_STS_OVER_HIGH;
    }
    else
    {
        temp_state = GBT_STS_NORMAL;
    }

    /* battery temperature state */
    GBT_Data_i.status_A.bit.battery_temp_state = temp_state;

    /* insulation - not implemented yet */
    GBT_Data_i.status_B.bit.insulation_state = GBT_STS_NORMAL;   // Need to implement this one till then use this hard coded value

    /* BMS temperature state - MCU temperature from Battery_controller.c
     * (temperature = Mcu_Temp(), 1 degC/bit, MCU internal sensor)
     * temperature limit GBT_BMS_TEMP_LIMIT (1 degC/bit)
     *   00 normal    - BMS temperature within GBT_BMS_TEMP_LIMIT
     *   01 over high - BMS temperature over GBT_BMS_TEMP_LIMIT, now
     *   10 untrusted - reading out of sensor range (ADC / calibration fault), value can not be trusted */
    if ((temperature < GBT_BMS_TEMP_VALID_MIN) || (temperature > GBT_BMS_TEMP_VALID_MAX))
    {
        bms_temp_state = GBT_STS_UNTRUSTED;
    }
    else if (temperature > GBT_BMS_TEMP_LIMIT)              // over temperature limit
    {
        bms_temp_state = GBT_STS_OVER_HIGH;
    }
    else
    {
        bms_temp_state = GBT_STS_NORMAL;
    }

    /* BMS temperature state */
    GBT_Data_i.status_B.bit.bms_temp_state = bms_temp_state;

    /* charge contactor state - contactor error flags from Battery_controller.c
     * FC_CONTACTOR = 1: FC contactor (weld at start up / no feedback after close)
     * FC_CONTACTOR = 0: no FC contactor, charging is through the main contactor
     *   00 normal    - contactor feedback follows the command
     *   01 abnormal  - contactor welded or not closing (feedback does not follow the command)
     *   10 untrusted - not used, contactor feedback is a GPIO input and can always be read */
#if (FC_CONTACTOR == 1)
    if (PDU_getData_i.fixSetChrg.bit.fc_contactor_error         // no feedback after close (debounced)
        || PDU_getData_local.fixSetG.bit.contactor_error)       // welded at start up, now
#else
    if (PDU_getData_i.fixSetG.bit.contactor_error)              // main contactor welded / no feedback
#endif
    {
        con_state = GBT_STS_ABNORMAL;
    }
    else
    {
        con_state = GBT_STS_NORMAL;
    }

    /* FC contactor state */
    GBT_Data_i.status_C.bit.fc_contactor_state = con_state;
}

/**
 * @brief   Rated capacity, total energy and max charge current from eprom_config
 *          (EEPROM / CAN configurable).
 * @pre     GBT_Data_i.rated_voltage is set.
 */
static void GBT_ratings_update(void)
{
    GBT_Data_i.rated_capacity = (Uint16)(nominal_capacity * 10);                     // Ah -> 0.1 Ah
    GBT_Data_i.total_energy = (Uint16)(((Uint32)GBT_Data_i.rated_voltage * GBT_Data_i.rated_capacity + 5000) / 10000);  // 0.1 kWh
    GBT_Data_i.max_charge_current = (Uint16)(-CHARGE_CURRENT_THRESHOLD / 10);        // 0.01 A, negative -> 0.1 A, positive
}

#endif /* GBT_CHARGING */

/* end of file */
