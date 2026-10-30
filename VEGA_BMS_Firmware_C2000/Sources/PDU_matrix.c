/*
 * PDU_matrix.c
 *
 *  Created on: Jun 9, 2022
 *      Author: Asith Devinda
 */

#include "PDU_matrix.h"
#include "main_data.h"
#include "GBT_data.h"
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
}

#if (GBT_CHARGING == 1)
Uint16 PDU_gbt_Data_frameCount(void)
{
    Uint16 cell_frames = (GBT_Data_i.cell_count + GBT_CELLS_PER_FRAME - 1) / GBT_CELLS_PER_FRAME;
    Uint16 temp_frames = (GBT_Data_i.temp_count + GBT_TEMPS_PER_FRAME - 1) / GBT_TEMPS_PER_FRAME;

    return GBT_STATUS_FRAME_COUNT + ((cell_frames > temp_frames) ? cell_frames : temp_frames);
}


void PDU_gbt_Data_write(uint16_t GBT_Data_index, Uint16* PDU_gbt_Data)
{
    Uint16 i;
    Uint16 frame_no;
    Uint16 cell_no;
    Uint16 cell_v;

    PDU_gbt_Data[0] = GBT_Data_index;

    switch (GBT_Data_index)
    {
    /* setup data, index 0..2 - sent once per EVCC request (GBT_setup_request) */
    case 0:
        PDU_gbt_Data[1] = GBT_Data_i.max_charge_voltage;
        PDU_gbt_Data[2] = GBT_Data_i.max_charge_voltage >> 8;
        PDU_gbt_Data[3] = GBT_Data_i.max_cell_charge_voltage;
        PDU_gbt_Data[4] = GBT_Data_i.max_cell_charge_voltage >> 8;
        PDU_gbt_Data[5] = GBT_Data_i.rated_voltage;
        PDU_gbt_Data[6] = GBT_Data_i.rated_voltage >> 8;
        PDU_gbt_Data[7] = GBT_Data_i.battery_type;
        break;

    case 1:
        PDU_gbt_Data[1] = GBT_Data_i.total_energy;
        PDU_gbt_Data[2] = GBT_Data_i.total_energy >> 8;
        PDU_gbt_Data[3] = (Uint16)GBT_Data_i.max_permit_temp;
        PDU_gbt_Data[4] = (Uint16)GBT_Data_i.max_permit_temp >> 8;
        PDU_gbt_Data[5] = GBT_Data_i.rated_capacity;
        PDU_gbt_Data[6] = GBT_Data_i.rated_capacity >> 8;
        PDU_gbt_Data[7] = 0;
        break;

    case 2:
        PDU_gbt_Data[1] = GBT_Data_i.max_charge_current;
        PDU_gbt_Data[2] = GBT_Data_i.max_charge_current >> 8;
        PDU_gbt_Data[3] = 0;
        PDU_gbt_Data[4] = 0;
        PDU_gbt_Data[5] = 0;
        PDU_gbt_Data[6] = 0;
        PDU_gbt_Data[7] = 0;
        break;

    /* live data, index 3.. - sent every cycle while charging */
    case 3:
        PDU_gbt_Data[1] = (Uint16)GBT_Data_i.lowest_temp;
        PDU_gbt_Data[2] = (Uint16)GBT_Data_i.lowest_temp >> 8;
        PDU_gbt_Data[3] = GBT_Data_i.lowest_temp_probe;
        PDU_gbt_Data[4] = GBT_Data_i.highest_temp_probe;
        PDU_gbt_Data[5] = GBT_Data_i.status_A.all;
        PDU_gbt_Data[6] = GBT_Data_i.status_B.all;
        PDU_gbt_Data[7] = GBT_Data_i.status_C.all;
        break;

    case 4:
        PDU_gbt_Data[1] = GBT_Data_i.lowest_cell_id;
        PDU_gbt_Data[2] = GBT_Data_i.cell_count;            // series cells in the pack (SERIES_CELLS)
        PDU_gbt_Data[3] = GBT_Data_i.temp_count;            // temperature probes (TOTAL_IC * TEMP_PER_IC)
        PDU_gbt_Data[4] = GBT_Data_i.highest_cell_id;
        PDU_gbt_Data[5] = 0;
        PDU_gbt_Data[6] = 0;
        PDU_gbt_Data[7] = 0;
        break;

        default:    // 3 cell voltages + 1 temperature
            if (GBT_Data_index < PDU_gbt_Data_frameCount())
            {
                frame_no = GBT_Data_index - GBT_STATUS_FRAME_COUNT;
                cell_no = frame_no * GBT_CELLS_PER_FRAME;

                /* temperature n + 1 in frame n */
                if (frame_no < GBT_Data_i.temp_count)
                {
                    PDU_gbt_Data[7] = GBT_Data_i.temp[frame_no] & 0xFF;
                }
                else
                {
                    PDU_gbt_Data[7] = GBT_TEMP_INVALID;
                }

                for (i = 0; i < GBT_CELLS_PER_FRAME; i++)
                {
                    if (cell_no < GBT_Data_i.cell_count)
                    {
                        cell_v = GBT_Data_i.cell_voltage[cell_no];
                    }
                    else
                    {
                        cell_v = GBT_CELL_V_INVALID;
                    }
                    PDU_gbt_Data[1 + (i * 2)] = cell_v;
                    PDU_gbt_Data[2 + (i * 2)] = cell_v >> 8;
                    cell_no++;
                }
            }
            break;
    }
}
#endif /* GBT_CHARGING */

/* series cell number (0-based) -> BMS_data_IC[ic].cell_voltages[pos] */
static Uint16 debug_cell_ic[DEBUG_CELL_COUNT];
static Uint16 debug_cell_pos[DEBUG_CELL_COUNT];
static Uint16 debug_cell_count = 0;

// Call once at start up, unused cell positions (CELL_UNUSED) are skipped
void PDU_debugData_init(void)
{
    Uint16 ic, c;

    debug_cell_count = 0;
    for (ic = 0; ic < TOTAL_IC; ic++)
    {
        for (c = 0; c < CELLS_PER_IC; c++)
        {
            if (!CELL_UNUSED(ic, c) && (debug_cell_count < DEBUG_CELL_COUNT))
            {
                debug_cell_ic[debug_cell_count] = ic;
                debug_cell_pos[debug_cell_count] = c;
                debug_cell_count++;
            }
        }
    }
}

Uint16 PDU_debugData_frameCount(void)
{
    Uint16 cell_frames = (debug_cell_count + DEBUG_CELLS_PER_FRAME - 1) / DEBUG_CELLS_PER_FRAME;
    Uint16 temp_frames = (DEBUG_TEMP_COUNT + DEBUG_TEMPS_PER_FRAME - 1) / DEBUG_TEMPS_PER_FRAME;

    return DEBUG_FIXED_FRAME_COUNT + ((cell_frames > temp_frames) ? cell_frames : temp_frames);
}

void PDU_debugData_write(uint16_t db_index, Uint16* PDU_debug_data)
{
    Uint16 i;
    Uint16 frame_no;
    Uint16 cell_no;
    Uint16 cell_v;

    PDU_debug_data[0] = db_index;

    switch (db_index)
    {
    case 0:
        PDU_debug_data[1] = BMS_data_IC[0].slave_humidity_val;
        PDU_debug_data[2] = BMS_data_IC[0].slave_humidity_val >> 8;
        PDU_debug_data[3] = BMS_data_IC[0].slave_temperature_val;
        PDU_debug_data[4] = BMS_data_IC[0].slave_temperature_val >> 8;
        PDU_debug_data[5] = BMS_data_IC[0].open_wire_lsb;
        PDU_debug_data[6] = BMS_data_IC[0].open_wire_msb;
        PDU_debug_data[7] = PDU_getData_i.sht30_error;
        break;

    case 1:
        PDU_debug_data[1] = BMS_data_IC[1].slave_humidity_val;
        PDU_debug_data[2] = BMS_data_IC[1].slave_humidity_val >> 8;
        PDU_debug_data[3] = BMS_data_IC[1].slave_temperature_val;
        PDU_debug_data[4] = BMS_data_IC[1].slave_temperature_val >> 8;
        PDU_debug_data[5] = BMS_data_IC[1].open_wire_lsb;
        PDU_debug_data[6] = BMS_data_IC[1].open_wire_msb;
        PDU_debug_data[7] = 0;
        break;

        default:    // cell voltages, 3 cells per frame
            if (db_index < PDU_debugData_frameCount())
            {
                frame_no = db_index - DEBUG_FIXED_FRAME_COUNT;
                cell_no = frame_no * DEBUG_CELLS_PER_FRAME;

                /* temperature n + 1 in frame n */
                if (frame_no < DEBUG_TEMP_COUNT)
                {
                    PDU_debug_data[7] = BMS_data_IC[frame_no / TEMP_PER_IC].temparature_val[frame_no % TEMP_PER_IC] & 0xFF;
                }
                else
                {
                    PDU_debug_data[7] = DEBUG_TEMP_INVALID;
                }

                for (i = 0; i < DEBUG_CELLS_PER_FRAME; i++)
                {
                    if (cell_no < debug_cell_count)
                    {
                        cell_v = BMS_data_IC[debug_cell_ic[cell_no]].cell_voltages[debug_cell_pos[cell_no]];
                    }
                    else
                    {
                        cell_v = DEBUG_CELL_V_INVALID;
                    }
                    PDU_debug_data[1 + (i * 2)] = cell_v;
                    PDU_debug_data[2 + (i * 2)] = cell_v >> 8;
                    cell_no++;
                }
            }
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
