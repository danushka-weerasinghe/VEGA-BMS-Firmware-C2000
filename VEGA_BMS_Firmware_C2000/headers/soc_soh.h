/*
 * soc_soh.h
 *
 *  Created on: Oct 14, 2021
 *      Author: Asith Devinda
 */

#ifndef HEADERS_SOC_SOH_H_
#define HEADERS_SOC_SOH_H_

#define DEADBAND          0.05

extern Uint16 charge_complete;
extern float batt_capacity;
extern volatile Uint8 can_force_soc_flag; //SOC force reset
extern volatile Uint8 can_target_soc_val; //SOC force reset


void calc_soc_soh(double current_value, unsigned int max_cell_voltage,
                  unsigned int min_cell_voltage, float *soc);
void init_charge_energy(void);
uint16_t soc_from_ocv(uint16_t ocv, uint16_t temp);
void clear_eepage(Uint16 slave_addr, Uint8 *page_addr, Uint16 addr_size);
void charge_ctrl(unsigned int highest_cell_v, int16 charge_current, float soc);
Uint32 calculate_charged_energy(double pack_voltage);
void reset_chg_energy(Uint16* chg_energy_reset);
void force_instant_soc_update(float *soc); // SOC force reset
void reset_charge_session(void);

void soc_init(float *soc);
void soc_calc_new(double current_value, unsigned int max_cell_voltage,unsigned int min_cell_voltage, float *soc);
void Write_EEP_16bit(Uint16 flag_val, Uint8 *addr);
void Write_EEP_Float(float soc_val, Uint8 *addr);
Uint16 get_Temp_soc(void);




#endif /* HEADERS_SOC_SOH_H_ */
