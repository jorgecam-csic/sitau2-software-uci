/*
 * net_params.h
 *
 *  Created on: 23/03/2016
 *      Author: csic
 */

#ifndef NET_PARAMS_H_
#define NET_PARAMS_H_


#include "xil_types.h"
#include "global.h"
#include "qspi_simple.h"

#ifdef TARGET_ZEDBOARD
//ULTIMA PÁGINA DE LA MEM QSPI
#define NETWORK_PARAMETERS_FLASH_ADDR	((16*1024*1024)-SECTOR_SIZE)
#endif
#define RESET_BUTTON_GPIO_CHANNEL	1
#define RESET_BUTTON_MASK			0x1
#if 0
#define ADDR_IP_DEFAULT		{161, 111,  29, 177}
#define GATEWAY_IP_DEFAULT	{161, 111,  29,   1}
#define MASK_IP_DEFAULT		{255, 255, 255,   0}
#else
#define ADDR_IP_DEFAULT		{192, 168,   2,  10}
#define GATEWAY_IP_DEFAULT	{192, 168,   2,   1}
#define MASK_IP_DEFAULT		{255, 255, 255,   0}
#endif

#define MAC_ADDR_DEFAULT	{ 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 }

#define CONF_PORT_DEFAULT	5002
#define CMMD_PORT_DEFAULT	6002
#define DATA_PORT_DEFAULT	6008
#define CMMD_PORT_DEFAULT	55902//0xDA5E, sustituye a 6002
#define DATA_PORT_DEFAULT	50460//0xC51C, sustituye a 6008
#define DHCP_ENABLE_DEFAULT	0
#define HEAD_DEFAULT		0xC51C
#define TAIL_DEFAULT		0xDA5E
#define OPTIONS_DEFAULT		(DHCP_ENABLE_DEFAULT&0x1<<0)

/*
 * #define CONF_PORT_DEFAULT	5002
#define CMMD_PORT_DEFAULT	55902//6002
#define DATA_PORT_DEFAULT	50460//6008
#define HEAD_DEFAULT		0xC51
*/

#define DHCP_OPTION_MASK	0x0001

struct network_parameters_s
{
	// Signature
	u16 signature_head;
	// MAC_PARAMETERS
	u8 mac_addr[6];

	//IP_PARAMETERS
	u16 options;
	u8 ip_addr[4];
	u8 gateway[4];
	u8 mask[4];

	// TCP_PARAMETERS
	u16 conf_port;
	u16 cmmd_port;
	u16 data_port;
	// Signature
	u16 signature_tail;

};

extern struct network_parameters_s Net_param;
void network_parameters_to_default(void);
void init_network_parameters(void);
void print_net_param(const struct network_parameters_s *p_net_params);

#endif /* NET_PARAMS_H_ */
