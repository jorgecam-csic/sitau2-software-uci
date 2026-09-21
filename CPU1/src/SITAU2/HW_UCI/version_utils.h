/*
 * version_utils.h
 *
 *  Created on: 11 de sept. de 2017
 *      Author: csic
 */

#ifndef SRC_VERSION_UTILS_H_
#define SRC_VERSION_UTILS_H_

#include "xil_types.h"
#include "shared_mem_def.h"
#include "xparameters.h"

#define CPU1_BRANCH_SW			2
#define CPU1_VERSION_MAJOR_SW	2
#define CPU1_VERSION_MINOR_SW	1
#define CPU1_VERSION_DATE		"24/03/2026"
#define CPU1_DESCRIPCION		"UCI: PWI, TFM continuo y imagen de fotoacustica."

#define MK32_FILT_FMC_PCIE_MSK	0x80


void version_control_init(void);
void print_version(void);

u32 get_HW_version(void);

#endif
