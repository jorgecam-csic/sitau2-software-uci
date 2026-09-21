/*
 * global.h
 *
 *  Created on: 23/03/2016
 *      Author: csic
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#include "encoder.h"
#define HILBERT 1


#define MSG_LOG2_AVRG

#define SITAU2
#define DDR_MINIBASE_START_ADDR	0x80000000
#define DDR_MINIBASE_SIZE		0X40000000
#define DDR_MINIBASE_LAST_ADDR	0XBFFFFFFF
#define DDR_MINIBASE_ALIGN_SIZE 16


#define PI	3.14159265359


#define CEILING(x,y) (((x) + (y) - 1) / (y))

#define CACHELINE_BYTE_SIZE		64
#define THIS_CPU_ID				XPAR_CPU_ID
#define THIS_CPU_GIC_MASK		(1<<XPAR_CPU_ID)

extern int gb_hw_sitau_enabled;

void version_control_init(void);

void void_printf( const char *ctrl1, ...);
#define dbg_printf	void_printf
//#define dbg_printf	xil_printf

#include "cons_prod_util.h"

extern cons_prod_t *UCI2HOST_prod_p;
extern cons_prod_t *HOST2UCI_cons_p;
extern cons_prod_t* IMAGE_prod_p;
//#define WFI_main()	__asm__("nop")
#define WFI_main()	__asm__("wfi")
//#define WFI_dma()	__asm__("nop")
#define WFI_dma()	__asm__("wfi")
#define WFI_timer()	__asm__("nop")
//#define WFI_timer()	__asm__("wfi")
//#define WFI_trigger()	__asm__("nop")
#define WFI_trigger()	__asm__("wfi")
#define WFI_bcc()	__asm__("wfi")
//#define WFI_bcc()	__asm__("nop")

//#define WFI()	__asm__("wfi")
#define WFE()	__asm__("wfe")
#define DMB()	__asm__("dmb")
#define DSB()	__asm__("dsb")
#define ISB()	__asm__("isb")

void usleep_for(int numero_absurdo);
void usleep_timer(u32 us);

#endif /* GLOBAL_H_ */
