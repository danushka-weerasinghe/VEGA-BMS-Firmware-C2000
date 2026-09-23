/*
 * eprom_config.c
 *
 * Created on: Mar 4, 2026
 * Author: miyurul
 */

#include "eprom_config.h"
#include "main_data.h"
#include <BMS_I2C.h>
#include "BMS_controller_CAN.h"


// ---------------------------------------------------------
// SAFETY BOUNDS & DEFAULTS
// ---------------------------------------------------------
// Capacity bounds
#if defined(ETX_10_kWH)

#define MIN_NOMINAL_CAPACITY      10
#define MAX_NOMINAL_CAPACITY      300
#define DEFAULT_NOMINAL_CAPACITY  130

// Current limits bounds (Scaled by 100)
#define DEFAULT_CHARGE_LIMIT      -15000  // -150 Amps
#define DEFAULT_DISCHARGE_LIMIT   15000   // 150 Amps
#define DEFAULT_SC_LIMIT          19000   // 190 Amps

#elif defined(BIKE_2_MODULE_TENPOWER)

#define MIN_NOMINAL_CAPACITY      10
#define MAX_NOMINAL_CAPACITY      300
#define DEFAULT_NOMINAL_CAPACITY  77

// Current limits bounds (Scaled by 100)
#define DEFAULT_CHARGE_LIMIT      -15000  // -150 Amps
#define DEFAULT_DISCHARGE_LIMIT   29000   // 150 Amps
#define DEFAULT_SC_LIMIT          30000   // 190 Amps

#elif defined(ETX_7_kWH)

#define MIN_NOMINAL_CAPACITY      10
#define MAX_NOMINAL_CAPACITY      300
#define DEFAULT_NOMINAL_CAPACITY  100

// Current limits bounds (Scaled by 100)
#define DEFAULT_CHARGE_LIMIT      -15000  // -150 Amps
#define DEFAULT_DISCHARGE_LIMIT   15000   // 150 Amps
#define DEFAULT_SC_LIMIT          19000   // 190 Amps

#elif defined(BIKE_3_MODULE_TENPOWER)

#define MIN_NOMINAL_CAPACITY      10
#define MAX_NOMINAL_CAPACITY      300
#define DEFAULT_NOMINAL_CAPACITY  117

// Current limits bounds (Scaled by 100)
#define DEFAULT_CHARGE_LIMIT      -15000  // -150 Amps
#define DEFAULT_DISCHARGE_LIMIT   15000   // 150 Amps
#define DEFAULT_SC_LIMIT          19000   // 190 Amps

#elif defined(BIKE_3_MODULE_MOLICELL)

#endif

#define MAX_RAW_AMP_LIMIT         500     // Do not accept > 500A

// ---------------------------------------------------------
// FIRMWARE VERSION CONTROL (e.g. BMSA 01 02 A)
// ---------------------------------------------------------
#define FW_VER_PREFIX_1 'B'
#define FW_VER_PREFIX_2 'M'
#define FW_VER_PREFIX_3 'S'
#define FW_VER_PREFIX_4 'A'
#define FW_VER_MAJOR    0x02
#define FW_VER_MINOR    0x03
#define FW_VER_PATCH    'A'

#define PACK_PREFIX_1 'E'
#define PACK_PREFIX_2 'T'
#define PACK_PREFIX_3 'X'
#define DEFAULT_PACK_YEAR   0x00 // Virgin State
#define DEFAULT_PACK_MONTH  0x00
#define DEFAULT_PACK_SERIAL 0x0000

#define DEFAULT_DATE_YEAR   0x00
#define DEFAULT_DATE_MONTH  0x00
#define DEFAULT_DATE_DAY    0x00

Uint8  pack_year   = DEFAULT_PACK_YEAR;
Uint8  pack_month  = DEFAULT_PACK_MONTH;
Uint16 pack_serial = DEFAULT_PACK_SERIAL;

Uint8 fw_update_year  = DEFAULT_DATE_YEAR;
Uint8 fw_update_month = DEFAULT_DATE_MONTH;
Uint8 fw_update_day   = DEFAULT_DATE_DAY;


// 1. RAM Variables
Uint32 nominal_capacity = DEFAULT_NOMINAL_CAPACITY;
int32_t CHARGE_CURRENT_THRESHOLD = DEFAULT_CHARGE_LIMIT;
int32_t DISCHARGE_CURRENT_THRESHOLD = DEFAULT_DISCHARGE_LIMIT;
int32_t SC_CURRENT_THRESHOLD = DEFAULT_SC_LIMIT;

// 2. EEPROM Addresses
Uint8 EEPROM_nominal_cap_addr[2] = {0x01, 0x54};
Uint8 EEPROM_charge_lim_addr[2]    = {0x01, 0x58};
Uint8 EEPROM_discharge_lim_addr[2] = {0x01, 0x5C};
Uint8 EEPROM_sc_lim_addr[2]        = {0x01, 0x60};
Uint8 EEPROM_pack_num_addr[2] = {0x01, 0x64};
Uint8 EEPROM_update_date_addr[2] = {0x01, 0x68};

// flag
volatile Uint8 pending_eeprom_save_param = 0;
volatile Uint8 capacity_health_status = 0x00;
volatile Uint8 currents_health_status = 0x00;
volatile Uint8 pack_health_status = 0x00;

//-----------------------------------------
//volatile int32_t debug_read_chg1 = 0;
//volatile int32_t debug_read_chg2 = 0;
//volatile int32_t debug_read_chg3 = 0;
//volatile int32_t debug_read_chg4 = 0;
//volatile int32_t debug_read_chg5 = 0;
//volatile int32_t debug_read_chg6 = 0;
//volatile int32_t debug_read_chg7 = 0;
//volatile int32_t debug_read_chg8 = 0;
//volatile int32_t debug_read_chg9 = 0;
//-----------------------------------------


// ---------------------------------------------------------
// INITIALIZATION FUNCTIONS
// ---------------------------------------------------------

void init_nominal_capacity(void)
{
    Uint8 rx_bytes[4] = {0};
    Uint16 status;
    Uint8 retry_count = 0;
    Uint8 read_successful = 0;

    do {
        status = I2CA_ReadFromManyRegs(EEPROM_ADDR, &EEPROM_nominal_cap_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, rx_bytes, 4);
        if (status == 0) { read_successful = 1; break; }
        retry_count++;
        DELAY_US(100);
    } while (retry_count < 3);

    if (read_successful == 1) {
        Uint32 read_cap = (Uint32)rx_bytes[0] | ((Uint32)rx_bytes[1] << 8) | ((Uint32)rx_bytes[2] << 16) | ((Uint32)rx_bytes[3] << 24);

        if (read_cap < MIN_NOMINAL_CAPACITY || read_cap > MAX_NOMINAL_CAPACITY)
        {
            // Invalid data detected. Load defaults and trigger a save.
            nominal_capacity = DEFAULT_NOMINAL_CAPACITY;
            pending_eeprom_save_param = PARAM_NOMINAL_CAPACITY;
            capacity_health_status = 0x01;
        } else {
            // Data is safe. Load to RAM.
            nominal_capacity = read_cap;
            capacity_health_status = 0x00;
        }
    } else {
        nominal_capacity = DEFAULT_NOMINAL_CAPACITY;
        capacity_health_status = 0x01;
    }
}

void init_current_limits(void)
{
    Uint8 rx_chg[4] = {0}, rx_dsg[4] = {0}, rx_sc[4] = {0};
    Uint16 status;
    Uint8 retry_count = 0;
    Uint8 read_successful = 0;

    do {
        status  = I2CA_ReadFromManyRegs(EEPROM_ADDR, &EEPROM_charge_lim_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, rx_chg, 4);
        DELAY_US(2000);
        status |= I2CA_ReadFromManyRegs(EEPROM_ADDR, &EEPROM_discharge_lim_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, rx_dsg, 4);
        DELAY_US(2000);
        status |= I2CA_ReadFromManyRegs(EEPROM_ADDR, &EEPROM_sc_lim_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, rx_sc, 4);
        DELAY_US(2000);

        if (status == 0) { read_successful = 1; break; }
        retry_count++;
        DELAY_US(1000);
    } while (retry_count < 3);

    if (read_successful == 1) {
        Uint32 temp_chg = ((Uint32)rx_chg[0]) | (((Uint32)rx_chg[1]) << 8) | (((Uint32)rx_chg[2]) << 16) | (((Uint32)rx_chg[3]) << 24);
        Uint32 temp_dsg = ((Uint32)rx_dsg[0]) | (((Uint32)rx_dsg[1]) << 8) | (((Uint32)rx_dsg[2]) << 16) | (((Uint32)rx_dsg[3]) << 24);
        Uint32 temp_sc  = ((Uint32)rx_sc[0])  | (((Uint32)rx_sc[1]) << 8)  | (((Uint32)rx_sc[2]) << 16)  | (((Uint32)rx_sc[3]) << 24);

        int32_t read_chg = (int32_t)temp_chg;
        int32_t read_dsg = (int32_t)temp_dsg;
        int32_t read_sc  = (int32_t)temp_sc;

        if (read_chg >= 0 || read_chg < -((int32_t)MAX_RAW_AMP_LIMIT * 100) ||
            read_dsg <= 0 || read_dsg >  ((int32_t)MAX_RAW_AMP_LIMIT * 100) ||
            read_sc  <= 0 || read_sc  >  ((int32_t)MAX_RAW_AMP_LIMIT * 100))
       {
            CHARGE_CURRENT_THRESHOLD = DEFAULT_CHARGE_LIMIT;
            DISCHARGE_CURRENT_THRESHOLD = DEFAULT_DISCHARGE_LIMIT;
            SC_CURRENT_THRESHOLD = DEFAULT_SC_LIMIT;
            pending_eeprom_save_param = PARAM_CURRENT_LIMITS;
            currents_health_status = 0x01;
        } else {
            CHARGE_CURRENT_THRESHOLD = read_chg;
            DISCHARGE_CURRENT_THRESHOLD = read_dsg;
            SC_CURRENT_THRESHOLD = read_sc;
            currents_health_status = 0x00;
        }
    } else {
        CHARGE_CURRENT_THRESHOLD = DEFAULT_CHARGE_LIMIT;
        DISCHARGE_CURRENT_THRESHOLD = DEFAULT_DISCHARGE_LIMIT;
        SC_CURRENT_THRESHOLD = DEFAULT_SC_LIMIT;
        currents_health_status = 0x01;
    }
}

void init_pack_number(void)
{
    Uint8 rx_bytes[4] = {0};
    Uint16 status;
    Uint8 retry_count = 0;
    Uint8 read_successful = 0;

    do {
        status = I2CA_ReadFromManyRegs(EEPROM_ADDR, &EEPROM_pack_num_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, rx_bytes, 4);
        if (status == 0) { read_successful = 1; break; }
        retry_count++;
        DELAY_US(100);
    } while (retry_count < 3);

    if (read_successful == 1) {
        Uint16 read_serial = (Uint16)rx_bytes[2] | ((Uint16)rx_bytes[3] << 8);

        // Safety: Year 0x20-0x50, Month 0x01-0x12. If unprogrammed, load 0x00 default.
        if (rx_bytes[0] < 0x20 || rx_bytes[0] > 0x50 || rx_bytes[1] == 0x00 || rx_bytes[1] > 0x12)
        {
            pack_year = DEFAULT_PACK_YEAR;
            pack_month = DEFAULT_PACK_MONTH;
            pack_serial = DEFAULT_PACK_SERIAL;
            pending_eeprom_save_param = PARAM_PACK_NUMBER;
            pack_health_status = 0x01; // Latch Fault
        } else {
            pack_year = rx_bytes[0];
            pack_month = rx_bytes[1];
            pack_serial = read_serial;
            pack_health_status = 0x00; // Healthy
        }
    } else {
        pack_year = DEFAULT_PACK_YEAR;
        pack_month = DEFAULT_PACK_MONTH;
        pack_serial = DEFAULT_PACK_SERIAL;
        pack_health_status = 0x01; // Latch Fault
    }
}

void init_fw_update_date(void)
{
    Uint8 rx_bytes[4] = {0};
    Uint16 status;
    Uint8 retry_count = 0;
    Uint8 read_successful = 0;

    do {
        status = I2CA_ReadFromManyRegs(EEPROM_ADDR, &EEPROM_update_date_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, rx_bytes, 4);
        if (status == 0) { read_successful = 1; break; }
        retry_count++;
        DELAY_US(100);
    } while (retry_count < 3);

    if (read_successful == 1) {
        // Safety: Year 0x20-0x50, Month 0x01-0x12, Day 0x01-0x31.
        if (rx_bytes[0] < 0x20 || rx_bytes[0] > 0x50 ||
            rx_bytes[1] == 0x00 || rx_bytes[1] > 0x12 ||
            rx_bytes[2] == 0x00 || rx_bytes[2] > 0x31)
        {
            fw_update_year  = DEFAULT_DATE_YEAR;
            fw_update_month = DEFAULT_DATE_MONTH;
            fw_update_day   = DEFAULT_DATE_DAY;
            pending_eeprom_save_param = PARAM_UPDATE_DATE;
        } else {
            fw_update_year  = rx_bytes[0];
            fw_update_month = rx_bytes[1];
            fw_update_day   = rx_bytes[2];
        }
    } else {
        fw_update_year  = DEFAULT_DATE_YEAR;
        fw_update_month = DEFAULT_DATE_MONTH;
        fw_update_day   = DEFAULT_DATE_DAY;
    }
}


// ---------------------------------------------------------
// CAN MESSAGE PARSER (Runs inside the ISR)
// ---------------------------------------------------------
void parse_config_update_cmd(Uint16* rx_data)
{
    Uint8 param_index = rx_data[0] & 0xFF;

    if (param_index == PARAM_NOMINAL_CAPACITY)
    {
        Uint32 new_val = 0;
        new_val |= (rx_data[1] & 0xFF);
        new_val |= (rx_data[2] & 0xFF) << 8;
        new_val |= ((Uint32)rx_data[3] & 0xFF) << 16;
        new_val |= ((Uint32)rx_data[4] & 0xFF) << 24;

        if (new_val >= MIN_NOMINAL_CAPACITY && new_val <= MAX_NOMINAL_CAPACITY) {
            nominal_capacity = new_val;
            pending_eeprom_save_param = PARAM_NOMINAL_CAPACITY;
        }
    }
    else if (param_index == PARAM_CURRENT_LIMITS)
    {
        Uint16 raw_chg = (rx_data[1] & 0xFF) | ((rx_data[2] & 0xFF) << 8);
        Uint16 raw_dsg = (rx_data[3] & 0xFF) | ((rx_data[4] & 0xFF) << 8);
        Uint16 raw_sc  = (rx_data[5] & 0xFF) | ((rx_data[6] & 0xFF) << 8);
//        debug_read_chg4 = raw_chg;
//        debug_read_chg5 = raw_dsg;
//        debug_read_chg6 = raw_sc;

        // Safety bounds check on the raw CAN data
        if (raw_chg > 0 && raw_chg <= MAX_RAW_AMP_LIMIT &&
            raw_dsg > 0 && raw_dsg <= MAX_RAW_AMP_LIMIT &&
            raw_sc  > 0 && raw_sc  <= MAX_RAW_AMP_LIMIT)
        {
            // Scale by 100, and apply negative sign to charge limit
            CHARGE_CURRENT_THRESHOLD = -1 * (int32_t)raw_chg * 100;
            DISCHARGE_CURRENT_THRESHOLD = (int32_t)raw_dsg * 100;
            SC_CURRENT_THRESHOLD = (int32_t)raw_sc * 100;

            // Trigger the background saver
            pending_eeprom_save_param = PARAM_CURRENT_LIMITS;
        }
    }
    else if (param_index == PARAM_PACK_NUMBER)
    {
        Uint8 new_year  = rx_data[1] & 0xFF;
        Uint8 new_month = rx_data[2] & 0xFF;

        // Safely reconstruct the 16-bit serial number from CAN bytes
        Uint16 new_serial = (rx_data[3] & 0xFF) | ((rx_data[4] & 0xFF) << 8);

        // Safety check before saving (Forcing the user to enter a valid year/month!)
        if (new_year >= 0x20 && new_year <= 0x50 && new_month >= 0x01 && new_month <= 0x12)
        {
            pack_year = new_year;
            pack_month = new_month;
            pack_serial = new_serial;
            pending_eeprom_save_param = PARAM_PACK_NUMBER; // Trigger save
        }
    }
    else if (param_index == PARAM_UPDATE_DATE)
    {
        Uint8 new_year  = rx_data[1] & 0xFF;
        Uint8 new_month = rx_data[2] & 0xFF;
        Uint8 new_day   = rx_data[3] & 0xFF;

        // Safety check before saving
        if (new_year >= 0x20 && new_year <= 0x50 &&
            new_month >= 0x01 && new_month <= 0x12 &&
            new_day >= 0x01 && new_day <= 0x31)
        {
            fw_update_year  = new_year;
            fw_update_month = new_month;
            fw_update_day   = new_day;
            pending_eeprom_save_param = PARAM_UPDATE_DATE; // Trigger save
        }
    }
}

void save_parameter_to_eeprom(void)
{
    if (pending_eeprom_save_param == PARAM_NOMINAL_CAPACITY)
    {
        Uint8 tx_bytes[4];

        tx_bytes[0] = nominal_capacity & 0xFF;
        tx_bytes[1] = (nominal_capacity >> 8) & 0xFF;
        tx_bytes[2] = (nominal_capacity >> 16) & 0xFF;
        tx_bytes[3] = (nominal_capacity >> 24) & 0xFF;

        I2CA_WriteToManyRegs(EEPROM_ADDR, &EEPROM_nominal_cap_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, tx_bytes, 4);
        DELAY_US(10000);
    }
    else if (pending_eeprom_save_param == PARAM_CURRENT_LIMITS)
    {
        Uint8 tx_chg[4], tx_dsg[4], tx_sc[4];

        tx_chg[0] = (Uint32)CHARGE_CURRENT_THRESHOLD & 0xFF;
        tx_chg[1] = ((Uint32)CHARGE_CURRENT_THRESHOLD >> 8) & 0xFF;
        tx_chg[2] = ((Uint32)CHARGE_CURRENT_THRESHOLD >> 16) & 0xFF;
        tx_chg[3] = ((Uint32)CHARGE_CURRENT_THRESHOLD >> 24) & 0xFF;

        tx_dsg[0] = (Uint32)DISCHARGE_CURRENT_THRESHOLD & 0xFF;
        tx_dsg[1] = ((Uint32)DISCHARGE_CURRENT_THRESHOLD >> 8) & 0xFF;
        tx_dsg[2] = ((Uint32)DISCHARGE_CURRENT_THRESHOLD >> 16) & 0xFF;
        tx_dsg[3] = ((Uint32)DISCHARGE_CURRENT_THRESHOLD >> 24) & 0xFF;

        tx_sc[0] = (Uint32)SC_CURRENT_THRESHOLD & 0xFF;
        tx_sc[1] = ((Uint32)SC_CURRENT_THRESHOLD >> 8) & 0xFF;
        tx_sc[2] = ((Uint32)SC_CURRENT_THRESHOLD >> 16) & 0xFF;
        tx_sc[3] = ((Uint32)SC_CURRENT_THRESHOLD >> 24) & 0xFF;

        I2CA_WriteToManyRegs(EEPROM_ADDR, &EEPROM_charge_lim_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, tx_chg, 4);
        DELAY_US(10000);

        I2CA_WriteToManyRegs(EEPROM_ADDR, &EEPROM_discharge_lim_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, tx_dsg, 4);
        DELAY_US(10000);

        I2CA_WriteToManyRegs(EEPROM_ADDR, &EEPROM_sc_lim_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, tx_sc, 4);
        DELAY_US(10000);
    }
    else if (pending_eeprom_save_param == PARAM_PACK_NUMBER)
    {
        Uint8 tx_bytes[4];

        tx_bytes[0] = pack_year;
        tx_bytes[1] = pack_month;

        tx_bytes[2] = pack_serial & 0xFF;         // Low Byte
        tx_bytes[3] = (pack_serial >> 8) & 0xFF;  // High Byte

        I2CA_WriteToManyRegs(EEPROM_ADDR, &EEPROM_pack_num_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, tx_bytes, 4);
        DELAY_US(10000);
    }

    else if (pending_eeprom_save_param == PARAM_UPDATE_DATE)
    {
        Uint8 tx_bytes[4];

        tx_bytes[0] = fw_update_year;
        tx_bytes[1] = fw_update_month;
        tx_bytes[2] = fw_update_day;
        tx_bytes[3] = 0x00; // Padding to align memory block

        I2CA_WriteToManyRegs(EEPROM_ADDR, &EEPROM_update_date_addr[EEP_ADDR_LOC], EEP_ADDR_LENGTH, tx_bytes, 4);
        DELAY_US(10000);
    }

    pending_eeprom_save_param = 0;
}


// ---------------------------------------------------------
// REAL-TIME EEPROM DATA BROADCAST (CAN ID 0x35)
// ---------------------------------------------------------
void broadcast_eeprom_data_over_can(void)
{
    static Uint8 mux_state = 1;

    if (mux_state == 1)
    {
        // --- MUX PAGE 1: NOMINAL CAPACITY ---
        canMap.msg[CAN_OBJ_9].sendData[0] = 0x01; // Mux ID

        canMap.msg[CAN_OBJ_9].sendData[1] = nominal_capacity & 0xFF;
        canMap.msg[CAN_OBJ_9].sendData[2] = (nominal_capacity >> 8) & 0xFF;
        canMap.msg[CAN_OBJ_9].sendData[3] = (nominal_capacity >> 16) & 0xFF;
        canMap.msg[CAN_OBJ_9].sendData[4] = (nominal_capacity >> 24) & 0xFF;

        canMap.msg[CAN_OBJ_9].sendData[5] = 0x00;
        canMap.msg[CAN_OBJ_9].sendData[6] = 0x00;
        canMap.msg[CAN_OBJ_9].sendData[7] = capacity_health_status;

        mux_state = 2;
    }
    else if (mux_state == 2)
    {
        // --- MUX PAGE 2: CURRENT LIMITS ---
        canMap.msg[CAN_OBJ_9].sendData[0] = 0x02; // Mux ID

        Uint16 send_chg = (Uint16)(CHARGE_CURRENT_THRESHOLD / -100L);
        Uint16 send_dsg = (Uint16)(DISCHARGE_CURRENT_THRESHOLD / 100L);
        Uint16 send_sc  = (Uint16)(SC_CURRENT_THRESHOLD / 100L);

        // Pack Charge (Bytes 1-2)
        canMap.msg[CAN_OBJ_9].sendData[1] = send_chg & 0xFF;
        canMap.msg[CAN_OBJ_9].sendData[2] = (send_chg >> 8) & 0xFF;

        // Pack Discharge (Bytes 3-4)
        canMap.msg[CAN_OBJ_9].sendData[3] = send_dsg & 0xFF;
        canMap.msg[CAN_OBJ_9].sendData[4] = (send_dsg >> 8) & 0xFF;

        // Pack Short Circuit (Bytes 5-6)
        canMap.msg[CAN_OBJ_9].sendData[5] = send_sc & 0xFF;
        canMap.msg[CAN_OBJ_9].sendData[6] = (send_sc >> 8) & 0xFF;

        canMap.msg[CAN_OBJ_9].sendData[7] = currents_health_status;

        mux_state = 3;
    }
    else if (mux_state == 3)
    {
        // --- MUX PAGE 3: FIRMWARE VERSION ---
        canMap.msg[CAN_OBJ_9].sendData[0] = 0x03; // Mux ID

        // Pack the version string: "BMSA 01 02 A"
        canMap.msg[CAN_OBJ_9].sendData[1] = FW_VER_PREFIX_1; // 'B' (ASCII: 0x42)
        canMap.msg[CAN_OBJ_9].sendData[2] = FW_VER_PREFIX_2; // 'M' (ASCII: 0x4D)
        canMap.msg[CAN_OBJ_9].sendData[3] = FW_VER_PREFIX_3; // 'S' (ASCII: 0x53)
        canMap.msg[CAN_OBJ_9].sendData[4] = FW_VER_PREFIX_4; // 'A' (ASCII: 0x41)
        canMap.msg[CAN_OBJ_9].sendData[5] = FW_VER_MAJOR;    // 0x01
        canMap.msg[CAN_OBJ_9].sendData[6] = FW_VER_MINOR;    // 0x02
        canMap.msg[CAN_OBJ_9].sendData[7] = FW_VER_PATCH;    // 'A' (ASCII: 0x41)

        mux_state = 4;
    }
    else if (mux_state == 4)
    {
        // --- MUX PAGE 4: PACK NUMBER ---
        canMap.msg[CAN_OBJ_9].sendData[0] = 0x04; // Mux ID

        // Pack the string: e.g. "ETX 26 01 0202"
        canMap.msg[CAN_OBJ_9].sendData[1] = PACK_PREFIX_1; // 'E'
        canMap.msg[CAN_OBJ_9].sendData[2] = PACK_PREFIX_2; // 'T'
        canMap.msg[CAN_OBJ_9].sendData[3] = PACK_PREFIX_3; // 'X'

        canMap.msg[CAN_OBJ_9].sendData[4] = pack_year;     // 0x26
        canMap.msg[CAN_OBJ_9].sendData[5] = pack_month;    // 0x01

        // Safely deconstruct the 16-bit serial into 8-bit chunks for CAN
        canMap.msg[CAN_OBJ_9].sendData[6] = (pack_serial >> 8) & 0xFF; // High byte (0x02)
        canMap.msg[CAN_OBJ_9].sendData[7] = pack_serial & 0xFF;        // Low byte  (0x02)

        mux_state = 5;
    }
    else if (mux_state == 5)
    {
        // --- MUX PAGE 5: FIRMWARE UPDATE DATE ---
        canMap.msg[CAN_OBJ_9].sendData[0] = 0x05; // Mux ID

        canMap.msg[CAN_OBJ_9].sendData[1] = fw_update_year;   // e.g. 0x26
        canMap.msg[CAN_OBJ_9].sendData[2] = fw_update_month;  // e.g. 0x03
        canMap.msg[CAN_OBJ_9].sendData[3] = fw_update_day;    // e.g. 0x25

        canMap.msg[CAN_OBJ_9].sendData[4] = 0x00;
        canMap.msg[CAN_OBJ_9].sendData[5] = 0x00;
        canMap.msg[CAN_OBJ_9].sendData[6] = 0x00;
        canMap.msg[CAN_OBJ_9].sendData[7] = 0x00;

        mux_state = 1;
    }

    CAN_sendMessage(CAN_OBJ_9, NOMINAL_DLC, canMap.msg[CAN_OBJ_9].sendData);
}
