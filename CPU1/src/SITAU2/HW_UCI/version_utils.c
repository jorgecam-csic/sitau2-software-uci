/*
 * version_utils.c
 *
 *  Created on: 11 de sept. de 2017
 *      Author: csic
 */

#include "version_utils.h"

//T_Version This_SW_version_struct = (T_Version) {CPU1_BRANCH_SW,CPU1_VERSION_MAJOR_SW,CPU1_VERSION_MINOR_SW,CPU1_VERSION_DATE,CPU1_DESCRIPCION};

//T_Version *Version_control_array = (T_Version *) SHRD_MEM_VERSION;

void version_control_init(void)
{
	//Version_control_array[VERSION_CONTROL_CPU1] = This_SW_version_struct;
	gb_shared->sw_version_cpu1.branch_sw = CPU1_BRANCH_SW;
	gb_shared->sw_version_cpu1.major_sw = CPU1_VERSION_MAJOR_SW;
	gb_shared->sw_version_cpu1.minor_sw = CPU1_VERSION_MINOR_SW;
	strcpy(gb_shared->sw_version_cpu1.date, (char *)CPU1_VERSION_DATE);
	strcpy(gb_shared->sw_version_cpu1.description, (char *)CPU1_DESCRIPCION);

	gb_shared->hw_version = get_HW_version();
}

void print_version(void)
{
//	xil_printf("\r\n%8X",Version_control_array);
//	xil_printf("\r\n\r\n%s Version v%d.%d.%d - %s",
//	Version_control_array[VERSION_CONTROL_FSBL].description,
//	Version_control_array[VERSION_CONTROL_FSBL].branch_sw,
//	Version_control_array[VERSION_CONTROL_FSBL].major_sw,
//	Version_control_array[VERSION_CONTROL_FSBL].minor_sw,
//	Version_control_array[VERSION_CONTROL_FSBL].date);

	xil_printf("\r\n\r\n%s Software Version v%d.%d.%d - %s\r\n",
			gb_shared->sw_version_cpu0.description,
			gb_shared->sw_version_cpu0.branch_sw,
			gb_shared->sw_version_cpu0.major_sw,
			gb_shared->sw_version_cpu0.minor_sw,
			gb_shared->sw_version_cpu0.date);

	xil_printf("\r\n\r\n%s Software Version v%d.%d.%d - %s\r\n",
			gb_shared->sw_version_cpu1.description,
			gb_shared->sw_version_cpu1.branch_sw,
			gb_shared->sw_version_cpu1.major_sw,
			gb_shared->sw_version_cpu1.minor_sw,
			gb_shared->sw_version_cpu1.date);

	xil_printf("\r\nHardware Version v0x%X\r\n", gb_shared->hw_version);
//	xil_printf("\r\n%s Version v%d.%d.%d - %s",
//	Version_control_array[VERSION_CONTROL_CPU0].description,
//	Version_control_array[VERSION_CONTROL_CPU0].branch_sw,
//	Version_control_array[VERSION_CONTROL_CPU0].major_sw,
//	Version_control_array[VERSION_CONTROL_CPU0].minor_sw,
//	Version_control_array[VERSION_CONTROL_CPU0].date);
//
//	xil_printf("\r\n%s Version v%d.%d.%d - %s",
//	Version_control_array[VERSION_CONTROL_CPU1].description,
//	Version_control_array[VERSION_CONTROL_CPU1].branch_sw,
//	Version_control_array[VERSION_CONTROL_CPU1].major_sw,
//	Version_control_array[VERSION_CONTROL_CPU1].minor_sw,
//	Version_control_array[VERSION_CONTROL_CPU1].date);

	//xil_printf("\r\nHardware Version v0x%X\r\n", get_HW_version());
}
u32 get_HW_version(void)
{
	u32 *Hw_version_reg=(u32*)XPAR_VERSION_HW_BASEADDR;
	return *Hw_version_reg;
}
