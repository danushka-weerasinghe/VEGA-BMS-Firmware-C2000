/*
 * PCA2129RTC.h
 *
 *  Created on: Jun 18, 2020
 *      Author: VEGA_1010
 */

#ifndef PCA2129RTC_H_
#define PCA2129RTC_H_

#define I2C_SLAVE_RTC_ADDR 0x51
#define NACK_CHECK 1

//RTC register addresses
#define CONTROL1_REG 0x00
#define CONTROL2_REG 0x01
#define CONTROL3_REG 0x02
#define SECONDS_REG 0x03
#define MINUTES_REG 0x04
#define HOURS_REG 0x05
#define DAYS_REG 0x06
#define WEEKDAYS_REG 0x07
#define MONTHS_REG 0x08
#define YEARS_REG 0x09
#define SECOND_ALARM_REG 0x0A
#define MINUTE_ALARM_REG 0x0B
#define HOUR_ALARM_REG 0x0C
#define DAY_ALARM_REG 0x0D
#define WEEKDAY_ALARM_REG 0x0E
#define CLKOUT_CTL_REG 0x0F
#define WATCHDG_TIM_CTL_REG 0x10
#define WATCHDG_TIM_VAL_REG 0x11
#define TIMESTP_CTL_REG 0x12
#define SEC_TIMESTP_REG 0x13
#define MIN_TIMESTP_REG 0x14
#define HOUR_TIMESTP_REG 0x15
#define DAY_TIMESTP_REG 0x16
#define MON_TIMESTP_REG 0x17
#define YEAR_TIMESTP_REG 0x18
#define AGING_OFFSET_REG 0x19
#define INTERNAL_REG_REG 0x1A

// REG_VALUE

#define CONTROL1_STOP_0_VAL 0x09
#define CONTROL1_STOP_1_VAL 0x29
#define WATCHDG_TIM_CTL_TI_TP_0_VAL 0x03

/*
 * Memory def
 */
#define I2C_SLAVE_EEPROM_ADDR1 0x54
#define I2C_SLAVE_EEPROM_ADDR2 0x55
#define I2C_SLAVE_EEPROM_ADDR3 0x56
#define I2C_SLAVE_EEPROM_ADDR4 0x57

typedef struct{
    Uint16 y;
    unsigned char m,d,hh,mm,ss;
}DateTime;

void InitDateTime (DateTime *real_time_x,unsigned char msg[]);
void decord_time(DateTime *real_time_x,unsigned char buffer[]);

unsigned char bcdToDec(unsigned char value);
unsigned char decToBcd(unsigned char value);

//Uint16 I2CA_WriteToReg(Uint16 slave_addr, Uint8 reg, Uint8 data[], Uint16 data_size);

Uint16 I2CA_WriteToManyRegs(Uint16 slave_addr, Uint8 reg[], Uint16 reg_size, Uint8 data[],
                            Uint16 data_size);

//Uint16 I2CA_ReadFromReg(Uint16 slave_addr, Uint8 reg, Uint8 data[], Uint16 data_size);

Uint16 I2CA_ReadFromManyRegs(Uint16 slave_addr, Uint8 reg[], Uint16 reg_size, Uint8 data[],
                             Uint16 data_size);

#endif /* PCA2129RTC_H_ */
