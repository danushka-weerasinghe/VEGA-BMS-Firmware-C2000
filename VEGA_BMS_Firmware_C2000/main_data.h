/*
 * main_data.h
 *
 *  Created on: Nov 20, 2019
 *      Author: HARSHANA RATHNAYAKE
 */


#ifndef MAIN_DATA
#define MAIN_DATA

#include <stdbool.h>

//Slope of temperature sensor (deg. C / ADC code, fixed pt Q15 format)
#define getTempSlope() (*(int (*)(void))0x3D7E82)()
//ADC code corresponding to temperature sensor output at 0-degreesC
#define getTempOffset() (*(int (*)(void))0x3D7E85)()

/*Define current sensor*/
#define shunt_CS
//#define hall_CS
//#define recovery_CS

/*Define battery*/
//#define old_version
#define sealed_version

/*Define master or slave*/
#define MASTER
//#define SLAVE

/*Define chiller*/
#define CAN_CHILLER
//#define SCI_CHILLER

/*Define update status*/
//#define J_TAG_BOOTLOADING
#define CAN_BOOTLOADING

#define EVCU_ACTIVE

/*Reset the SOC to defined value when moving avg is 3.00V*/
//#define FIX_SOC
#define SOC_LV_CORRECTION 5


#ifdef MASTER
#define PRECHARGE_DELAY 20
#endif

#ifdef SLAVE
#define PRECHARGE_DELAY 60
#endif

//#define nominal_capacity 130
#define ADC_ADDR         0x49
#define EEPROM_ADDR      0x54
#define EEPROM_SOC_ADDR  0x55
#define EEPROM_ST_ADDR   0x56
#define EEP_ADDR_LENGTH  2
#define EEP_ADDR_LOC     0

/*Define EEPROM*/
/*
 *
 */
#define ACTIVE 1
#define DEACTIVE 0
#define CANBUS ACTIVE
#define run

//#define AUXILARY_PACK ACTIVE

#define TOTAL_IC 3 //max 16
#define AUX_PER_IC 6
#define TEMP_PER_IC 3  //max 16
#define CELLS_PER_IC 9 //max 16
#define STAT_REG 4
#define DATA_LOOP_COUNT_n 5
#define SEND_DATA_FREQ 8 //(val/10) seconds
//#define RS485_current_sensor
//#define CAN_current_sensor

/*
 * AUX data
 */

#define CELLS_PER_AUX_IC 4
#define TEMP_PER_AUX_IC 8

/*
 * SOC data
 */
#define NUMBER_OF_HOURS_TO_SOC_INIT 6

/*
 * digital current sensor
 */
//#define RS485_CS_SET
//#define CAN_CS_SET

/*
 * Definition of operation modes
 */
#define INITIAL 0x00
#define CHARGE 0x43
#define OP_MODE 0x64
#define SAFETY 0x73
#define DEBUG 0x47


#define DUMMY 0xFF

/*
 * Definition of control bytes
 */
#define REQUEST 0x52    //%
#define SETUP 0x24      //$
#define CONTROL 0x63    //c
#define UPDATE 0x55     //U
#define DEFAULT 0xDD
#define RESEND 0x81
#define ERROR_C 0x0E

/*
 * CAN MSG IDs
 */
#define OTA_FLAG_ADDR 0x17B84
#define OTA_RESPONSE_ADDR 0x67B84
#define EVCC_ADDR 0x62
#define BMS_CMD_ADDR 0xDB
#define BMS_DEBUG_ADDR 0x69
#define BMS_CAN_DATA_ADDR 0x32
#define EVCU_ADDR 0x31
#define EVCU_ODO_ADDR 0x41
#define GET_SLAVE_ADDR 0x30
#define SET_SLAVE_ADDR 0x29
#define CHILLER_RX_ADDR 0x248
#define CHILLER_TX_ADDR 0x238
#define SET_CHG_ENERGY 0xEC
#define BMS_CONFIG_UPDATE_ADDR 0xED
#define EEPROM_BROADCAST_ADDR 0x35
/*
 * Request type & update type
 */
#define ALL_DATA 0x41
#define OPEN_WIRE 0x4F
#define CLASSIFIED 0x66
#define RECORDED 0x75
#define AUX_DATA 0x8A
#define HV_DATA 0x8D
#define HUMIDITY_TEMPERATURE 0xAD

/*
 * safety data
 */
#ifdef RS485_current_sensor
#define CHARGE_CURRENT_THRESHOLD 300
#define DISCHARGE_CURRENT_THRESHOLD 10000
#endif
#ifdef CAN_current_sensor
#define CHARGE_CURRENT_THRESHOLD -12000
#define DISCHARGE_CURRENT_THRESHOLD 20000
#endif
//#define CHARGE_CURRENT_THRESHOLD -15000
//#define DISCHARGE_CURRENT_THRESHOLD 15000
//#define SC_CURRENT_THRESHOLD 19000

#define COM_ERROR_COUNT_THRESHOLD 3
#define HIGHEST_CELL_VOLTAGE_LIMIT 36000
#define CHARGE_CELL_VOLTAGE_LIMIT 35000
#define LOWEST_CELL_VOLTAGE_LIMIT 28000
#define LOWEST_CELL_VOLTAGE_CUTOFF 25000

#define HIGHEST_TEMPERATURE_LIMIT 51
#define TEMPERATURE_CUTOFF 53
#define PRECHARGE_TIME_LIMIT 20             /*1000ms/50ms*/
#define CONT_DELAY_TIME 120                  /*Delay in seconds after error issued*/
#define DISCHARGE_MODE 0
#define CHARGE_MODE 1

/*
 * battery data
 */
#define CHARGE_COMP_CURRENT_THRESHOLD -500
#define CV_END_CURRENT 500
#define dcov_threshold 35500
#define dcuv_threshold 28000
#define dcov_recovery_threshold 36000
#define dcuv_recovery_threshold 25000
#define dcov_time 10
#define dcuv_time 10

#define dpack_ot_threshold 50
#define dpack_ot_time 300

#define dpack_end_of_charge_voltage 43200
#define dpack_end_of_discharge_voltage 33600
#define dmax_charge_current 72

#define dcell_imbalance_fail_threshold 10000
#define dcell_imbalance_fail_time 7200
#define dbalancing_volt_threshold 100
#define dmin_balance_volts 28000

#define lowest_cell_dev_threshold 30000
#define highest_cell_dev_threshold 41000

#define highest_temp_dev_threshold 45
#define lowest_temp_dev_threshold -10

#define CONTACTOR_1_K 0
#define CONTACTOR_2_K 1

//|4-Balancing_timer|1-Refon|1-ADC_Mode|1-ADC_freq|1-Discharge_en|
#define config_byte 0b11111011

/*
 * OTA_Update
 */

//MEMORY ADDRESSES
#define FLASH_FLAG_ADDRESS 0x3E4000
#define JUMP_FLAG_ADDRESS 0x3E4001
#define BACKUP_FLAG_ADDRESS 0x3E4002
#define PACKET_FLAG_ADDRESS 0x3E4003
#define KERNAL_ADDRESS 0x3F7FF6
#define FLASH_FLAG_LENGTH 4

//STATE FLAGS
#define BOOTLOADER 0xAAAA
#define BOOT_CONFIG 0x5555
#define APPLICATION 0x4444
#define BACKUP 0xBBBB

typedef enum
{
    NULL_STATE,
    FULL_STATE,
    OTA_HANDSHAKE,
    OTA_RESET,
    BOOTLOADER_STATE,
    BACKUP_STATE
}kernal_state;

//#define INIT_ACK 1
//#define START_ACK 2

typedef enum
{
    INIT_ACK,
    START_ACK
}acknowlagements;

typedef enum
{
    FLASH_FLAG,
    JUMP_FLAG,
    BACKUP_FLAG,
    PACKET_FLAG
}flags;

/*
 * LED indicators
 */

#define LED_ON

/*
 * lowest cell voltage moving avg window
 */
#define MOVING_AVG_WINDOW 40


/*
 * Functions
 */

void BMS_can_data();
void SCI_reset(void);
/*
 * BMS data communication function declaration
 */
void data_seperator();
//Uint8 request_packet_ready();
Uint8 control_packet_ready();
Uint8 setup_packet_ready();
//void packet_maker_MSP(Uint8 address, Uint8 control_byte, Uint8 operating_mode_d);

//Read temperature
void readTemperatures();
//Read Voltages
void readVoltages();
//Read Current
void readCurrent();
//Analyze data
void seperateBMSdata(unsigned char receved_data_length,
                     unsigned char *SPI_rec_dat_buffer);
void master_dataSort(void);
//BMS CAN data
void BMS_CAN_data();
//LOG data
void logData();


//------------------------------------//
void board_temperature();
void MspConfig(Uint8 pack_num, Uint8 controll_byte, Uint8 op_mode);
void Request_classified(Uint8 pack_num, Uint8 op_mode_con);
void Request_humidity(Uint8 pack_num, Uint8 op_mode_con);
void Request_openWire(Uint8 pack_num, Uint8 op_mode_con);
void Resend_packet(Uint8 pack_num, Uint8 op_mode_con);
void Request_all_data(Uint8 IC_no, Uint8 pack_num, Uint8 op_mode_con);
void analyze_rdata();
void analyse_humidity_temp(Uint8 local_IC);
//------------------------------------//
void update_bad_crc(unsigned short ch);
Uint16 crc_1021(unsigned char data_length, unsigned char data_crc[]);
unsigned char spia_send_packet(unsigned char data[], Uint8 data_length);
unsigned char spia_send_byte(unsigned char data);
unsigned char byte_stuffing(unsigned char length, unsigned char data_packet[]);
//------------------------------------//

Uint16 ModRTU_CRC(volatile Uint16 buf[], int len);

//------------------------------------//

void scan_contactor_fb();
void scan_emergency();
void Msp1_reset();
void Msp2_reset();
Uint8 slave_controller(void);
void timer_task_can();
void contactor_operator();
void contactor_operator_fire();
void time_out_request();
void send_to_pc();
void main_msp_config_loop();
void scondary_can_send();
void hv_batterry_read(Uint8 *C_ic);
void time_rtc();
void sys_check();
//void send_time_and_soc_eeprom(Uint8 buff_local[]);
//void EEPROM_op();
//void soc_int_fun(Uint8 set_val,Uint8 buff_local[]);
//Uint8 time_diff_cal(Uint8 buff_local[]);

double adc_read_asd(Uint16 adc_addr);

enum trip_event_enum{no_error=0,hi_v_error,hi_t_error,com_error,cc_threshold, dc_threshold, l_v_error,one_time_emg_evt};
enum bms_opMode_enum{not_initialized = 0, initialized, contactor_closed, error, emergency_event, trip_event};
enum chiller_comp_state_enum{off = 0, on, compressor_error};
enum chiller_error_enum{no_fault = 0, over_current, running_ov, running_uv, standby_ov, standby_uv, comm_failure, speed_error};
enum humidity_sensor_error_enum{read_okay = 0, data_read_error, humidity_error, temperature_error};
/*! Cell Voltage data structure. */
typedef struct
{
    unsigned int c_voltage[12]; //!< Cell Voltage Codes
    unsigned int max_cell_v_local; //maximum cell voltage @ the current ic
    unsigned int min_cell_v_local; //minimum cell voltage @ the current ic
    unsigned char max_cell_id_local :4; //maximum cell id @ the current ic
    unsigned char min_cell_id_local :4; //minimum cell id @ the current ic
} cv;

/*! AUX Reg Voltage Data structure */
typedef struct
{
    unsigned int a_voltage[6];
} ax;

/*! Status Reg data structure. */
typedef struct
{
    unsigned int stat_codes[4]; //!< Status codes.
    unsigned char flags[3]; //!< Byte array that contains the uv/ov flag data
    unsigned char mux_fail :1; //!< Mux self test status flag
    unsigned char thsd :1; //!< Thermal shutdown status
} st;

/*! Register configuration structure */
typedef struct
{
    unsigned char cell_channels; //!< Number of Cell channels
    unsigned char stat_channels; //!< Number of Stat channels
    unsigned char aux_channels;  //!< Number of Aux channels
    unsigned char num_cv_reg;    //!< Number of Cell voltage register
    unsigned char num_gpio_reg;  //!< Number of Aux register
    unsigned char num_stat_reg;  //!< Number of  Status register
} register_cfg;

/*! IC register structure. */
typedef struct
{
    unsigned char gpio_config :5;
    unsigned char refon :1;
    unsigned char DTEN :1;
    unsigned char ADC_opt :1;
    Uint16 UV_config :12;
    Uint16 OV_config :12;
    Uint16 discharge_cells :12;
    Uint8 discharge_timer :4;
} ic_register;

typedef struct
{
    Uint8 tx_data[6];  //!< Stores data to be transmitted
} ic_register_b;

typedef struct
{
    Uint16 highest_v;
    Uint16 lowest_v;
    Uint16 highest_t;
    Uint16 lowest_t;
    Uint16 internal_t;
    Uint32 h_v_time :24;
    Uint32 l_v_time :24;
    Uint32 h_t_time :24;
    Uint32 l_t_time :24;
    Uint32 internal_t_time :24;
} recorded_data;

typedef struct
{
    Uint16 discharge_en_cells :12;
    Uint8 discharge_en;
} ctrl_byte;


typedef enum Battery_config
{
    iso_spi_reverse,

    /*voltage data*/
    cov_treshold,
    cuv_treshold,
    cov_recovery_treshold,
    cuv_recovery_treshold,
    cov_time,
    cuv_time,

    /*temperature data*/
    pot_treshold,
    pot_time,

    /*charge and discharge data*/
    pack_end_of_charge_voltage,
    pack_end_of_discharge_voltage,
    max_charge_current,

    /*Balancing*/
    cell_imbalance_fail_treshold,
    cell_imbalance_fail_time,
    balancing_time,
    balance_volt_treshold,
    min_balance_volts,
    max_balance_time
} battery_config_t;

//typedef enum
//{
//    OTA_FLAG_ADDR = 0x17B84,
//    OTA_RESPONSE_ADDR = 0x67B84,
//    EVCC_ADDR = 0x62,
//    BMS_CMD_ADDR = 0xDB,
//    BMS_DEBUG_ADDR = 0x69,
//    BMS_CAN_DATA_ADDR = 0x32,
//    EVCU_ADDR = 0x31,
//    GET_SLAVE_ADDR = 0x30,
//    SET_SLAVE_ADDR = 0x29,
//    CHILLER_RX_ADDR = 0x248,
//    CHILLER_TX_ADDR = 0x238
//}CAN_msg_addr;

typedef struct
{
    Uint8 sht30_read_enable_flag:1;
    Uint8 open_wire_read_enable_flag:1;
    Uint8 reserved:6;
}slave_data_flag_bits;

typedef union
{
    Uint8 all;
    slave_data_flag_bits bit;
}slave_data_flags;

typedef struct
{
    Uint8 uv_flag_0:1;
    Uint8 ov_flag_0:1;
    Uint8 uv_flag_1:1;
    Uint8 ov_flag_1:1;
    Uint8 uv_flag_2:1;
    Uint8 ov_flag_2:1;
    Uint8 uv_flag_3:1;
    Uint8 ov_flag_3:1;
    Uint8 uv_flag_4:1;
    Uint8 ov_flag_4:1;
    Uint8 uv_flag_5:1;
    Uint8 ov_flag_5:1;
    Uint8 uv_flag_6:1;
    Uint8 ov_flag_6:1;
    Uint8 uv_flag_7:1;
    Uint8 ov_flag_7:1;
    Uint8 uv_flag_8:1;
    Uint8 ov_flag_8:1;
    Uint8 uv_flag_9:1;
    Uint8 ov_flag_9:1;
    Uint8 uv_flag_10:1;
    Uint8 ov_flag_10:1;
    Uint8 uv_flag_11:1;
    Uint8 ov_flag_11:1;
} v_flag_bits;

typedef union
{
    Uint16 all[2];
    v_flag_bits bit;
} v_flags;

typedef struct
{
    Uint16 cell_0:1;
    Uint16 cell_1:1;
    Uint16 cell_2:1;
    Uint16 cell_3:1;
    Uint16 cell_4:1;
    Uint16 cell_5:1;
    Uint16 cell_6:1;
    Uint16 cell_7:1;
    Uint16 cell_8:1;
    Uint16 cell_9:1;
    Uint16 cell_10:1;
    Uint16 cell_11:1;
} cell_bits;

typedef union
{
    Uint16 all;
    cell_bits bit;
} d_cells;

typedef struct
{
    Uint16 gpio_1:1;
    Uint16 gpio_2:1;
    Uint16 gpio_3:1;
    Uint16 gpio_4:1;
    Uint16 gpio_5:1;
} gp_bits;

typedef union
{
    Uint16 all;
    gp_bits bit;
} gpiox;

typedef struct
{
    Uint16 pwm_0:4;
    Uint16 pwm_1:4;
    Uint16 pwm_2:4;
    Uint16 pwm_3:4;
    Uint16 pwm_4:4;
    Uint16 pwm_5:4;
    Uint16 pwm_6:4;
    Uint16 pwm_7:4;
    Uint16 pwm_8:4;
    Uint16 pwm_9:4;
    Uint16 pwm_10:4;
    Uint16 pwm_11:4;
} PWM_regs;

typedef union
{
    Uint16 all[3];
    PWM_regs pwm_reg;
} pwm_dat_regs;


typedef struct
{
    Uint16 pwm_0:4;
    Uint16 pwm_1:4;
    Uint16 pwm_2:4;
    Uint16 pwm_3:4;
    Uint16 pwm_4:4;
    Uint16 pwm_5:4;
    Uint16 pwm_6:4;
    Uint16 pwm_7:4;
    Uint16 pwm_8:4;
    Uint16 pwm_9:4;
    Uint16 pwm_10:4;
    Uint16 pwm_11:4;
} S_ctrl_regs;

typedef union
{
    Uint16 all[3];
    S_ctrl_regs sctrl_regs;
} Sctrl_dat_regs;



typedef struct
{
    Uint16 cell_voltages[12];
    Uint8 highest_cell_id_local:4;
    Uint8 lowest_cell_id_local:4;
    Uint16 aux_voltages[6];
    Uint16 Pack_voltage; //status data
    Uint16 highest_cell_local;
    Uint16 lowest_cell_local;
    char highest_temp:8;
    char lowest_temp:8;
    double I_temp; //Internal temperature
    Uint16 V_regA;
    Uint16 V_regD;
    v_flags UV_OV_flags;
    Uint8 mux_fail:1;
    Uint8 THSD:1;
    Uint8 ADC_test_count[3];
    Uint8 ADC_overlap_error;
    Uint8 Digital_redundancy_error[2];
    d_cells d_cell; //discharge cells
    Uint8 Discharge_cell_timer:4;
    Uint16 OV_config;
    Uint16 UV_config;
    gpiox gpioReg;
    Uint8 refon_bit:1;
    Uint8 D_timer_en:1;
    Uint8 adc_opt:1;
    pwm_dat_regs pwm_reg;
    Sctrl_dat_regs sctrl_reg;
    signed int temparature_val[16];
    Uint8 highest_temp_id_local:4;
    Uint8 lowest_temp_id_local:4;
    Uint8 temp_msb;
    Uint8 temp_lsb;
    Uint8 temp_crc_flag;
    Uint8 humi_msb;
    Uint8 humi_lsb;
    Uint8 humi_crc_flag;
    Uint16 slave_humidity_val;
    Uint16 slave_temperature_val;
    Uint8 open_wire_msb;
    Uint8 open_wire_lsb;
} BMS_DATA;

typedef struct
{
    Uint8 ttl_v[4];
} ttv;
typedef union
{
    Uint32 volt;
    ttv byte;
} ttl_v;

typedef struct
{
    Uint16      BYTE3:8;     // 7:0
    Uint16      BYTE2:8;     // 15:8
    Uint16      BYTE1:8;     // 23:16
    Uint16      BYTE0:8;     // 31:24
}coulomb_bytes;
typedef union
{
    float coulomb_count;
    coulomb_bytes send_coulomb_bytes;
}cumulative_energy;
extern cumulative_energy c_count;

/*
 * Controller configuration
 */
typedef struct
{
    unsigned char bmsData_update_flag:1;
    unsigned char op_mode_main:4;
    unsigned char op_mode_main_set:4;
    unsigned char op_mode_aux_set:4;
    unsigned char op_mode_aux:4;
    unsigned int highest_cell_volt;
    unsigned char highest_cell_id;
    unsigned char highest_pack_id:4;
    unsigned int lowest_cell_volt;
    unsigned char lowest_cell_id;
    unsigned char lowest_pack_id:4;
    //ttl_v total_pack_voltage;
    double total_pack_voltage;
    char highest_temp:8;
    unsigned char highest_temp_id;
    unsigned char highest_temp_pack_id:8;
    char lowest_temp:8;
    unsigned char lowest_temp_id;
    unsigned char lowest_temp_pack_id:8;
    unsigned int highest_temp_ic;
    unsigned char highest_temp_ic_id;
    int highest_board_temp:8;
    unsigned char highest_board_temp_id;
    int lowest_board_temp:8;
    unsigned char lowest_board_temp_id;
    Uint16 error_ltc_hv_count;
    Uint16 error_ltc_aux_count;
    Uint16 error_ltc_hv_count_old;
    Uint16 error_ltc_aux_count_old;
    Uint16 error_msp_hv_count;
    Uint16 error_msp_aux_count;
    unsigned char last_update_hv:8;
    unsigned char last_update_aux:8;
    Uint8 sent_packet_code_hv:8;
    Uint8 received_packet_code_hv:8;
    Uint8 sent_packet_code_aux:8;
    Uint8 received_packet_code_aux:8;
    unsigned char contactor_fb[4];
    unsigned char emergency_on:1;
    unsigned char enable_mb:1;
    unsigned char enable_rb:1;
    unsigned char enable_led:1;
    unsigned int avg_pack_voltage;
    int temperature_power :8;
    int temperature_main :8;
    int temperature_msp1 :8;
    int temperature_msp2 :8;
    Uint16 lowest_pack_v;
    d_cells error_cell_bit;
    float SOC_value;
    float SOH_value;
    double power_kW;
    Uint8 open_wire_ditect_flag;
} CONTROLLER_DATA;
/*
 * MSP configuration
 */
typedef struct{
    Uint8 refon:1;
    Uint8 adcopt:1;
    bool gpio[5];
    bool dcc[12];
    bool dcto[4];
    Uint16 uv_msp;
    Uint16 ov_msp;
}BMS_config_data;

/*External variables*/
extern Uint8 t50_ms_timer;
extern Uint8 charge_state;
extern double current_val_A;
extern Uint8 slave_status;
extern volatile float INA229_data[5];
extern volatile char contactor_state_m;
extern Uint16 volatile INA229_reset_flag;
extern BMS_DATA BMS_data_IC[TOTAL_IC];
extern Uint16 soc_reset;
//extern struct ECAN_REGS ECanaShadow;
//extern CAN_BufferMap canMap;


/**************************************************************************
    I2C ADDRESS/BITS
**************************************************************************/
    #define ADS1100_DEFAULT_ADDRESS         (0x48)    // 1001 000 (ADDR = GND)

/**************************************************************************
    CONVERSION DELAY (in mS)
**************************************************************************/
    #define ADS1100_CONVERSIONDELAY         (100)

/**************************************************************************
    CONFIG REGISTER
**************************************************************************/
    #define ADS1100_REG_CONFIG_OS_MASK      (0x80)      // Conversion
    #define ADS1100_REG_CONFIG_OS_NOEFFECT  (0x00)      // Write: Bit = 0 No effect
    #define ADS1100_REG_CONFIG_OS_SINGLE    (0x80)      // Write: Bit = 1 Begin a conversion (default)
    #define ADS1100_REG_CONFIG_OS_BUSY      (0x00)      // Read: Bit = 0 Device is not performing a conversion
    #define ADS1100_REG_CONFIG_OS_NOTBUSY   (0x80)      // Read: Bit = 1 Device is busy performing a conversion

    #define ADS1100_REG_CONFIG_MODE_MASK    (0x10)      // Device operating mode
    #define ADS1100_REG_CONFIG_MODE_CONTIN  (0x00)      // Continuous conversion mode (default)
    #define ADS1100_REG_CONFIG_MODE_SINGLE  (0x10)      // Single-conversion mode

    #define ADS1100_REG_CONFIG_DR_MASK      (0x0C)      // Data rate
    #define ADS1100_REG_CONFIG_DR_128SPS    (0x00)      // 128 samples per second
    #define ADS1100_REG_CONFIG_DR_32SPS     (0x04)      // 32 samples per second
    #define ADS1100_REG_CONFIG_DR_16SPS     (0x08)      // 16 samples per second
    #define ADS1100_REG_CONFIG_DR_8SPS      (0x0C)      // 8 samples per second (default)

    #define ADS1100_REG_CONFIG_PGA_MASK     (0x03)      // Programmable gain amplifier configuration
    #define ADS1100_REG_CONFIG_PGA_1        (0x00)      // Gain 1 (default)
    #define ADS1100_REG_CONFIG_PGA_2        (0x01)      // Gain 2
    #define ADS1100_REG_CONFIG_PGA_4        (0x02)      // Gain 4
    #define ADS1100_REG_CONFIG_PGA_8        (0x03)      // Gain 8

/**************************************************************************/
#endif /* MAIN_DATA */
