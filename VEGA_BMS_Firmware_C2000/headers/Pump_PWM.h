#include "DSP28x_Project.h"

/*
 * Pump_PWM.h
 *
 *  Created on: May 20, 2024
 *      Author: Danushka Weerasinghe
 */

#ifndef PUMP_PWM_H_
#define PUMP_PWM_H_

//
// Defines that configure the start/end period for the timer
//
#define PWM3_TIMER_MIN     10
#define PWM3_TIMER_MAX     8000

//
// Defines to keep track of which way the timer value is moving
//
#define EPWM_TIMER_UP   1
#define EPWM_TIMER_DOWN 0

//

//
// Globals
//

//    Uint32  ECap1IntCount;
//    Uint32  ECap1PassCount;
//    Uint32  EPwm3TimerDirection;
//
//    Uint16 tbprd;
//    Uint16 desired_freq ; // Desired frequency in Hz
//    Uint16 duty_cycle_percent ; // Desired duty cycle in percentage


//
// Function Prototypes
//
__interrupt void ecap1_isr(void);
void InitECapture(void);
void InitEPwmTimer(Uint8 pumpStatus);



#endif /* PUMP_PWM_H_ */
