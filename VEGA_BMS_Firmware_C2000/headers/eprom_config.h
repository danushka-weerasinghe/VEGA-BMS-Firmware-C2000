/*
 * eprom_config.h
 *
 *  Created on: Mar 4, 2026
 *      Author: miyurul
 */

#include "DSP28x_Project.h"
#include <stdint.h>

#ifndef HEADERS_EPROM_CONFIG_H_
#define HEADERS_EPROM_CONFIG_H_




// Parameter Indexes
#define PARAM_NOMINAL_CAPACITY 0x01
#define PARAM_CURRENT_LIMITS   0x02
#define PARAM_PACK_NUMBER      0x03
#define PARAM_UPDATE_DATE      0x04

#define CALIB_DUE_CYCLES   10UL      /* pack-equivalents charged without a full charge -> reminder     */

#define CALIB_FLAG_FULL_CHARGE_DUE  0x01   /* bit 0 of 0x35 page 1 byte 5; bits 1-7 reserved = 0       */



extern Uint32 nominal_capacity;
extern int32_t CHARGE_CURRENT_THRESHOLD;
extern int32_t DISCHARGE_CURRENT_THRESHOLD;
extern int32_t SC_CURRENT_THRESHOLD;

// Global flag to tell the main loop what needs to be saved
extern volatile Uint8 pending_eeprom_save_param;

void init_nominal_capacity(void);
void init_current_limits(void);
void init_pack_number(void);
void init_fw_update_date(void);

void save_parameter_to_eeprom(void);
void parse_config_update_cmd(Uint16* rx_data);
void broadcast_eeprom_data_over_can(void);



void  calib_init(void);              /* once, after init_charge_energy() and init_nominal_capacity()   */
void  calib_full_charge_done(void);  /* at the SOC = 100 % reset (soc_calc_new case 94)               */
void  calib_save_step(Uint8 step);   /* from soc_calc_new: step 0 = main copy, step 1 = backup copy    */
Uint8 calib_flags(void);             /* byte for 0x35 page 1 byte 5                                    */
Uint8 calib_cycles_since_full(void); /* optional: 0x35 page 1 byte 6, pack-equivalents 0-255           */


#endif /* HEADERS_EPROM_CONFIG_H_ */
