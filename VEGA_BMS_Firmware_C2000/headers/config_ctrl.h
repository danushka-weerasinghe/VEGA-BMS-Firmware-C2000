/*#############################################################
 *
 * RS485
 *   GPIO9  - RS485 drive enable
 *   GPIO29 - RS485 driver input
 *   GPIO28 - RS485 receiver output
 *
 * MSP 1
 *   GPIO22 - Reset
 *   GPIO16 - SIMO
 *   GPIO17 - SOMI
 *   GPIO18 - CLK
 *   GPIO19 - SPI-CS
 *   GPIO4  - DIGITAL Com_out
 *   GPIO5  - DIGITAL Com_in
 *   GPIO43 - TX
 *
 * LED
 *   GPIO13 - LED1
 *   GPIO14 - LED2
 *   GPIO21 - LED3
 *   GPIO20 - LED4
 *
 * MOSFET drivers
 *   GPIO50 - K1 - precharge_enable
 *   GPIO25 - K2 - precharge_drive
 *   GPIO12 - K3 - contactor_drive
 *   GPIO51 - K4 - contactor_enable
 *
 * I2C
 *   GPIO32 - SDA_I2C
 *   GPIO33 - SCL_I2C
 *
 *   GPIO8  - RTC interrupt
 *   GPIO7  - RTC Time stamp
 *
 * CAN
 *   GPIO30 - CAN RX
 *   GPIO31 - CAN TX
 *
 *#############################################################*/

#ifndef config_ctrl
#define config_ctrl

#include "DSP28x_Project.h"

#define BMS_DATA_RDY GpioDataRegs.GPADAT.bit.GPIO5
#define CTRLR_RDY GpioDataRegs.GPASET.bit.GPIO4 = 1
#define CTRLR_BSY GpioDataRegs.GPACLEAR.bit.GPIO4 = 1

#define LED1_ON GpioDataRegs.GPASET.bit.GPIO13 = 1 //LED
#define LED2_ON GpioDataRegs.GPASET.bit.GPIO14 = 1 //LED
#define LED3_ON GpioDataRegs.GPASET.bit.GPIO20 = 1 //LED
#define LED4_ON GpioDataRegs.GPASET.bit.GPIO21 = 1 //LED

#define LED1_OFF GpioDataRegs.GPACLEAR.bit.GPIO13 = 1 //LED
#define LED2_OFF GpioDataRegs.GPACLEAR.bit.GPIO14 = 1 //LED
#define LED3_OFF GpioDataRegs.GPACLEAR.bit.GPIO20 = 1 //LED
#define LED4_OFF GpioDataRegs.GPACLEAR.bit.GPIO21 = 1 //LED

#define LED1_TGL GpioDataRegs.GPATOGGLE.bit.GPIO13 = 1 //LED
#define LED2_TGL GpioDataRegs.GPATOGGLE.bit.GPIO14 = 1 //LED
#define LED3_TGL GpioDataRegs.GPATOGGLE.bit.GPIO20 = 1 //LED
#define LED4_TGL GpioDataRegs.GPATOGGLE.bit.GPIO21 = 1 //LED

#define CHILLER_STATUS GpioDataRegs.GPADAT.bit.GPIO21

//#define RS485_send GpioDataRegs.GPBSET.bit.GPIO42 = 1
//#define RS485_receive GpioDataRegs.GPBCLEAR.bit.GPIO42 = 1

#define EMG_FB GpioDataRegs.GPADAT.bit.GPIO9
#define CON_FB GpioDataRegs.GPADAT.bit.GPIO12
#define FC_CON_FB GpioDataRegs.GPBDAT.bit.GPIO44

#define PRECHARGER_EN GpioDataRegs.GPBSET.bit.GPIO50 = 1
#define CON_DRIVER_EN GpioDataRegs.GPBSET.bit.GPIO51 = 1
#define FC_CON_DRIVER_EN GpioDataRegs.GPASET.bit.GPIO6 = 1

#define PRECHARGER_DIS GpioDataRegs.GPBCLEAR.bit.GPIO50 = 1
#define CON_DRIVER_DIS GpioDataRegs.GPBCLEAR.bit.GPIO51 = 1
#define FC_CON_DRIVER_DIS GpioDataRegs.GPACLEAR.bit.GPIO6 = 1

//#define EMG_INPUT GpioDataRegs.GPBDAT.bit.GPIO41

unsigned char cell_balancing_enable = 0;

extern void CONFIG_Gpio(void);
extern void ADC_CONFIG(void);

void CONFIG_Gpio()
{
    EALLOW;
    //------------------------------------------------------------//
    //1) RS485 Enable
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 0; //configure as INPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO9 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO9 = 0; //Drive to 0
    //------------------------------------------------------------//
    //2) MSP Reset
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO22 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO22 = 0; //Drive to 0

    //3) MSP430-1 digital input
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 0; //configure as INPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO5 = 0; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO5 = 0; //Drive to 0

    //4) MSP430-1 digital output
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO4 = 0; //Drive to 0
    //------------------------------------------------------------//
    //5) LED1
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO13 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO13 = 0; //Drive to 0

    //6) LED2
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO14 = 0; //Drive to 0

    //7) LED3
    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO20 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO20 = 0; //Drive to 0

    //8) LED4
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO21 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO21 = 0; //Drive to 0
    //------------------------------------------------------------//
    //9) K1 - precharge_enable
    GpioCtrlRegs.GPBMUX2.bit.GPIO50 = 0; //configure as GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPBPUD.bit.GPIO50 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPBDAT.bit.GPIO50 = 0; //Drive to 0

    //10) K2 - precharge_drive
    GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO25 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO25 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO25 = 0; //Drive to 0

    //11) K3 - contactor_drive
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO12 = 0; //configure as INPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO12 = 0; //Drive to 0

    //12) K4 - contactor_enable
    GpioCtrlRegs.GPBMUX2.bit.GPIO51 = 0; //configure as GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO51 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPBPUD.bit.GPIO51 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPBDAT.bit.GPIO51 = 0; //Drive to 0

    //13) K5 - FC_contactor_drive
    GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0; /*configure as GPIO*/
    GpioCtrlRegs.GPBDIR.bit.GPIO44 = 0; /*configure as INPUT*/
    GpioCtrlRegs.GPBPUD.bit.GPIO44 = 1; /*Pull-up resistor disabled*/
    GpioDataRegs.GPBDAT.bit.GPIO44 = 0; /*Drive to 0*/

    //14) K6 - FC_contactor_enable
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0; /*configure as GPIO*/
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1; /*configure as OUTPUT*/
    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1; /*Pull-up resistor disabled*/
    GpioDataRegs.GPADAT.bit.GPIO6 = 0; /*Drive to 0*/
    //------------------------------------------------------------//
    //15) RTC interrupt
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO8 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO8 = 0; //Drive to 0

    //16) RTC Time stamp
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0; //configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPAPUD.bit.GPIO7 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPADAT.bit.GPIO7 = 0; //Drive to 0
    //------------------------------------------------------------//

    //17) RS485 Transceiver send/receive
    GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 0; //configure as GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO42 = 1; //configure as OUTPUT
    GpioCtrlRegs.GPBPUD.bit.GPIO42 = 1; //Pull-up resistor disabled
    GpioDataRegs.GPBDAT.bit.GPIO42 = 0; //Drive to 0



//    //31) GPIO1 used as can-b interrupt
////    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;        // GPIO
////    GpioCtrlRegs.GPADIR.bit.GPIO1 = 0;         // input
//    GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL = 1;   // XINT2 is GPIO1
//    GpioCtrlRegs.GPAQSEL1.bit.GPIO1 = 0;        // XINT1 Synch to SYSCLKOUT only
//    // Configure XINT1 and XINT2
//    XIntruptRegs.XINT1CR.bit.POLARITY = 0;      // falling edge interrupt
//    // Enable XINT1 and XINT2
//    XIntruptRegs.XINT1CR.bit.ENABLE = 1;        // Enable XINT1
//
//    EDIS;
//    // Configure XINT1 and XINT2
//    XIntruptRegs.XINT1CR.bit.POLARITY = 0;      // falling edge interrupt
//    //XIntruptRegs.XINT2CR.bit.POLARITY = 0;    // falling edge interrupt
//    // Enable XINT1 and XINT2
//    XIntruptRegs.XINT1CR.bit.ENABLE = 1;        // Enable XINT1
//    //XIntruptRegs.XINT2CR.bit.ENABLE = 1;      // Enable XINT2
}

void ADC_CONFIG()
{
    EALLOW;
    AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = 1;  // Enable non-overlap mode
    AdcRegs.ADCCTL1.bit.INTPULSEPOS = 1; // ADCINT1 trips after AdcResults latch

//  AdcRegs.INTSEL1N2.bit.INT1E = 1;    // Enabled ADCINT1
//  AdcRegs.INTSEL1N2.bit.INT1CONT = 0; // Disable ADCINT1 Continuous mode
//  AdcRegs.INTSEL1N2.bit.INT1SEL = 1;  // setup EOC1 to trigger ADCINT1 to fire

    AdcRegs.ADCCTL1.bit.TEMPCONV = 1; //Connect A5 - temp sensor

    AdcRegs.ADCSOC0CTL.bit.CHSEL = 0;    // set SOC0 channel select to ADCINA0
    AdcRegs.ADCSOC1CTL.bit.CHSEL = 1;    // set SOC1 channel select to ADCINA1
    AdcRegs.ADCSOC2CTL.bit.CHSEL = 2;    // set SOC2 channel select to ADCINA2
    AdcRegs.ADCSOC3CTL.bit.CHSEL = 3;    // set SOC3 channel select to ADCINA3
    AdcRegs.ADCSOC4CTL.bit.CHSEL = 4;    // set SOC4 channel select to ADCINA4
    AdcRegs.ADCSOC5CTL.bit.CHSEL = 5;    // set SOC5 channel select to ADCINA5
    AdcRegs.ADCSOC6CTL.bit.CHSEL = 6;    // set SOC6 channel select to ADCINA6
    AdcRegs.ADCSOC7CTL.bit.CHSEL = 7;    // set SOC7 channel select to ADCINA7

    AdcRegs.ADCSOC8CTL.bit.CHSEL = 8;    // set SOC8 channel select to ADCINB0
    AdcRegs.ADCSOC9CTL.bit.CHSEL = 9;    // set SOC9 channel select to ADCINB1
    AdcRegs.ADCSOC10CTL.bit.CHSEL = 10;    // set SOC10 channel select to ADCINB2
    AdcRegs.ADCSOC11CTL.bit.CHSEL = 11;    // set SOC11 channel select to ADCINB3
    AdcRegs.ADCSOC12CTL.bit.CHSEL = 12;    // set SOC12 channel select to ADCINB4
    AdcRegs.ADCSOC13CTL.bit.CHSEL = 13;    // set SOC13 channel select to ADCINB5
    AdcRegs.ADCSOC14CTL.bit.CHSEL = 14;    // set SOC14 channel select to ADCINB6
    AdcRegs.ADCSOC15CTL.bit.CHSEL = 15;    // set SOC15 channel select to ADCINB7

//    AdcRegs.ADCSOC0CTL.bit.TRIGSEL = 7; // set SOC0 start trigger on EPWM2A
//    AdcRegs.ADCSOC1CTL.bit.TRIGSEL = 7; // set SOC1 start trigger on EPWM2A
//    AdcRegs.ADCSOC2CTL.bit.TRIGSEL = 7; // set SOC2 start trigger on EPWM2A
//    AdcRegs.ADCSOC3CTL.bit.TRIGSEL = 7; // set SOC3 start trigger on EPWM2A
//    AdcRegs.ADCSOC4CTL.bit.TRIGSEL = 7; // set SOC4 start trigger on EPWM2A
//    AdcRegs.ADCSOC5CTL.bit.TRIGSEL = 7; // set SOC5 start trigger on EPWM2A
//    AdcRegs.ADCSOC6CTL.bit.TRIGSEL = 7; // set SOC6 start trigger on EPWM2A
//    AdcRegs.ADCSOC7CTL.bit.TRIGSEL = 7; // set SOC7 start trigger on EPWM2A
//
//    AdcRegs.ADCSOC8CTL.bit.TRIGSEL = 7; // set SOC8 start trigger on EPWM2A
//    AdcRegs.ADCSOC9CTL.bit.TRIGSEL = 7; // set SOC9 start trigger on EPWM2A
//    AdcRegs.ADCSOC10CTL.bit.TRIGSEL = 7; // set SOC10 start trigger on EPWM2A
//    AdcRegs.ADCSOC11CTL.bit.TRIGSEL = 7; // set SOC11 start trigger on EPWM2A
//    AdcRegs.ADCSOC12CTL.bit.TRIGSEL = 7; // set SOC12 start trigger on EPWM2A
//    AdcRegs.ADCSOC13CTL.bit.TRIGSEL = 7; // set SOC13 start trigger on EPWM2A
//    AdcRegs.ADCSOC14CTL.bit.TRIGSEL = 7; // set SOC14 start trigger on EPWM2A
//    AdcRegs.ADCSOC15CTL.bit.TRIGSEL = 7; // set SOC15 start trigger on EPWM2A

    AdcRegs.ADCSOC0CTL.bit.ACQPS = 20; // set SOC0 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC1CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC2CTL.bit.ACQPS = 20; // set SOC2 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC3CTL.bit.ACQPS = 20; // set SOC S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC4CTL.bit.ACQPS = 20; // set SOC S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC5CTL.bit.ACQPS = 20; // set SOC S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC6CTL.bit.ACQPS = 20; // set SOC S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC7CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)

    AdcRegs.ADCSOC8CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC9CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC10CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC11CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC12CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC13CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC14CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)
    AdcRegs.ADCSOC15CTL.bit.ACQPS = 20; // set SOC1 S/H Window to 15 ADC Clock Cycles, (14 ACQPS plus 1)

    AdcRegs.INTSEL1N2.bit.INT1SEL = 5; //Connect ADCINT1 to EOC5
    AdcRegs.INTSEL1N2.bit.INT1E = 1; //Enable ADCINT1

    AdcRegs.INTSEL1N2.bit.INT2SEL = 14; //Connect ADCINT2 to EOC14
    AdcRegs.INTSEL1N2.bit.INT2E = 1; //Enable ADCINT2

    EDIS;

//    EPwm1Regs.ETSEL.bit.SOCAEN = 1; // Enable SOC on A group
//    EPwm1Regs.ETSEL.bit.SOCASEL = 4;    // Select SOC from CMPA on upcount
//    EPwm1Regs.ETPS.bit.SOCAPRD = 1; // Generate pulse on 1st event
//    EPwm1Regs.CMPA.half.CMPA = 0x0010;  // Set compare A value
//    EPwm1Regs.TBPRD = 0x100;    // Set period for ePWM1
//    EPwm1Regs.TBCTL.bit.CTRMODE = 0;    // count up and start
//
//    EPwm2Regs.ETSEL.bit.SOCAEN = 1; // Enable SOC on A group
//    EPwm2Regs.ETSEL.bit.SOCASEL = 4;    // Select SOC from CMPA on upcount
//    EPwm2Regs.ETPS.bit.SOCAPRD = 1; // Generate pulse on 1st event
//    EPwm2Regs.CMPA.half.CMPA = 0x0010;  // Set compare A value
//    EPwm2Regs.TBPRD = 0x100;    // Set period for ePWM1
//    EPwm2Regs.TBCTL.bit.CTRMODE = 0;    // count up and start
}
#endif /*config_ctrl*/
