/*
 * timestamp.h
 *
 *  Created on: 4 de jul. de 2017
 *      Author: csic
 */

#ifndef SRC_TIMESTAMP_H_
#define SRC_TIMESTAMP_H_

#include "xparameters.h"
#include "xscugic.h"
#include "xil_types.h"
#include "gic_utils.h"

#define TIMESTAMP_RST_CHANNEL	2
#define TIMESTAMP_RST_MASK		0x2
#define TIMESTAMP_RST_GPIO_BASEADDRESS		XPAR_AXI_GPIO_0_BASEADDR
//Del product guide del gpio:
#define TIMESTAMP_RST_TRISTATE_REG	(((TIMESTAMP_RST_CHANNEL-1)*2)+1)
#define TIMESTAMP_RST_DATAVAL_REG	(((TIMESTAMP_RST_CHANNEL-1)*2))
#define TIMESTAMP_RST_INTENA_REG	(XGPIO_IER_OFFSET/4)
#define TIMESTAMP_RST_INTSTS_REG	(XGPIO_ISR_OFFSET/4)
#define TIMESTAMP_RST_INTGLO_REG	(XGPIO_GIE_OFFSET/4)

void timestamp_rst(void);
void timestamp_init(void);
void timestamp_rst_int_function();
void timestamp_get_ticks(u32* least_significant_word32,u32* most_significant_word32);
u64 timestamp_get_ticks_64(void);
float timestamp_get_us_float(u64 ini, u64 end);
u32 timestamp_get_ms_32(u64 ini, u64 end);


#endif /* SRC_TIMESTAMP_H_ */
