/*
 * axi2gtx.h
 *
 *  Created on: 23 ago. 2022
 *      Author: csic
 */

#ifndef SRC_SITAU2_GTX_AXI2GTX_H_
#define SRC_SITAU2_GTX_AXI2GTX_H_

#include "xil_types.h"

int axi2gtx_fifo_init();
int axi2gtx_fifo_send(u32  *SourceAddr, u32 Numwords);

#endif /* SRC_SITAU2_GTX_AXI2GTX_H_ */
