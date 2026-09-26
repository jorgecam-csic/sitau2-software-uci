/*
 * tcpcom.h
 *
 *  Created on: 05/09/2014
 *      Author: csic
 */

#ifndef TCPCOM_H_
#define TCPCOM_H_
#include <stdio.h>
#include <string.h>

#include "lwip/err.h"
#include "lwip/tcp.h"
#include "tcpcom.h"
#include "platform.h"
#include "xscutimer.h"
#include "xil_io.h"
#include "ssbl.h"
#include "xil_exception.h"
#include "qspi_simple.h"

#include <xil_types.h>
#include <xil_cache.h>
#include "net_params.h"
#include "cons_prod_util.h"
#include "tcpimage.h"
#include "mmu_config.h"

#define PUT_MEM			0x0100
#define DBG_FUNCTION	0x0101
#define GET_MEM			0x0102
#define LOAD_N_RUN		0x0103
#define WRITE_QSPI		0x0104
#define READ_QSPI		0x0105
#define RST_SYS			0x0106
#define CURR_NET_PARAM	0x0107
#define SAVE_NET_PARAM	0x0108
#define FLSH_NET_PARAM	0x0109
#define UCI_SEND		0x010A
#define UCI_RECV		0x010B
#define UCI_C_S_F_R		0x010C
#define UCI_C_S_B_R		0x010D
#define UCI_UCI2HOST_N	0x010E
#define IMAGE_CLEAR_BUF	0x010F
#define CACHE_ENABLED	0x0110
#define CACHE_DISABLED	0x0111
#define IMAGE_DISABLED	0x0112
#define IMAGE_ENABLED	0x0113
#define GET_SHARED_MEM	0x0114

extern u8* buffer_imagen;

#define CMD_HEADER		0XC51CDA5E
#define COM_MIN_SIZE	12
int transfer_tcpcom_data(void);
int start_tcpcom_application(void);
void print_tcpcom_app_header(void);
u16 uci_send(u8*command,u32 bytes_left_on_buffer);
int uci_recv(u8* command);
int uci_clear_send_block_recv(u8* command);
int uci_uci2host_n_data(u8* command);
int tcp_image_clear_buffer(void);

#endif /* TCPCOM_H_ */
