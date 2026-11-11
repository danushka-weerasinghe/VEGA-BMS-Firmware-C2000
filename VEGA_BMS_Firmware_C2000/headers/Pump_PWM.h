/**
 * @file    Pump_PWM.h
 * @brief   Chiller coolant pump control: PWM pump (ePWM2 + eCAP2) or CAN pump (0x258 / 0x268).
 * @details The pump type is selected per vehicle in main_data.h (PWM_PUMP or CAN_PUMP).
 *          chillerCtrl() (Battery_controller.c) calls pump_cmd() every pass, uses pump_ok()
 *          before the chiller may run, and pump_fault_latch() to stop cooling on a pump fault.
 * @author  Danushka Weerasinghe
 * @date    2024-05-20
 */

#ifndef PUMP_PWM_H_
#define PUMP_PWM_H_

/* Includes ------------------------------------------------------------------*/
#include "DSP28x_Project.h"         /* Uint16, __interrupt */

/* Public defines ------------------------------------------------------------*/
/* pump_fault_code values. Sent on CAN_OBJ_9 (0x35) mux page 6 byte 1 (eprom_config.c),
   DBC signal EEP_PumpFaultCode. Latched: cleared only by BMS reset. */
#define PUMP_FAULT_NONE         0   /**< No pump fault */
#define PUMP_FAULT_DRY_RUN      1   /**< No water: PWM speed too high / CAN pump reports dry running */
#define PUMP_FAULT_NO_FEEDBACK  2   /**< Pump ON but no feedback: no speed pulse / 0x268 missing or not running */
#define PUMP_FAULT_PUMP_ERROR   3   /**< CAN pump reports over current / over voltage / under voltage / block */

/* Extern declarations -------------------------------------------------------*/
/** @brief Pump fault that latched cooling OFF (PUMP_FAULT_xxx), 0 until then. Cleared only by BMS reset. */
extern Uint16 pump_fault_code;

/* Public function prototypes ------------------------------------------------*/
/**
 * @brief   Set up eCAP2 on GPIO15 to measure the PWM pump speed feedback.
 * @note    Call once at start-up, after InitScibGpio() so the GPIO15 mux is not overwritten.
 *          The capture interrupt stays OFF until the pump is switched ON.
 */
void InitPumpFeedback(void);

/**
 * @brief   Store a received 0x268 CAN pump feedback frame.
 * @param[in] pump_data_read  8 data bytes of the frame, one byte per element.
 * @note    Called from the eCAN receive ISR. Frames are ignored while the pump is OFF.
 */
void pumpData_read(Uint16 *pump_data_read);

/**
 * @brief   Switch the pump ON / OFF and run the pump fault detection.
 * @param[in] on  1 = pump ON, 0 = pump OFF.
 * @note    Call on every chillerCtrl() pass.
 *          PWM pump: ePWM2 is reconfigured only when the duty changes.
 *          CAN pump: 0x258 is sent on every call, ON or OFF (the pump runs at full speed
 *          when it receives no command).
 */
void pump_cmd(Uint16 on);

/**
 * @brief   Check the pump feedback.
 * @retval  1  Pump running, no fault, feedback fresh.
 * @retval  0  Pump not confirmed running.
 */
Uint16 pump_ok(void);

/**
 * @brief   Check for a pump fault and record it.
 * @retval  1  A pump fault is present now: pump_fault_code and its counter are set
 *             (priority: dry run, pump error, no feedback).
 * @retval  0  No pump fault.
 */
Uint16 pump_fault_latch(void);

/**
 * @brief   eCAP2 capture interrupt: one rising edge of the PWM pump speed feedback.
 * @note    Installed in the PIE vector table by InitPumpFeedback(). Never call from code.
 */
__interrupt void ecap2_isr(void);

#endif /* PUMP_PWM_H_ */
