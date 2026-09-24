// -----------------------------------------------------------------------------
/**
 @file shared_mem.c

 @brief Este archivo contiene la implementación de las funciones de configuración
 y acceso de los encoders
 
 @author (rg) Ricardo González

 <pre>
 MODIFICATION HISTORY:

 Ver   Who  Date     Changes
 ----- ---- -------- -----------------------------------------------------------
 1.00a (rg) 13/03/2024 First release
 </pre>
*/
// -----------------------------------------------------------------------------
// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "shared_mem.h"
#include "shared_mem_def.h"
#include "version_utils.h"
#include "ACQ_common.h"
#include "uci_reg.h"
#include "timestamp.h"
#include "gtx_control.h"

TSHRD_0x00000002 *gb_shared = (TSHRD_0x00000002 *)SHRD_MEM;

// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//void SHRD_Init(void)
//{
//	int i, j;
//
//	xil_printf("\r\nResetting Shared Memory...", sizeof(TSHRD_0x00000001), (int)LAST_SHRD_KB_SIZE);
//	*((u32 *)SHRD_MEM_HEADER) = (u32)SHRD_INFO;
//	if (sizeof(TSHRD_0x00000001) >= LAST_SHRD_KB_SIZE) xil_printf("Error Overflow");
//	else xil_printf("OK");
//	xil_printf("\r\n   Used Shared Memory:      %d Bytes", sizeof(TSHRD_0x00000001));
//	xil_printf("\r\n   Available Shared Memory: %d Bytes", (int)LAST_SHRD_KB_SIZE);
//
//	gb_shared->cpu0.status = 0;
//	gb_shared->cpu0.flags.REG = 0;
//	for (i=0; i<SHRD_CPU0_INFO_SIZE; i++) gb_shared->cpu0.info[i] = 0;
//
//	gb_shared->cpu1.status = 0;
//	gb_shared->cpu1.flags.REG = 0;
//	for (i=0; i<SHRD_CPU1_INFO_SIZE; i++) gb_shared->cpu1.info[i] = 0;
//	gb_shared->cpu1.acquisition.time_stamp = 0;
//	for (i=0; i<SHRD_SIZE_ENCODERS; i++) gb_shared->cpu1.acquisition.encoder_value[i] = 0;
//	//gb_shared->cpu1.acquisition.trg_status = 0;
//
//	gb_shared->sw_version_cpu0.branch_sw = 0;
//	gb_shared->sw_version_cpu0.major_sw = 0;
//	gb_shared->sw_version_cpu0.minor_sw = 0;
//	gb_shared->sw_version_cpu0.date[0] = 0;
//	gb_shared->sw_version_cpu0.description[0] = 0;
//
//	gb_shared->sw_version_cpu1.branch_sw = 0;
//	gb_shared->sw_version_cpu1.major_sw = 0;
//	gb_shared->sw_version_cpu1.minor_sw = 0;
//	gb_shared->sw_version_cpu1.date[0] = 0;
//	gb_shared->sw_version_cpu1.description[0] = 0;
//
//	gb_shared->hw_version = 0;
//
//	//SHRD_ResetLog();
//}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
//void SHRD_ResetLog(void)
//{
//	int i, j;
//
//	gb_shared->log.count = 0;
//	for (i=0; i<SHRD_LOG_ERROR_SIZE; i++)
//	{
//		gb_shared->log.e[i].count = 0;
//		gb_shared->log.e[i].code = 0;
//		gb_shared->log.e[i].line = 0;
//		gb_shared->log.e[i].msg[0] = 0;
//		gb_shared->log.e[i].file[0] = 0;
//		for (j=0; j<SHRD_LOG_ERROR_SIZE; j++)
//		{
//			gb_shared->log.e[i].s[j].code = 0;
//			gb_shared->log.e[i].s[j].file[0] = 0;
//			gb_shared->log.e[i].s[j].line = 0;
//		}
//	}
//}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void SHRD_UpdateStatus(void)
{
	int i;
	u8 base_QSFP=1;
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	gtx_misc_fsm_ctrl_reg_t gtx_cltrl_reg;
	gtx_status_counters_reg_t gtx_counters_reg;

	//xil_printf("\r\nRead Status");
	gb_shared->cpu1.acquisition.time_stamp = timestamp_get_ticks_64();
	for (i=0; i<SHRD_SIZE_ENCODERS; i++) gb_shared->cpu1.acquisition.encoder_value[i] = 0;//encoder_value[i];
	gb_shared->cpu1.acquisition.status_code = 0xF005C51CDA5EDECA;
	gb_shared->cpu1.acquisition.rd_address = ACQ->read_addr_index;
	gb_shared->cpu1.acquisition.wr_address = ACQ->write_addr_index;
	gb_shared->cpu1.acquisition.n_overflows = ACQ->n_overflows;
	gb_shared->cpu1.acquisition.sitau_status = gb_sitau_status;
	gb_shared->cpu1.acquisition.trigger_source = gb_uci.trigger_source;
	gb_shared->cpu1.acquisition.n_acquisitions = gb_uci.n_acquisitions;
	gb_shared->cpu1.acquisition.ind_acquisitions = gb_uci.ind_acquisitions;//gb_uci.ind_acquisitions;
	if (gb_uci.status_pcie == 1 && gb_shared->cpu1.flags.BIT.MINI_BASE_LOADED == 1)
	{
		//xil_printf("\r\nRead Status PCIe");
		gtx_counters_reg = gtx_get_link_errors_dn(base_QSFP);
		gb_shared->qsfp_link_errors_dn.gtx_status_counters_reg_32 = gtx_counters_reg.gtx_status_counters_reg_32;
		gtx_counters_reg = gtx_get_link_errors_up(base_QSFP);
		gb_shared->qsfp_link_errors_up.gtx_status_counters_reg_32 = gtx_counters_reg.gtx_status_counters_reg_32;

		gtx_cltrl_reg=gtx_get_ctrl_reg(base_QSFP);
		gb_shared->qsfp_channel_rdy_up = gtx_cltrl_reg.BITS.gtx_rdy_up;
		gb_shared->qsfp_channel_rdy_dn = gtx_cltrl_reg.BITS.gtx_rdy_dn;
	}

}

//void SHRD_HW_Version(u32 data) { gb_shared->hw_version = data; };
void SHRD_CPU1_StatusEnabled(u32 data) { gb_shared->cpu1.flags.BIT.STATUS_ENABLED = data; }
void SHRD_CPU1_Loaded(u32 data) { gb_shared->cpu1.flags.BIT.CPU1_LOADED = data; }
void SHRD_CPU1_Started(u32 data) { gb_shared->cpu1.flags.BIT.CPU1_STARTED = data; }
void SHRD_CPU1_MiniBaseLoaded(u32 data) { gb_shared->cpu1.flags.BIT.MINI_BASE_LOADED = data; }
void SHRD_CPU1_Status(u32 data) { gb_shared->cpu1.status = data; }
void SHRD_CPU1_CacheEnabled(u32 data) { gb_shared->cpu1.flags.BIT.ENABLED_CACHE = data; }
void SHRD_CPU1_HighVoltage(u32 data) { gb_shared->cpu1.flags.BIT.HIGH_VOLTAGE = data; }
void SHRD_CPU1_FSM_Started(u32 data) { gb_shared->cpu1.flags.BIT.FSM_STARTED = data; }

//void SHRD_CPU0_PlatformError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_PLATFORM = data; }
//void SHRD_CPU0_NetworkError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_NETWORK = data; }
//void SHRD_CPU0_GICError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_GIC = data; }
//void SHRD_CPU0_MUTEXError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_MUTEX = data; }
//void SHRD_CPU0_CacheEnabled(u32 data) { gb_shared->cpu0.flags.BIT.ENABLED_CACHE = data; }
//void SHRD_CPU0_ImageEnabled(u32 data) { gb_shared->cpu0.flags.BIT.ENABLED_IMAGE = data; }
