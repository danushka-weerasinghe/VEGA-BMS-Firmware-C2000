/*
 * RS485_shunt.h
 *
 *  Created on: Jul 18, 2022
 *      Author: Asith Devinda
 */

#ifndef HEADERS_RS485_SHUNT_H_
#define HEADERS_RS485_SHUNT_H_

#define SCI_Timeout 2
#define RS485_DriverEn GpioDataRegs.GPBSET.bit.GPIO42 = 1
#define RS485_ReceiverEn GpioDataRegs.GPBCLEAR.bit.GPIO42 = 1

/*
 * Typedefs
 */
typedef struct{
    Uint16 byte_0:8;
    Uint16 byte_1:8;
    Uint16 byte_2:8;
    Uint16 byte_3:8;
}byte_8bit;

typedef union {
    float     data_f;
    double    data_d;
    byte_8bit d_array;
}Sci_data;

/*Variable defs*/
extern Uint16 volatile shuntCS_watchdog;
extern Uint16 volatile request_code;
extern Sci_data modbus_rx_data;

/*Function defs*/

void RS485_Serial_Init(void);

void SCI_reset(void);

void MODBUS_RTU_Init(void);

Uint16 RS485_read(Uint16 *data);

Uint16 RS485_Write(Uint16 *data, Uint16 length);

Uint16 MODBUS_send(Uint16 function, Uint16 address);

__interrupt void sciaRxFifoIsr(void);

double read_MODBUS(void);

#endif /* HEADERS_RS485_SHUNT_H_ */
