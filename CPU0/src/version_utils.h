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

#define BRANCH_SW			1
#define VERSION_MAJOR_SW	0
#define VERSION_MINOR_SW	4
#define VERSION_DATE		"08/09/2025"
#define DESCRIPCION			"TCPCOM 1Gb prog lenta FPGAs"

//#define VERSION_CONTROL_INDEX		0

//#define VERSION_DATE_SIZE           24
//#define VERSION_DESCRIPTION_SIZE    24
//
//typedef struct T_Version
//{
//	u8 branch_sw;
//	u8 major_sw;
//	u8 minor_sw;
//	char date[VERSION_DATE_SIZE];
//	char description[VERSION_DESCRIPTION_SIZE];
//} T_Version;

void version_control_init(void);
void print_version(void);

#endif /* SRC_VERSION_UTILS_H_ */
