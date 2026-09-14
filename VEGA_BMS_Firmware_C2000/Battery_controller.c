/* Battery controller
 * BMS_Central_Control.c
 *
 *  Created on: 10 Mar, 2021
 *      Author: Harshana rathnayake
 *      2021/12/31
 */

#include "DSP28x_Project.h"
#include "F2806x_Cla_defines.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "PDU_matrix.h"
#include "stdint.h"
#include "main_data.h"
#include "config_ctrl.h"
#include "CLAShared.h"
#include "mcp_can.h"
#include "soc_soh.h"
#include "math.h"
#include "Flash2806x_API_Library.h"
//#include "bootloader.h"
#include "BMS_controller_CAN.h"
#include <RS485_shunt.h>
#include <BMS_I2C.h>
#include <chiller.h>
#include <eprom_config.h>
#include <Pump_PWM.h>

Uint16 count;

//
// Flash Status Structure
//
FLASH_ST FlashStatus;

extern Uint32 Flash_CPUScaleFactor;
extern void (*Flash_CallbackPtr) (void);
Uint16 fail;

//void (*ApplicationPtr) (void);  // Function Prototypes
extern CAN_BufferMap canMap;
extern Uint16 soc_count;
extern Uint8 save_chg_energy_flag;
extern volatile Uint8 eeprom_save_done_flag;

/**
 * CAN Data
 */

#if(CANBUS == ACTIVE)
void ECAN_CONFIG();
//void CLEAR_INTERUPT_FLAGS();
enum bDataType
{
    packData = 0, deviceData
};
//void canData();

volatile int ecana_interrupt_counter = 0;
volatile long ecana_int_set_counter = 0;
volatile long ecana_int_slaveData_counter = 0;
volatile long can_set_count_old = 0;
volatile long chiller_rec_count = 0;

/*
 * lowest cell voltage moving avg window
 */
Uint16 lowest_cell_v_moving_avg = 0xFFFF;


/*
 * Watchdog counters
 */
Uint16 volatile slave_watchdog = 0;
Uint16 volatile shuntCS_watchdog = 0;

long current_com_counts = 0;
unsigned char current_com_count_int = 0;

struct ECAN_REGS ECanaShadow;

#endif
//////////////////////////////
extern Uint16 RamfuncsLoadStart;
/*extern Uint16 RamfuncsLoadEnd;*/
extern Uint16 RamfuncsRunStart;
extern Uint16 RamfuncsLoadSize;

#pragma CODE_SECTION(ecan1_inta_isr, "ramfuncs");
//#pragma CODE_SECTION(txinta, "ramfuncs");
//#pragma CODE_SECTION(rxinta, "ramfuncs");
#pragma CODE_SECTION(sciaRxFifoIsr, "ramfuncs");
#pragma CODE_SECTION(cpu_timer0_isr, "ramfuncs");
#pragma CODE_SECTION(cpu_timer1_isr, "ramfuncs");
//#pragma CODE_SECTION(txintb, "ramfuncs");
//#pragma CODE_SECTION(rxintb, "ramfuncs");
//#pragma CODE_SECTION(xint1_isr, "ramfuncs");

#pragma CODE_SECTION(move_to_kernal, "ramfuncs");
#pragma CODE_SECTION(clear_jump_flag, "ramfuncs");
#pragma CODE_SECTION(resetBMS, "ramfuncs");
#pragma CODE_SECTION(update_BMS, "ramfuncs");

float32 X[FILTER_LEN];
Uint16 VoltFilt;

#pragma DATA_SECTION(A,"CpuToCla1MsgRAM");

float32 A[FILTER_LEN] = { 0.0625L, -0.25L, 0.375L, -0.25L, 0.0625L };

/**
 * ISR
 */
__interrupt void ecan1_inta_isr();
//__interrupt void txinta(void);
//__interrupt void rxinta(void);
//__interrupt void txintb(void);
//__interrupt void rxintb(void);
__interrupt void cpu_timer0_isr();
__interrupt void cpu_timer1_isr();
//__interrupt void xint1_isr(void);

/**
 * Configuration
 */

void C2000_config(void);
void C2000_SCI_int(void);
void C2000_SPI_MSP(void);
void I2CA_Init(void);
//void ECAN_CONFIG(void);

unsigned char SPI_packet_receive();
//----------------//

void SYS_CONFIG();

//void can_bus_tx_flush(void);
int16 Mcu_Temp(void);
int16 prechargerFeedback(void);
void chillerCtrl(uint16_t highestT);
Uint16 OTA_handshake_connect_reset(void);
void move_to_kernal(Uint8 kernal_state);
void send_OTA_ACK(Uint8 ack_state);
void clear_jump_flag();
//Uint16 get_jump_flag(void);
//Uint16 read_OTA_flag(void);
//Uint16 get_backup_flag(void);
Uint16 read_flag(Uint8 flag_status);
void resetBMS(void);
void update_BMS(void);
void setDebugCMD();
int16 voltageBuff[100];
int16 temperature;

Uint16 seconds_counter = 0;

Uint8 contactor_on = 0;
Uint8 contactor_feedback = 0;

volatile Uint16 succed_packet_error_count = 0;

volatile short ENABLE_CAN = 1;
short first_data_cycle_ok = 0;

/*
 * BMS data structures
 */
CONTROLLER_DATA Controller;

BMS_DATA BMS_data_IC[TOTAL_IC];
BMS_DATA BMS_data_aux;
slave_data_flags slave_read;

/*
 * Communication variables
 */

uint16_t debug_CMD = 0;
Uint8 send_packet_code = 0;
Uint8 received_packet_code = 0;
volatile Uint8 Resend_data_flag = 1;
Uint8 config_finished_flag = 0;
Uint8 Resend_request_flag = 0;
enum bms_opMode_enum bms_opMode = not_initialized;
enum trip_event_enum trip_cause = no_error;
enum bms_fc_Mode_enum bms_fc_Mode = not_initialized_fc;
enum humidity_sensor_error_enum SHT30_error = read_okay;
Uint8 slave_status = 0; /* 0 = no slave, 1 = slave present, 2 = slave ok */
Uint8 master_status = 0; /* 0 = no master, 1 = master prsent */
Uint8 soc_soh_flag = 0;
uint16_t contactor_init_flag = 0;
Uint8 charge_state = 2; /*0 = Charging not complete, 1 = charge complete, 2 = not initialized*/

unsigned char SPI_receiver_state = 0;
unsigned char SPI_rec_error = 0;
unsigned char SPI_Packet_rvd_f = 0;
volatile unsigned char SPI_rec_packed_length = 0;
volatile unsigned char timer_100ms = 0;
volatile unsigned char timer_100ms_cfg = 0;
volatile unsigned char msp_no_responce = 1;
Uint8 request_packet_type = CLASSIFIED;
Uint8 aux_request_packet_type = CLASSIFIED;

Uint16 CRC_val_c = 0;
unsigned char classified_all_count = 0;
unsigned char humidity_temperature_count = 0;

volatile unsigned char secondary_can_send_en = 0;
volatile unsigned char timer_flag_can = 0;

unsigned char hv_reset_set = 1; //enable for initial reset of HV-MSP

Uint8 first_timer_config_flag = 0;
Uint8 all_data_requst_over_flag = 0;
Uint8 humidity_temperature_request_over_flag = 0;
Uint8 openWire_request_over_flag = 0;
Uint8 set_time_config_flag = 1;
Uint8 BMS_data_sep_flag = 0;
//Uint8 BMS_Update_flag = 0;
Uint8 OTA_flag = 0;
Uint8 dummy1 = 0;

//I2C buffers
DateTime real_time;
DateTime set_time;
unsigned char timer_ready_rtc = 1;
unsigned char timer_ready_adc = 1;

Uint16 hv_loop_count_bms = 0;
int16 Current_value = 0;
unsigned long communication_error_msp = 0;
unsigned char communication_error = 0;

unsigned long received_packet_counter_0 = 0;
unsigned long config_packet_counter_0 = 0;
unsigned long loop_counter_x = 0;

double current_val_A = 0;
double current_ofst = 0;
Uint8 t50_ms_timer = 0;

/*
 * Data variables
 */
Uint8 discharg_timer[16] = { 0, 1, 1, 2, 3, 4, 5, 10, 15, 20, 30, 40, 60, 75, 90, 120 };
/*
 * SPI msp data buffers
 */

Uint8 send_data_buffer[2][50];
Uint8 received_data_buffer[200];

/*
 * I2C data buffers
 */
Uint8 reg[2]; // Register bytes buffer
Uint8 tx_data_buff[64]; // Data bytes buffer
Uint8 rx_data_buff[64]; // Data bytes buffer

/*
 * On-board temperature sensors lookup table
 */

char temp_board[] = { 127, 123, 118, 114, 110, 107, 104, 101, 99, 97, 94, 92, 91, 89, 87, 86, 84,
                      83, 81, 80, 79, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 65, 64,
                      63, 62, 62, 61, 60, 59, 59, 58, 57, 57, 56, 55, 55, 54, 54, 53, 52, 52, 51,
                      51, 50, 50, 49, 49, 48, 48, 47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42,
                      42, 41, 41, 40, 40, 39, 39, 39, 38, 38, 37, 37, 37, 36, 36, 35, 35, 35, 34,
                      34, 34, 33, 33, 32, 32, 32, 31, 31, 31, 30, 30, 30, 29, 29, 28, 28, 28, 27,
                      27, 27, 26, 26, 26, 25, 25, 25, 24, 24, 24, 23, 23, 23, 22, 22, 22, 21, 21,
                      21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15, 15, 15,
                      14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8,
                      7, 7, 7, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 2, 2, 2, 1, 1, 0, 0, 0, -1, -1, -2, -2,
                      -2, -3, -3, -4, -4, -5, -5, -5, -6, -6, -7, -7, -8, -8, -9, -9, -10, -10, -11,
                      -12, -12, -13, -13, -14, -15, -15, -16, -17, -17, -18, -19, -20, -20 };

Uint16 req_count = 0;

/*RS485 test*/
Uint16 testVar = 0;
Uint16 testStat = 0;
Uint16 timeout_cunt = 0;
volatile Uint8 recovery_current_val = 0;
volatile Uint8 chiller_test = 0;

void main(void)
{
    memset(&BMS_data_IC, 0, sizeof(BMS_data_IC));
    memset(&PDU_getData_local, 0, sizeof(PDU_getData_local));
    memset(&PDU_setDataSlave_local, 0, sizeof(PDU_setDataSlave_local));
    memset(&PDU_getData_i, 0, sizeof(PDU_getData_i));

    memset(&PDU_setDataSlave_i, 0, sizeof(PDU_setDataSlave_i));
    memset(&BMS_data_aux, 0, sizeof(BMS_data_aux));
    memset(&Controller, 0, sizeof(Controller));

    memset(&canMap, 0, sizeof(canMap));

    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32) &RamfuncsLoadSize);

#ifdef CAN_BOOTLOADING
    /*OTA jump initialize*/
    if (read_flag(FLASH_FLAG) == BOOT_CONFIG)
    {
        if(read_flag(JUMP_FLAG) > 1)
        {
            clear_jump_flag();
        }
        else
        {
            move_to_kernal(BACKUP_STATE);
        }
    }
#endif

    /*Initialization*/
    SYS_CONFIG();
    CONFIG_Gpio();
    ADC_CONFIG();
    RS485_ReceiverEn;

    LED1_ON;
    /*configuration MSP430s*/
    hv_reset_set = 1; //enable for initial reset of HV-MSP
    cell_balancing_enable = 0;
    recovery_current_val = 0;

    Uint8 C_ic = 0;
    slave_read.all = 0;
    timeout_cunt = 0;
#ifdef hall_CS
    DELAY_US(5000);
    current_ofst = adc_read_asd(ADC_ADDR);
#endif

    /*
     * Initialising volatile structs
     */
    PDU_setData_local.fixSetS.all = 0;
    PDU_setData_local.fixSetS_EVCC.all = 0;
    PDU_setData_local.contactor_on = 0;
    PDU_setData_local.contactor_on_inverse = 0;

    init_nominal_capacity();
    soc_init(&(Controller.SOC_value));
    init_charge_energy();
    init_current_limits();
    init_pack_number();
    init_fw_update_date();

    for (;;)
    {
        if (pending_eeprom_save_param != 0)
        {
            save_parameter_to_eeprom(); // Safely executes the 10ms delay here
        }
        testVar = temp_board[testStat++] + (temp_board[testStat++] << 8);
        if (hv_reset_set)
        {
            main_msp_config_loop();
            hv_reset_set = 0;
            first_timer_config_flag = 1;
            config_packet_counter_0++;
        }

        if (BMS_data_sep_flag)
        {
            BMS_data_sep_flag = 0;
        }

        if (t50_ms_timer > 5)
        {
            contactor_init_flag = 1;
        }
        else
        {
            PDU_getData_local.fixSetG.all = 0;
        }

        if (first_timer_config_flag)
        {

            //EVCU can send task
            if ((timer_flag_can > 0) && (!OTA_flag))
            {
                current_com_counts++;
                timer_flag_can = 0;
                timer_task_can();
            }

            // data read
            hv_batterry_read(&C_ic);
            //EVCU CAN data ready
            BMS_can_data();

            //read time from RTC

            if (I2caRegs.I2CMDR.bit.STP != 1)
            {
                if (I2caRegs.I2CSTR.bit.BB != 1)
                {
                    if (timer_ready_rtc)
                    {
                        timer_ready_rtc = 0;
                    }
#ifdef hall_CS
                    if (timer_ready_adc)
                    {
                        timer_ready_adc = 0;
                        current_val_A = adc_read_asd(ADC_ADDR) - current_ofst;
                    }
#endif
                }
            }
#ifdef shunt_CS
            current_val_A = INA229_data[1];
            c_count.coulomb_count = INA229_data[4];
//            memcpy((void *)& c_count., (const void *)& INA229_data[4], sizeof(float));
#endif

#ifdef recovery_CS
            current_val_A = recovery_current_val;
#endif

#ifdef sealed_version
            current_val_A *= -1;
#endif

            Current_value = (int16) (current_val_A * 100);

            /*Calculate SOC and SOH*/

            // SOC force Update
            if (soc_soh_flag)
            {
                if (can_force_soc_flag)
                {
                    can_force_soc_flag = 0;
                    force_instant_soc_update(&(Controller.SOC_value));
                }

                calc_soc_soh(((-1) * current_val_A), Controller.highest_cell_volt,
                             Controller.lowest_cell_volt, &(Controller.SOC_value));

                soc_calc_new(((-1) * current_val_A), Controller.highest_cell_volt,
                                             Controller.lowest_cell_volt, &(Controller.SOC_value));

                soc_soh_flag = 0;

                if (PDU_setData_local.fixSetS.bit.EVCU_State == 4) /*Charging*/
                {
                    charge_ctrl(Controller.highest_cell_volt, Current_value, Controller.SOC_value);
                    PDU_getData_local.charged_energy += calculate_charged_energy(Controller.total_pack_voltage);
                    cell_balancing_enable = 1;
                    charge_state = PDU_getData_local.fixSetChrg.bit.charge_complete_flag;
                }
                else
                {
                    charge_state = 2;
                    PDU_getData_local.fixSetChrg.all = 0;
                    charge_complete = 0;
                    reset_charge_session();
                }

                temperature = Mcu_Temp();

                chillerCtrl(Controller.highest_temp);

            }
        }

//---------------------------------------------------------------------------------------------------
#ifdef FIX_SOC
        if(Controller.SOC_value > SOC_LV_CORRECTION && lowest_cell_v_moving_avg < 30000)
        {
            soc_reset = SOC_LV_CORRECTION;
        }
#endif
//---------------------------------------------------------------------------------------------------

//        if (BMS_Update_flag)
//        {
////            count = 0;
//           if (OTA_handshake_connect_reset() == OTA_HANDSHAKE)
//           {
//               do
//               {
//                   send_OTA_ACK(INIT_ACK);
//               }while (OTA_handshake_connect_reset() != OTA_RESET);
//
////               while (OTA_handshake_connect_reset() != OTA_RESET)
////               {
////                   send_OTA_ACK(INIT_ACK);
////               }
//#ifdef CAN_BOOTLOADING
//               send_OTA_ACK(INIT_ACK);
////               ECanaRegs.CANRMP.all = 0xFFFFFFFF;
//               CLEAR_INTERUPT_FLAGS();
//               move_to_kernal(BOOTLOADER_STATE);
//#endif
//
//           }
//           else
//           {
//
//           }
//        }
        else
        {
        }
    }
}
//end of the main
void SYS_CONFIG()
{
    InitFlash();
    InitSysCtrl();
    DINT;
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();
    InitCpuTimers();
    InitAdc();
//    InitECanGpio();
//    InitECana();
//    CAN_Init();
    InitSciaGpio();
    InitScibGpio();

    InitSpiaGpio();

    InitEPwm2Gpio();
//    InitSpibGpio();
    InitI2CGpio();
#if(CANBUS == ACTIVE)
    CAN_Init();
//    ECAN_CONFIG();
    EALLOW;
    PieVectTable.ECAN1INTA = &ecan1_inta_isr;
    EDIS;

//  PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    PieCtrlRegs.PIEIER9.bit.INTx6 = 1;

//  IER |= M_INT1;
    IER |= M_INT9;
    ECAN_CONFIG();
    CAN_enableGlobalInterrupt();
#endif
    I2CA_Init();
    C2000_config();
    C2000_SCI_int();

    C2000_SPI_MSP();

    EINT;
// Enable Global Interrupts
    ERTM;

    ConfigCpuTimer(&CpuTimer0, 90, 50000); // 90MHz CPU Freq,  Period          50ms
    ConfigCpuTimer(&CpuTimer1, 90, 100000);     //100ms
    CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer1Regs.TCR.all = 0x4000; // timer

//    ECanaShadow.CANTRS.all = 0;
}

void C2000_config(void)
{
    EALLOW;
// This is needed to write to EALLOW protected registers
    //PieVectTable.SCIRXINTA = &rxinta;
    //PieVectTable.SCITXINTA = &txinta;
//    PieVectTable.SCIRXINTB = &rxintb;
//    PieVectTable.SCITXINTB = &txintb;
    PieVectTable.SCIRXINTA = &sciaRxFifoIsr;

    PieVectTable.TINT1 = &cpu_timer1_isr;
    PieVectTable.TINT0 = &cpu_timer0_isr;

//    PieVectTable.XINT1 = &xint1_isr;
    EDIS;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;     // PIE Group 9, INT1
    PieCtrlRegs.PIEIER9.bit.INTx2 = 0;     // PIE Group 9, INT2 //txinta

    PieCtrlRegs.PIEIER9.bit.INTx3 = 1;     // PIE Group 9, INT1
    PieCtrlRegs.PIEIER9.bit.INTx4 = 0;     // PIE Group 9, INT2 //txintb

    PieCtrlRegs.PIEIER1.bit.INTx4 = 1;  // PIE Group 1, INT4 (XINT-1)

    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
    IER |= M_INT1;
    IER |= M_INT13;
    IER |= 0x100; // Enable CPU INT
    EINT;
}

void ECAN_CONFIG()
{
//    ECanaShadow.CANMIL.all = 0;
    CAN_setupMessageObject(CAN_OBJ_3, OTA_FLAG_ADDR, NOMINAL_DLC,
                           RX_DIR, EXT_FRAME);
    CAN_setupMessageObject(CAN_OBJ_4, OTA_RESPONSE_ADDR, NOMINAL_DLC,
                           TX_DIR, EXT_FRAME);
    CAN_setupMessageObject(CAN_OBJ_9, EEPROM_BROADCAST_ADDR, NOMINAL_DLC,
                           TX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_11, EVCC_ADDR, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_12, BMS_CMD_ADDR, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_14, BMS_DEBUG_ADDR, NOMINAL_DLC,
                           TX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_15, BMS_CAN_DATA_ADDR, NOMINAL_DLC,
                           TX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_16, EVCU_ADDR, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_17, GET_SLAVE_ADDR, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_18, SET_SLAVE_ADDR, NOMINAL_DLC,
                           TX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_19, CHILLER_RX_ADDR, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_20, CHILLER_TX_ADDR, NOMINAL_DLC,
                           TX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_21, EVCU_ODO_ADDR, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_26, SET_CHG_ENERGY, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);
    CAN_setupMessageObject(CAN_OBJ_27, BMS_CONFIG_UPDATE_ADDR, NOMINAL_DLC,
                           RX_DIR, STD_FRAME);

}

/*
 *
 */
void I2CA_Init(void)
{
    I2caRegs.I2CPSC.all = 8;        // Prescaler - need 7-12 Mhz on module clk
    I2caRegs.I2CCLKL = 20;          // NOTE: must be non zero
    I2caRegs.I2CCLKH = 10;          // NOTE: must be non zero
    I2caRegs.I2CIER.all = 0x0;      //0x24-Enable SCD & ARDY interrupts

    //
    // Take I2C out of reset. Stop I2C when suspended
    //
    I2caRegs.I2CMDR.all = 0x0020;

//    I2caRegs.I2CFFTX.all = 0x6000;  // Enable FIFO mode and TXFIFO
//    I2caRegs.I2CFFRX.all = 0x2040;  // Enable RXFIFO, clear RXFFINT,

}
void C2000_SCI_int(void)
{
    SciaRegs.SCICCR.all = 0x0007;

    SciaRegs.SCICTL1.all = 0;
    SciaRegs.SCICTL1.bit.RXENA = 1;
    //SciaRegs.SCICTL1.bit.TXENA = 1;

    SciaRegs.SCICTL2.all = 0;
    //SciaRegs.SCICTL2.bit.TXINTENA = 1;
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;

    SciaRegs.SCIHBAUD = 0x0000; // 115200 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
    SciaRegs.SCILBAUD = 0x0017;

    //RX fifo
    SciaRegs.SCIFFRX.bit.RXFFIL = 0x04;
    SciaRegs.SCIFFRX.bit.RXFFIENA = 1;
    SciaRegs.SCIFFTX.all = 0xC000;
    SciaRegs.SCIFFCT.all = 0;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;

//    SciaRegs.SCIHBAUD = 0x0001;  // 9600 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
//    SciaRegs.SCILBAUD = 0x0024;

//    SciaRegs.SCIHBAUD = 0x0000; // 128000 baud @LSPCLK = 22.5MHz.(90 MHz SYSCLK)
//    SciaRegs.SCILBAUD    =0x0016;

//  SciaRegs.SCIHBAUD    =0x0000;
//  SciaRegs.SCILBAUD    =0x0030;       //0x0030;//57600

//  SciaRegs.SCIHBAUD    =0x0000;
//  SciaRegs.SCILBAUD    =0x0092;//19200        working

    SciaRegs.SCICTL1.bit.SWRESET = 1;  // Relinquish SCI from Reset

    // port B
    ScibRegs.SCICCR.all = 0;
    ScibRegs.SCICCR.bit.SCICHAR = 7;

    ScibRegs.SCICTL1.all = 0;
    ScibRegs.SCICTL1.bit.RXENA = 0;
    ScibRegs.SCICTL1.bit.TXENA = 1;

    ScibRegs.SCICTL2.all = 0;
    ScibRegs.SCICTL2.bit.TXINTENA = 0;
    ScibRegs.SCICTL2.bit.RXBKINTENA = 0;

    ScibRegs.SCIHBAUD = 0x12;  // 9600 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
    ScibRegs.SCILBAUD = 0x50;

    //SciaRegs.SCIHBAUD    =0x0000;
    //SciaRegs.SCILBAUD    =0x0030;     ////57600

    //    ScibRegs.SCIHBAUD = 0x0000;
    //    ScibRegs.SCILBAUD = 0x0017;         //baud rate : 115200
    ScibRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset

}

__interrupt void cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++; // Acknowledge this interrupt to receive more interrupts from group 1

    if (contactor_init_flag)
    {
        if (PDU_setData_local.fixSetS.bit.Fire_Mode)
        {
            PDU_setData_local.contactor_on = 1;
            PDU_setData_local.contactor_on_inverse = 2;
            contactor_operator_fire();


        }
        else
        {
#ifndef EVCU_ACTIVE
            PDU_setData_local.contactor_on = 1;
            PDU_setData_local.contactor_on_inverse = 2;
#endif

            contactor_operator();
            if (PDU_setData_local.fixSetS.bit.EVCU_State == 4) /*Charging*/
                                    {
                                        fc_contactor_operator();
                                    }


#ifdef MASTER
            slave_status = slave_controller();
            if (slave_status && PDU_setData_local.fixSetS.bit.charger_connected && charge_complete
                    && (PDU_getData_slave.SOC_val < 100))
            {
                PDU_setData_local.contactor_on = 0;
                PDU_setData_local.contactor_on_inverse = 0;
                PDU_setDataSlave_local.contactor_on = 1;
                PDU_setDataSlave_local.contactor_on_inverse = 2;
            }
#endif
        }
    }
    if(t50_ms_timer > 50)
    {

        soc_soh_flag = 1; /*Set flag to calculate soc_soh*/
    }
    timer_ready_adc = 1;
    t50_ms_timer++;
    shuntCS_watchdog++;
    if (t50_ms_timer > 65500)
    {
        t50_ms_timer = 0;
    }
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
__interrupt void cpu_timer1_isr(void)
{
    time_out_request();
    seconds_counter++;
    if (seconds_counter > 65500)
    {
        seconds_counter = 11;
    }
//    printf("%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u",
//               BMS_data_IC[test2].cell_voltages[0],
//               BMS_data_IC[test2].cell_voltages[1],
//               BMS_data_IC[test2].cell_voltages[2],
//               BMS_data_IC[test2].cell_voltages[3],
//               BMS_data_IC[test2].cell_voltages[4],
//               BMS_data_IC[test2].cell_voltages[5],
//               BMS_data_IC[test2].cell_voltages[6],
//               BMS_data_IC[test2].cell_voltages[7],
//               BMS_data_IC[test2].cell_voltages[8],
//               BMS_data_IC[test2].cell_voltages[9],
//               BMS_data_IC[test2].cell_voltages[10]);
//        test2++;
//        if (test2 == 2)
//        {
//            test2 = 0;
//            printf("\n");
//        }
//        else
//        {
//            printf(", ");
//        }
    if (seconds_counter > 10)
    {
        timer_flag_can = 1;
    }
    CpuTimer1.InterruptCount++; // Acknowledge this interrupt to receive more interrupts from group 1
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//////////////////////////////can
//volatile Uint8 chiller_test = 0;
#if(CANBUS == ACTIVE)
__interrupt void ecan1_inta_isr()
{
    Uint8 status;
//    Uint16 readBuffer[NOMINAL_DLC] = {0};
    status = CAN_getInterruptCause();
    switch(status)
    {
        case CAN_OBJ_3:
//            chller_operation();
//            CAN_readMessage(CAN_OBJ_3,readBuffer)
            update_BMS();
//            BMS_Update_flag = 1;
            break;
        case CAN_OBJ_11:
            CAN_readMessage(CAN_OBJ_11, canMap.msg[CAN_OBJ_11].readData);
            PDU_setData_read_EVCC(canMap.msg[CAN_OBJ_11].readData);
            break;
        case CAN_OBJ_12:
//            CAN_readMessage(CAN_OBJ_12, canMap.msg[CAN_OBJ_12].readData);
            setDebugCMD();
            break;
        case CAN_OBJ_16:
            CAN_readMessage(CAN_OBJ_16, canMap.msg[CAN_OBJ_16].readData);
            PDU_setData_read(canMap.msg[CAN_OBJ_16].readData);
            ecana_int_set_counter++;
            break;
        case CAN_OBJ_17:
            CAN_readMessage(CAN_OBJ_17, canMap.msg[CAN_OBJ_17].readData);
            PDU_getDataSlave_read(canMap.msg[CAN_OBJ_17].readData);
            ecana_int_slaveData_counter++;
            slave_watchdog = 0;
            break;
        case CAN_OBJ_19:
            CAN_readMessage(CAN_OBJ_19, canMap.msg[CAN_OBJ_19].readData);
            chillerData_read(canMap.msg[CAN_OBJ_19].readData);
            chiller_rec_count++;
            break;
        case CAN_OBJ_21:
            CAN_readMessage(CAN_OBJ_21, canMap.msg[CAN_OBJ_21].readData);
            PDU_setData_read_ODO(canMap.msg[CAN_OBJ_21].readData);
            chiller_rec_count++;
            break;
        case CAN_OBJ_26:
            CAN_readMessage(CAN_OBJ_26, canMap.msg[CAN_OBJ_26].readData);
            reset_chg_energy(canMap.msg[CAN_OBJ_26].readData);
            break;
        case CAN_OBJ_27:
            CAN_readMessage(CAN_OBJ_27, canMap.msg[CAN_OBJ_27].readData);
            parse_config_update_cmd(canMap.msg[CAN_OBJ_27].readData);
            break;
        default:
            break;
    }

    CLEAR_INTERUPT_FLAGS();
    ecana_interrupt_counter++;
//    PieCtrlRegs.PIEACK.bit.ACK9 = 1;
}

void setDebugCMD()
{
    CAN_readMessage(CAN_OBJ_12, canMap.msg[CAN_OBJ_12].readData);

    debug_CMD = canMap.msg[CAN_OBJ_12].readData[0];

    // Check if an SOC reset was requested via CAN
    Uint8 incoming_soc_reset = canMap.msg[CAN_OBJ_12].readData[1];
    if (incoming_soc_reset > 0)
    {
        can_target_soc_val = incoming_soc_reset;
        can_force_soc_flag = 1; // Trigger the main loop to execute the update
    }

    chiller_test = canMap.msg[CAN_OBJ_12].readData[2];
    cell_balancing_enable = canMap.msg[CAN_OBJ_12].readData[3];
    slave_read.all = canMap.msg[CAN_OBJ_12].readData[4];
    recovery_current_val = canMap.msg[CAN_OBJ_12].readData[5];
}

//void canData()
//{
//
//    ECanaMboxes.MBOX5.MDL.all = 0x00000000;
//    ECanaMboxes.MBOX5.MDH.all = 0x00000000;
//
//////////////////////////////////
//
//    ECanaMboxes.MBOX5.MDL.byte.BYTE0 = 0xA0;
//    ECanaMboxes.MBOX5.MDL.byte.BYTE1 = 0xA1;
//    ECanaMboxes.MBOX5.MDL.byte.BYTE2 = 0xA2;
//    ECanaMboxes.MBOX5.MDL.byte.BYTE3 = 0xA3;
//////////////////////////////////////////////byte4,5
////temp = vehicle.maximum_baterry_voltage;
//    ECanaMboxes.MBOX5.MDH.byte.BYTE4 = 0xA4;
//    ECanaMboxes.MBOX5.MDH.byte.BYTE5 = 0xA5;
//    ECanaMboxes.MBOX5.MDH.byte.BYTE6 = 0xA6;
//    ECanaMboxes.MBOX5.MDH.byte.BYTE7 = 0xA7;
//
//    EALLOW;
//    ECanaRegs.CANTRR.all = 0x00000000; //added 09/05/2016
//    EDIS;
//    EALLOW;
//    ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
//    ECanaShadow.CANTRS.all = 0x00000000;
//    ECanaShadow.CANTRS.bit.TRS5 = 1;
//    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
//    EDIS;
//}

#endif
/*
void can_bus_tx_flush(void)
{
    EALLOW;
    //2,3,5,6,16,18,29,30
    ECanaRegs.CANME.bit.ME14 = 0;
    ECanaRegs.CANME.bit.ME15 = 0;
    ECanaRegs.CANME.bit.ME16 = 0;
    ECanaRegs.CANME.bit.ME17 = 0;
    EDIS;
    DELAY_US(10);
    EALLOW;
    //2,3,5,6,16,18,29,30
    ECanaRegs.CANME.bit.ME14 = 1;
    ECanaRegs.CANME.bit.ME15 = 1;
    ECanaRegs.CANME.bit.ME16 = 1;
    ECanaRegs.CANME.bit.ME17 = 1;

    EDIS;
}
*/
void C2000_SPI_MSP(void)
{
    /* Initialize SPI-A Settings MSP */

    SpiaRegs.SPICTL.all = 0x007;
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1; //master
    SpiaRegs.SPICTL.bit.CLK_PHASE = 0;

    SpiaRegs.SPICCR.all = 0x00C7;
    SpiaRegs.SPICCR.bit.SPILBK = 0;
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 1;

    SpiaRegs.SPIBRR = 0x006; // 4MHz

    SpiaRegs.SPIPRI.bit.FREE = 1;   // Set so breakpoints don't disturb mission
    SpiaRegs.SPICCR.bit.SPISWRESET = 1;
}

/*temperature reading of the board*/
void board_temperature()
{
    //Init
    int temp_power = 0;
    int temp_main = 0;
    int temp_msp1 = 0;
    int temp_msp2 = 0;
    //read
    temp_power = (AdcResult.ADCRESULT7 >> 4) - 7;
    temp_main = (AdcResult.ADCRESULT11 >> 4) - 7;
    temp_msp1 = (AdcResult.ADCRESULT15 >> 4) - 7;
    temp_msp2 = (AdcResult.ADCRESULT10 >> 4) - 7;
    //lookup table
    if (temp_power < 0)
    {
        Controller.temperature_power = 127;
    }
    else if (temp_power > 229)
    {
        Controller.temperature_power = -25;
    }
    else
    {
        Controller.temperature_power = temp_board[temp_power];
    }
    if (temp_main < 0)
    {
        Controller.temperature_main = 127;
    }
    else if (temp_power > 229)
    {
        Controller.temperature_main = -25;
    }
    else
    {
        Controller.temperature_main = temp_board[temp_power];
    }
    if (temp_msp1 < 0)
    {
        Controller.temperature_msp1 = 127;
    }
    else if (temp_power > 229)
    {
        Controller.temperature_msp1 = -25;
    }
    else
    {
        Controller.temperature_msp1 = temp_board[temp_power];
    }
    if (temp_msp2 < 0)
    {
        Controller.temperature_msp2 = 127;
    }
    else if (temp_power > 229)
    {
        Controller.temperature_msp2 = -25;
    }
    else
    {
        Controller.temperature_msp2 = temp_board[temp_power];
    }
}
/*configuration MSP430s*/
void MspConfig(Uint8 pack_num, Uint8 controll_byte, Uint8 op_mode_con)
{
    Uint8 total_data_length = 0;
    Uint16 CRC_val = 0xFFFF;
    //send_data_buffer[];
    send_data_buffer[pack_num][0] = 0xAA;
    send_data_buffer[pack_num][1] = 0x0B;
    send_data_buffer[pack_num][4] = controll_byte;
    send_data_buffer[pack_num][5] = op_mode_con;
    send_data_buffer[pack_num][8] = dcov_threshold >> 8; //Each code is send as two bytes and is combined to
    send_data_buffer[pack_num][5] = dcov_threshold; // create the parsed code
    send_data_buffer[pack_num][6] = dcuv_threshold >> 8; //Each code is send as two bytes and is combined to
    send_data_buffer[pack_num][7] = dcuv_threshold; // create the parsed code
    send_data_buffer[pack_num][8] = dmin_balance_volts; //Each code is send as two bytes and is combined to
    send_data_buffer[pack_num][9] = dmin_balance_volts >> 8; // create the parsed code
    send_data_buffer[pack_num][10] = dbalancing_volt_threshold;
    send_data_buffer[pack_num][11] = config_byte; //balancing timer
    send_data_buffer[pack_num][12] = send_packet_code;
    send_data_buffer[pack_num][13] = received_packet_code;
    send_data_buffer[pack_num][14] = 0x23;
    CRC_val = crc_1021(15, &send_data_buffer[pack_num][0]);
    send_data_buffer[pack_num][15] = CRC_val >> 8;
    send_data_buffer[pack_num][16] = CRC_val;
    total_data_length = byte_stuffing(17, &send_data_buffer[pack_num][0]);
    send_data_buffer[pack_num][total_data_length + 1] = 0xAA;
    send_data_buffer[pack_num][total_data_length + 2] = 0x0E;
    spia_send_packet(send_data_buffer[pack_num], total_data_length + 3);
}

/*
 * Function name : Resend_packet
 * Description   : If a received packet was corrupted
 *                 then this packet was sent to the upper level
 */
void Resend_packet(Uint8 pack_num, Uint8 op_mode_con)
{
    Uint16 CRC_val = 0;
    Uint8 total_data_length = 0;
//flag
    send_data_buffer[pack_num][0] = 0xAA;
    send_data_buffer[pack_num][1] = 0x0B;
//header
    send_data_buffer[pack_num][4] = RESEND;
    send_data_buffer[pack_num][5] = op_mode_con;
//ack
    send_data_buffer[pack_num][8] = send_packet_code;
    send_data_buffer[pack_num][9] = received_packet_code;
//crc
    CRC_val = crc_1021(10, &send_data_buffer[pack_num][0]);
    send_data_buffer[pack_num][10] = CRC_val >> 8;
    send_data_buffer[pack_num][11] = CRC_val;
//stuffing
    total_data_length = byte_stuffing(12, &send_data_buffer[pack_num][0]);
//flag
    send_data_buffer[pack_num][total_data_length + 1] = 0xAA;
    send_data_buffer[pack_num][total_data_length + 2] = 0x0E;
//send data
    spia_send_packet(send_data_buffer[pack_num], total_data_length + 3);
}

/*
 * Function name : Request_classified
 * Description   : If a received packet was corrupted
 *                 then this packet was sent to the upper level
 */
void Request_classified(Uint8 pack_num, Uint8 op_mode_con)
{
    Uint16 CRC_val = 0;
    Uint8 total_data_length = 0;
//flag
    send_data_buffer[pack_num][0] = 0xAA;
    send_data_buffer[pack_num][1] = 0x0B;
//header
    send_data_buffer[pack_num][3] = CLASSIFIED;
    send_data_buffer[pack_num][4] = REQUEST;
    send_data_buffer[pack_num][5] = op_mode_con;

    send_data_buffer[pack_num][7] = cell_balancing_enable;
//Time data
//    send_data_buffer[pack_num][8] = year;
//    send_data_buffer[pack_num][9] = month;
//    send_data_buffer[pack_num][10] = date;
//    send_data_buffer[pack_num][11] = hour;
//    send_data_buffer[pack_num][12] = Minute;
//ack
    send_data_buffer[pack_num][15] = send_packet_code;
    send_data_buffer[pack_num][16] = received_packet_code;
//crc
    CRC_val = crc_1021(17, &send_data_buffer[pack_num][0]);
    send_data_buffer[pack_num][17] = CRC_val >> 8;
    send_data_buffer[pack_num][18] = CRC_val; // 0x0223
//stuffing
    total_data_length = byte_stuffing(19, &send_data_buffer[pack_num][0]);
//flag
    send_data_buffer[pack_num][total_data_length + 1] = 0xAA;
    send_data_buffer[pack_num][total_data_length + 2] = 0x0E;
//send data
    spia_send_packet(send_data_buffer[pack_num], total_data_length + 3);
}

/*
 * Function name : Request_humidity
 * Description   : If a received packet was corrupted
 *                 then this packet was sent to the upper level
 */
void Request_humidity(Uint8 pack_num, Uint8 op_mode_con)
{
    Uint16 CRC_val = 0;
    Uint8 total_data_length = 0;
//flag
    send_data_buffer[pack_num][0] = 0xAA;
    send_data_buffer[pack_num][1] = 0x0B;
//header
    send_data_buffer[pack_num][3] = HUMIDITY_TEMPERATURE;
    send_data_buffer[pack_num][4] = REQUEST;
    send_data_buffer[pack_num][5] = op_mode_con;

    send_data_buffer[pack_num][7] = cell_balancing_enable;
//Time data
//    send_data_buffer[pack_num][8] = year;
//    send_data_buffer[pack_num][9] = month;
//    send_data_buffer[pack_num][10] = date;
//    send_data_buffer[pack_num][11] = hour;
//    send_data_buffer[pack_num][12] = Minute;
//ack
    send_data_buffer[pack_num][15] = send_packet_code;
    send_data_buffer[pack_num][16] = received_packet_code;
//crc
    CRC_val = crc_1021(17, &send_data_buffer[pack_num][0]);
    send_data_buffer[pack_num][17] = CRC_val >> 8;
    send_data_buffer[pack_num][18] = CRC_val; // 0x0223
//stuffing
    total_data_length = byte_stuffing(19, &send_data_buffer[pack_num][0]);
//flag
    send_data_buffer[pack_num][total_data_length + 1] = 0xAA;
    send_data_buffer[pack_num][total_data_length + 2] = 0x0E;
//send data
    spia_send_packet(send_data_buffer[pack_num], total_data_length + 3);
}

/*
 * Function name : Request_openWire
 * Description   : If a received packet was corrupted
 *                 then this packet was sent to the upper level
 */
void Request_openWire(Uint8 pack_num, Uint8 op_mode_con)
{
    Uint16 CRC_val = 0;
    Uint8 total_data_length = 0;
//flag
    send_data_buffer[pack_num][0] = 0xAA;
    send_data_buffer[pack_num][1] = 0x0B;
//header
    send_data_buffer[pack_num][3] = OPEN_WIRE;
    send_data_buffer[pack_num][4] = REQUEST;
    send_data_buffer[pack_num][5] = op_mode_con;

    send_data_buffer[pack_num][7] = cell_balancing_enable;
//Time data
//    send_data_buffer[pack_num][8] = year;
//    send_data_buffer[pack_num][9] = month;
//    send_data_buffer[pack_num][10] = date;
//    send_data_buffer[pack_num][11] = hour;
//    send_data_buffer[pack_num][12] = Minute;
//ack
    send_data_buffer[pack_num][15] = send_packet_code;
    send_data_buffer[pack_num][16] = received_packet_code;
//crc
    CRC_val = crc_1021(17, &send_data_buffer[pack_num][0]);
    send_data_buffer[pack_num][17] = CRC_val >> 8;
    send_data_buffer[pack_num][18] = CRC_val; // 0x0223
//stuffing
    total_data_length = byte_stuffing(19, &send_data_buffer[pack_num][0]);
//flag
    send_data_buffer[pack_num][total_data_length + 1] = 0xAA;
    send_data_buffer[pack_num][total_data_length + 2] = 0x0E;
//send data
    spia_send_packet(send_data_buffer[pack_num], total_data_length + 3);
}

/*
 * Function name : Request_all_data
 * Description   : If a received packet was corrupted
 *                 then this packet was sent to the upper level
 */
void Request_all_data(Uint8 IC_no, Uint8 pack_num, Uint8 op_mode_con)
{
    Uint16 CRC_val = 0;
    Uint8 total_data_length = 0;
    //flag
    send_data_buffer[pack_num][0] = 0xAA;
    send_data_buffer[pack_num][1] = 0x0B;
    //header
    send_data_buffer[pack_num][2] = IC_no;
    send_data_buffer[pack_num][3] = ALL_DATA;
    send_data_buffer[pack_num][4] = REQUEST;
    send_data_buffer[pack_num][5] = op_mode_con;

    send_data_buffer[pack_num][7] = cell_balancing_enable;
    //Time data
    //    send_data_buffer[pack_num][8] = year;
    //    send_data_buffer[pack_num][9] = month;
    //    send_data_buffer[pack_num][10] = date;
    //    send_data_buffer[pack_num][11] = hour;
    //    send_data_buffer[pack_num][12] = Minute;
    //ack
    send_data_buffer[pack_num][15] = send_packet_code;
    send_data_buffer[pack_num][16] = received_packet_code;
    //crc
    CRC_val = crc_1021(17, &send_data_buffer[pack_num][0]);
    send_data_buffer[pack_num][17] = CRC_val >> 8;
    send_data_buffer[pack_num][18] = CRC_val;
    //stuffing
    total_data_length = byte_stuffing(19, &send_data_buffer[pack_num][0]);
    //flag
    send_data_buffer[pack_num][total_data_length + 1] = 0xAA;
    send_data_buffer[pack_num][total_data_length + 2] = 0x0E;
    //send data
    spia_send_packet(send_data_buffer[pack_num], total_data_length + 3);
}

/*Initialize SOC(Read form eeprom and calculate)*/
//SOC_value = soc_estimate();
/*wait for the BMS data read*/

/*read BMS data*/
//BMS_data_read_write();
/*Analyze data*/
//analyse_data();
/*prepare data*/
//BMS_data_make();
/*CAN send*/
//BMS_data_write();
/*Enable timers*/
//EOF
unsigned short crc_reslt_lock;
Uint16 crc_1021(unsigned char data_length, unsigned char *data_crc)
{
    Uint8 count;
    crc_reslt_lock = 0xffff;
    for (count = 0; count < data_length; count++)
    {
        update_bad_crc(data_crc[count]);
    }
    return crc_reslt_lock;
}

void update_bad_crc(unsigned short ch)
{
    unsigned short i, xor_flag;
    ch <<= 8;

    for (i = 0; i < 8; i++)
    {
        if ((crc_reslt_lock ^ ch) & 0x8000)
        {
            xor_flag = 1;
        }
        else
        {
            xor_flag = 0;
        }
        crc_reslt_lock = crc_reslt_lock << 1;
        if (xor_flag)
        {
            crc_reslt_lock = crc_reslt_lock ^ 0x1021;
        }
        ch = ch << 1;
    }
}
unsigned char spia_send_packet(unsigned char data[], Uint8 data_length)
{
    Uint8 l_count;
    for (l_count = 0; l_count < data_length; l_count++)
    {
        while (SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1)
        {
        }
        SpiaRegs.SPITXBUF = data[l_count] << 8;
        /*
         * don't use this int flag with fifo mode
         */
        while (SpiaRegs.SPISTS.bit.INT_FLAG != 1)
        {
        }
        DELAY_US(20);
    }
    return (SpiaRegs.SPIRXBUF & 0xFF);
}

unsigned char spia_send_byte(unsigned char data)
{
    while (SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1)
    {
    }
    SpiaRegs.SPITXBUF = data << 8;
    /*
     * don't use this int flag with fifo mode
     */
    while (SpiaRegs.SPISTS.bit.INT_FLAG != 1)
    {
    }
    return (SpiaRegs.SPIRXBUF & 0xFF);
}

unsigned char spia_receive()
{
    while (SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1)
    {
    }
    SpiaRegs.SPITXBUF = 0x0000;
    /*
     * don't use this int flag with fifo mode
     */
    while (SpiaRegs.SPISTS.bit.INT_FLAG != 1)
    {
    }
    return (SpiaRegs.SPIRXBUF & 0xFF);

}
void spia_disable(void)
{
    //    USICTL0 |= USISWRST;        // put USI in reset mode

    SpiaRegs.SPICCR.bit.SPISWRESET = 0;
}

/*
 * Function name : byte_stuffing
 * Description   : If there is a byte that contained '0xAA' then replace it with '#'.
 *                 if there is '#' then it is converted into '##'.
 *                 *The packet length should be without last 2 bytes(start with one)
 *                 * "data_packet" is the pointer of data array 1st element
 */
unsigned char byte_stuffing(unsigned char length, unsigned char *data_packet)
{
    //the replacing byte = 0x23

    unsigned char temp_length_local = 0;
    unsigned char length_local;
//    unsigned char *temp_storage;
    unsigned char temp_storage[40]; //size of the array needed to be changed
//    temp_storage = (Uint8*) malloc((length + 20) * sizeof(Uint8));
    unsigned char temp_count_local = 0;
    for (length_local = 2; length_local < length; length_local++)
    {
        if ((data_packet[length_local] & 0x00FF) == 0xAA)
        {
            temp_storage[temp_length_local] = 0x23;
            temp_length_local++;
            temp_storage[temp_length_local] = 0xAA;
            temp_length_local++;
        }
        else if ((data_packet[length_local] & 0x00FF) == 0x23)
        {
            temp_storage[temp_length_local] = 0x23;
            temp_length_local++;
            temp_storage[temp_length_local] = 0x23;
            temp_length_local++;
        }
        else
        {
            temp_storage[temp_length_local] = data_packet[length_local];
            temp_length_local++;
        }
    }
    for (temp_count_local = 0; temp_count_local < temp_length_local; temp_count_local++)
    {
        data_packet[temp_count_local + 2] = temp_storage[temp_count_local];
    }
//    free(temp_storage);
    return temp_length_local + 1;
}

/*
 * Function Name    - SPI_packet_receive
 * Description      - Detect start and end flag
 *                      save data to received_data_buffer
 */

unsigned char SPI_packet_receive()
{
    unsigned char SPI_received_byte;
    unsigned char received_flag_l = 1;
    unsigned char l_data_count = 0;
    unsigned char l_data_byte_count = 0;
    SPI_rec_packed_length = 0;

    memset(received_data_buffer, 0, 150);

    while (received_flag_l & (l_data_byte_count < 150))
    {
        SPI_received_byte = spia_send_byte(DUMMY);
        l_data_byte_count++;
        DELAY_US(30);
        switch (SPI_receiver_state)
        {
            case 0:
                //starting state
                if (SPI_received_byte == 0xAA)
                {
                    SPI_receiver_state = 1;
                    received_data_buffer[l_data_count++] = 0xAA;
                }
                else
                {
                    SPI_rec_error = 1;
                    SPI_receiver_state = 5;
                }
                break;
            case 1:
                //starting state
                if (SPI_received_byte == 0x0B)
                {
                    SPI_receiver_state = 2;
                    received_data_buffer[l_data_count++] = 0x0B;
                }
                else
                {
                    SPI_rec_error = 1;
                    SPI_receiver_state = 5;
                }
                break;
            case 2:
                //data save state
                if (SPI_received_byte == 0x23)
                {
                    SPI_receiver_state = 4;
                }
                else if (SPI_received_byte == 0xAA)
                {
                    SPI_receiver_state = 3;
                }
                else
                {
                    //save data in receiver buffer
                    received_data_buffer[l_data_count++] = SPI_received_byte;
                }
                break;
            case 3:
                //end state
                if (SPI_received_byte == 0x0E)
                {
                    SPI_receiver_state = 0;
                    SPI_rec_packed_length = l_data_count;
                    received_flag_l = 0;
                    SPI_Packet_rvd_f = 1;
                }
                else
                {
                    SPI_rec_error = 1;
                    SPI_receiver_state = 5;
                }
                break;
            case 4:
                switch (SPI_received_byte)
                {
                    case 0xAA:
                        SPI_receiver_state = 2;
                        received_data_buffer[l_data_count++] = SPI_received_byte;
                        break;
                    case 0x23:
                        SPI_receiver_state = 2;
                        received_data_buffer[l_data_count++] = SPI_received_byte;
                        break;
                    default:
                        SPI_receiver_state = 5;
                        SPI_rec_error = 1;
                        break;
                }
                break;
            case 5:
                received_flag_l = 0;
                //error state
                break;
            default:
                break;
        }
    }
    return l_data_count;
}

/*
 * Function Name - seperateBMSdata
 * Description   - If BMS data packet was received
 *                  the data separate into relevant registers
 */

typedef union
{
    float Current_f;
    Uint16 byte[2];
} cc_float;
cc_float current_pkt;
double total_pack_voltage_temp = 0;

Uint16 erroe_local_ltc_c = 0, erroe_local_ltc_c_old = 0;
Uint16 succed_packet_rec_count_c = 0;

void seperateBMSdata(unsigned char receved_data_length, unsigned char *SPI_rec_dat_buffer)
{
    Uint8 local_counter = 0;
    Uint16 I_temp_local_v;
    switch (SPI_rec_dat_buffer[4])
    {
        case REQUEST:
            req_count++;
            if (SPI_rec_dat_buffer[2] == HV_DATA)
            {
                Controller.op_mode_main = SPI_rec_dat_buffer[5];
//            Current_value
                switch (SPI_rec_dat_buffer[3])
                {
                    case ALL_DATA:
                        erroe_local_ltc_c_old = erroe_local_ltc_c;

                        erroe_local_ltc_c = SPI_rec_dat_buffer[20 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[19 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        if (erroe_local_ltc_c - erroe_local_ltc_c_old != 0)
                        {
                            break;
                        }
                        succed_packet_rec_count_c++;

                        for (local_counter = 0; local_counter < CELLS_PER_IC; local_counter++)
                        {
                            BMS_data_IC[SPI_rec_dat_buffer[6]].cell_voltages[local_counter] =
                                    SPI_rec_dat_buffer[9 + (local_counter * 2)]
                                            | SPI_rec_dat_buffer[8 + (local_counter * 2)] << 8;
                        }
                        for (local_counter = 0; local_counter < AUX_PER_IC; local_counter++)
                        {
                            BMS_data_IC[SPI_rec_dat_buffer[6]].aux_voltages[local_counter] =
                                    SPI_rec_dat_buffer[9 + (CELLS_PER_IC * 2) + (local_counter * 2)]
                                            | SPI_rec_dat_buffer[8 + (CELLS_PER_IC * 2)
                                                    + (local_counter * 2)] << 8;
                        }
                        //Pack voltage
                        BMS_data_IC[SPI_rec_dat_buffer[6]].Pack_voltage =
                                (SPI_rec_dat_buffer[9 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)]
                                        | SPI_rec_dat_buffer[8 + (CELLS_PER_IC * 2)
                                                + (AUX_PER_IC * 2)] << 8) * 2;
                        //Itemp
                        I_temp_local_v = (SPI_rec_dat_buffer[11 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2)]
                                | SPI_rec_dat_buffer[10 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)]
                                        << 8);
                        BMS_data_IC[SPI_rec_dat_buffer[6]].I_temp =
                                (double) ((((double) I_temp_local_v) * (0.0001 / 0.0075)) - 273); //Internal Die Temperature(�C) = itmp � (100 �V / 7.5mV)�C - 273�C
                        //V analog
                        BMS_data_IC[SPI_rec_dat_buffer[6]].V_regA = SPI_rec_dat_buffer[13
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)]
                                | SPI_rec_dat_buffer[12 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)]
                                        << 8;
                        //V digital
                        BMS_data_IC[SPI_rec_dat_buffer[6]].V_regD = SPI_rec_dat_buffer[15
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)]
                                | SPI_rec_dat_buffer[14 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)]
                                        << 8;
                        //ov and uv flags
                        BMS_data_IC[SPI_rec_dat_buffer[6]].UV_OV_flags.all[0] = SPI_rec_dat_buffer[8
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[9 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        BMS_data_IC[SPI_rec_dat_buffer[6]].UV_OV_flags.all[1] =
                                SPI_rec_dat_buffer[10 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)];
                        //mux fail
                        BMS_data_IC[SPI_rec_dat_buffer[6]].mux_fail = SPI_rec_dat_buffer[11
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //thsd
                        BMS_data_IC[SPI_rec_dat_buffer[6]].THSD = SPI_rec_dat_buffer[12
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //ADC test
                        BMS_data_IC[SPI_rec_dat_buffer[6]].ADC_test_count[0] = SPI_rec_dat_buffer[13
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        BMS_data_IC[SPI_rec_dat_buffer[6]].ADC_test_count[1] = SPI_rec_dat_buffer[14
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        BMS_data_IC[SPI_rec_dat_buffer[6]].ADC_test_count[2] = SPI_rec_dat_buffer[15
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //ADC overlap
                        BMS_data_IC[SPI_rec_dat_buffer[6]].ADC_overlap_error = SPI_rec_dat_buffer[16
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //Digital redundancy check
                        BMS_data_IC[SPI_rec_dat_buffer[6]].Digital_redundancy_error[0] =
                                SPI_rec_dat_buffer[17 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)];
                        BMS_data_IC[SPI_rec_dat_buffer[6]].Digital_redundancy_error[1] =
                                SPI_rec_dat_buffer[18 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)];
                        //error count

                        Controller.error_ltc_hv_count_old = Controller.error_ltc_hv_count;

                        Controller.error_ltc_hv_count = SPI_rec_dat_buffer[20 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[19 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        Controller.error_msp_hv_count = SPI_rec_dat_buffer[22 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[21 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        //discharge cells and timer
                        BMS_data_IC[SPI_rec_dat_buffer[6]].Discharge_cell_timer =
                                SPI_rec_dat_buffer[23 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] >> 4;
                        BMS_data_IC[SPI_rec_dat_buffer[6]].d_cell.all = SPI_rec_dat_buffer[24
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[23 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        BMS_data_IC[SPI_rec_dat_buffer[6]].d_cell.all &= 0x0FFF;

                        //ov config
                        BMS_data_IC[SPI_rec_dat_buffer[6]].OV_config = SPI_rec_dat_buffer[26
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[25 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        //uv
                        BMS_data_IC[SPI_rec_dat_buffer[6]].UV_config = SPI_rec_dat_buffer[28
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[27 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        //gpio
                        BMS_data_IC[SPI_rec_dat_buffer[6]].gpioReg.all = SPI_rec_dat_buffer[29
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)] >> 3;
                        BMS_data_IC[SPI_rec_dat_buffer[6]].refon_bit = SPI_rec_dat_buffer[29
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)] >> 2;
                        BMS_data_IC[SPI_rec_dat_buffer[6]].D_timer_en = SPI_rec_dat_buffer[29
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)] >> 1;
                        BMS_data_IC[SPI_rec_dat_buffer[6]].adc_opt = SPI_rec_dat_buffer[29
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //pwm
                        for (local_counter = 0; local_counter < 3; local_counter++)
                        {    //Pack voltage//Itemp//V analog//V digital
                            BMS_data_IC[SPI_rec_dat_buffer[6]].pwm_reg.all[local_counter] =
                                    SPI_rec_dat_buffer[30 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                            + (STAT_REG * 2) + (local_counter * 2)]
                                            | SPI_rec_dat_buffer[31 + (CELLS_PER_IC * 2)
                                                    + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                                    + (local_counter * 2)] << 8;
                        }
                        //sctrl
                        for (local_counter = 0; local_counter < 3; local_counter++)
                        {    //Pack voltage//I-temp//V analog//V digital
                            BMS_data_IC[SPI_rec_dat_buffer[6]].sctrl_reg.all[local_counter] =
                                    SPI_rec_dat_buffer[36 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                            + (STAT_REG * 2) + (local_counter * 2)]
                                            | SPI_rec_dat_buffer[37 + (CELLS_PER_IC * 2)
                                                    + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                                    + (local_counter * 2)] << 8;
                        }
                        for (local_counter = 0; local_counter < TEMP_PER_IC; local_counter++)
                        {
                            signed int temp_buf = SPI_rec_dat_buffer[42 + (CELLS_PER_IC * 2)
                                    + (AUX_PER_IC * 2) + (STAT_REG * 2) + local_counter];
                            if (temp_buf < 200)
                            {
                                BMS_data_IC[SPI_rec_dat_buffer[6]].temparature_val[local_counter] =
                                        temp_buf;
                            }
                            else
                            {
                            }

                        }
                        Controller.last_update_hv = SPI_rec_dat_buffer[42 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_IC];
                        //-----------------------------//
                        current_pkt.byte[0] = (SPI_rec_dat_buffer[45 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_IC] & 0xFF)
                                | (SPI_rec_dat_buffer[46 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2) + TEMP_PER_IC] & 0xFF) << 8;
                        current_pkt.byte[1] = (SPI_rec_dat_buffer[47 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_IC] & 0xFF)
                                | (SPI_rec_dat_buffer[48 + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2) + TEMP_PER_IC] & 0xFF) << 8;
//                current_pkt.byte[1] = SPI_rec_dat_buffer[46 + (CELLS_PER_IC * 2)
//                        + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_IC];
//                current_pkt.byte[2] = SPI_rec_dat_buffer[47 + (CELLS_PER_IC * 2)
//                        + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_IC];
//                current_pkt.byte[3] = SPI_rec_dat_buffer[48 + (CELLS_PER_IC * 2)
//                        + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_IC];
                        //-----------------------------//
                        Controller.received_packet_code_hv = SPI_rec_dat_buffer[49
                                + (CELLS_PER_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                + TEMP_PER_IC];
                        Controller.sent_packet_code_hv = SPI_rec_dat_buffer[50 + (CELLS_PER_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_IC];
                        //total pack voltage calculation
                        if (SPI_rec_dat_buffer[6] == (TOTAL_IC - 1))
                        {
                            total_pack_voltage_temp = 0;
                            for (local_counter = 0; local_counter < TOTAL_IC; local_counter++)
                            {
                                total_pack_voltage_temp +=
                                        ((double) BMS_data_IC[local_counter].Pack_voltage) / 1000;
                            }
                            Controller.total_pack_voltage = total_pack_voltage_temp;
                        }
                        break;
                    case CLASSIFIED:
                        erroe_local_ltc_c_old = erroe_local_ltc_c;

                        erroe_local_ltc_c = SPI_rec_dat_buffer[26] | SPI_rec_dat_buffer[25] << 8;

                        if (erroe_local_ltc_c - erroe_local_ltc_c_old != 0)
                        {
                            break;
                        }
                        succed_packet_rec_count_c++;

//                        Controller.highest_cell_volt = SPI_rec_dat_buffer[9]
//                                | SPI_rec_dat_buffer[8] << 8;
//                        Controller.highest_cell_id = SPI_rec_dat_buffer[10];
//                        Controller.lowest_cell_volt = SPI_rec_dat_buffer[12]
//                                | SPI_rec_dat_buffer[11] << 8;
//                        Controller.lowest_cell_id = SPI_rec_dat_buffer[13];
//                        Controller.highest_temp = SPI_rec_dat_buffer[14];
//                        Controller.highest_temp_id = SPI_rec_dat_buffer[15];
//                        Controller.lowest_temp = SPI_rec_dat_buffer[16];
//                        Controller.lowest_temp_id = SPI_rec_dat_buffer[17];
                        Controller.highest_temp_ic = SPI_rec_dat_buffer[19]
                                | SPI_rec_dat_buffer[18] << 8;
                        Controller.highest_temp_ic_id = SPI_rec_dat_buffer[20];
                        Controller.highest_board_temp = SPI_rec_dat_buffer[21];
                        Controller.highest_board_temp_id = SPI_rec_dat_buffer[22];
                        Controller.lowest_board_temp = SPI_rec_dat_buffer[23];
                        Controller.lowest_board_temp_id = SPI_rec_dat_buffer[24];

                        Controller.error_ltc_hv_count_old = Controller.error_ltc_hv_count;

                        Controller.error_ltc_hv_count = SPI_rec_dat_buffer[26]
                                | SPI_rec_dat_buffer[25] << 8;
                        Controller.error_msp_hv_count = SPI_rec_dat_buffer[28]
                                | SPI_rec_dat_buffer[27] << 8;

                        Controller.last_update_hv = SPI_rec_dat_buffer[33];

                        Controller.error_cell_bit.all = (SPI_rec_dat_buffer[34] << 8)
                                | SPI_rec_dat_buffer[35];
//                current_pkt.byte[0] =SPI_rec_dat_buffer[36];
//                current_pkt.byte[1] =SPI_rec_dat_buffer[37];
//                current_pkt.byte[2] =SPI_rec_dat_buffer[38];
//                current_pkt.byte[3] =SPI_rec_dat_buffer[39];
                        Controller.received_packet_code_hv = SPI_rec_dat_buffer[40];
                        Controller.sent_packet_code_hv = SPI_rec_dat_buffer[41];
                        break;
                    case HUMIDITY_TEMPERATURE:
                        erroe_local_ltc_c_old = erroe_local_ltc_c;

                        erroe_local_ltc_c = SPI_rec_dat_buffer[26] | SPI_rec_dat_buffer[25] << 8;

                        if (erroe_local_ltc_c - erroe_local_ltc_c_old != 0)
                        {
                            break;
                        }
                        succed_packet_rec_count_c++;

                        for(local_counter = 0; local_counter < TOTAL_IC; local_counter++)
                        {
                            BMS_data_IC[local_counter].humi_msb = SPI_rec_dat_buffer[8 + 6*local_counter];
                            BMS_data_IC[local_counter].humi_lsb = SPI_rec_dat_buffer[9 + 6*local_counter];
                            BMS_data_IC[local_counter].temp_msb = SPI_rec_dat_buffer[10 + 6*local_counter];
                            BMS_data_IC[local_counter].temp_lsb = SPI_rec_dat_buffer[11 + 6*local_counter];
                            BMS_data_IC[local_counter].humi_crc_flag = SPI_rec_dat_buffer[12 + 6*local_counter];
                            BMS_data_IC[local_counter].temp_crc_flag = SPI_rec_dat_buffer[13 + 6*local_counter];
                        }

                        Controller.error_ltc_hv_count_old = Controller.error_ltc_hv_count;

                        Controller.error_ltc_hv_count = SPI_rec_dat_buffer[21]
                                | SPI_rec_dat_buffer[20] << 8;
                        Controller.error_msp_hv_count = SPI_rec_dat_buffer[23]
                                | SPI_rec_dat_buffer[22] << 8;

                        Controller.last_update_hv = SPI_rec_dat_buffer[24];

                        Controller.error_cell_bit.all = (SPI_rec_dat_buffer[25] << 8)
                                | SPI_rec_dat_buffer[26];

                        Controller.received_packet_code_hv = SPI_rec_dat_buffer[27];
                        Controller.sent_packet_code_hv = SPI_rec_dat_buffer[28];
                        break;
                    case OPEN_WIRE:
                        erroe_local_ltc_c_old = erroe_local_ltc_c;

                        erroe_local_ltc_c = SPI_rec_dat_buffer[18] | SPI_rec_dat_buffer[17] << 8;

                        if (erroe_local_ltc_c - erroe_local_ltc_c_old != 0)
                        {
                            break;
                        }
                        succed_packet_rec_count_c++;

                        for(local_counter = 0; local_counter < TOTAL_IC; local_counter++)
                        {
                            BMS_data_IC[local_counter].open_wire_lsb = SPI_rec_dat_buffer[8 + 2*local_counter];
                            BMS_data_IC[local_counter].open_wire_msb = SPI_rec_dat_buffer[9 + 2*local_counter];
                        }

                        Controller.error_ltc_hv_count_old = Controller.error_ltc_hv_count;

                        Controller.error_ltc_hv_count = SPI_rec_dat_buffer[13]
                                | SPI_rec_dat_buffer[12] << 8;
                        Controller.error_msp_hv_count = SPI_rec_dat_buffer[15]
                                | SPI_rec_dat_buffer[14] << 8;

                        Controller.last_update_hv = SPI_rec_dat_buffer[16];

                        Controller.error_cell_bit.all = (SPI_rec_dat_buffer[17] << 8)
                                | SPI_rec_dat_buffer[18];

                        Controller.received_packet_code_hv = SPI_rec_dat_buffer[19];
                        Controller.sent_packet_code_hv = SPI_rec_dat_buffer[20];
                        break;
                    default:
                        break;
                }
            }
            else
            {
                //AUX ----------------------------------------------------------------------- data
                Controller.op_mode_aux = SPI_rec_dat_buffer[5];
                switch (SPI_rec_dat_buffer[3])
                {
                    case ALL_DATA:
                        //cell voltages
                        for (local_counter = 0; local_counter < CELLS_PER_AUX_IC; local_counter++)
                        {
                            BMS_data_aux.cell_voltages[local_counter] = SPI_rec_dat_buffer[9
                                    + (local_counter * 2)]
                                    | SPI_rec_dat_buffer[8 + (local_counter * 2)] << 8;
                        }
                        //aux voltages
                        for (local_counter = 0; local_counter < AUX_PER_IC; local_counter++)
                        {
                            BMS_data_aux.aux_voltages[local_counter] = SPI_rec_dat_buffer[9
                                    + (CELLS_PER_AUX_IC * 2) + (local_counter * 2)]
                                    | SPI_rec_dat_buffer[8 + (CELLS_PER_AUX_IC * 2)
                                            + (local_counter * 2)] << 8;
                            //Pack voltage
                            BMS_data_aux.Pack_voltage = (SPI_rec_dat_buffer[9
                                    + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)]
                                    | SPI_rec_dat_buffer[8 + (CELLS_PER_AUX_IC * 2)
                                            + (AUX_PER_IC * 2)] << 8) * 2;
                            //Itemp
                            I_temp_local_v = (SPI_rec_dat_buffer[11 + (CELLS_PER_AUX_IC * 2)
                                    + (AUX_PER_IC * 2)]
                                    | SPI_rec_dat_buffer[10 + (CELLS_PER_AUX_IC * 2)
                                            + (AUX_PER_IC * 2)] << 8);
                            BMS_data_aux.I_temp = ((((double) I_temp_local_v) * (0.0001 / 0.0075))
                                    - 273); //Internal Die Temperature(�C) = itmp � (100 �V / 7.5mV)�C - 273�C
                        }
                        //V analog
                        BMS_data_aux.V_regA = SPI_rec_dat_buffer[13 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2)]
                                | SPI_rec_dat_buffer[12 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)]
                                        << 8;
                        //V digital
                        BMS_data_aux.V_regD = SPI_rec_dat_buffer[15 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2)]
                                | SPI_rec_dat_buffer[14 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)]
                                        << 8;
                        //ov and uv flags
                        BMS_data_aux.UV_OV_flags.all[0] = SPI_rec_dat_buffer[8
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[9 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        BMS_data_aux.UV_OV_flags.all[1] = SPI_rec_dat_buffer[10
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //mux fail
                        BMS_data_aux.mux_fail = SPI_rec_dat_buffer[11 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //thsd
                        BMS_data_aux.THSD = SPI_rec_dat_buffer[12 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //ADC test
                        BMS_data_aux.ADC_test_count[0] = SPI_rec_dat_buffer[13
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        BMS_data_aux.ADC_test_count[1] = SPI_rec_dat_buffer[14
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        BMS_data_aux.ADC_test_count[2] = SPI_rec_dat_buffer[15
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //ADC overlap
                        BMS_data_aux.ADC_overlap_error = SPI_rec_dat_buffer[16
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //Digital redundancy check
                        BMS_data_aux.Digital_redundancy_error[0] = SPI_rec_dat_buffer[17
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        BMS_data_aux.Digital_redundancy_error[1] = SPI_rec_dat_buffer[18
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //error count

                        Controller.error_ltc_aux_count_old = Controller.error_ltc_aux_count;

                        Controller.error_ltc_aux_count = SPI_rec_dat_buffer[20
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[19 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        Controller.error_msp_aux_count = SPI_rec_dat_buffer[22
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[21 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        //discharge cells and timer
                        BMS_data_aux.Discharge_cell_timer = SPI_rec_dat_buffer[23
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)] >> 4;
                        BMS_data_aux.d_cell.all = SPI_rec_dat_buffer[24 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[23 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        BMS_data_aux.d_cell.all &= 0x0FFF;

                        //ov config
                        BMS_data_aux.OV_config = SPI_rec_dat_buffer[26 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[25 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        //uv
                        BMS_data_aux.UV_config = SPI_rec_dat_buffer[28 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)]
                                | SPI_rec_dat_buffer[27 + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2)
                                        + (STAT_REG * 2)] << 8;
                        //gpio
                        BMS_data_aux.gpioReg.all = SPI_rec_dat_buffer[29 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)] >> 3;
                        BMS_data_aux.refon_bit = SPI_rec_dat_buffer[29 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)] >> 2;
                        BMS_data_aux.D_timer_en = SPI_rec_dat_buffer[29 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)] >> 1;
                        BMS_data_aux.adc_opt = SPI_rec_dat_buffer[29 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2)];
                        //pwm
                        for (local_counter = 0; local_counter < 3; local_counter++)
                        {    //Pack voltage//Itemp//V analog//V digital
                            BMS_data_aux.pwm_reg.all[local_counter] = SPI_rec_dat_buffer[30
                                    + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                    + (local_counter * 2)]
                                    | SPI_rec_dat_buffer[31 + (CELLS_PER_AUX_IC * 2)
                                            + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                            + (local_counter * 2)] << 8;
                        }
                        //sctrl
                        for (local_counter = 0; local_counter < 3; local_counter++)
                        {    //Pack voltage//I-temp//V analog//V digital
                            BMS_data_aux.sctrl_reg.all[local_counter] = SPI_rec_dat_buffer[36
                                    + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                    + (local_counter * 2)]
                                    | SPI_rec_dat_buffer[37 + (CELLS_PER_AUX_IC * 2)
                                            + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                            + (local_counter * 2)] << 8;
                        }
                        for (local_counter = 0; local_counter < TEMP_PER_AUX_IC; local_counter++)
                        {
                            BMS_data_aux.temparature_val[local_counter] = SPI_rec_dat_buffer[42
                                    + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                    + local_counter];
                        }
                        Controller.last_update_aux = SPI_rec_dat_buffer[42 + (CELLS_PER_AUX_IC * 2)
                                + (AUX_PER_IC * 2) + (STAT_REG * 2) + TEMP_PER_AUX_IC];
                        Controller.received_packet_code_aux = SPI_rec_dat_buffer[47
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                + TEMP_PER_AUX_IC];
                        Controller.sent_packet_code_aux = SPI_rec_dat_buffer[48
                                + (CELLS_PER_AUX_IC * 2) + (AUX_PER_IC * 2) + (STAT_REG * 2)
                                + TEMP_PER_AUX_IC];
                        break;
                    case CLASSIFIED:
//                Controller.highest_cell_volt = SPI_rec_dat_buffer[9]
//                        | SPI_rec_dat_buffer[8] << 8;
                        BMS_data_aux.highest_cell_local = SPI_rec_dat_buffer[9]
                                | SPI_rec_dat_buffer[8] << 8;
                        BMS_data_aux.highest_cell_id_local = SPI_rec_dat_buffer[10];

                        BMS_data_aux.lowest_cell_local = SPI_rec_dat_buffer[12]
                                | SPI_rec_dat_buffer[11] << 8;
                        BMS_data_aux.lowest_cell_id_local = SPI_rec_dat_buffer[13];
                        BMS_data_aux.highest_temp = SPI_rec_dat_buffer[14];
                        BMS_data_aux.highest_temp_id_local = SPI_rec_dat_buffer[15];
                        BMS_data_aux.lowest_temp = SPI_rec_dat_buffer[16];
                        BMS_data_aux.lowest_temp_id_local = SPI_rec_dat_buffer[17];

                        Controller.error_ltc_aux_count_old = Controller.error_ltc_aux_count;

                        Controller.error_ltc_aux_count = SPI_rec_dat_buffer[26]
                                | SPI_rec_dat_buffer[25] << 8;

                        Controller.error_msp_aux_count = SPI_rec_dat_buffer[28]
                                | SPI_rec_dat_buffer[27] << 8;

                        Controller.last_update_aux = SPI_rec_dat_buffer[33];

                        Controller.received_packet_code_aux = SPI_rec_dat_buffer[39];
                        Controller.sent_packet_code_aux = SPI_rec_dat_buffer[38];
                        break;
                    default:
                        break;
                }
            }
            break;
        case RESEND:
            break;
        case ERROR_C:
            break;
        case AUX_DATA:
            break;
        default:
            break;
    }
}
/*
 * Function name    : analyze_rdata
 * Description      : classified received data
 */
void analyze_rdata()
{
    Uint8 local_counter_ic = 0;
    Uint8 local_counter = 0;
    Uint16 highest_cell_v_local = 0;
    Uint16 lowest_cell_v_local = 0xFFFF;
    Uint16 highest_cell_id_local = 0xFF;
    Uint16 lowest_cell_id_local = 0xFF;
    Uint8 highest_t_local = 0;
    Uint8 lowest_t_local = 0xFF;
    Uint8 highest_t_id_local = 0xFF;
    Uint8 lowest_t_id_local = 0xFF;
    Uint8 lowest_pack_v_local = 0xFF;

    static Uint16 lowest_cell_v_buffer[MOVING_AVG_WINDOW] = {0};
    static Uint8 moving_avg_index = 0;

    for (local_counter_ic = 0; local_counter_ic < TOTAL_IC; local_counter_ic++)
    {
        for (local_counter = 0; local_counter < CELLS_PER_IC; local_counter++)
        {
            if ((local_counter == 4) || (local_counter == 5) || (local_counter == 9) ||(local_counter == 10)|| (local_counter == 11))
            {
                local_counter++;
            }
            else
            {
                if (BMS_data_IC[local_counter_ic].cell_voltages[local_counter]
                        > highest_cell_v_local)
                {
                    highest_cell_v_local =
                            BMS_data_IC[local_counter_ic].cell_voltages[local_counter];
                    highest_cell_id_local = (local_counter_ic * CELLS_PER_IC) + local_counter;
                }
                if (BMS_data_IC[local_counter_ic].cell_voltages[local_counter]
                        < lowest_cell_v_local)
                {
                    lowest_cell_v_local =
                            BMS_data_IC[local_counter_ic].cell_voltages[local_counter];
                    lowest_cell_id_local = (local_counter_ic * CELLS_PER_IC) + local_counter;
                }
            }
        }
        if (BMS_data_IC[local_counter_ic].Pack_voltage < lowest_pack_v_local)
        {
            lowest_pack_v_local = BMS_data_IC[local_counter_ic].Pack_voltage;
        }
    }
    for (local_counter_ic = 0; local_counter_ic < TOTAL_IC; local_counter_ic++)
        {
            for (local_counter = 0; local_counter < TEMP_PER_IC; local_counter++)
            {
                if (BMS_data_IC[local_counter_ic].temparature_val[local_counter] > highest_t_local)
                {
                    highest_t_local = BMS_data_IC[local_counter_ic].temparature_val[local_counter];
                    highest_t_id_local = 1 + local_counter;
                }
                if (BMS_data_IC[local_counter_ic].temparature_val[local_counter] < lowest_t_local)
                {
                    lowest_t_local = BMS_data_IC[local_counter_ic].temparature_val[local_counter];
                    lowest_t_id_local = 1 + local_counter;
                }
            }
        }
    Controller.highest_cell_volt = highest_cell_v_local;
    Controller.lowest_cell_volt = lowest_cell_v_local;
    Controller.highest_cell_id = highest_cell_id_local;
    Controller.lowest_cell_id = lowest_cell_id_local;
    Controller.highest_temp = highest_t_local;
    Controller.lowest_temp = lowest_t_local;
    Controller.highest_temp_id = highest_t_id_local;
    Controller.lowest_temp_id = lowest_t_id_local;
    Controller.lowest_pack_v = lowest_pack_v_local;

    // === Moving Average Calculation ===

    lowest_cell_v_buffer[moving_avg_index] = lowest_cell_v_local;

    moving_avg_index++;

    if (moving_avg_index >= MOVING_AVG_WINDOW)
    {
        moving_avg_index = 0;

        Uint32 sum = 0;
        Uint8 i;

        for (i = 0; i < MOVING_AVG_WINDOW; i++)
        {
            sum += lowest_cell_v_buffer[i];
        }

        lowest_cell_v_moving_avg = (Uint16)(sum / MOVING_AVG_WINDOW);
    }


}
/*
 * Function name    : analyse_humidity_temp
 * Description      : Humidity sensor data received
 */
void analyse_humidity_temp(Uint8 local_IC)
{
    Uint16 slave_humidity;
    Uint16 slave_temperature;

    slave_humidity = BMS_data_IC[local_IC].humi_msb<<8 | BMS_data_IC[local_IC].humi_lsb;
    slave_temperature = BMS_data_IC[local_IC].temp_msb<<8 | BMS_data_IC[local_IC].temp_lsb;

    BMS_data_IC[local_IC].slave_humidity_val = (((float)(slave_humidity)/65535.0)*100);
    BMS_data_IC[local_IC].slave_temperature_val = (((float)(slave_temperature)/65535.0)*175 - 45);
}

/*
 * Function name    : scan_contactor_fb
 * Description      : check the contactor status
 */
void scan_contactor_fb()
{
    Controller.contactor_fb[0] = GpioDataRegs.GPADAT.bit.GPIO3;
    Controller.contactor_fb[1] = GpioDataRegs.GPBDAT.bit.GPIO40;
    Controller.contactor_fb[2] = GpioDataRegs.GPADAT.bit.GPIO26;
    Controller.contactor_fb[3] = GpioDataRegs.GPADAT.bit.GPIO27;
}

/*
 * Function name    : scan_emergency
 * Description      : check the contactor status
 */
void scan_emergency()
{
    Controller.emergency_on = GpioDataRegs.GPBDAT.bit.GPIO41;
}
/*
 * Function name    : Msp1_reset
 * Description      : reset main battery controller mcu
 */
void Msp1_reset()
{
    GpioDataRegs.GPACLEAR.bit.GPIO22 = 1;
    DELAY_US(1000);
    GpioDataRegs.GPASET.bit.GPIO22 = 1;
}

/*
 * Function name    : BMS_can_data
 * Description      : Ready data to send (evcu)
 */
volatile char can_data_setting = 0;
void BMS_can_data()
{
    can_data_setting = 1;
    //PDU_getData_local.fixSetG &= ~com_error_BMS;
    //PDU_getData_local.fixSetG.bit.op_mode_error = 0;
    //lowest cell details
    //resolution of the voltage 3 digits
    if (Controller.error_cell_bit.all == 0)
    {
        //lowest cell details
        PDU_getData_local.lowest_cell_voltage = Controller.lowest_cell_volt / 10;
        //highest cell details
        PDU_getData_local.highest_cell_voltage = Controller.highest_cell_volt / 10;

        //highest temperature details
        PDU_getData_local.highest_pack_temparature = Controller.highest_temp;
    }
    else
    {
        PDU_getData_local.lowest_cell_voltage = 0;
        PDU_getData_local.highest_cell_voltage = 0;
        PDU_getData_local.highest_pack_temparature = 0;
    }


    //total pack voltage
    PDU_getData_local.total_pack_voltage = (Uint16) (Controller.total_pack_voltage * 10);

    PDU_getData_local.current_Axx = Current_value;
    //PDU_getData_local.batt_power = (Controller.total_pack_voltage * current_val_A);


    Uint16 display_soc = get_Temp_soc();

    if ((charge_state == 0) && (display_soc == 10000))
    {
        display_soc = 9900;
    }
    else if ((current_val_A > DEADBAND) && (display_soc < 100))
    {
        display_soc = 100;
    }


    Uint8 soc_out = (Uint8) ceil(Controller.SOC_value);

    if ((charge_state == 0) && (soc_out == 100))
    {
        soc_out = 99;
    }
    else if ((current_val_A > DEADBAND) && (Controller.SOC_value < 1))
    {
        soc_out = 1;
    }

    PDU_getData_local.SOC_val = soc_out;
    PDU_getData_local.Temp_soc_val = display_soc;

    PDU_getData_local.display_soc_val = (Uint8)ceil(display_soc / 100.0f);

    if ((charge_state == 0) && (PDU_getData_local.display_soc_val == 100))
    {
        PDU_getData_local.display_soc_val = 99;
    }
    else if ((current_val_A > DEADBAND) && (PDU_getData_local.display_soc_val < 1))
    {
        PDU_getData_local.display_soc_val = 1;
    }

    PDU_getData_local.SOH_val = ceil((Uint8) Controller.SOH_value);

    PDU_getData_local.temp_val[0] = BMS_data_IC[0].temparature_val[0];
    PDU_getData_local.temp_val[1] = BMS_data_IC[0].temparature_val[1];
    PDU_getData_local.temp_val[2] = BMS_data_IC[0].temparature_val[2];
    PDU_getData_local.temp_val[3] = BMS_data_IC[1].temparature_val[0];
    PDU_getData_local.temp_val[4] = BMS_data_IC[1].temparature_val[1];
    PDU_getData_local.temp_val[5] = BMS_data_IC[1].temparature_val[2];

    if (first_data_cycle_ok == 0)
    {
        ENABLE_CAN = 1;
        first_data_cycle_ok = 1;
    }
    can_data_setting = 0;
}

/*
 * Function Name    : timer_task_can
 * Description      : can data send task
 */
void timer_task_can()
{
    static uint16_t getData_index = 0;
    static uint16_t debugData_index = 0;

    PDU_getData_local.highest_pack_temparature = Controller.highest_temp;
#ifdef MASTER
    if (slave_status)
    {
        master_dataSort();
        PDU_setDataSlave_local.fixSetS = PDU_setData_local.fixSetS;
        PDU_setDataSlave_i = PDU_setDataSlave_local;
    }
#endif

    PDU_getData_i = PDU_getData_local;
    PDU_getData_write(getData_index++, canMap.msg[CAN_OBJ_15].sendData);
    CAN_sendMessage(CAN_OBJ_15, NOMINAL_DLC, canMap.msg[CAN_OBJ_15].sendData);
    if (getData_index > 3)
    {
        getData_index = 0;
    }

    if (debug_CMD)
    {
        PDU_debugData_write(debugData_index++, canMap.msg[CAN_OBJ_14].sendData);
        CAN_sendMessage(CAN_OBJ_14, NOMINAL_DLC, canMap.msg[CAN_OBJ_14].sendData);
        if(slave_read.bit.sht30_read_enable_flag || slave_read.bit.open_wire_read_enable_flag)
        {
            if(debugData_index > 9)
            {
                debugData_index = 0;
            }
        }
        else
        {
            debugData_index &= 0x07;
        }
    }
    else
    {
        PDU_tempData_write(canMap.msg[CAN_OBJ_14].sendData);
        CAN_sendMessage(CAN_OBJ_14, NOMINAL_DLC, canMap.msg[CAN_OBJ_14].sendData);
    }

#ifdef MASTER
    if (slave_status)
    {
        PDU_setDataSlave_write(canMap.msg[CAN_OBJ_18].sendData);
        CAN_sendMessage(CAN_OBJ_18, NOMINAL_DLC, canMap.msg[CAN_OBJ_18].sendData);
    }
#endif

//    EALLOW;
//    ECanaRegs.CANTRR.all = 0x00000000;
//    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
//    EDIS;
//    ECanaShadow.CANTRS.all = 0x00000000;
    loop_counter_x++;

    if (loop_counter_x % 10 == 0)
    {
        broadcast_eeprom_data_over_can();
    }

}

/*
 * Function Name    : Slave controller
 * Description      : Control parallely connected battery pack
 */

Uint8 slave_controller(void)
{
    Uint8 status = 0;
    if (ecana_int_slaveData_counter)
    {
        status = 1;
        slave_watchdog++;
    }
    else
    {
        PDU_getData_slave.current_Axx = 0;
        PDU_getData_slave.Battery_capacity = 0;
    }

    if (status == 1)
    {
        if ((PDU_getData_slave.op_mode == 1)
                && ((abs(PDU_getData_slave.total_pack_voltage / 10) - Controller.total_pack_voltage)
                        <= 2))
        {
            PDU_setDataSlave_local.contactor_on = PDU_setData_local.contactor_on;
            PDU_setDataSlave_local.contactor_on_inverse = PDU_setData_local.contactor_on_inverse;
        }
        else if ((PDU_setData_local.fixSetS.bit.charger_connected == 1)
                && (PDU_getData_slave.op_mode == 1)
                && ((abs(PDU_getData_slave.total_pack_voltage / 10) - Controller.total_pack_voltage)
                        <= 1))
        {
            PDU_setDataSlave_local.contactor_on = PDU_setData_local.contactor_on;
            PDU_setDataSlave_local.contactor_on_inverse = PDU_setData_local.contactor_on_inverse;
        }
        else if ((PDU_getData_slave.op_mode == 1)
                && ((abs(PDU_getData_slave.total_pack_voltage / 10) - Controller.total_pack_voltage)
                        > 2))
        {
            bms_opMode = error;
            status = 3;
        }
        else if ((PDU_getData_slave.op_mode == 3))
        {
            bms_opMode = error;
            status = 3;
        }
        else if ((PDU_getData_slave.op_mode == 2))
        {
            status = 2;
        }
        else
        {

        }
    }

    if (slave_watchdog > 400)
    {
        bms_opMode = error;
        //PDU_getData_local.fixSetG = com_error_SLAVE;
    }

    return status;
}

void master_dataSort(void)
{
    /* fixSetG*/
    PDU_getData_local.fixSetG.all |= PDU_getData_slave.fixSetG.all;

    /*Lowest cell voltage*/
    if (PDU_getData_slave.lowest_cell_voltage < PDU_getData_local.lowest_cell_voltage)
    {
        PDU_getData_local.lowest_cell_voltage = PDU_getData_slave.lowest_cell_voltage;
    }

    /*Highest cell voltage*/
    if (PDU_getData_slave.highest_cell_voltage > PDU_getData_local.highest_cell_voltage)
    {
        PDU_getData_local.highest_cell_voltage = PDU_getData_slave.highest_cell_voltage;
    }

    /*Highest pack temperature*/
    if (PDU_getData_slave.highest_pack_temparature > PDU_getData_local.highest_pack_temparature)
    {
        PDU_getData_local.highest_pack_temparature = PDU_getData_slave.highest_pack_temparature;
    }

    /*Total pack voltage*/
    PDU_getData_local.total_pack_voltage = (PDU_getData_slave.total_pack_voltage
            + PDU_getData_local.total_pack_voltage) / 2;

    /*Current*/
    PDU_getData_local.current_Axx += PDU_getData_slave.current_Axx;

    /*SOC*/
    float total_Ah = (Controller.SOC_value * batt_capacity / 100)
            + (PDU_getData_slave.Battery_capacity * PDU_getData_slave.SOC_val / 100);
    float total_capacity = batt_capacity + PDU_getData_slave.Battery_capacity;
    Uint8 combinedSOC = (Uint8) ceil(total_Ah * 100 / total_capacity);
    if ((charge_state == 0) && (combinedSOC == 100))
    {
        combinedSOC = 99;
    }
    else if ((current_val_A > DEADBAND) && (Controller.SOC_value < 1))
    {
        combinedSOC = 1;
    }

    PDU_getData_local.SOC_val = combinedSOC;

    /*SOH*/
    if (PDU_getData_slave.SOH_val < PDU_getData_local.SOH_val)
    {
        PDU_getData_local.SOH_val = PDU_getData_slave.SOH_val;
    }
}

/*
 * Function Name    : contactor_operator
 * Description      : Enable correct sequence of contact
 */
Uint16 contactor_state = 0;
Uint16 succed_packet_rec_count_c_old = 0;
Uint16 lv_trip_delay_count = 0;
Uint16 ov_trip_delay_count = 0;
Uint16 oc_trip_delay_count = 0;
Uint16 emg_delay = 0;
Uint16 precharge_timer = 0;
Uint16 n_tries = 0;
Uint16 con_fb_try_count = 0;

void contactor_operator()
{
    // check safety conditions
    // 1. voltage error
    // 2. temperature error
    // 3. over current error
    // 4. OP-mode error
    // 5. communication error(BMS-msp/ltc & EVCU)
    // 6. contactor fb error

    if (succed_packet_rec_count_c_old < succed_packet_rec_count_c)
    {
        succed_packet_error_count = 0;
    }
    else
    {
        succed_packet_error_count++;
    }
    succed_packet_rec_count_c_old = succed_packet_rec_count_c;


    if (Controller.highest_cell_volt > HIGHEST_CELL_VOLTAGE_LIMIT)
    {
        if ((ov_trip_delay_count > 40) && ((PDU_setData_local.fixSetS.bit.EVCU_State == 4) || (current_val_A < -1) ))
        {

            bms_opMode = error;
            trip_cause = hi_v_error;

            if (eeprom_save_done_flag == 1)
            {

                PDU_getData_local.fixSetG.bit.cell_voltage_error = 1;
                ov_trip_delay_count = 0;
            }
        }
        else
        {
            ov_trip_delay_count++;
        }
    }
    else
    {
        ov_trip_delay_count = 0;
        PDU_getData_local.fixSetG.bit.cell_voltage_error = 0;
    }

    if ((Controller.highest_temp > HIGHEST_TEMPERATURE_LIMIT) && (Controller.highest_temp < 100))
    {
        PDU_getData_local.fixSetG.bit.batt_temperature_error = 1;
        if (Controller.highest_temp > TEMPERATURE_CUTOFF)
        {
            trip_cause = hi_t_error;
            bms_opMode = error;
        }
    }
    else
    {
        PDU_getData_local.fixSetG.bit.batt_temperature_error = 0;
    }

#ifndef recovery_CS
    if ((succed_packet_error_count > 4000) || (shuntCS_watchdog > 4000))
    {
        bms_opMode = error;
        PDU_getData_local.fixSetG.bit.internal_comm_error = 1;
        trip_cause = com_error;
    }
    else
    {
        PDU_getData_local.fixSetG.bit.internal_comm_error = 0;
    }
#endif

    if (Current_value > DISCHARGE_CURRENT_THRESHOLD)
    {
        if ((oc_trip_delay_count > 100) || (bms_opMode != contactor_closed)
                || (Current_value > SC_CURRENT_THRESHOLD))
        {
            bms_opMode = error;
            PDU_getData_local.fixSetG.bit.batt_current_error = 1;
            trip_cause = dc_threshold;
        }
        else if (oc_trip_delay_count > 40)
        {
            PDU_getData_local.fixSetG.bit.batt_current_error = 1;
            oc_trip_delay_count++;
        }
        else
        {
            oc_trip_delay_count++;
        }
    }
    else if (Current_value < CHARGE_CURRENT_THRESHOLD)
    {
        if ((oc_trip_delay_count > 100) || (bms_opMode != contactor_closed))
        {
            bms_opMode = error;
            PDU_getData_local.fixSetG.bit.batt_current_error = 1;
            trip_cause = cc_threshold;
        }
        else if (oc_trip_delay_count > 40)
        {
            PDU_getData_local.fixSetG.bit.batt_current_error = 1;
            oc_trip_delay_count++;
        }
        else
        {
            oc_trip_delay_count++;
        }
    }
    else
    {
        oc_trip_delay_count = 0;
        PDU_getData_local.fixSetG.bit.batt_current_error = 0;
    }

    if (Controller.lowest_cell_volt < LOWEST_CELL_VOLTAGE_LIMIT)
    {
        if (((PDU_setData_local.fixSetS.bit.charger_connected == 0) && (lv_trip_delay_count > 100))
                || ((Controller.lowest_cell_volt < LOWEST_CELL_VOLTAGE_CUTOFF)
                        && (succed_packet_rec_count_c > 10)))
        {
            bms_opMode = error;
            PDU_getData_local.fixSetG.bit.cell_voltage_error = 1;
            trip_cause = l_v_error;
//            soc_reset = 1;
        }
        else if ((PDU_setData_local.fixSetS.bit.charger_connected)
                || (PDU_setData_local.fixSetS_EVCC.all))
        {
            PDU_getData_local.fixSetG.bit.cell_voltage_error = 0;
        }
        else if (lv_trip_delay_count > 40)
        {
            lv_trip_delay_count++;
            PDU_getData_local.fixSetG.bit.cell_voltage_error = 1;
        }
        else
        {
            lv_trip_delay_count++;
        }
    }
    else
    {
        lv_trip_delay_count = 0;
    }

    if (bms_opMode == error)
    {
        PRECHARGER_DIS;
        CON_DRIVER_DIS;
    }
    else if ((bms_opMode == emergency_event) || (!EMG_FB))
    {
        PRECHARGER_DIS;
        CON_DRIVER_DIS;
        bms_opMode = emergency_event;
        emg_delay++;
        if ((emg_delay > 100) && EMG_FB)
        {
            bms_opMode = not_initialized;
            emg_delay = 0;
        }
    }
    else if (bms_opMode == not_initialized)
    {
        if (!CON_FB)
        {
            bms_opMode = initialized;
            contactor_state = 0;
        }
        else
        {
            bms_opMode = error;
            PDU_getData_local.fixSetG.bit.contactor_error = 1;
        }
    }
    else if (((bms_opMode == initialized) || (bms_opMode == contactor_closed)) && (PDU_setData_local.contactor_on == 1)
            && (PDU_setData_local.contactor_on_inverse == 2))
    {
        switch (contactor_state)
        {
            case 0:
                PRECHARGER_EN;
                CON_DRIVER_DIS;
                precharge_timer = 0;
                contactor_state++;
                break;

            case 1:
                if (precharge_timer > PRECHARGE_DELAY)
                {
                    PRECHARGER_EN;
                    CON_DRIVER_EN;
                    contactor_state++;
                    precharge_timer = 0;
                }
                else
                {
                    precharge_timer++;
                }
                break;

            case 2:
                CON_DRIVER_EN;
                if (CON_FB)
                {
                    contactor_state++;
                    n_tries = 0;
                }
                else if (n_tries == 5)
                {
                    PDU_getData_local.fixSetG.bit.contactor_error = 1;
                    bms_opMode = error;
                    n_tries = 0;
                }
                else
                {
                    n_tries++;
                    CON_DRIVER_DIS;
                }
                break;

            case 3:
                CON_DRIVER_EN;
                PRECHARGER_DIS;
                if(!CON_FB)
                {
                    if(con_fb_try_count > 5)
                    {
                        PDU_getData_local.fixSetG.bit.contactor_error = 1;
                        bms_opMode = error;
                    }
                    else
                    {
                        con_fb_try_count++;
                    }
                }
                else
                {
                    bms_opMode = contactor_closed;
                    con_fb_try_count = 0;
                }
                break;

            default:
                contactor_state = 0;
                break;
        }
    }
    PDU_getData_local.op_mode = bms_opMode;
}

Uint16 con_fb_try_count_fc = 0;

void fc_contactor_operator()
{

    FC_CON_DRIVER_EN;
    //Check for Battery error
    if (bms_opMode == error)
    {
        bms_fc_Mode = error_fc;
    }
//    else
//    {
//        bms_fc_Mode = not_initialized_fc;
//    }

    //charge FCcon enable while checking for FCcon error
    else if (bms_fc_Mode == error_fc)
    {
//        FC_CON_DRIVER_DIS;

    }
    else if (bms_fc_Mode == not_initialized_fc)
    {
        if (!FC_CON_FB)
        {
            bms_fc_Mode = initialized_fc;
        }
        else
        {
            bms_fc_Mode = error_fc;
        //    PDU_getData_local.fixSetChrg.bit.fc_con_error = 1;
        }
    }
    else if (((bms_fc_Mode == initialized_fc) || (bms_fc_Mode == contactor_closed_fc))
            && (PDU_setData_local.fixSetS_EVCC.bit.fc_con_enble == 1))
    {
        FC_CON_DRIVER_EN;
//        bms_fc_Mode = contactor_closed_fc;
//        con_fb_try_count_fc = 0;
//        PDU_getData_local.fixSetChrg.bit.fc_con_error = 0;
        if(!FC_CON_FB)
        {
            if(con_fb_try_count_fc > 5)
            {
//                PDU_getData_local.fixSetChrg.bit.fc_con_error = 1;
                bms_fc_Mode = error_fc;
            }
            else
            {
                con_fb_try_count_fc++;
            }
        }
        else
        {
            bms_fc_Mode = contactor_closed_fc;
            con_fb_try_count_fc = 0;
//            PDU_getData_local.fixSetChrg.bit.fc_con_error = 0;
        }
    }
//    PDU_getData_local.fixSetChrg.bit.fc_con_fb = FC_CON_FB;
}

/*
 * Function Name    : time_out_request
 *
 */
Uint8 secondry_can_send_counter = 0;
Uint8 RTC_read_timer = 0;
Uint8 comm_msp_error_timer_c = 0;
Uint8 ltc_com_error_count = 0;
Uint8 RS485_timer_counter = 0;
Uint8 RS485_timer_counter_reset = 0;
void time_out_request()
{
    if (request_packet_type == ALL_DATA)
    {
        timer_100ms++;
        if (timer_100ms >= 60)
        {
            timer_100ms = 0;
            hv_reset_set = 1; //hv_battery configuration enable
            config_finished_flag = 0;
        }
        timer_100ms_cfg++;
        if (timer_100ms_cfg >= 60)
        {
            timer_100ms_cfg = 0;
            config_finished_flag = 1;
        }
    }
    else if (request_packet_type == OPEN_WIRE)
    {
        timer_100ms++;
        if (timer_100ms >= 100)
        {
//            timeout_cunt++;
            timer_100ms = 0;
            hv_reset_set = 1; //hv_battery configuration enable
            config_finished_flag = 0;
        }
        timer_100ms_cfg++;
        if (timer_100ms_cfg >= 100)
        {
//            timeout_cunt++;
            timer_100ms_cfg = 0;
            config_finished_flag = 1;
        }
    }
    else
    {
        timer_100ms++;
        if (timer_100ms >= 20)
        {
            timer_100ms = 0;
            hv_reset_set = 1;
            config_finished_flag = 0;
        }
        timer_100ms_cfg++;
        if (timer_100ms_cfg >= 20)
        {
            secondary_can_send_en = 1;
            timer_100ms_cfg = 0;
            config_finished_flag = 1;
        }
    }
//---------------
    RTC_read_timer++;
    if (RTC_read_timer >= 50)
    {
        timer_ready_rtc = 1;
        RTC_read_timer = 0;
    }
    if (communication_error_msp != 0)
    {
        comm_msp_error_timer_c++;
        if (comm_msp_error_timer_c >= 40)
        {
            communication_error = 1;
            comm_msp_error_timer_c = 0;
        }
    }
    else
    {
        comm_msp_error_timer_c = 0;
    }
    if (Controller.error_ltc_hv_count != Controller.error_ltc_hv_count_old)
    {
        ltc_com_error_count++;
        if (ltc_com_error_count > 40)
        {
            communication_error = 1;
        }
    }
}

/*
 * Function Name    : main_msp_config_loop
 * Description      : configure auxiliary MSP430 (loop)
 */
void main_msp_config_loop()
{
    Msp1_reset();
    CTRLR_RDY;  //controller ready
    DELAY_US(100000); //Msp430 boot-up delay
    SPI_receiver_state = 0;
    config_finished_flag = 0;
    Resend_data_flag = 1;
    Resend_request_flag = 0;

    while (!config_finished_flag)
    {
        //not responding error
        if (BMS_DATA_RDY)
        {
            timer_100ms_cfg = 0;
            CTRLR_BSY;
            if (Resend_data_flag)
            {
                Resend_data_flag = 0;
                MspConfig(send_packet_code & 0x3, DEFAULT, Controller.op_mode_main_set);
                send_packet_code++;
            }
            else if (Resend_request_flag)
            {
                //for now these direct to re-send
                //this should be change to re-send previous msg
                Resend_request_flag = 0;
                Resend_data_flag = 1;
                //Resends request
                send_packet_code++;
            }
            else
            {
                SPI_Packet_rvd_f = 0;
                SPI_rec_error = 0;
                SPI_packet_receive();
                if (SPI_Packet_rvd_f)
                {
                    timer_100ms_cfg = 0;
                    //CRC check
                    CRC_val_c = crc_1021(SPI_rec_packed_length, received_data_buffer);
                    SPI_Packet_rvd_f = 0;
                    if (CRC_val_c == 0)
                    {
                        if (received_data_buffer[4] == REQUEST)
                        {
                            //configured successfully and received the cell data correctly
                            config_finished_flag = 1;
                        }
                        else if (received_data_buffer[4] == RESEND)
                        {
                            //ready to send configuration again
                            CTRLR_RDY;
                            Resend_data_flag = 1;
                            SPI_rec_error = 0;
                        }
                        else
                        {
                            SPI_rec_error = 1;
                        }
                    }
                    else
                    {
                        SPI_rec_error = 1;
                    }
                }
                if (SPI_rec_error)
                {
                    timer_100ms_cfg = 0;
                    Resend_request_flag = 1;
                    CTRLR_RDY;
                }
            }
        }
        else
        {

        }
    }
    seperateBMSdata(SPI_rec_packed_length, received_data_buffer); //analyze received data
    Resend_data_flag = 1;
    Resend_request_flag = 0;
    request_packet_type = ALL_DATA;
    config_finished_flag = 0;
    CTRLR_RDY;
}

void hv_batterry_read(Uint8 *C_ic)
{
    Uint8 local_IC_count;
    if (BMS_DATA_RDY)
    {
        timer_100ms = 0;
        CTRLR_BSY;
        if (Resend_data_flag)
        {
            Resend_data_flag = 0;
            switch (request_packet_type)
            {
                case RECORDED:
                    break;
                case OPEN_WIRE:
                    Request_openWire(0, Controller.op_mode_main_set);
                    openWire_request_over_flag = 1;
                    send_packet_code++;
                    request_packet_type = CLASSIFIED;
                    break;
                case ALL_DATA:
                    Request_all_data((*C_ic), 0, Controller.op_mode_main_set);
                    (*C_ic)++;
                    if ((*C_ic) >= TOTAL_IC)
                    {
                        humidity_temperature_count++;
                        all_data_requst_over_flag = 1;
                        (*C_ic) = 0;
                        if(humidity_temperature_count >= 10 && slave_read.bit.sht30_read_enable_flag)
                        {
                            humidity_temperature_count = 0;
                            request_packet_type = HUMIDITY_TEMPERATURE;
                        }
                        else if(slave_read.bit.open_wire_read_enable_flag)
                        {
                            request_packet_type = OPEN_WIRE;
                        }
                        else
                        {
                            request_packet_type = CLASSIFIED;
                        }
                    }
                    send_packet_code++;
                    break;
                case HUMIDITY_TEMPERATURE:
                    Request_humidity(0, Controller.op_mode_main_set);
                    humidity_temperature_request_over_flag = 1;
                    send_packet_code++;
                    request_packet_type = CLASSIFIED;
                    break;
                default:
                    Request_classified(0, Controller.op_mode_main_set);
                    send_packet_code++;
                    classified_all_count++;
                    if (classified_all_count >= DATA_LOOP_COUNT_n)
                    {
                        classified_all_count = 0;
                        request_packet_type = ALL_DATA;
                    }

                    break;
            }
        }
        else if (Resend_request_flag)
        {
            Resend_request_flag = 0;
            //Resends request
            send_packet_code++;
        }
        else
        {
            SPI_Packet_rvd_f = 0;
            SPI_rec_error = 0;
            SPI_packet_receive();
            if (SPI_Packet_rvd_f)
            {
                timer_100ms = 0;
                //CRC check
                CRC_val_c = crc_1021(SPI_rec_packed_length, received_data_buffer);
                communication_error_msp += CRC_val_c;
                SPI_Packet_rvd_f = 0;
                if (CRC_val_c == 0)
                {
                    communication_error_msp = 0;
                    hv_loop_count_bms++;
                    if (received_data_buffer[4] == REQUEST)
                    {
                        //configured successfully and received the cell data correctly
                        //LED4_TGL;
                        seperateBMSdata(SPI_rec_packed_length, received_data_buffer);
                        BMS_data_sep_flag = 1;
                        if (all_data_requst_over_flag == 1)
                        {
                            analyze_rdata();
                            received_packet_counter_0++;
                            all_data_requst_over_flag = 0;
                        }
                        else if (humidity_temperature_request_over_flag == 1)
                        {
                            for(local_IC_count = 0; local_IC_count < TOTAL_IC; local_IC_count++)
                            {
                                if(BMS_data_IC[local_IC_count].humi_crc_flag == 1 &&
                                        BMS_data_IC[local_IC_count].temp_crc_flag == 1)
                                {
                                    analyse_humidity_temp(local_IC_count);
                                }
                                else
                                {
                                    SHT30_error = data_read_error;
                                }
                            }
                            PDU_getData_local.sht30_error = SHT30_error;
                            humidity_temperature_request_over_flag = 0;
                        }
                        else if (openWire_request_over_flag == 1)
                        {
                            for(local_IC_count = 0; local_IC_count < TOTAL_IC; local_IC_count++)
                            {
                                if((BMS_data_IC[local_IC_count].open_wire_lsb | BMS_data_IC[local_IC_count].open_wire_msb<<8) == 0xFFFF)
                                {
                                    Controller.open_wire_ditect_flag = 1;
                                }
                                else
                                {
                                    Controller.open_wire_ditect_flag = 0;
                                }
                            }
                            openWire_request_over_flag = 0;
                        }
                        Resend_data_flag = 1;
                        CTRLR_RDY;
                    }
                    else if (received_data_buffer[4] == RESEND)
                    {
                        //ready to send configuration again
                        CTRLR_RDY;
                        Resend_data_flag = 1;
                        SPI_rec_error = 0;
                    }
                    else
                    {
                        SPI_rec_error = 1;
                    }
                }
                else
                {
                    SPI_rec_error = 1;

                }
            }
            if (SPI_rec_error)
            {
                Resend_request_flag = 1;
                CTRLR_RDY;
            }
            else
            {
            }
        }
    }
    else
    {
    }
}

/*
 * Function name    : time_rtc
 * Description      : set time to actual time
 */
Uint16 Status = 0x0000;
void time_rtc()
{
    if (!set_time_config_flag)
    {
        set_time_config_flag = 1;
        reg[0] = SECONDS_REG;
        InitDateTime(&set_time, tx_data_buff);
        Status = I2CA_WriteToManyRegs(I2C_SLAVE_RTC_ADDR, reg, 1, tx_data_buff, 7);
    }
    else
    {
        if (Status == 0)
        {
            reg[0] = 0x1B;
            Status = I2CA_ReadFromManyRegs(I2C_SLAVE_RTC_ADDR, &reg[0], 1, rx_data_buff, 11);
            decord_time(&real_time, rx_data_buff);
        }
    }
}

/*
 * ADC config
 */
Uint16 Status_adc = 0x0000;
int16 raw_adc_xxx;
double adc_read_asd(Uint16 adc_addr)
{
    int16 raw_adc;
    Uint8 reg_adc;
    Uint8 dat_adc[2];
    double current_value_local = 0;
// Wait for the conversion to complete

    reg_adc = 0x8C;
    // Read the conversion results
    Status_adc = I2CA_ReadFromManyRegs(adc_addr, &reg_adc, 1, dat_adc, 3);
//    I2CA_WriteToManyRegs(0x48, reg_adc, 0, dat_adc, 0);
//    DELAY_US(20000);
    raw_adc = (dat_adc[1] & 0xFF) | (dat_adc[0] << 8);
    raw_adc_xxx = raw_adc;
    /*HAS-50s*/
//    current_value_local = 0.01224297936879 * raw_adc;
    /*Tamura-100s*/
    current_value_local = 0.024246034847899 * raw_adc + 0.558928990180693;
    return current_value_local;
}

// Compute the MODBUS RTU CRC
Uint16 ModRTU_CRC(volatile Uint16 buf[], int len)
{
    Uint16 crc = 0xFFFF;
    int pos, i;

    for (pos = 0; pos < len; pos++)
    {
        crc ^= (Uint16) buf[pos];        // XOR byte into least sig. byte of crc

        for (i = 8; i != 0; i--)
        {    // Loop over each bit
            if ((crc & 0x0001) != 0)
            {      // If the LSB is set
                crc >>= 1;                    // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else
                // Else LSB is not set
                crc >>= 1;                    // Just shift right
        }
    }
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}

Uint16 con_fb_try_count_fire = 0;

void contactor_operator_fire()
{
    if (Controller.highest_cell_volt > HIGHEST_CELL_VOLTAGE_LIMIT)
    {
        if (current_val_A < 0)
        {
            bms_opMode = error;
            trip_cause = hi_v_error;
            PDU_getData_local.fixSetG.bit.cell_voltage_error = 1;
            ov_trip_delay_count = 0;
        }
        else
        {
            ov_trip_delay_count++;
        }
    }
    else
    {
        ov_trip_delay_count = 0;
        PDU_getData_local.fixSetG.bit.cell_voltage_error = 0;
    }

    if ((Controller.highest_temp > TEMPERATURE_CUTOFF) && (Controller.highest_temp < 100))
    {
        trip_cause = hi_t_error;
        bms_opMode = error;
    }

    if (Controller.lowest_cell_volt < LOWEST_CELL_VOLTAGE_CUTOFF)
    {
        if (((PDU_setData_local.fixSetS.bit.charger_connected == 0) && (lv_trip_delay_count > 6000))
                || (Controller.lowest_cell_volt < LOWEST_CELL_VOLTAGE_CUTOFF))
        {
            bms_opMode = error;
            PDU_getData_local.fixSetG.bit.cell_voltage_error = 1;
            trip_cause = l_v_error;
        }
        else if (PDU_setData_local.fixSetS.bit.charger_connected)
        {
            PDU_getData_local.fixSetG.bit.cell_voltage_error = 0;
        }
        else
        {
            lv_trip_delay_count++;
        }
    }
    else
    {
        lv_trip_delay_count = 0;
    }

    if (bms_opMode == error)
    {
        PRECHARGER_DIS;
        CON_DRIVER_DIS;
    }
    else if ((bms_opMode == emergency_event) || (!EMG_FB))
    {
        PRECHARGER_DIS;
        CON_DRIVER_DIS;
        bms_opMode = emergency_event;
        emg_delay++;
        if ((emg_delay > 100) && EMG_FB)
        {
            bms_opMode = not_initialized;
            emg_delay = 0;
        }
    }
    else if (bms_opMode == not_initialized)
    {
        bms_opMode = initialized;
        contactor_state = 0;
    }
    else if ((bms_opMode == initialized) && (PDU_setData_local.contactor_on == 1)
            && (PDU_setData_local.contactor_on_inverse == 2))
    {
        switch (contactor_state)
        {
            case 0:
//                if((Controller.lowest_cell_volt < 32000) && (Controller.SOC_value > 30.0))
//                {
//                    soc_reset = 21;
//                }
//                else if((Controller.lowest_cell_volt > 33000) && (Controller.SOC_value < 50.0))
//                {
//                    soc_reset = 61;
//                }
//                else if((Controller.lowest_cell_volt > 33330) && (Controller.SOC_value < 90.0))
//                {
//                    soc_reset = 96;
//                }
//                else
//                {
//
//                }
                PRECHARGER_EN;
                CON_DRIVER_DIS;
                precharge_timer = 0;
                contactor_state++;
                break;

            case 1:
                if (precharge_timer > PRECHARGE_DELAY)
                {
                    PRECHARGER_EN;
                    CON_DRIVER_EN;
                    contactor_state++;
                    precharge_timer = 0;
                }
                else
                {
                    precharge_timer++;
                }
                break;

            case 2:
                CON_DRIVER_EN;
                bms_opMode = contactor_closed;
                contactor_state++;
                break;

            case 3:
                CON_DRIVER_EN;
                PRECHARGER_DIS;
                if(!CON_FB)
                {
                    if(con_fb_try_count_fire > 5)
                    {
                        PDU_getData_local.fixSetG.bit.contactor_error = 1;
                        bms_opMode = error;
                    }
                    else
                    {
                        con_fb_try_count_fire++;
                    }
                }
                else
                {
                    bms_opMode = contactor_closed;
                    con_fb_try_count_fire = 0;
                }
                break;

            default:
                contactor_state = 0;
                break;
        }
    }
}

int16 prechargerFeedback(void)
{
    // Sample the optocoupler output
    AdcRegs.ADCSOCFRC1.bit.SOC14 = 1;
    while (AdcRegs.ADCINTFLG.bit.ADCINT2 == 0)
    {
    } //Wait for ADCINT2
    AdcRegs.ADCINTFLGCLR.bit.ADCINT2 = 1; //Clear ADCINT2
    return AdcResult.ADCRESULT14;
    //return (AdcResult.ADCRESULT14 * 3.3)/4096;
}

int16 Mcu_Temp(void)
{
    // Sample the temperature sensor
    AdcRegs.ADCSOCFRC1.bit.SOC5 = 1; //Sample temp sensor
    while (AdcRegs.ADCINTFLG.bit.ADCINT1 == 0)
    {
    } //Wait for ADCINT1
    AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //Clear ADCINT1
    Uint16 sensorSample = AdcResult.ADCRESULT5; //Get temp sensor sample result
    //Convert raw temperature sensor output to a temperature (degC)
    int16 t_slope = getTempSlope();
    int16 t_offset = getTempOffset();
    int16 temp_value = (int16) ((sensorSample - t_offset) * (t_slope / 32767.0f));

    return temp_value;
}

uint16_t chiller_status = 0;
uint16_t pump_speed = 0;
/*chiller control*/
void chillerCtrl(uint16_t highestT)
{
    if ((highestT > 33) || chiller_test)
    {
#if defined(SCI_CHILLER)
        chller_operation();
#elif defined(CAN_CHILLER)
        chillerData_write(canMap.msg[CAN_OBJ_20].sendData);
        CAN_sendMessage(CAN_OBJ_20, NOMINAL_DLC, canMap.msg[CAN_OBJ_20].sendData);
#endif
        PDU_getData_local.fixSetChrg.bit.chiller_enable = 1;
        chiller_status = 1;
        if (!pump_speed)
        {
            pump_speed = 60;
            InitEPwmTimer(pump_speed);
        }
    }
    else if (highestT < 33)
    {
        PDU_getData_local.fixSetChrg.bit.chiller_enable = 0;
        chiller_status = 0;
        pump_speed = 0;
        InitEPwmTimer(pump_speed);
    }
    else if (chiller_status)
    {
#if defined(SCI_CHILLER)
        chller_operation();
#elif defined(CAN_CHILLER)
        chillerData_write(canMap.msg[CAN_OBJ_20].sendData);
        CAN_sendMessage(CAN_OBJ_20, NOMINAL_DLC, canMap.msg[CAN_OBJ_20].sendData);
#endif
        CAN_sendMessage(CAN_OBJ_20, NOMINAL_DLC, canMap.msg[CAN_OBJ_20].sendData);
        PDU_getData_local.fixSetChrg.bit.chiller_enable = 1;
        chiller_status = 1;
    }
}

Uint16 OTA_handshake_connect_reset()
 {
    Uint16 wordData[4];
    Uint16 byteData[4];
//    Uint16 canData[8];
    int i;
    Uint16 sum;
    Uint16 status = 0;

    CAN_readMessage(CAN_OBJ_3, canMap.msg[CAN_OBJ_3].readData);

    for(i = 0; i<4; i++){
        wordData[i] = 0x0000;
        byteData[i] = 0x0000;
    }

    for(i = 0; i<4; i++){
 // Fetch the LSB
        wordData[i] =  canMap.msg[CAN_OBJ_3].readData[2*i];   // LS byte

 // Fetch the MSB
        byteData[i] =  canMap.msg[CAN_OBJ_3].readData[2*i+1];  // MS byte

 // form the wordData from the MSB:LSB
        wordData[i] |= (byteData[i] << 8);
    }

    sum = 0x0000;

    for(i = 0; i<4; i++){
        sum = sum + wordData[i];
    }

    if (canMap.msg[CAN_OBJ_3].readData[0] == 0x01 && sum == 0x01){
        status = OTA_HANDSHAKE;
    }
    else if (canMap.msg[CAN_OBJ_3].readData[0] == 0x11 && sum == 0x11){
        status = OTA_RESET;
    }

    return status;
}

void send_OTA_ACK(Uint8 ack_state) {

    // Configure mailbox 4 for transmission
    canMap.msg[CAN_OBJ_4].sendData[0] = 0xFF;

    switch(ack_state) {
        case INIT_ACK:
            canMap.msg[CAN_OBJ_4].sendData[1] = 0;
            break;
        case START_ACK:
            canMap.msg[CAN_OBJ_4].sendData[1] = 0x23;
            break;
    }

    canMap.msg[CAN_OBJ_4].sendData[2] = 0;
    canMap.msg[CAN_OBJ_4].sendData[3] = 0;
    canMap.msg[CAN_OBJ_4].sendData[4] = 0;
    canMap.msg[CAN_OBJ_4].sendData[5] = 0;
    canMap.msg[CAN_OBJ_4].sendData[6] = 0;
    canMap.msg[CAN_OBJ_4].sendData[7] = 0;

    CAN_sendMessage(CAN_OBJ_4, NOMINAL_DLC, canMap.msg[CAN_OBJ_4].sendData);
    CAN_transmission_acknowledge(CAN_OBJ_4);

//    EALLOW; // Enable write access to protected registers

    // Load shadow registers
//    ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
//    ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;

    // Set transmission request using shadow register
//    ECanaShadow.CANTRS.bit.TRS4 = 1;
//    ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all; // Write back to main register

//    // Wait for transmission to complete using shadow register
//    do {
//        ECanaShadow.CANTA.all = ECanaRegs.CANTA.all;
//    } while (ECanaShadow.CANTA.bit.TA4 != 1);
//
//    // Clear the transmission flag using shadow register
//    ECanaShadow.CANTA.bit.TA4 = 1;
//    ECanaRegs.CANTA.all = ECanaShadow.CANTA.all; // Write back to main register

//    EDIS; // Disable write access to protected registers
}


void move_to_kernal(Uint8 kernal_state)
{
    count = 111;
    Uint16 status;
    Uint16 flag_buff[FLASH_FLAG_LENGTH];

    if(kernal_state == BOOTLOADER_STATE)
    {
        flag_buff[FLASH_FLAG] = BOOTLOADER;
        flag_buff[JUMP_FLAG] = NULL_STATE;
        flag_buff[BACKUP_FLAG] = read_flag(BACKUP_FLAG);
        flag_buff[PACKET_FLAG] = read_flag(PACKET_FLAG);
    }
    else if(kernal_state == BACKUP_STATE)
    {
        InitFlash();
        DisableDog();
        XtalOscSel();
        InitPll(9,2);
        flag_buff[FLASH_FLAG] = BACKUP;
        flag_buff[JUMP_FLAG] = read_flag(JUMP_FLAG);
        flag_buff[BACKUP_FLAG] = read_flag(BACKUP_FLAG);
        flag_buff[PACKET_FLAG] = read_flag(PACKET_FLAG);
    }

    fail = 0;
    DINT; // Disable CPU interrupts
    InitPieCtrl(); //PIE control registers to their default state
    //
    // Disable CPU interrupts and clear all CPU interrupt flags
    //
    IER = 0x0000;
    IFR = 0x0000;

    CsmUnlock();

    EALLOW;
    Flash_CPUScaleFactor = SCALE_FACTOR;
    Flash_CallbackPtr = NULL;
    EDIS;

    status = Flash_Erase(SECTORE,
                         &FlashStatus);
    if(status != STATUS_SUCCESS)
    {
        fail++;
        resetBMS();
        return;
    }

    status = Flash_Program((Uint16 *) FLASH_FLAG_ADDRESS,
             (Uint16 *)flag_buff, FLASH_FLAG_LENGTH, &FlashStatus);
    count = 112;

    if(status != STATUS_SUCCESS)
    {
        fail++;
        resetBMS();
        return;
    }
    count = 113;

    if(kernal_state == BOOTLOADER_STATE)
    {
        send_OTA_ACK(START_ACK);
    }

//    DELAY_US(5000000);
    resetBMS();

    for(;;);

//    ((void(*)())KERNAL_ADDRESS)();


}

void clear_jump_flag()
{
    InitFlash();

    Uint16 status;
    Uint16 flag_buff[FLASH_FLAG_LENGTH];
    fail = 0;

    DINT; // Disable CPU interrupts
    InitPieCtrl(); //PIE control registers to their default state
    //
    // Disable CPU interrupts and clear all CPU interrupt flags
    //
    IER = 0x0000;
    IFR = 0x0000;

    DisableDog();
    XtalOscSel();
    InitPll(9,2);

    CsmUnlock();

    EALLOW;
    Flash_CPUScaleFactor = SCALE_FACTOR;
    Flash_CallbackPtr = NULL;
    EDIS;

    flag_buff[FLASH_FLAG] = APPLICATION;
    flag_buff[JUMP_FLAG] = 0x0001;
    flag_buff[BACKUP_FLAG] = NULL_STATE;
    flag_buff[PACKET_FLAG] = NULL_STATE;

    status = Flash_Erase(SECTORE,
                         &FlashStatus);
    if(status != STATUS_SUCCESS)
    {
        fail++;
        resetBMS();
        return;
    }

    status = Flash_Program((Uint16 *) FLASH_FLAG_ADDRESS,
             (Uint16 *)flag_buff, FLASH_FLAG_LENGTH, &FlashStatus);
    if(status != STATUS_SUCCESS)
    {
        fail++;
        resetBMS();
        return;
    }
}

Uint16 read_flag(Uint8 flag_status)
{
    Uint16 flag_val;
    if(flag_status == FLASH_FLAG)
    {
        flag_val = *(Uint16 *)FLASH_FLAG_ADDRESS;
    }
    else if(flag_status == JUMP_FLAG)
    {
        flag_val = *(Uint16 *)JUMP_FLAG_ADDRESS;
    }
    else if(flag_status == BACKUP_FLAG)
    {
        flag_val = *(Uint16 *)BACKUP_FLAG_ADDRESS;
    }
    else if(flag_status == PACKET_FLAG)
    {
        flag_val = *(Uint16 *)PACKET_FLAG_ADDRESS;
    }

    return flag_val;
}



//Uint16 get_jump_flag(void)
//{
//    Uint16 jump_flag;
//
//    jump_flag = *(Uint16 *)JUMP_FLAG_ADDRESS;
//
//    return jump_flag;
//}
//
//Uint16 read_OTA_flag(void)
//{
//    Uint16 ota_flag;
//
//    ota_flag = *(Uint16 *)FLASH_FLAG_ADDRESS;
//
//    return ota_flag;
//}
//
//Uint16 get_backup_flag(void){
//    Uint16 backup_flag;
//
//    backup_flag = *(Uint16 *)BACKUP_FLAG_ADDRESS;
//
//    return backup_flag;
//}

void resetBMS()
{
    EALLOW;  // Enable write access to protected registers
    SysCtrlRegs.WDCR = 0;  // Write an incorrect value to the WDCR register
    EDIS;    // Disable write access to protected registers
}

void update_BMS()
{
    Uint16 otacmd;
    otacmd = OTA_handshake_connect_reset();

    switch(otacmd)
    {
    case OTA_HANDSHAKE:
        send_OTA_ACK(INIT_ACK);
        break;
    case OTA_RESET:
        send_OTA_ACK(INIT_ACK);
        CLEAR_INTERUPT_FLAGS();
#ifdef CAN_BOOTLOADING
        move_to_kernal(BOOTLOADER_STATE);
#endif
        break;
    default:
        send_OTA_ACK(INIT_ACK);
        break;
    }
}
/*
 * EOF
 */
