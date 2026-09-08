/*
 * Chiller.h
 *
 *  Created on: Nov 2, 2023
 *      Author: Danushka Weerasinghe
 */

#ifndef CHILLER_H_
#define CHILLER_H_

#include "DSP28x_Project.h"
#include "main_data.h"
#include "BMS_controller_CAN.h"

void SCIb_SEND(Uint8 length_sci_rev);
void chller_operation();
void chillerData_write(Uint16* chiller_data_write);
void chillerData_read(Uint16* chiller_data_read);

struct chillerCtrl_data_struct

{

    Uint8 comp_status;
    Uint16 comp_speed;
    Uint16 comp_power_limit;

};

extern struct chillerCtrl_data_struct  chillerCtrl_data;

struct chillerDiag_data_struct

{

    Uint16 comp_speed_actual;
    enum chiller_comp_state_enum chiller_comp_state;
    enum chiller_error_enum chiller_error;
    Uint16 current;
    Uint16 voltage;
};

extern struct chillerDiag_data_struct  chillerDiag_data;

#endif /* CHILLER_H_ */
