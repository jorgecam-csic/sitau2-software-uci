/*
 * global.h
 *
 *  Created on: 23/03/2016
 *      Author: csic
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#define TARGET_ZEDBOARD
#define CEILING(x,y) (((x) + (y) - 1) / (y))

#define CACHELINE_BYTE_SIZE		64

#define THIS_CPU_ID				XPAR_CPU_ID
#define THIS_CPU_GIC_MASK		(1<<XPAR_CPU_ID)

#define WFI()	__asm__("wfi")
#define WFE()	__asm__("wfe")

#define DMB()	__asm__("dmb")
#define DSB()	__asm__("dsb")
#define ISB()	__asm__("isb")

#define dbg_printf	xil_printf

extern int gb_cache_enabled;

#endif /* GLOBAL_H_ */
