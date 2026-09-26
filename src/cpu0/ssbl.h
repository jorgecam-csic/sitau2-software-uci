/*
 * sslb.h
 *
 *  Created on: 17/03/2016
 *      Author: csic
 */

#ifndef SSLB_H_
#define SSLB_H_
#include <xil_types.h>

#define MAX_BITSTREAM_SIZE_MB 32

#define MAX_APPS	8

#define MOD_BOOT_S				1
#define SLCR_UNLOCK_REG			0xF8000008
#define SLCR_UNLOCK_CODE		0x0000DF0D
#define SLCR_LOCK_REG			0xF8000004
#define SLCR_LOCK_CODE			0xF800767B
#define MOD_BOOT_CPU1STARTADDR	0x00000024
#define NORMAL_CPU1STARTADDR	0xFFFFFFF0
#define A9_CPU_RST_CTRL_REG		0xF8000244
#define A9_RST1_MASK			0x00000002
#define FPGA_RST_CTRL_REG		0x00000240
#define FPGA0_OUT_RST_MASK		0x0000000F

#define sev() __asm__("sev")

u8 ssbl_ddr(u32 ImageBaseaddress);
//extern u32 AppStartAddr[];


#endif /* SSLB_H_ */
