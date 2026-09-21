/*
 * Copyright (c) 2007 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#define XILINX_SDK_APP
//#define PRUEBA_INI_AUTOFOCUS
#define __arm__ 1

#include <stdio.h>
#include "xparameters.h"
#include "xil_printf.h"
#include "qspi_simple.h"
#include "xscutimer.h"
#define USAR_ETHERNET
#ifdef USAR_ETHERNET
#include "platform.h"
#include "platform_config.h"
#include "network.h"
#include "tcpcom.h"
#include "qspi_simple.h"
#endif

#include "cons_prod_util.h"
#include "gic_utils.h"
#include "mutex_utils.h"
#include "mmu_config.h"

#include "version_utils.h"
#include "shared_mem.h"

const char Version_sw[] = "Net_link flashed 13-11-2024";
#define SDK_RELEASE_YEAR	2022
#define SDK_RELEASE_QUARTER	2

int gb_cache_enabled = 1;


#define MUTEX_MAIN_INIT_SEMAPHORE_ID	(MUTEX_START_NUMBER+MAX_CONS_PROD_STRUCTS)


void desactivar_fw_bit(void)
{

	/* Write to ACTLR */
	__asm__("mrc	p15, 0, r0, c1, c0, 1		/* Read ACTLR*/ \r\n\
			 bic	r0, r0, #(0x01)				/* unset FW bit */\r\n\
			 mcr	p15, 0, r0, c1, c0, 1		/* Write ACTLR*/\r\n");
}

int prueba_envio()
{

	char caracter;
	cons_prod_t *cons_prod_HOST2UCI_p;
	char cadena[]="hola monos!!!";
	int num_datos,num_datos_max;

	cons_prod_HOST2UCI_p=prod_init(HOST2UCI_BUF_ID);

	for(;;)
	{
	   caracter=inbyte();
	   outbyte(caracter);
	   xil_printf("\r\n");
	   if(caracter=='h' || caracter == 'H')
	   {
		   num_datos_max=prod_num_data_to_write(cons_prod_HOST2UCI_p);
		   xil_printf("Puedo escribir %d datos\r\n",num_datos);
		   num_datos=strlen(cadena)+1;
		   if(num_datos<=num_datos_max)xil_printf("Bien, puedo escribir los %d datos de la cadena: %s!!\r\n",num_datos,cadena);
		   {
			   prod_memcpy(cons_prod_HOST2UCI_p,num_datos,cadena);
			   prod_refresh_write(cons_prod_HOST2UCI_p,num_datos);
		   }
	   }


	   if (caracter=='x' || caracter == 'X') break;
	}

	xil_printf("Saliendo del programa.\r\n");
	return XST_SUCCESS;

}
static void Data_Abort_Handler_mod (void)
{
	xil_printf("DATA_ABORT_HANDLER MODIFICADO!!!\r\n");

}

int main()
{
	u16 velocidad_ethernet;
	SHRD_Init();
	version_control_init();

	xil_printf("\r\n");

	print_version();

	xil_printf("Iniciando CPU0.\r\n");
	xil_printf("Net LINK Release %d.%d	%s-%s\r\n", SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER, __DATE__,__TIME__);
	xil_printf("Versión del SW de la CPU0: %s.\r\n",Version_sw);

#ifdef TCP_HEADER_FLUSH
	xil_printf("User LWIP Library %d.\r\n",TCP_HEADER_FLUSH);
#endif

	InitQspi();

	if (gic_utils_init())
	{
		SHRD_CPU0_GICError(1);
		xil_printf("\r\nProblema en el inicio del GIC\r\n");
	}

	if (mutex_utils_init())
	{
		SHRD_CPU0_MUTEXError(1);
		xil_printf("\r\nProblema en el inicio del Mutex\r\n");
	}

	cons_prod_init_structs();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT, (Xil_ExceptionHandler)Data_Abort_Handler_mod, (void *) 0);

	xil_printf("\r\nConfigurando MMU...");
	cpu0_net_mmu_config();
	xil_printf("OK");

	desactivar_fw_bit();

	if (init_platform() < 0)
	{
		SHRD_CPU0_PlatformError(1);
		xil_printf("ERROR initializing platform.\r\n");
		return -1;
	}
#ifdef USAR_ETHERNET

	if (init_network() < 0)
	{
		SHRD_CPU0_NetworkError(1);
		xil_printf("ERROR initializing network.\r\n");
		return -1;
	}

//	velocidad_ethernet=monitor_ethernet_link();
//
//	if(velocidad_ethernet != 1000)
//	{
//		xil_printf("Link error, only 1000Mbps its allowed, connected at %d Mbps\n\r",velocidad_ethernet);
//		sleep(5);
//		rst_sys(NULL);
//	}

	while(1)
	{
		network_loop();
	}
#else
	while(1);
#endif

    /* never reached */
    cleanup_platform();
}

