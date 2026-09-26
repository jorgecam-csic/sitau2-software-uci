/*
 * network.h
 *
 *  Created on: 05/09/2014
 *      Author: csic
 */

#ifndef NETWORK_H_
#define NETWORK_H_

#include <stdio.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "platform_config.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "config_net_apps.h"
#include "lwip/inet.h"
#include "lwip/ip_addr.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "tcpcom.h"
#include "tcpimage.h"
#include "net_params.h"
#include "xemacps.h"  // O "xemaclite.h", dependiendo del controlador
#include "lwip/etharp.h"

u16 monitor_ethernet_link(void);

void renegotiate_link(struct netif *netif);

void network_loop(void);

int init_network(void);

#endif /* NETWORK_H_ */
