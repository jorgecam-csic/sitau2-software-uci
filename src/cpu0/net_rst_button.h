/*
 * gpio.h
 *
 *  Created on: 23/03/2016
 *      Author: csic
 */

#ifndef GPIO_H_
#define NET_RST_BUTTON_H_

#include "xparameters.h"
#include "xil_types.h"
#include "global.h"

#define NET_RST_SWITCH_CHANNEL	2
#define NET_RST_SWITCH_MASK		0x4
#define NET_RST_SWITCH_GPIO_BASEADDRESS		XPAR_MISCELLANEOUS_GPIO_0_DEVICE_ID
//Del product guide del gpio:
#define NET_RST_SWITCH_TRISTATE_REG	(((NET_RST_SWITCH_CHANNEL-1)*2)+1)
#define NET_RST_SWITCH_DATAVAL_REG	(((NET_RST_SWITCH_CHANNEL-1)*2))

u32 net_rst_button(void);

#endif /* GPIO_H_ */
