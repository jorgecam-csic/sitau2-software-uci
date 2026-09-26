/*
 * gpio.c
 *
 *  Created on: 23/03/2016
 *      Author: csic
 */
#include "net_rst_button.h"

u32 net_rst_button(void)
{
	u32 *base_gpio=NET_RST_SWITCH_GPIO_BASEADDRESS;
	u32 tmp_tristate;
	u32 data_value;
	tmp_tristate=base_gpio[NET_RST_SWITCH_TRISTATE_REG];
	base_gpio[NET_RST_SWITCH_TRISTATE_REG]=tmp_tristate|NET_RST_SWITCH_MASK; //Se pone a uno el bit de triestado, seleccionando el pin como entrada.
	data_value=base_gpio[NET_RST_SWITCH_DATAVAL_REG]&NET_RST_SWITCH_MASK;
	base_gpio[NET_RST_SWITCH_TRISTATE_REG]=tmp_tristate;

	return data_value;
}

