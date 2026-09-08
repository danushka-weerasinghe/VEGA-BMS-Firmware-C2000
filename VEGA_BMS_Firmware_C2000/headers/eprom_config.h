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


#endif /* HEADERS_EPROM_CONFIG_H_ */
