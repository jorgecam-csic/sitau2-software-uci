/*
 * axil2stream_fifo_driver.h
 *
 *  Created on: 15 sept. 2020
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_AXIL2STREAM_AXIL2STREAM_FIFO_DRIVER_H_
#define SRC_SITAU2_AXIL2STREAM_AXIL2STREAM_FIFO_DRIVER_H_

#include "xil_types.h"

int axil2stream_fifo_init();
int axil2stream_fifo_send(u32  *SourceAddr, u32 Numwords);
int axil2stream_fifo_fill(u32  data, u32 Numwords);

#endif /* SRC_SITAU2_AXIL2STREAM_AXIL2STREAM_FIFO_DRIVER_H_ */
