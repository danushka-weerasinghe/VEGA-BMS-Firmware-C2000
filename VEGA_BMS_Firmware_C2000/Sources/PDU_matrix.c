/*
 * PDU_matrix.c
 *
 *  Created on: Jun 9, 2022
 *      Author: Asith Devinda
 */

#include "PDU_matrix.h"
#include "main_data.h"
//#include "BMS_controller_CAN.h"

//getData *********************************************************************
struct PDU_getData PDU_getData_i;
struct PDU_getData PDU_getData_local;
volatile struct PDU_getData PDU_getData_slave;
CAN_BufferMap canMap;
cumulative_energy c_count;

void PDU_getDataSlave_read(Uint16* PDU_slave_data_read)
{
//    CAN_readMessage(CAN_OBJ_17, canMap.msg[CAN_OBJ_17].readData);
    PDU_getData_slave.dataID = PDU_slave_data_read[0];
    switch (PDU_getData_slave.dataID)
    {
        case 0:

            PDU_getData_slave.fixSetG.all = PDU_slave_data_read[1];
            PDU_getData_slave.op_mode = PDU_slave_data_read[2];
            PDU_getData_slave.lowest_cell_voltage = PDU_slave_data_read[3];
            PDU_getData_slave.lowest_cell_voltage |= (PDU_slave_data_read[4] << 8);
            PDU_getData_slave.highest_cell_voltage = PDU_slave_data_read[5];
            PDU_getData_slave.highest_cell_voltage |= (PDU_slave_data_read[6]<< 8);
            PDU_getData_slave.highest_pack_temparature = PDU_slave_data_read[7];
            break;

        case 1:
            PDU_getData_slave.fixSetG.all = PDU_slave_data_read[1];
//            (Uint8) (PDU_getData_slave.time_to_80 / 60) = slave_data[2];
//            (Uint8) (PDU_getData_slave.time_to_80 % 60) = slave_data[3];
            PDU_getData_slave.Battery_capacity = PDU_slave_data_read[4];
            PDU_getData_slave.fixSetChrg.all = PDU_slave_data_read[5];
            PDU_getData_slave.total_pack_voltage = PDU_slave_data_read[6];
            PDU_getData_slave.total_pack_voltage |= (PDU_slave_data_read[7] << 8);
            break;

        case 2:
            PDU_getData_slave.fixSetG.all = PDU_slave_data_read[1];
//            (Uint8) (PDU_getData_slave.time_to_100 / 60) = slave_data[2];
//            (Uint8) (PDU_getData_slave.time_to_100 % 60) = slave_data[3];
            PDU_getData_slave.current_Axx = PDU_slave_data_read[4];
            PDU_getData_slave.current_Axx |= (PDU_slave_data_read[5] << 8);
            PDU_getData_slave.SOC_val = PDU_slave_data_read[6];
            PDU_getData_slave.SOH_val = PDU_slave_data_read[7];
            break;

        default:
            break;
    }

}
void PDU_getData_write(uint16_t getData_index, Uint16* PDU_get_data)
{
    PDU_get_data[0] = getData_index;

    switch (getData_index)
    {
        case 0:
            PDU_get_data[1] = PDU_getData_i.fixSetG.all;
            PDU_get_data[2] = PDU_getData_i.op_mode;
            PDU_get_data[3] = PDU_getData_i.lowest_cell_voltage;
            PDU_get_data[4] = PDU_getData_i.lowest_cell_voltage >> 8;
            PDU_get_data[5] = PDU_getData_i.highest_cell_voltage;
            PDU_get_data[6] = PDU_getData_i.highest_cell_voltage >> 8;
            PDU_get_data[7] = PDU_getData_i.highest_pack_temparature;
            break;
        case 1:
            PDU_get_data[1] = PDU_getData_i.fixSetG.all;
            PDU_get_data[2] = (Uint8) (PDU_getData_i.time_to_80 / 60);
            PDU_get_data[3] = (Uint8) (PDU_getData_i.time_to_80 % 60);
            PDU_get_data[4] = 130;
            PDU_get_data[5] = PDU_getData_i.fixSetChrg.all;
            PDU_get_data[6] = PDU_getData_i.total_pack_voltage;
            PDU_get_data[7] = PDU_getData_i.total_pack_voltage >> 8;
            break;
        case 2:
            PDU_get_data[1] = PDU_getData_i.fixSetG.all;
            PDU_get_data[2] = (Uint8) (PDU_getData_i.time_to_100 / 60);
            PDU_get_data[3] = (Uint8) (PDU_getData_i.time_to_100 % 60);
            PDU_get_data[4] = PDU_getData_i.current_Axx;
            PDU_get_data[5] = PDU_getData_i.current_Axx >> 8;
            PDU_get_data[6] = PDU_getData_i.display_soc_val;
            PDU_get_data[7] = 100;
            break;
        case 3:
            PDU_get_data[1] = PDU_getData_i.fixSetG.all;
            PDU_get_data[2] = 0;
            PDU_get_data[3] = 0;
            PDU_get_data[4] = c_count.send_coulomb_bytes.BYTE0;
            PDU_get_data[5] = c_count.send_coulomb_bytes.BYTE1;
            PDU_get_data[6] = c_count.send_coulomb_bytes.BYTE2;
            PDU_get_data[7] = c_count.send_coulomb_bytes.BYTE3;
//            memcpy((void *)& ECanaMboxes.MBOX15.MDH.all, (const void *)& INA229_data[4], sizeof(float));
        default:
            break;
    }

//    CAN_sendMessage(CAN_OBJ_15, NOMINAL_DLC, canMap.msg[CAN_OBJ_15].sendData);
//    ECanaShadow.CANTRS.bit.TRS15 = 1;
}

void PDU_debugData_write(uint16_t db_index, Uint16* PDU_debug_data)
{
    PDU_debug_data[0] = db_index;

    switch (db_index)
    {
        case 0:
            PDU_debug_data[1] = BMS_data_IC[0].cell_voltages[0];
            PDU_debug_data[2] = BMS_data_IC[0].cell_voltages[0] >> 8;
            PDU_debug_data[3] = BMS_data_IC[0].cell_voltages[1];
            PDU_debug_data[4] = BMS_data_IC[0].cell_voltages[1] >> 8;
            PDU_debug_data[5] = BMS_data_IC[0].cell_voltages[2];
            PDU_debug_data[6] = BMS_data_IC[0].cell_voltages[2] >> 8;
            PDU_debug_data[7] = BMS_data_IC[0].temparature_val[0];
            break;

        case 1:
            PDU_debug_data[1] = BMS_data_IC[0].cell_voltages[3];
            PDU_debug_data[2] = BMS_data_IC[0].cell_voltages[3] >> 8;
            PDU_debug_data[3] = BMS_data_IC[0].cell_voltages[4];
            PDU_debug_data[4] = BMS_data_IC[0].cell_voltages[4] >> 8;
            PDU_debug_data[5] = BMS_data_IC[0].cell_voltages[5];
            PDU_debug_data[6] = BMS_data_IC[0].cell_voltages[5] >> 8;
            PDU_debug_data[7] = BMS_data_IC[0].temparature_val[1];
            break;
        case 2:
            PDU_debug_data[1] = BMS_data_IC[0].cell_voltages[6];
            PDU_debug_data[2] = BMS_data_IC[0].cell_voltages[6] >> 8;
            PDU_debug_data[3] = BMS_data_IC[0].cell_voltages[7];
            PDU_debug_data[4] = BMS_data_IC[0].cell_voltages[7] >> 8;
            PDU_debug_data[5] = BMS_data_IC[0].cell_voltages[8];
            PDU_debug_data[6] = BMS_data_IC[0].cell_voltages[8] >> 8;
            PDU_debug_data[7] = BMS_data_IC[0].temparature_val[2];
            break;
        case 3:
            PDU_debug_data[1] = BMS_data_IC[0].cell_voltages[9];
            PDU_debug_data[2] = BMS_data_IC[0].cell_voltages[9] >> 8;
            PDU_debug_data[3] = BMS_data_IC[0].cell_voltages[10];
            PDU_debug_data[4] = BMS_data_IC[0].cell_voltages[10] >> 8;
            PDU_debug_data[5] = BMS_data_IC[0].cell_voltages[11];
            PDU_debug_data[6] = BMS_data_IC[0].cell_voltages[11] >> 8;
            PDU_debug_data[7] = BMS_data_IC[1].temparature_val[0];
            break;
        case 4:
            PDU_debug_data[1] = BMS_data_IC[1].cell_voltages[0];
            PDU_debug_data[2] = BMS_data_IC[1].cell_voltages[0] >> 8;
            PDU_debug_data[3] = BMS_data_IC[1].cell_voltages[1];
            PDU_debug_data[4] = BMS_data_IC[1].cell_voltages[1] >> 8;
            PDU_debug_data[5] = BMS_data_IC[1].cell_voltages[2];
            PDU_debug_data[6] = BMS_data_IC[1].cell_voltages[2] >> 8;
            PDU_debug_data[7] = BMS_data_IC[1].temparature_val[1];
            break;
        case 5:
            PDU_debug_data[1] = BMS_data_IC[1].cell_voltages[3];
            PDU_debug_data[2] = BMS_data_IC[1].cell_voltages[3] >> 8;
            PDU_debug_data[3] = BMS_data_IC[1].cell_voltages[4];
            PDU_debug_data[4] = BMS_data_IC[1].cell_voltages[4] >> 8;
            PDU_debug_data[5] = BMS_data_IC[1].cell_voltages[5];
            PDU_debug_data[6] = BMS_data_IC[1].cell_voltages[5] >> 8;
            PDU_debug_data[7] = BMS_data_IC[1].temparature_val[2];
            break;
        case 6:
            PDU_debug_data[1] = BMS_data_IC[1].cell_voltages[6];
            PDU_debug_data[2] = BMS_data_IC[1].cell_voltages[6] >> 8;
            PDU_debug_data[3] = BMS_data_IC[1].cell_voltages[7];
            PDU_debug_data[4] = BMS_data_IC[1].cell_voltages[7] >> 8;
            PDU_debug_data[5] = BMS_data_IC[1].cell_voltages[8];
            PDU_debug_data[6] = BMS_data_IC[1].cell_voltages[8] >> 8;
            PDU_debug_data[7] = 0;
            break;
        case 7:
            PDU_debug_data[1] = BMS_data_IC[1].cell_voltages[9];
            PDU_debug_data[2] = BMS_data_IC[1].cell_voltages[9] >> 8;
            PDU_debug_data[3] = BMS_data_IC[1].cell_voltages[10];
            PDU_debug_data[4] = BMS_data_IC[1].cell_voltages[10] >> 8;
            PDU_debug_data[5] = PDU_getData_i.sht30_error;
            PDU_debug_data[6] = 0;
            PDU_debug_data[7] = 0;
            break;
        case 8:
            PDU_debug_data[1] = BMS_data_IC[0].slave_humidity_val;
            PDU_debug_data[2] = BMS_data_IC[0].slave_humidity_val>> 8;
            PDU_debug_data[3] = BMS_data_IC[0].slave_temperature_val;
            PDU_debug_data[4] = BMS_data_IC[0].slave_temperature_val>> 8;
            PDU_debug_data[5] = BMS_data_IC[0].open_wire_lsb;
            PDU_debug_data[6] = BMS_data_IC[0].open_wire_msb;
            PDU_debug_data[7] = 0;
            break;
        case 9:
            PDU_debug_data[1] = BMS_data_IC[1].slave_humidity_val;
            PDU_debug_data[2] = BMS_data_IC[1].slave_humidity_val>> 8;
            PDU_debug_data[3] = BMS_data_IC[1].slave_temperature_val;
            PDU_debug_data[4] = BMS_data_IC[1].slave_temperature_val>> 8;
            PDU_debug_data[5] = BMS_data_IC[1].open_wire_lsb;
            PDU_debug_data[6] = BMS_data_IC[1].open_wire_msb;
            PDU_debug_data[7] = 0;
            break;
        default:
            break;
    }

//    CAN_sendMessage(CAN_OBJ_14, NOMINAL_DLC, canMap.msg[CAN_OBJ_14].sendData);
//    ECanaShadow.CANTRS.bit.TRS14 = 1;
}

void PDU_tempData_write(Uint16* temp_data)
{
    temp_data[0] = (PDU_getData_i.charged_energy) & 0xFF;
    temp_data[1] = (PDU_getData_i.charged_energy >> 8) & 0xFF;
    temp_data[2] = (PDU_getData_i.charged_energy >> 16) & 0xFF;
    temp_data[3] = (PDU_getData_i.charged_energy >> 24) & 0xFF;
    temp_data[4] = (PDU_getData_i.charged_energy_acc) & 0xFF;
    temp_data[5] = (PDU_getData_i.charged_energy_acc >> 8) & 0xFF;
    temp_data[6] = (PDU_getData_i.charged_energy_acc >> 16) & 0xFF;
    temp_data[7] = (PDU_getData_i.charged_energy_acc >> 24) & 0xFF;
//    CAN_sendMessage(CAN_OBJ_14, NOMINAL_DLC, canMap.msg[CAN_OBJ_14].sendData);

//    ECanaShadow.CANTRS.bit.TRS14 = 1;
}

//setData *********************************************************************
volatile struct PDU_setData PDU_setData_local;
struct PDU_setData PDU_setDataSlave_local;
struct PDU_setData PDU_setDataSlave_i;

void PDU_setData_read(Uint16* EVCU_data)
{
//    CAN_readMessage(CAN_OBJ_16, canMap.msg[CAN_OBJ_16].readData);

    PDU_setData_local.dataID = EVCU_data[0];
    if (PDU_setData_local.dataID == 0)
    {
        PDU_setData_local.fixSetS.all = EVCU_data[1];
        PDU_setData_local.charge_complete = EVCU_data[2];
//        PDU_setData_local.GetDataID = ECanaMboxes.MBOX16.MDL.byte.BYTE3;
        PDU_setData_local.contactor_on = EVCU_data[4];
        PDU_setData_local.contactor_on_inverse = EVCU_data[5];
//        PDU_setData_local.set_op_mode = ECanaMboxes.MBOX16.MDH.byte.BYTE6;
//        PDU_setData_local.cell_balancing_enable = ECanaMboxes.MBOX16.MDH.byte.BYTE7;
    }
}

void PDU_setData_read_ODO(Uint16* ODO_bytes)
{
    PDU_setData_local.dataID = ODO_bytes[0];
    if (PDU_setData_local.dataID == 0)
    {
        PDU_setData_local.odo_value.byte.ODO_BYTE1 = ODO_bytes[1];
        PDU_setData_local.odo_value.byte.ODO_BYTE2 = ODO_bytes[2];
        PDU_setData_local.odo_value.byte.ODO_BYTE3 = ODO_bytes[3];
        PDU_setData_local.odo_value.byte.ODO_BYTE4 = ODO_bytes[4];
    }
}

void PDU_setData_read_EVCC(Uint16* EVCC_data)
{
//    CAN_readMessage(CAN_OBJ_11, canMap.msg[CAN_OBJ_11].readData);
//    CAN_readMessage(CAN_OBJ_3, readBuffer);
    PDU_setData_local.fixSetS_EVCC.all = canMap.msg[CAN_OBJ_11].readData[1];
//    PDU_setData_local.fixSetS_EVCC.all = ECanaMboxes.MBOX11.MDL.byte.BYTE1;
}

void PDU_setDataSlave_write(Uint16* slave_data_write)
{
    slave_data_write[0] = 0;
    slave_data_write[1] = PDU_setDataSlave_i.fixSetS.all;
    slave_data_write[2] = PDU_setDataSlave_i.charge_complete;
    slave_data_write[3] = 0;
    slave_data_write[4] = PDU_setDataSlave_i.contactor_on;
    slave_data_write[5] = PDU_setDataSlave_i.contactor_on_inverse;
    slave_data_write[6] = 0;
    slave_data_write[7] = 0;

//    CAN_sendMessage(CAN_OBJ_18, NOMINAL_DLC, canMap.msg[CAN_OBJ_18].sendData);

}
