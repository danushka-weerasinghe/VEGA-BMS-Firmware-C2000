/*
 * soc_soh.c
 *
 *  Created on: Oct 14, 2021
 *      Author: Asith Devinda
 */

#include <eprom_config.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "DSP28x_Project.h"
#include "main_data.h"
#include "soc_soh.h"
#include "BMS_I2C.h"
#include "charge_profile.h"
#include "PDU_matrix.h"

#define CHARGING          1
#define DISCHARGING       0
#define OPENCCT           2
#define SOH_WAIT_LIMIT   60                    /*Delay to recalculate SOH in seconds*/
#define mu                1    /*Coulombic efficiency*/

#define START_ACC_ENERGY 111
#define FRESH_EEPROM_32  0xFFFFFFFF

//----------------------------------------------------------------------------------------------------------------------------------------------------------
float last_saved_soc = 0.0f;
float confirmed_saved_soc = 0.0f;
float confirmed_soc = 0.0f;
Uint8 pending_backup_save = 0;
float Temp_soc = 0.0f;

#define FLAG_VALID   0x5A5A
#define FLAG_INVALID 0x0000

Uint8 ADDR_MAIN_FLAG[2]   = {0x02, 0x00};
Uint8 ADDR_MAIN_SOC[2]    = {0x02, 0x02};
Uint8 ADDR_BACKUP_FLAG[2] = {0x02, 0x06};
Uint8 ADDR_BACKUP_SOC[2]  = {0x02, 0x08};
//----------------------------------------------------------------------------------------------------------------------------------------------------------


/*
 *
 * Function prototypes
 */
uint16_t format_dataIn(void);
void format_dataOut(uint16_t index);
float check_index(uint16_t check1, uint16_t check2, float val1, float val2);
void Write_to_EEP(float val, Uint8 index, Uint8 *addr1, Uint8 *addr2);
Uint8 getSaveIndex(Uint8 EEPaddress, Uint8 EEPreg[], Uint8 difference);
float calc_SOH(float soh);
void clearEEpage(Uint8 EEPaddress, Uint8 EEPpage);
void reset_soh(void);
void calculate_chargeTime(int16 charge_current, float total_capacity, float available_capacity);
void calculate_chargeTime_GBT(int16 charge_current, float total_capacity, float available_capacity);

/*
 * Typedefs
 */
typedef struct
{
    uint16_t byte_0 :8;
    uint16_t byte_1 :8;
    uint16_t byte_2 :8;
    uint16_t byte_3 :8;
} byte_8bit;

/*
 * Union violates MISRA-C rule 18.4
 * The violation is deemed acceptable for packing and unpacking of data.
 * In this instance, the deviation is required as it's the most efficient method to
 * pack and unpack I2C data.
 * To mitigate the risk of misinterpretation of data, padding, alignment,
 * endianness and bit-order should be carefully checked.
 */
typedef union
{
    float value;
    byte_8bit d_array;
} Eep_data;

/*
 * Globals
 */
//CAN_data CAN_cmb_ct;
Eep_data input;
Eep_data output;

static uint16_t charge_started = 0;
static float prev_cc = 0;
static double prev_pack_voltage = 0;

//uint16_t soc_est;
static Uint32 chg_energy_acc = 0;
static Uint32 last_saved_chg_energy = 0;
float batt_capacity;
float Ah;
Uint8 Eeprom_pgaddr[2] = { 0x03, 0x00 };
Uint8 Eeprom_SOHaddr1[2] = { 0x02, 0x1A };
Uint8 Eeprom_SOHaddr2[2] = { 0x02, 0x1F };
Uint8 Eeprom_SOHestaddr1[2] = { 0x02, 0x24 };
Uint8 Eeprom_SOHestaddr2[2] = { 0x02, 0x29 };
Uint8 Eeprom_SOCaddr[2] = { 0x00, 0x00 };
Uint8 Eeprom_STATaddr[2] = { 0x00, 0x00 };
Uint8 EEPROM_chg_energy_addr [2] = {0x01, 0x4C};
Uint8 EEPROM_chg_energy_backup_addr [2] = {0x01,0x50};
Uint8 soh_flag = 0;
Uint8 Eeprom_page = 0;
Uint16 Eeprom_soc_index = 0;
Uint8 Eeprom_soh_index = 0;
float soh_est;
Uint8 soh_count = 0;
Uint16 soc_count = 0;
Uint16 wait_count = 0;
Uint16 lvcount = 0;
Uint16 hvcount = 0;
float start_Ah = 0;
float dummy_cap, dummy_80, dummy_100;
int16 dummy_filt;
Uint8 inbuff[5], outbuff[5];
Uint16 Status_read, Status_write;
static Uint16 failCnt = 0;

Uint16 charge_complete = 0;
int16 current_filt_buff[10] = {0};
Uint8 filt_index = 0;
Uint16 sampling_timer = 0;


Uint16 offset;
double soc_value;
Uint16 soc_reset = 0;
//Uint16 chg_energy_reset_bit = 0;
Uint8 save_chg_energy_flag = 0;

volatile Uint8 can_force_soc_flag = 0;
volatile Uint8 can_target_soc_val = 0;
volatile Uint8 eeprom_save_done_flag = 0;

uint16_t format_dataIn(void)
{
    input.d_array.byte_0 = inbuff[0];
    input.d_array.byte_1 = inbuff[1];
    input.d_array.byte_2 = inbuff[2];
    input.d_array.byte_3 = inbuff[3];
    uint16_t temp = inbuff[4];
    return temp;

}

void format_dataOut(uint16_t sv_count)
{
    outbuff[0] = output.d_array.byte_0;
    outbuff[1] = output.d_array.byte_1;
    outbuff[2] = output.d_array.byte_2;
    outbuff[3] = output.d_array.byte_3;
    outbuff[4] = sv_count;
}

/*
 * The library stores each value in two addresses alternatively to prevent data corruption.
 * Following function checks the stored values for the last saved and for any corruption.
 */
float check_index(uint16_t check1, uint16_t check2, float val1, float val2)
{
    if ((check1 <= 3) && (check2 <= 3))
    {
        if ((check2 == 3) && (check1 == 0))
        {
            return val1;
        }
        else if (check2 > check1)
        {
            return val2;
        }
        else
        {
            return val1;
        }
    }
    else if (check1 <= 3)
    {
        return val1;
    }
    else if (check2 <= 3)
    {
        return val2;
    }
    else if (failCnt >= 9)
    {
        return 100; /*to initiate for the 1st time*/
    }
    else
    {
        return 200; /*to fail the test*/
    }
}




void init_charge_energy(void)
{
    Uint32 chg_energy;
    Uint32 backup_chg_energy;
    Uint8 inbuff_chg[4];
    Uint8 inbuff_chg_backup[4];
    Uint8 writebuff_chg[4] = {0};

    /* --- CHARGED ENERGY INITIALIZATION --- */

    // 1. Read Main Energy Address
    Status_read = I2CA_ReadFromManyRegs(EEPROM_ADDR,
                                        &EEPROM_chg_energy_addr[EEP_ADDR_LOC],
                                        EEP_ADDR_LENGTH,
                                        inbuff_chg, 4);
    chg_energy = inbuff_chg[0] | (Uint32)inbuff_chg[1] << 8 | (Uint32)inbuff_chg[2] << 16 | (Uint32)inbuff_chg[3] << 24;

    // 2. Read Backup Energy Address
    Status_read = I2CA_ReadFromManyRegs(EEPROM_ADDR,
                                        &EEPROM_chg_energy_backup_addr[EEP_ADDR_LOC],
                                        EEP_ADDR_LENGTH,
                                        inbuff_chg_backup, 4);
    backup_chg_energy = inbuff_chg_backup[0] | (Uint32)inbuff_chg_backup[1] << 8 | (Uint32)inbuff_chg_backup[2] << 16 | (Uint32)inbuff_chg_backup[3] << 24;

    // 3. Consistency Check & "Start Value" Logic
    if (backup_chg_energy == FRESH_EEPROM_32)
    {
        // Case: Backup is empty (Fresh chip). Initialize Main.
        chg_energy_acc = START_ACC_ENERGY;

        // Prepare buffer to sync backup immediately
        writebuff_chg[3] = (chg_energy_acc >> 24) & 0xFF;
        writebuff_chg[2] = (chg_energy_acc >> 16) & 0xFF;
        writebuff_chg[1] = (chg_energy_acc >> 8) & 0xFF;
        writebuff_chg[0] = (chg_energy_acc) & 0xFF;

        Status_write = I2CA_WriteToManyRegs(EEPROM_ADDR, EEPROM_chg_energy_backup_addr,
                                            EEP_ADDR_LENGTH,
                                            writebuff_chg, 4);
    }
    else if(labs((long)chg_energy - (long)backup_chg_energy) > 12000)
    {
        // Case: Corruption detected, trust backup.
        chg_energy_acc = backup_chg_energy;
    }
    else
    {
        // Case: Normal operation.
        if (chg_energy == 0)
        {
             chg_energy_acc = START_ACC_ENERGY;
        }
        else
        {
             chg_energy_acc = chg_energy;
        }

        // Sync logic if needed
        writebuff_chg[3] = (chg_energy_acc >> 24) & 0xFF;
        writebuff_chg[2] = (chg_energy_acc >> 16) & 0xFF;
        writebuff_chg[1] = (chg_energy_acc >> 8) & 0xFF;
        writebuff_chg[0] = (chg_energy_acc) & 0xFF;

        Status_write = I2CA_WriteToManyRegs(EEPROM_ADDR, EEPROM_chg_energy_backup_addr,
                                            EEP_ADDR_LENGTH,
                                            writebuff_chg, 4);
        DELAY_US(5000);
    }

    // 4. Initialize Local Structs
    PDU_getData_local.charged_energy = 0;       // Reset session energy (important for save trigger)
    PDU_getData_local.charged_energy_acc = chg_energy_acc; // Load total directly
    last_saved_chg_energy = chg_energy_acc;
}


void Write_to_EEP(float val, Uint8 index, Uint8 *addr1, Uint8 *addr2)
{
    output.value = val;
    format_dataOut(index);
    if (index % 2)
    {
        Status_write = I2CA_WriteToManyRegs(EEPROM_ADDR, addr2, EEP_ADDR_LENGTH,
                                            outbuff, 5);
        DELAY_US(300);
    }
    else
    {
        Status_write = I2CA_WriteToManyRegs(EEPROM_ADDR, addr1, EEP_ADDR_LENGTH,
                                            outbuff, 5);
        DELAY_US(300);
    }
}

static volatile float soc_value_float = 0;

void calc_soc_soh(double current_value, unsigned int max_cell_voltage,
                  unsigned int min_cell_voltage, float *soc)
{
    Uint16 state;
    float coulomb_count = 0;
    float deltaAh = 0;
    float tempAh = 0;
    static Uint8 power_down_phase = 0;

#ifdef hall_CS
    coulomb_count = (current_value * 100E-3);
#endif

#ifdef recovery_CS
    coulomb_count = (current_value * 100E-3);
#endif

#ifdef shunt_CS
    /*Charging state detection*/
#ifdef old_version
    coulomb_count = ((start_Ah - Ah) * 3600.0) - INA229_data[4];
#endif

#ifdef sealed_version
    coulomb_count = ((start_Ah - Ah) * 3600.0) + INA229_data[4];
#endif
#endif


    PDU_getData_local.Battery_capacity = batt_capacity;

    if (current_value > DEADBAND)
    {
        state = CHARGING;
        tempAh = Ah + ((mu * coulomb_count) / 3600.0); /*Multiplied by Coulombic efficiency*/
    }
    else if (current_value < (-1 * DEADBAND))
    {
        state = DISCHARGING;
        tempAh = Ah + (coulomb_count / 3600.0);
    }
    else
    {
        state = OPENCCT;
        current_value = 0.0;
        tempAh = Ah;
    }

    if (abs(tempAh - Ah) > 0.01)
    {
        deltaAh = tempAh - Ah;
        start_Ah -= deltaAh;
    }

    Ah = tempAh - deltaAh;

    if (Ah > batt_capacity)
    {
        Ah = batt_capacity;
    }
    else if (Ah < 0.0)
    {
        Ah = 0.0;
    }
    else
    {
        /*do nothing*/
    }

    if (soc_reset)
    {
#ifdef shunt_CS
        start_Ah = ((soc_reset - 1)*batt_capacity*0.01 - INA229_data[4] / 3600);
#endif
#ifdef recovery_CS
        start_Ah = (soc_reset - 1) * batt_capacity * 0.01;
#endif
        Ah = (soc_reset - 1)*batt_capacity*0.01;
        soc_reset = 0;
    }


    *soc = ((Ah / batt_capacity) * 100.0);

    soc_value_float = *soc;

    /*Change this to include max cell voltage if charging and min cell voltage if discharging*/
    /*Add current condition*/
    if ((state == CHARGING) && (max_cell_voltage >= CHARGE_CELL_VOLTAGE_LIMIT)
            && (wait_count == 0)) /*Charge mode*/
    {
        if (hvcount >= 40)
        {
            charge_complete = 1;
            wait_count++;
            hvcount = 0;
        }
        else
        {
            hvcount++;
        }
    }
    else if ((state == DISCHARGING)
            && (min_cell_voltage <= LOWEST_CELL_VOLTAGE_LIMIT)
            && (wait_count == 0) && (*soc < 5)) /*Discharge mode*/
    {
        if (lvcount >= 40)
        {

#ifdef shunt_CS
            start_Ah = (INA229_data[4] / 3600.0);
#else
            Ah = 0;
#endif
            *soc = 0.0;
            wait_count++;
            lvcount = 0;
        }
        else
        {
            lvcount++;
        }
    }
    else
    {
        if ((wait_count < ((SOH_WAIT_LIMIT * 1000U) / 50)) && (wait_count != 0))
        {
            wait_count++;
        }
        else
        {
            wait_count = 0;
        }
    }




    if (PDU_setData_local.fixSetS.bit.EVCU_powerdown_ready)
    {
        switch (power_down_phase)
        {
            case 0:
                if (PDU_getData_local.charged_energy > 0)
                {
                    save_chg_energy_flag = 1;
                }
                power_down_phase = 1;
                break;
            case 1:
                if (soc_count == 0)
                {
                    power_down_phase = 2;
                }
                break;

            case 2:
                if (soc_count == 100)
                {
                    power_down_phase = 3;
                }
                break;

            case 3:
                PDU_getData_local.fixSetChrg.bit.BMS_power_down_ready = 1;
                break;
        }
    }
    else
    {
        power_down_phase = 0;
        PDU_getData_local.fixSetChrg.bit.BMS_power_down_ready = 0;
    }


}


void charge_ctrl(unsigned int highest_cell_v, int16 charge_current, float soc)
{

    Uint8 static cv_slave;
    Uint8 static chargeComp_slave;
    Uint8 static charge_comp_phase = 0;

#ifdef MASTER
    if (slave_status)
    {
        cv_slave = PDU_getData_slave.fixSetChrg.bit.cv_mode;
        if ((PDU_getData_slave.fixSetChrg.bit.charge_complete_flag) && (!chargeComp_slave))
        {
            chargeComp_slave = 1;
        }
    }
    else
    {
        cv_slave = 0;
        chargeComp_slave = 1;
    }
#endif

#ifdef SLAVE
    cv_slave = 0;
    chargeComp_slave = 1;
#endif

    (void)cv_slave;
    (void)chargeComp_slave;

    if ((!PDU_getData_local.fixSetChrg.bit.charging) && (soc > 99)
            && (!PDU_getData_local.fixSetChrg.bit.charge_complete_flag) && (highest_cell_v > 34000)) /*Initial SOC check to allow charging*/
    {
        PDU_getData_local.fixSetChrg.bit.charge_complete_flag = 1;
    }
    else
    {
        PDU_getData_local.fixSetChrg.bit.charging = 1;
    }

    if (charge_complete == 1 || charge_complete == 2 || charge_complete == 3)
    {
        PDU_getData_local.fixSetChrg.bit.cv_mode = 1;
    }
    else if(charge_complete == 0)
    {
        PDU_getData_local.fixSetChrg.bit.cv_mode = 0;
    }

    if ((PDU_getData_local.fixSetChrg.bit.cv_mode) && (abs(charge_current) < CV_END_CURRENT) && (charge_complete == 2))
    {
        charge_complete = 3;
    }

    if (charge_current < CHARGE_CURRENT_THRESHOLD) /* Charge current exceeded limit */
    {
        PDU_getData_local.fixSetChrg.bit.charge_current_lim = 1;
    }
    else
    {
        PDU_getData_local.fixSetChrg.bit.charge_current_lim = 0;
    }

    if (soc < 5)
    {
        PDU_getData_local.fixSetChrg.bit.low_SOC = 1;
    }
    else
    {
        PDU_getData_local.fixSetChrg.bit.low_SOC = 0;
    }



    if ((charge_complete == 3) || (PDU_getData_local.fixSetChrg.bit.charge_complete_flag) || (highest_cell_v >= HIGHEST_CELL_VOLTAGE_LIMIT))
    {
        switch (charge_comp_phase)
        {
            case 0:

                charge_comp_phase = 1;
                eeprom_save_done_flag = 0;
                break;

            case 1:
                if (soc_count == 0)
                {
                    save_chg_energy_flag = 1;

                    soc_reset = 101;
                    soc = 100.0f;
                    Temp_soc = 100.0f;
                    confirmed_soc = 100.0f;
                    last_saved_soc = 100.0f;
                    pending_backup_save = 1;

                    charge_comp_phase = 2;
                }
                break;

            case 2:
                if (soc_count == 100)
                {
                    charge_comp_phase = 3;
                }
                break;

            case 3:
                eeprom_save_done_flag = 1;
                PDU_getData_local.fixSetChrg.bit.charge_complete_flag = 1;
                break;
        }
    }

    else
    {
        PDU_getData_local.fixSetChrg.bit.charge_complete_flag = 0;
        charge_comp_phase = 0;
        eeprom_save_done_flag = 0;
    }

    int16 total_current = PDU_getData_local.current_Axx + PDU_getData_slave.current_Axx;
    float total_capacity = batt_capacity + PDU_getData_slave.Battery_capacity;
    float available_capacity = Ah + (PDU_getData_slave.Battery_capacity * PDU_getData_slave.SOC_val / 100);

    if (PDU_setData_local.fixSetS_EVCC.bit.GBT_connected) /*Fast charger connected*/
    {
        calculate_chargeTime_GBT(total_current, total_capacity, available_capacity);
    }
    else
    {
        calculate_chargeTime(total_current, total_capacity, available_capacity);
    }
}

void calculate_chargeTime(int16 charge_current, float total_capacity, float available_capacity)
{
    /*Charge time calculation*/
    float time_to_80, time_to_100, remaining_capacity;
    int16 filt_current = 0;

    for (int i = 0; i < 10; i++)
    {
        filt_current += (current_filt_buff[i] * 0.1);
    }
    if (filt_current > -100)
    {
        filt_current = -300;
    }
    remaining_capacity = total_capacity - available_capacity;
    time_to_100 = (remaining_capacity * 60 * (-100) / filt_current);
    if ((time_to_100 == 0) && (!charge_complete))
    {
        time_to_100 = 1;
    }

    if (remaining_capacity > (total_capacity * 0.21))
    {
        time_to_80 = ((available_capacity - (total_capacity * 0.79)) * 60 * 100 / filt_current);
    }
    else
    {
        time_to_80 = 0;
    }

    sampling_timer++;
    if ((charge_current < CHARGE_COMP_CURRENT_THRESHOLD)
            && (sampling_timer > 10))
    {
        PDU_getData_local.time_to_80 = (Uint16) ceil(time_to_80);
        PDU_getData_local.time_to_100 = (Uint16) ceil(time_to_100);
        current_filt_buff[filt_index++] = charge_current;
        if (filt_index >= 10)
        {
            filt_index = 0;
        }
        sampling_timer = 0;
    }

    dummy_80 = time_to_80;
    dummy_100 = time_to_100;
    dummy_cap = remaining_capacity;
    dummy_filt = filt_current;
}

void calculate_chargeTime_GBT(int16 charge_current, float total_capacity, float available_capacity)
{
    uint16_t soc = available_capacity * 100 / total_capacity;
    float remaining_capacity = total_capacity - available_capacity;
    float block_time;

    if (soc > 95)
    {
        block_time = ceil(remaining_capacity * (-100 * 60) / charge_current);
        PDU_getData_local.time_to_80 = 0;
        PDU_getData_local.time_to_100 = (uint16_t) block_time;
    }
    else if (soc > 85)
    {
        block_time = ceil((remaining_capacity - 0.05*total_capacity) * (-100 * 60) / charge_current);
        PDU_getData_local.time_to_80 = 0;
        PDU_getData_local.time_to_100 = (uint16_t) (block_time + t_95_100(total_capacity));
    }
    else if (soc > 15)
    {
        block_time = ceil((remaining_capacity - 0.15*total_capacity) * (-100 * 60) / charge_current);
        PDU_getData_local.time_to_80 = (uint16_t) ceil((remaining_capacity - 0.2*total_capacity) * (-100 * 60) / charge_current);
        PDU_getData_local.time_to_100 = (uint16_t) (block_time + t_85_100(total_capacity));

    }
    else if (soc > 5)
    {
        block_time = ceil((remaining_capacity - 0.85*total_capacity) * (-100 * 60) / charge_current);
        PDU_getData_local.time_to_80 = (uint16_t) (block_time + t_15_80(total_capacity));
        PDU_getData_local.time_to_100 = (uint16_t) (block_time + t_15_100(total_capacity));
    }
    else
    {
        block_time = ceil((remaining_capacity - 0.95*total_capacity) * (-100 * 60) / charge_current);
        PDU_getData_local.time_to_80 = (uint16_t) (block_time + t_5_80(total_capacity));
        PDU_getData_local.time_to_100 = (uint16_t) (block_time + t_5_100(total_capacity));
    }
}

Uint8 getSaveIndex(Uint8 EEPaddress, Uint8 EEPreg[], Uint8 difference)
{
    Uint8 static statusBuff[256];
    Uint8 temp = 0, index = 0;
    Status_read = I2CA_ReadFromManyRegs(EEPaddress, EEPreg, 2, statusBuff, 256);

    /* Identify the last written element of the status buffer */
    do
    {
        temp = statusBuff[index];

        if ((temp/difference) != index)
        {

        }

        index++;
        if (index == 255) // Break if end of buffer, so we don't compare out-of-bounds.
        {
            break;
        }

    }
    while (statusBuff[index] == (temp + difference));

    return statusBuff[index - 1]; // Point to the last used element of the parameter buffer
}

void clearEEpage(Uint8 EEPaddress, Uint8 EEPpage)
{
    Uint8 zeroArray[256] = { 0 };
    Uint8 address[2] = { EEPpage, 0x00 };
    Status_write = I2CA_WriteToManyRegs(EEPaddress, address, 2, zeroArray, 256);

}

void reset_soh(void)
{
    float dummy = 100;
    Write_to_EEP(dummy, Eeprom_soh_index, &Eeprom_SOHaddr1[EEP_ADDR_LOC],
                 &Eeprom_SOHaddr2[EEP_ADDR_LOC]);
    DELAY_US(6000);
    Eeprom_soh_index++;
    Write_to_EEP(dummy, Eeprom_soh_index, &Eeprom_SOHaddr1[EEP_ADDR_LOC],
                 &Eeprom_SOHaddr2[EEP_ADDR_LOC]);
    DELAY_US(6000);
    Write_to_EEP(dummy, Eeprom_soh_index, &Eeprom_SOHaddr1[EEP_ADDR_LOC],
                 &Eeprom_SOHaddr2[EEP_ADDR_LOC]);
    DELAY_US(6000);
    soh_count = 0;
    Write_to_EEP(dummy, soh_count, &Eeprom_SOHestaddr1[EEP_ADDR_LOC],
                 &Eeprom_SOHestaddr2[EEP_ADDR_LOC]);
    DELAY_US(6000);
    soh_count++;
    dummy = 200;
    Write_to_EEP(dummy, soh_count, &Eeprom_SOHestaddr1[EEP_ADDR_LOC],
                 &Eeprom_SOHestaddr2[EEP_ADDR_LOC]);
    DELAY_US(6000);

}


Uint32 calculate_charged_energy(double pack_voltage)
{
    Uint32 energy = 0;
    float cc_diff;

    if (charge_started == 0)
    {
        prev_cc = INA229_data[4];
        prev_pack_voltage = pack_voltage;
        charge_started = 1;
    }

    cc_diff = INA229_data[4] - prev_cc;

    if (fabs(cc_diff) >= 500.0f)
    {
        // Calculate and accumulate
        energy = (Uint32)((prev_pack_voltage + pack_voltage) * 0.5 * fabs(cc_diff) / 3600);
        chg_energy_acc += energy;
        PDU_getData_local.charged_energy_acc = chg_energy_acc;

        prev_pack_voltage = pack_voltage;
        prev_cc = INA229_data[4];

        // Instead of writing to EEPROM here, just raise the flag!
        if ((chg_energy_acc - last_saved_chg_energy) >= 100)
        {
            save_chg_energy_flag = 1;
        }
    }
    return energy;
}


void reset_chg_energy(Uint16* chg_energy_reset)
{
    // Load new value
    PDU_getData_local.charged_energy_acc = chg_energy_reset[0];
    PDU_getData_local.charged_energy_acc |= chg_energy_reset[1] << 8;
    PDU_getData_local.charged_energy_acc |= (Uint32)chg_energy_reset[2] << 16;
    PDU_getData_local.charged_energy_acc |= (Uint32)chg_energy_reset[3] << 24;

    chg_energy_acc = PDU_getData_local.charged_energy_acc;

    // Trigger the state machine!
    save_chg_energy_flag = 1;


}

void reset_charge_session(void)
{
    charge_started = 0;
    prev_cc = 0;
    prev_pack_voltage = 0;
}

void force_instant_soc_update(float *soc)
{
    float new_soc_percentage = (float)(can_target_soc_val - 1);

    *soc = new_soc_percentage;
    soc_value = new_soc_percentage;
    Ah = (new_soc_percentage * batt_capacity) / 100.0;

    #ifdef shunt_CS
    start_Ah = ((new_soc_percentage * batt_capacity) / 100.0) - (INA229_data[4] / 3600.0);
    #endif

    #ifdef recovery_CS
    start_Ah = (new_soc_percentage * batt_capacity) / 100.0;
    #endif

    last_saved_soc = new_soc_percentage;
    Temp_soc = new_soc_percentage;

    pending_backup_save = 1;

}


void soc_init(float *soc)
{
    Uint8 buff[4];
    Uint16 main_flag, backup_flag;
    float main_soc, backup_soc;
    Uint16 main_status = 0;
    Uint16 backup_status = 0;



    // ---------------------------------------------------------
    // 1. Read Main Bank (Flag and SOC)
    // ---------------------------------------------------------
    memset(buff, 0, sizeof(buff));
    main_status |= I2CA_ReadFromManyRegs(EEPROM_SOC_ADDR, ADDR_MAIN_FLAG, EEP_ADDR_LENGTH, buff, 2);
    main_flag = (buff[0] << 8) | buff[1];
    DELAY_US(300);

    memset(buff, 0, sizeof(buff));
    main_status |= I2CA_ReadFromManyRegs(EEPROM_SOC_ADDR, ADDR_MAIN_SOC, EEP_ADDR_LENGTH, buff, 2);
    Uint16 main_soc_int = (buff[0] << 8) | buff[1];
    main_soc = (float)main_soc_int / 100.0f;
    DELAY_US(300);

    // ---------------------------------------------------------
    // 2. Read Backup Bank (Flag and SOC)
    // ---------------------------------------------------------
    memset(buff, 0, sizeof(buff));
    backup_status |= I2CA_ReadFromManyRegs(EEPROM_SOC_ADDR, ADDR_BACKUP_FLAG, EEP_ADDR_LENGTH, buff, 2);
    backup_flag = (buff[0] << 8) | buff[1];
    DELAY_US(300);

    memset(buff, 0, sizeof(buff));
    backup_status |= I2CA_ReadFromManyRegs(EEPROM_SOC_ADDR, ADDR_BACKUP_SOC, EEP_ADDR_LENGTH, buff, 2);
    Uint16 backup_soc_int = (buff[0] << 8) | buff[1];
    backup_soc = (float)backup_soc_int / 100.0f;
    DELAY_US(300);

    // ---------------------------------------------------------
    // 3. Hardware & Data Integrity Checks
    // ---------------------------------------------------------
    if (main_status != 0 && backup_status != 0)
    {
        // Hardware read failed. Fallback to default.
        *soc = 35.0;
    }
    else if ((isnan(main_soc) && isnan(backup_soc)) || (main_flag == 0xFFFF && backup_flag == 0xFFFF))
    {
        // Both are corrupted, or the EEPROM is totally blank (fresh chip)
        *soc = 40.0;
    }
    else
    {
        // If the Main I2C read failed, or data is NaN, mark Main as invalid
        if (main_status != 0 || isnan(main_soc)) main_flag = FLAG_INVALID;

        // If the Backup I2C read failed, or data is NaN, mark Backup as invalid
        if (backup_status != 0 || isnan(backup_soc)) backup_flag = FLAG_INVALID;

        // Decide which SOC value to trust
        if ((main_soc == backup_soc) || (fabs(main_soc - backup_soc) <= 1.0)) {
            *soc = main_soc;
        }
        else if (main_flag == FLAG_VALID) {
            *soc = main_soc;
        }
        else if (backup_flag == FLAG_VALID) {
            *soc = backup_soc;
        }
        else {
            *soc = 45.0;
        }
    }

    // ---------------------------------------------------------
    // 4. Initialize Baseline Tracking Variables
    // ---------------------------------------------------------
    last_saved_soc = *soc;
    Temp_soc = *soc;
    confirmed_saved_soc = *soc;
    confirmed_soc = *soc;
    pending_backup_save = 0;

    batt_capacity = nominal_capacity;
    Ah = (*soc * batt_capacity) / 100.0;
    start_Ah = Ah;
    INA229_reset_flag = 1;
}


void soc_calc_new(double current_value, unsigned int max_cell_voltage, unsigned int min_cell_voltage, float *soc)
{
    // ---------------------------------------------------------
    // 1. The 0.2% Delta Trigger
    // ---------------------------------------------------------
    if (!pending_backup_save)
    {
        if (soc_count < 2 || soc_count > 52)
        {
            // Check if the SOC has drifted by 0.2% or more from the last saved value
            if (fabs(*soc - last_saved_soc) >= 0.2f)
            {
                Temp_soc = *soc;
                pending_backup_save = 1;
                last_saved_soc = *soc;

            }
        }
    }

    // ---------------------------------------------------------
    // 2. The Non-Blocking EEPROM Save State Machine
    // ---------------------------------------------------------
    switch (soc_count)
    {
        case 2: // Invalidate Main Bank
            if (pending_backup_save) Write_EEP_16bit(FLAG_INVALID, ADDR_MAIN_FLAG);
            soc_count++;
            break;

        case 3: // Write Main SOC (Scaled Integer)
            if (pending_backup_save) {
                confirmed_soc = Temp_soc;
                Uint16 scaled_soc_int = (Uint16)(confirmed_soc * 100.0f);
                Write_EEP_16bit(scaled_soc_int, ADDR_MAIN_SOC);
            }
            soc_count++;
            break;

        case 4: // Validate Main Bank
            if (pending_backup_save) Write_EEP_16bit(FLAG_VALID, ADDR_MAIN_FLAG);
            soc_count++;
            break;

        case 50: // Invalidate Backup Bank
            if (pending_backup_save) Write_EEP_16bit(FLAG_INVALID, ADDR_BACKUP_FLAG);
            soc_count++;
            break;

        case 51: // Write Backup SOC (Scaled Integer)
            if (pending_backup_save) {
                Uint16 scaled_soc_int = (Uint16)(confirmed_soc * 100.0f);
                Write_EEP_16bit(scaled_soc_int, ADDR_BACKUP_SOC);
            }
            soc_count++;
            break;

        case 52: // Validate Backup Bank & Complete Transaction
            if (pending_backup_save) {
                Write_EEP_16bit(FLAG_VALID, ADDR_BACKUP_FLAG);
                confirmed_saved_soc = confirmed_soc;
                pending_backup_save = 0; // Lower the flag, transaction is complete
            }
            soc_count++;
            break;

        case 10:
            if (save_chg_energy_flag)
            {
                Uint8 write_buff[4];
                write_buff[3] = (chg_energy_acc >> 24) & 0xFF;
                write_buff[2] = (chg_energy_acc >> 16) & 0xFF;
                write_buff[1] = (chg_energy_acc >> 8) & 0xFF;
                write_buff[0] = (chg_energy_acc) & 0xFF;

                // Write to Main Address
                Status_write = I2CA_WriteToManyRegs(EEPROM_ADDR, EEPROM_chg_energy_addr,
                                                    EEP_ADDR_LENGTH, write_buff, 4);
            }
            soc_count++;
            break;

        case 11:
            // Idle tick. We wait 50ms to ensure the Main EEPROM page write is complete.
            soc_count++;
            break;

        case 12:
            if (save_chg_energy_flag)
            {
                Uint8 write_buff[4];
                write_buff[3] = (chg_energy_acc >> 24) & 0xFF;
                write_buff[2] = (chg_energy_acc >> 16) & 0xFF;
                write_buff[1] = (chg_energy_acc >> 8) & 0xFF;
                write_buff[0] = (chg_energy_acc) & 0xFF;

                // Write to Backup Address
                Status_write = I2CA_WriteToManyRegs(EEPROM_ADDR, EEPROM_chg_energy_backup_addr,
                                                    EEP_ADDR_LENGTH, write_buff, 4);
            }
            soc_count++;
            break;

        case 13:
            if (save_chg_energy_flag)
            {
                // Both writes successful. Update tracking and lower the flag.
                last_saved_chg_energy = chg_energy_acc;
                save_chg_energy_flag = 0;
            }
            soc_count++;
            break;


        case 94:
            if (max_cell_voltage > CHARGE_CELL_VOLTAGE_LIMIT)
            {
                if (charge_complete == 1)
                {
                    soc_reset = 101;
                    *soc = 100;

                    Temp_soc = 100.0f;
                    confirmed_soc = 100.0f;
                    last_saved_soc = 100.0f;
                    pending_backup_save = 1;

                    charge_complete = 2;
                }
            }
            else if(PDU_getData_local.fixSetChrg.bit.cv_mode == 0)
            {
                charge_complete = 0;
            }

            soc_count++;
            break;

        case 100:
            soc_count = 0;
            break;
        default:
            soc_count++;
            break;

    }
}


void Write_EEP_16bit(Uint16 flag_val, Uint8 *addr)
{
    Uint8 buff[2];
    buff[0] = (flag_val >> 8) & 0xFF;
    buff[1] = flag_val & 0xFF;

    Status_write = I2CA_WriteToManyRegs(EEPROM_SOC_ADDR, addr, EEP_ADDR_LENGTH, buff, 2);
    DELAY_US(300);
}

void Write_EEP_Float(float soc_val, Uint8 *addr)
{
    Eep_data temp_data;
    temp_data.value = soc_val;
    Uint8 buff[4];

    buff[0] = temp_data.d_array.byte_0;
    buff[1] = temp_data.d_array.byte_1;
    buff[2] = temp_data.d_array.byte_2;
    buff[3] = temp_data.d_array.byte_3;

    Status_write = I2CA_WriteToManyRegs(EEPROM_SOC_ADDR, addr, EEP_ADDR_LENGTH, buff, 4);
    DELAY_US(300);
}

Uint16 get_Temp_soc(void)
{
    Uint16 safe_val;
    safe_val = (Uint16)(confirmed_soc * 100.0f);
    return safe_val;
}


