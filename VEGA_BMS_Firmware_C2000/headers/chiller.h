/**
 * @file    chiller.h
 * @brief   Chiller unit interface: CAN command 0x238, CAN feedback 0x248, SCI command, chiller fault.
 * @details chillerCtrl() (Battery_controller.c) sends the command with chillerData_write(),
 *          the eCAN ISR stores the feedback with chillerData_read(), and chiller_fault_update()
 *          detects chiller faults that stop cooling (COOL_CHILLER_FAULT).
 * @author  Danushka Weerasinghe
 * @date    2023-11-02
 */

#ifndef CHILLER_H_
#define CHILLER_H_

/* Includes ------------------------------------------------------------------*/
#include "DSP28x_Project.h"         /* Uint8, Uint16, Uint32 */
#include "main_data.h"              /* enum chiller_comp_state_enum, enum chiller_error_enum */

/* Public defines ------------------------------------------------------------*/
/* chiller_test command (0xDB byte 2) */
#define CHILLER_TEST_AUTO           0   /**< Normal: chiller controlled by pack temperature */
#define CHILLER_TEST_ON             1   /**< Force chiller + pump ON (any other value = auto) */

/* chiller_fault_code values. Sent on CAN_OBJ_9 (0x35) mux page 6 byte 2 (eprom_config.c),
   DBC signal EEP_ChillerFaultCode. A chiller fault stops cooling (relay, pump, chiller OFF),
   latched until BMS reset. 1..7 = reported by the chiller in 0x248 byte 4, 8..9 = found by the BMS */
#define CHILLER_FAULT_NONE          0   /**< No chiller fault */
#define CHILLER_FAULT_OVER_CURRENT  1   /**< Chiller reports over current */
#define CHILLER_FAULT_RUNNING_OV    2   /**< Chiller reports over voltage while running */
#define CHILLER_FAULT_RUNNING_UV    3   /**< Chiller reports under voltage while running */
#define CHILLER_FAULT_STANDBY_OV    4   /**< Chiller reports over voltage in standby */
#define CHILLER_FAULT_STANDBY_UV    5   /**< Chiller reports under voltage in standby */
#define CHILLER_FAULT_COMM_FAILURE  6   /**< Chiller reports lost 0x238 command */
#define CHILLER_FAULT_SPEED_ERROR   7   /**< Chiller reports compressor speed error */
#define CHILLER_FAULT_NO_REPLY      8   /**< Chiller ON but no 0x248 for 2 s */
#define CHILLER_FAULT_COMPRESSOR    9   /**< 0x248 compressor state = error, without an error code */

/* Public structs ------------------------------------------------------------*/
/**
 * @brief Chiller command data, sent in 0x238.
 */
typedef struct chillerCtrl_data_struct
{
    Uint8  comp_status;             /**< Byte 3: 1 = compressor ON */
    Uint16 comp_speed;              /**< Bytes 0-1: compressor speed, rpm */
    Uint16 comp_power_limit;        /**< Byte 2: power limit, 0.1 kW/bit */
} chiller_ctrl_data_t;

/**
 * @brief Chiller feedback data, received in 0x248.
 */
typedef struct chillerDiag_data_struct
{
    Uint16 comp_speed_actual;                       /**< Bytes 0-1: actual compressor speed, rpm */
    enum chiller_comp_state_enum chiller_comp_state;/**< Byte 3: off / on / compressor_error */
    enum chiller_error_enum chiller_error;          /**< Byte 4: error code, no_fault = 0 */
    Uint16 current;                                 /**< Byte 6: current, raw */
    Uint16 voltage;                                 /**< Byte 7 x 4: voltage, V */
    Uint32 rx_tick;                                 /**< CpuTimer0 tick of the last 0x248 frame */
    Uint16 rx_ok;                                   /**< 1 = 0x248 received since the chiller was switched ON */
} chiller_diag_data_t;

/* Extern declarations -------------------------------------------------------*/
/** @brief Last chiller command written by chillerData_write(). */
extern chiller_ctrl_data_t chillerCtrl_data;

/** @brief Last chiller feedback stored by chillerData_read() (eCAN ISR). */
extern chiller_diag_data_t chillerDiag_data;

/** @brief First confirmed chiller fault (CHILLER_FAULT_xxx), kept until BMS reset. */
extern Uint16 chiller_fault_code;

/** @brief 1 = chiller fault confirmed now: chillerCtrl() stops cooling (COOL_CHILLER_FAULT). */
extern Uint16 chiller_fault_now;

/* Public function prototypes ------------------------------------------------*/
/**
 * @brief   Send a frame over SCI-B to the chiller (SCI chiller builds).
 * @param[in] length_sci_rev  Number of bytes to send.
 * @note    Declared for the SCI chiller interface; not defined in chiller.c.
 */
void SCIb_SEND(Uint8 length_sci_rev);

/**
 * @brief   SCI chiller: send the compressor command frame once per second.
 * @note    Call every 50 ms while the chiller is ON (SCI_CHILLER builds). Blocking while the
 *          16 bytes are sent.
 */
void chller_operation(void);

/**
 * @brief   Fill the 0x238 chiller command bytes.
 * @param[out] chiller_data_write  8 data bytes of the frame, one byte per element.
 * @param[in]  enable              1 = compressor ON at 3400 rpm, 0.5 kW limit; 0 = all zero (OFF).
 */
void chillerData_write(Uint16 *chiller_data_write, Uint8 enable);

/**
 * @brief   Store a received 0x248 chiller feedback frame.
 * @param[in] chiller_data_read  8 data bytes of the frame, one byte per element.
 * @note    Called from the eCAN receive ISR.
 */
void chillerData_read(Uint16 *chiller_data_read);

/**
 * @brief   Detect chiller faults. Call on every chiller_cmd() pass.
 * @param[in] on  1 = chiller commanded ON.
 * @details While ON: no 0x248 for 2 s -> CHILLER_FAULT_NO_REPLY; error code in 0x248 for 1 s
 *          -> that code (1..7); compressor_error state for 1 s -> CHILLER_FAULT_COMPRESSOR.
 *          Sets chiller_fault_now and, for the first fault, chiller_fault_code.
 */
void chiller_fault_update(Uint16 on);

#endif /* CHILLER_H_ */
