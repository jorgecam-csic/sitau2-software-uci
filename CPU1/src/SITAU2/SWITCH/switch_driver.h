/*
 * switch_driver.h
 *
 *  Created on: 5 nov. 2019
 *      Author: csic
 */

#define SWICH_LOCAL_ID	0
#define SWICH_REMOTO_ID	4

#include "xaxis_switch.h"
#include "bcc_bussar.h"


//----------------------------------------------------------
//----------------------------------------------------------
#define DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX		0
#define DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX		1
#define DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX		2
#define DSP_DMA_SWITCH_SLAVE_FROM_ENVOLV_IDX		3
#define DSP_DMA_SWITCH_SLAVE_FROM_IPHASE_IDX		4
#define DSP_DMA_SWITCH_SLAVE_FROM_QUADRA_IDX		5
#define DSP_DMA_SWITCH_SLAVE_FROM_AXI4ST_IDX		6
#define DSP_DMA_SWITCH_SLAVE_FROM_ZEROIN_IDX		7
#define DSP_DMA_SWITCH_SLAVE_FROM_DDRZYN_IDX		8

//----------------------------------------------------------
#define DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX			0
#define DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX			1
#define DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX			2
#define DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX			3
#define DSP_DMA_SWITCH_MASTER_TO_GTXSWT_IDX			4
#define DSP_DMA_SWITCH_MASTER_TO_BCCMST_IDX			5

//----------------------------------------------------------
//----------------------------------------------------------
#define REGFIFO_SWITCH_REGFIFO_SLAVE_FROM_AXILFIFO_IDX		0
//----------------------------------------------------------
#define REGFIFO_SWITCH_MASTER_TO_FPGA_PROG_IDX		0
#define REGFIFO_SWITCH_MASTER_TO_FIR_RELOAD_IDX		1
#define REGFIFO_SWITCH_MASTER_TO_FIR_CONFIG_IDX		2
#define REGFIFO_SWITCH_MASTER_TO_HIL_RELOAD_IDX		3
#define REGFIFO_SWITCH_MASTER_TO_HIL_CONFIG_IDX		4
//----------------------------------------------------------
//----------------------------------------------------------
#define SSWITCH032_SLAVE_FROM_BCC_IDX		0 //Desde el bus circular
#define SSWITCH032_SLAVE_FROM_DDR_IDX		1 //Desde el DMA32
#define SSWITCH032_SLAVE_FROM_SUM_IDX		2 //Desde el módulo de suma
//#define SSWITCH032_SLAVE_FROM_PRM_IDX		2 //Desde el módulo de promediado
//#define SSWITCH032_SLAVE_FROM_BFD_IDX		4 //Desde el DMA del conformador

#define SSWITCH032_MASTER_TO_DDR_IDX		0 //Hacia el DMA32
#define SSWITCH032_MASTER_TO_BFM_IDX		1 //Hacia el los datos de enfoque del conformador
#define SSWITCH032_MASTER_TO_BFI_IDX		2 //Hacia el los datos de enfoque del conformador (IDA, para TFM y PWI)
#define SSWITCH032_MASTER_TO_TGC_IDX		3 //Hacia los datos de la TGC
#define SSWITCH032_MASTER_TO_BCC_IDX		4 //Hacia el bus circular
//#define SSWITCH032_MASTER_TO_PRM_IDX		5 //Hacia los datos de promediado
#define SSWITCH032_MASTER_TO_SMT_IDX		5 //Hacia el módulo de suma de este módulo (THIS)
#define SSWITCH032_MASTER_TO_SMP_IDX		6 //Hacia el módulo de suma del módulo anterior (PREVIOUS)
//#define SSWITCH032_MASTER_TO_BFD_IDX		8 //Hacia el DMA del conformador
#define SSWITCH032_MASTER_TO_COE_IDX		7 //Hacia coeficientes del filtro de adquisicion SOLO MODO SIN CONFORMADOR
#define SSWITCH032_MASTER_TO_CFG_IDX		8 //Hacia configuración del filtro de adquisicion SOLO MODO SIN CONFORMADOR

//----------------------------------------------------------
//----------------------------------------------------------
#define SSWITCH512_SLAVE_FROM_AFE_IDX		0
#define SSWITCH512_SLAVE_FROM_DMA_IDX		1

#define SSWITCH512_MASTER_TO_DMA_IDX		0
#define SSWITCH512_MASTER_TO_BFM_IDX		1
//----------------------------------------------------------
//----------------------------------------------------------
#define SSWITCHGTX_SLAVE_FROM_DWRX_IDX		0
#define SSWITCHGTX_SLAVE_FROM_UPRX_IDX		1
#define SSWITCHGTX_SLAVE_FROM_MEM_IDX		2
#define SSWITCHGTX_SLAVE_FROM_HEA_IDX		3

#define SSWITCHGTX_MASTER_TO_DWTX_IDX		0
#define SSWITCHGTX_MASTER_TO_UPTX_IDX		1
//----------------------------------------------------------
//----------------------------------------------------------
#define SSWITCH256_SLAVE_FROM_MEM_IDX		0

#define SSWITCH256_MASTER_TO_GTX_IDX		0
#define SSWITCH256_MASTER_TO_512_IDX		1
//----------------------------------------------------------
//----------------------------------------------------------
#define SSWITCHGTX_UCI_SLAVE_FROM_EURXUP_IDX	0
#define SSWITCHGTX_UCI_SLAVE_FROM_SURXDN_IDX	1
#define SSWITCHGTX_UCI_SLAVE_FROM_MEM_IDX		2

#define SSWITCHGTX_UCI_MASTER_TO_EUTXUP_IDX		0
#define SSWITCHGTX_UCI_MASTER_TO_SUTXDN_IDX		1
//----------------------------------------------------------
//----------------------------------------------------------
#define DSP_DMA_SWITCH_TABLE_IDX	0
#define REGFIFO_SWITCH_TABLE_IDX	1
#define ZYNQGTX_SWITCH_TABLE_IDX	2
#define REMOTE_SWITCH_TABLE_IDX		3
#define NUM_SWITCHES				4

extern u8* stream_switch_base_addrs[];

int switch_remote_032_select_slave_get_command(u8 minibase_addr,u8 master_out, u8 slave_in,u8 disable,commnd_buffer_t *commnd_buf);

int switch_remote_256_select_slave_get_command(u8 minibase_addr,u8 master_out, u8 slave_in,u8 disable,commnd_buffer_t *commnd_buf);

int switch_remote_512_select_slave_get_command(u8 minibase_addr,u8 master_out, u8 slave_in,u8 disable,commnd_buffer_t *commnd_buf);

int switch_remote_032_update_get_command(u8 minibase_addr,commnd_buffer_t *commnd_buf);

int switch_remote_256_update_get_command(u8 minibase_addr,commnd_buffer_t *commnd_buf);

int switch_remote_512_update_get_command(u8 minibase_addr,commnd_buffer_t *commnd_buf);

int switch_set_master_2_slave_enable_connect(u8 *switch_baseaddress, u8 master_out, u8 slave_in,u8 disable);

int switch_disable_master(u8 *switch_baseaddress, u8 master_out);

int switch_reg_update(u8 *switch_baseaddress);
