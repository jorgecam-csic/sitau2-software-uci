/*
 * datapath_sw.c
 *
 *  Created on: 25 ago. 2022
 *      Author: csic
 */

#include "dspdma_func.h"
#include "xil_types.h"
#include "xaxis_switch.h"
#include "switch_driver.h"
#include "mcbcc_mst_driver.h"
#include "bussar_addr.h"



void set_axi4_2_gtx(int disabled)
{
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_GTXSWT_IDX,DSP_DMA_SWITCH_SLAVE_FROM_AXI4ST_IDX,disabled);
}

