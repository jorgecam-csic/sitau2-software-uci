/*
 * net_params.c
 *
 *  Created on: 23/03/2016
 *      Author: csic
 */
#include "net_params.h"
#include "string.h"
#include "net_rst_button.h"


struct network_parameters_s Net_param;

const u8 Const_Addr_IP_default[4]			= ADDR_IP_DEFAULT;
const u8 Const_Gateway_default[4]			= GATEWAY_IP_DEFAULT;
const u8 Const_Mask_default[4]				= MASK_IP_DEFAULT;
const u8 Const_MAC_eth_addr[6]				= MAC_ADDR_DEFAULT;


u32 get_network_reset_button(void)
{
	return (net_rst_button());
}

void network_parameters_to_default(void)
{
	Net_param.signature_head			= HEAD_DEFAULT;
	memcpy(&(Net_param.mac_addr)		,Const_MAC_eth_addr				,6*(sizeof(u8)));
	Net_param.options					= OPTIONS_DEFAULT;
	memcpy(&(Net_param.ip_addr)			,Const_Addr_IP_default			,4*(sizeof(u8)));
	memcpy(&(Net_param.gateway)			,Const_Gateway_default			,4*(sizeof(u8)));
	memcpy(&(Net_param.mask)			,Const_Mask_default				,4*(sizeof(u8)));

	Net_param.conf_port					= CONF_PORT_DEFAULT;
	Net_param.cmmd_port		 			= CMMD_PORT_DEFAULT;
	Net_param.data_port 				= DATA_PORT_DEFAULT;
	Net_param.signature_tail			= TAIL_DEFAULT;
}

void print_net_param(const struct network_parameters_s *p_net_params)
{
	xil_printf("\r\n");
	xil_printf("HEADER:\t\t\t%04X\r\n",p_net_params->signature_head);

	xil_printf("MAC:\t\t\t%02x:%02x:%02x:%02x:%02x:%02x\r\n",p_net_params->mac_addr[0],
										  	  	  	  	  	 p_net_params->mac_addr[1],
										  	  	  	  	  	 p_net_params->mac_addr[2],
										  	  	  	  	  	 p_net_params->mac_addr[3],
										  	  	  	  		 p_net_params->mac_addr[4],
										  	  	  	  	  	 p_net_params->mac_addr[5]);
	xil_printf("Options:\t\t%04X\r\n",p_net_params->options);


	xil_printf("IP:\t\t\t%3u.%3u.%3u.%3u\r\n",p_net_params->ip_addr[0],
										  	  p_net_params->ip_addr[1],
										  	  p_net_params->ip_addr[2],
										  	  p_net_params->ip_addr[3]);

	xil_printf("Gateway:\t\t%3u.%3u.%3u.%3u\r\n",p_net_params->gateway[0],
										     	 p_net_params->gateway[1],
										     	 p_net_params->gateway[2],
										     	 p_net_params->gateway[3]);

	xil_printf("Mask:\t\t\t%3u.%3u.%3u.%3u\r\n",p_net_params->mask[0],
										    	p_net_params->mask[1],
										    	p_net_params->mask[2],
										    	p_net_params->mask[3]);

	xil_printf("CONF TCP port:\t\t%u\r\n",p_net_params->conf_port);
	xil_printf("CMMD TCP port:\t\t%u\r\n",p_net_params->cmmd_port);
	xil_printf("DATA TCP port:\t\t%u\r\n",p_net_params->data_port);

	xil_printf("TAIL:\t\t\t%04X\r\n",p_net_params->signature_tail);
	xil_printf("\r\n");
}

void init_network_parameters(void)
{
	if(!get_network_reset_button())
	{
		xil_printf("Default network parameters switch ON\r\n");
		network_parameters_to_default();
	}
	else
	{
		FlashRead_simple(NETWORK_PARAMETERS_FLASH_ADDR,(void*) &Net_param, sizeof(Net_param));
		if(Net_param.signature_head!=HEAD_DEFAULT||Net_param.signature_tail!=TAIL_DEFAULT)
		{
			xil_printf("Error in saved network parameters, loading default\r\n");
			network_parameters_to_default();
		}
	}
	print_net_param(&Net_param);
}


