/*
 * version_utils.c
 *
 *  Created on: 11 de sept. de 2017
 *      Author: csic
 */

#include "version_utils.h"
#include "shared_mem.h"

//T_Version This_SW_version_struct = (T_Version) {BRANCH_SW,VERSION_MAJOR_SW,VERSION_MINOR_SW,VERSION_DATE,DESCRIPCION};

//T_Version *Version_control_array = (T_Version *) SHRD_MEM_VERSION;

void version_control_init(void)
{
	gb_shared->sw_version_cpu0.branch_sw = BRANCH_SW;
	gb_shared->sw_version_cpu0.major_sw = VERSION_MAJOR_SW;
	gb_shared->sw_version_cpu0.minor_sw = VERSION_MINOR_SW;
	strcpy(gb_shared->sw_version_cpu0.date, (char *)VERSION_DATE);
	strcpy(gb_shared->sw_version_cpu0.description, (char *)DESCRIPCION);
	//Version_control_array[VERSION_CONTROL_INDEX] = This_SW_version_struct;
}

void print_version(void)
{
	xil_printf("\r\n\r\n%s Software Version v%d.%d.%d - %s\r\n",
			gb_shared->sw_version_cpu0.description,
			gb_shared->sw_version_cpu0.branch_sw,
			gb_shared->sw_version_cpu0.major_sw,
			gb_shared->sw_version_cpu0.minor_sw,
			gb_shared->sw_version_cpu0.date);

//			Version_control_array[VERSION_CONTROL_INDEX].description,
//			Version_control_array[VERSION_CONTROL_INDEX].branch_sw,
//			Version_control_array[VERSION_CONTROL_INDEX].major_sw,
//			Version_control_array[VERSION_CONTROL_INDEX].minor_sw,
//			Version_control_array[VERSION_CONTROL_INDEX].date);
}
