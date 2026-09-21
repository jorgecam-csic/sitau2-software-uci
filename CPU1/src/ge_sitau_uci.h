/*
 * ge_sitau_uci.h
 *
 *  Created on: 21/11/2016
 *      Author: csic
 */

#ifndef GE_SITAU_UCI_H_
#define GE_SITAU_UCI_H_

#include "mcbcc_mst_driver.h"
#include "sleep.h"
#include "global.h"
#include "timer_util.h"
#include "datamover_driver.h"

u32 uci_fsm_decode(u8* data_ref,u32 bytes_ref);
void usleep_timer(u32 us);

#define dbg_uci_printf void_printf
//#define dbg_uci_printf xil_printf

#endif /* GE_SITAU_UCI_H_ */
