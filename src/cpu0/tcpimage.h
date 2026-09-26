/*
 * tcpimage.h
 *
 *  Created on: 23 de feb. de 2017
 *      Author: csic
 */

#ifndef SRC_TCPIMAGE_H_
#define SRC_TCPIMAGE_H_

#include <stdio.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/tcp.h"
#include "tcpcom.h"
#include "platform.h"
#include "xscutimer.h"
#include "xil_io.h"

#include <xil_types.h>
#include <xil_cache.h>
#include "net_params.h"
#include "cons_prod_util.h"
#include "version_utils.h"

int start_tcpimage_application();
void print_tcpimage_app_header();
int transfer_tcpimage_data();

#endif /* SRC_TCPIMAGE_H_ */
