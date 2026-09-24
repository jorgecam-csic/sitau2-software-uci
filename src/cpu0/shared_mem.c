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
#include "shared_mem_type.h"
#include "version_utils.h"

TSHRD_0x00000002 *gb_shared = (TSHRD_0x00000002 *)SHRD_MEM;

// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void SHRD_Init(void)
{
	xil_printf("\r\nResetting Shared Memory...", sizeof(TSHRD_0x00000002), (int)LAST_SHRD_KB_SIZE);
	memset((u8 *)SHRD_MEM_HEADER, 0, LAST_SHRD_KB_SIZE);
	*((u32 *)SHRD_MEM_HEADER) = (u32)SHRD_INFO;
	if (sizeof(TSHRD_0x00000002) >= LAST_SHRD_KB_SIZE) xil_printf("Error Status Overflow");
	else xil_printf("OK");
	gb_shared->check_word = 0x1234;
	gb_shared->check_byte = 0x5A;
	xil_printf("\r\n   Used Shared Memory:      %d Bytes", sizeof(TSHRD_0x00000002));
	xil_printf("\r\n   Available Shared Memory: %d Bytes", (int)LAST_SHRD_KB_SIZE);


//	gb_shared->cpu0.status = 0;
//	gb_shared->cpu0.flags.REG = 0;
//	for (i=0; i<SHRD_CPU0_INFO_SIZE; i++) gb_shared->cpu0.info[i] = 0;
//
//	gb_shared->cpu1.status = 0;
//	gb_shared->cpu1.flags.REG = 0;
//	for (i=0; i<SHRD_CPU1_INFO_SIZE; i++) gb_shared->cpu1.info[i] = 0;
//	gb_shared->cpu1.acquisition.time_stamp = 0;
//	for (i=0; i<SHRD_SIZE_ENCODERS; i++) gb_shared->cpu1.acquisition.encoder_value[i] = 0;
//	gb_shared->cpu1.acquisition.trg_status = 0;
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
//	SHRD_ResetLog();
}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void SHRD_ResetLog(void)
{
	int i, j;

	gb_shared->log.count = 0;
	for (i=0; i<SHRD_LOG_ERROR_SIZE; i++)
	{
		gb_shared->log.e[i].count = 0;
		gb_shared->log.e[i].code = 0;
		gb_shared->log.e[i].line = 0;
		gb_shared->log.e[i].msg[0] = 0;
		gb_shared->log.e[i].file[0] = 0;
		for (j=0; j<SHRD_LOG_ERROR_SIZE; j++)
		{
			gb_shared->log.e[i].s[j].code = 0;
			gb_shared->log.e[i].s[j].file[0] = 0;
			gb_shared->log.e[i].s[j].line = 0;
		}
	}
}
// ----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void SHRD_HW_Version(u32 data) { gb_shared->hw_version = data; };
void SHRD_CPU0_PlatformError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_PLATFORM = data; }
void SHRD_CPU0_NetworkError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_NETWORK = data; }
void SHRD_CPU0_GICError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_GIC = data; }
void SHRD_CPU0_MUTEXError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_MUTEX = data; }
void SHRD_CPU0_CacheEnabled(u32 data) { gb_shared->cpu0.flags.BIT.ENABLED_CACHE = data; }
void SHRD_CPU0_ImageEnabled(u32 data) { gb_shared->cpu0.flags.BIT.ENABLED_IMAGE = data; }
//void SHRD_SetTriggerInfo(u32 time_stamp, u32 *encoder_value, u32 trg_status)
//{
//	int i;
//
//	gb_shared->cpu1.acquisition.time_stamp = time_stamp;
//	for (i=0; i<SHRD_SIZE_ENCODERS; i++) gb_shared->cpu1.acquisition.encoder_value[i] = encoder_value[i];
//	gb_shared->cpu1.acquisition.trg_status = trg_status;
//}
//void SHRD_HW_Version(u32 data) { gb_shared->hw_version = data; };
////void SHRD_CPU1_Loaded(u32 data) { gb_shared->cpu1.flags.BIT.CPU1_LOADED = data; }
////void SHRD_CPU1_Started(u32 data) { gb_shared->cpu1.flags.BIT.CPU1_STARTED = data; }
////void SHRD_CPU1_MiniBaseLoaded(u32 data) {gb_shared->cpu1.flags.BIT.MINI_BASE_LOADED = data;}
////void SHRD_CPU1_Status(u32 data) { gb_shared->cpu1.status = data; }
////void SHRD_CPU1_CacheEnabled(u32 data) { gb_shared->cpu1.flags.BIT.ENABLED_CACHE = data; }
////void SHRD_CPU1_HighVoltage(u32 data) { gb_shared->cpu1.flags.BIT.HIGH_VOLTAGE = data; }
////void SHRD_CPU1_FSM_Started(u32 data) { gb_shared->cpu1.flags.BIT.FSM_STARTED = data; }
//
//void SHRD_CPU0_PlatformError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_PLATFORM = data; }
//void SHRD_CPU0_NetworkError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_NETWORK = data; }
//void SHRD_CPU0_GICError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_GIC = data; }
//void SHRD_CPU0_MUTEXError(u32 data) { gb_shared->cpu0.flags.BIT.ERROR_MUTEX = data; }
//void SHRD_CPU0_CacheEnabled(u32 data) { gb_shared->cpu0.flags.BIT.ENABLED_CACHE = data; }
//void SHRD_CPU0_ImageEnabled(u32 data) { gb_shared->cpu0.flags.BIT.ENABLED_IMAGE = data; }
