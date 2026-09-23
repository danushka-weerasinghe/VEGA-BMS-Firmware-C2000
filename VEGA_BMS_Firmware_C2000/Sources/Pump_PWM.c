#include "DSP28x_Project.h"
#include "Pump_PWM.h" /*
 * Pump_PWM.c
 *
 *  Created on: May 20, 2024
 *      Author: Danushka Weerasinghe
 */


//    Uint32  ECap1IntCount;
//    Uint32  ECap1PassCount;
    Uint32  EPwm2TimerDirection;

    Uint16 tbprd;
    Uint16 desired_freq = 1000; // Desired frequency in Hz
    Uint16 duty_cycle_percent =20 ; // Desired duty cycle in percentage
    Uint32 PWM_Period , PWM_Duty;

    Uint32 f_PWM ,Duty_Cycle ;

//
// InitEPwmTimer -
//
void
InitEPwmTimer(Uint8 pumpSpeed)
{


        EALLOW; // Allow protected register writes
        SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC =0 ; // Disable time-base clock
        EDIS; // Disallow protected register writes


        // Calculate period register (TBPRD) based on desired frequency
        tbprd = (Uint16)((90000000 / (desired_freq * 2)) - 1);

        // Time-Base Control Register (TBCTL)
        EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP; // Count up mode
        EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE; // Disable phase loading
        EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW; // Load the period register from the shadow register
        EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; // Sync input selected
        EPwm2Regs.TBCTL.bit.HSPCLKDIV = 1; // High-speed time-base clock prescaler
        EPwm2Regs.TBCTL.bit.CLKDIV = 0; // Time-base clock prescaler

        // Set the period for the PWM signal
        // Time-Base Period Register (TBPRD)
        EPwm2Regs.TBPRD = tbprd; // Set timer period

        // Time-Base Phase Register (TBPHS)
        EPwm2Regs.TBPHS.all = 0x00000000; // Set phase to zero

        // Time-Base Counter Register (TBCTR)
        EPwm2Regs.TBCTR = 0; // Clear counter

        // Counter-Compare Control Register (CMPCTL)
        EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // Shadow mode for CMPA
        EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW; // Shadow mode for CMPB
        EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // Load on CTR=0
        EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO; // Load on CTR=0

        // Calculate CMPA value for the desired duty cycle
        EPwm2Regs.CMPA.half.CMPA = (Uint16)(((100-pumpSpeed)/100.0 ) * tbprd);

        // Action Qualifier Control Register for Output A (AQCTLA)
       EPwm2Regs.AQCTLA.all = 0x0000; // Clear all action qualifier bits
       EPwm2Regs.AQCTLA.bit.CAU = AQ_SET; // Set output when counter equals CMPA up
       EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR; // Clear output when counter equals CMPA down
       EPwm2Regs.AQCTLA.bit.PRD = AQ_CLEAR; // Clear output when counter equals PRD


            // Force the PWM output low initially
//            EPwm2Regs.AQSFRC.bit.RLDCSF = AQSFRC_IMMEDIATE; // Load on immediate
       EPwm2Regs.AQSFRC.bit.OTSFA = 1; // One-time software force action A
       EPwm2Regs.AQSFRC.bit.ACTSFA = AQ_CLEAR; // Force PWM output low


       EPwm2TimerDirection = EPWM_TIMER_UP;

        // Enable TBCLK within the ePWM module
       EALLOW;
       SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
       EDIS;
}

//
// InitECapture -
//
void
InitECapture()
{
    ECap1Regs.ECEINT.all = 0x0000;          // Disable all capture interrupts
    ECap1Regs.ECCLR.all = 0xFFFF;           // Clear all CAP interrupt flags
    ECap1Regs.ECCTL1.bit.CAPLDEN = 0;       // Disable CAP1-CAP4 register loads
    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 0;     // Make sure the counter is stopped

    //
    // Configure peripheral registers
    //
    ECap1Regs.ECCTL2.bit.CONT_ONESHT = 1;   // One-shot
    ECap1Regs.ECCTL2.bit.STOP_WRAP = 3;     // Stop at 4 events
    ECap1Regs.ECCTL1.bit.CAP1POL = 1;       // Falling edge
    ECap1Regs.ECCTL1.bit.CAP2POL = 0;       // Rising edge
    ECap1Regs.ECCTL1.bit.CAP3POL = 1;       // Falling edge
    ECap1Regs.ECCTL1.bit.CAP4POL = 0;       // Rising edge
    ECap1Regs.ECCTL1.bit.CTRRST1 = 1;       // Difference operation
    ECap1Regs.ECCTL1.bit.CTRRST2 = 1;       // Difference operation
    ECap1Regs.ECCTL1.bit.CTRRST3 = 1;       // Difference operation
    ECap1Regs.ECCTL1.bit.CTRRST4 = 1;       // Difference operation
    ECap1Regs.ECCTL2.bit.SYNCI_EN = 1;      // Enable sync in
    ECap1Regs.ECCTL2.bit.SYNCO_SEL = 0;     // Pass through
    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable capture units

    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 1;     // Start Counter
    ECap1Regs.ECCTL2.bit.REARM = 1;         // arm one-shot
    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable CAP1-CAP4 register loads
    ECap1Regs.ECEINT.bit.CEVT4 = 1;         // 4 events = interrupt
}

//
// ecap1_isr -
//
__interrupt void
ecap1_isr(void)
{

    // Read captured values
        PWM_Period = ECap1Regs.CAP1 + ECap1Regs.CAP2;
        PWM_Duty = ECap1Regs.CAP1 ;

       // Calculate frequency and duty cycle
        f_PWM = (1.0*90000000) / PWM_Period; // 9000000 equl to sysclk


        Duty_Cycle =  (float)PWM_Duty / (float)PWM_Period * 100.0;
       // Clear interrupt flag



    ECap1Regs.ECCLR.bit.CEVT4 = 1;
    ECap1Regs.ECCLR.bit.INT = 1;
    ECap1Regs.ECCTL2.bit.REARM = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 4
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;
}

struct pumpTx pump_cntrl_cmd;

void pumpData_write(Uint16* pump_data_write, enum pump_state pump_status)
{
    pump_cntrl_cmd.pump_power = pump_status;
    pump_cntrl_cmd.target_duty.all = 150;//originally 1000

    pump_data_write[0] = pump_cntrl_cmd.target_duty.byte.DUTY_BYTE1;
    pump_data_write[1] = pump_cntrl_cmd.target_duty.byte.DUTY_BYTE2;
    pump_data_write[2] = pump_cntrl_cmd.pump_power;
    pump_data_write[3] = 0;
    pump_data_write[4] = 0;
    pump_data_write[5] = 0;
    pump_data_write[6] = 0;
    pump_data_write[7] = 0;
}

struct pumpRx pump_debug_cmd;

void pumpData_read(Uint16* pump_data_read)
{
    pump_debug_cmd.target_speed_fb = pump_data_read[0];
    pump_debug_cmd.target_speed_fb |= pump_data_read[1] << 8;
    pump_debug_cmd.pump_power_fb = pump_data_read[2];
    pump_debug_cmd.pump_temp = pump_data_read[3];
    pump_debug_cmd.bus_v = pump_data_read[4];
    pump_debug_cmd.pump_current = pump_data_read[5];
    pump_debug_cmd.pump_error_code.all = pump_data_read[6];
}
