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
#include "tcpcom.h"


#define	DEBUG_PROBLEMA_HEADER 1
#define NUM_VOLCADO 10

cons_prod_t *cons_prod_HOST2UCI_p;
cons_prod_t *cons_prod_UCI2HOST_p;

volatile char uci2host_data_available=0;


static unsigned tcpcom_server_running = 0;

unsigned int Total_bytes_recibidos=0;
unsigned int Total_MB_recibidos=0;

unsigned int Total_bytes_enviados=0;
unsigned int Total_MB_enviados=0;

#if (USE_JUMBO_FRAMES==1)
#define SEND_BUFSIZE (9000)
#else
#define SEND_BUFSIZE (1400)
#endif

#define RECV_BUFSIZE	(8*1024*1024)
static struct tcp_pcb *Connected_pcb = NULL;
volatile extern int TxPerfConnMonCntr;

//static char send_buf[SEND_BUFSIZE];
static u8 recv_buf[RECV_BUFSIZE];
u8* Direccion_pcb_comandos=NULL;

#define MAX_MEGAS_IMAGEN	64
#define MAX_TAM_IMAGEN 		(MAX_MEGAS_IMAGEN*1024*1024/4)

u32 buffer_imagen_u32[MAX_TAM_IMAGEN];

u8 *buffer_imagen=(u8*)buffer_imagen_u32;

static u32	Rx_large_left = 0;
static u8*	Rx_large_left_curr_addr = NULL;
static u8	Rx_rqt_ack = 0;

static u32	Tx_large_left = 0;
static u8*	Tx_large_left_curr_addr = NULL;
static u32  Tranfering_uci2host_num_data = 0;
static u8 Connected = 0;

static u32 Rx_host2uci_total_data_left = 0;

#define BUFFER_INTERMEDIO	0
#if BUFFER_INTERMEDIO == 1
u8 mid_buffer[32*1024*1024];
#endif
#define SEND_SIZE	1400

void uci2host_buffer_interrupt_function(void)
{
	//xil_printf("INT CPU1 to CPU0\r\n");
	uci2host_data_available=1;
}

void close_tcpcom_and_disconnect(void)
{
	u32 num_datos_to_read;
	Connected_pcb=NULL;
	Connected = 0;
	Rx_large_left = 0;
	Tx_large_left = 0;
	Tranfering_uci2host_num_data = 0;
	uci2host_data_available=0;
	num_datos_to_read=cons_num_data_to_read(cons_prod_UCI2HOST_p);
	cons_refresh_read(cons_prod_UCI2HOST_p,num_datos_to_read);
}
int transfer_tcpcom_data()
{
#if __arm__
	u8 copy = 1;
	u8 more = 0;
#else
	int copy = 0;
	int more = 0;
#endif
	err_t err;

	struct tcp_pcb *tpcb = Connected_pcb;
	int btt,abtt;


	if (!Connected_pcb)
		return ERR_OK;

	if(Tx_large_left==0)
	{
		if (uci2host_data_available==0)
			return ERR_OK;
		else if (uci2host_data_available!=0)
		{
			int num_datos_to_read;
			uci2host_data_available=0;
			num_datos_to_read=cons_num_data_to_read(cons_prod_UCI2HOST_p);
			if(num_datos_to_read==0)
				return ERR_OK;
			Tranfering_uci2host_num_data = cons_memref(cons_prod_UCI2HOST_p,num_datos_to_read,&Tx_large_left_curr_addr);
			if(Tranfering_uci2host_num_data!=num_datos_to_read)
				uci2host_data_available = 1;
			Tx_large_left=Tranfering_uci2host_num_data;
		}
	}

	abtt=tcp_sndbuf(tpcb);
	//xil_printf("abtt = %d\r\n",abtt);
	if(abtt>Tx_large_left)
	{
		//xil_printf("\r\n%d",Tx_large_left);
		err = tcp_write(tpcb, Tx_large_left_curr_addr, Tx_large_left, copy|more);

		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_write a: %d\r\n", err);
			close_tcpcom_and_disconnect();
			return -1;
		}

		err = tcp_output(tpcb);
		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_output b: %d\r\n",err);
		}
		//xil_printf("Envío masivo: Tx_large_left = %d\r\n",Tx_large_left);
		Tx_large_left=0;
	}
	else if(abtt > SEND_SIZE)
	{
		while ((abtt > SEND_SIZE) && (Tx_large_left>0))
		{
			btt = (SEND_SIZE>Tx_large_left?Tx_large_left:SEND_SIZE);
			//more = (SEND_SIZE>Tx_large_left?0:TCP_WRITE_FLAG_MORE);
			err = tcp_write(tpcb, Tx_large_left_curr_addr, btt, copy|more);
			if (err != ERR_OK)
			{
				xil_printf("tcpcom: Error on tcp_write c: %d\r\n", err);
				close_tcpcom_and_disconnect();
				return -1;
			}

			err = tcp_output(tpcb);
			if (err != ERR_OK)
			{
				xil_printf("tcpcom: Error on tcp_output d: %d\r\n",err);
			}
			Tx_large_left-=btt;
			Tx_large_left_curr_addr=&Tx_large_left_curr_addr[btt];
			abtt=tcp_sndbuf(tpcb);
		}
	}

	else if ((TxPerfConnMonCntr == 20) && (Tx_large_left>0)) // Si la velocidad es demasiado lenta se envia lo posible...
	{
		if(abtt==0) return ERR_OK;
		btt = (abtt>Tx_large_left?Tx_large_left:abtt);
		//more = (SEND_SIZE>Tx_large_left?0:TCP_WRITE_FLAG_MORE);
		err = tcp_write(tpcb, Tx_large_left_curr_addr, btt, copy|more<<1);
		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_write e: %d\r\n", err);
			close_tcpcom_and_disconnect();
			return -1;
		}

		err = tcp_output(tpcb);
		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_output f: %d\r\n",err);
		}
		Tx_large_left-=btt;
		Tx_large_left_curr_addr=&Tx_large_left_curr_addr[btt];
	}
//	else
//		xil_printf("\r\nabtt: %d - Tx_large_left: %d",abtt,Tx_large_left);

	//xil_printf("Tx_large_left=%d\r\n",Tx_large_left);
	if(Tx_large_left==0)
	{
		//xil_printf("Envío largo realizado con éxito: Tranfering_uci2host_num_data = %d.\r\n",Tranfering_uci2host_num_data);
		if(Tranfering_uci2host_num_data)
		{
			cons_refresh_read(cons_prod_UCI2HOST_p,Tranfering_uci2host_num_data);
			Tranfering_uci2host_num_data=0;
		}
	}
    return ERR_OK;
}

static err_t tcpcom_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	TxPerfConnMonCntr = 0;
	transfer_tcpcom_data();
	return ERR_OK;
}
int dbg_function()
{
	/*
	   timer_ini=XScuTimer_GetCounterReg(XPAR_PS7_SCUTIMER_0_BASEADDR);
		//for(i=0;i<(128*128/32/16);i++)
		{
			//XFermat_top_Start(&Fermat_instance);
			//while(!XFermat_top_IsDone(&Fermat_instance));
		}
		timer_fin=XScuTimer_GetCounterReg(XPAR_PS7_SCUTIMER_0_BASEADDR);

		us_transcurridos=(int)(timer_ini-timer_fin)*1e6/(XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ/2);
		xil_printf("Tiempo transcurrido HW: %d us\r\n",us_transcurridos);


		//fermat_copy_sw_to_hw_parameters();
		//fermat_sw_load_params();

		timer_ini=XScuTimer_GetCounterReg(XPAR_PS7_SCUTIMER_0_BASEADDR);
		//for(i=0;i<(128*128/32/16);i++)
		{
			//fermat_sw_call();
		}
		timer_fin=XScuTimer_GetCounterReg(XPAR_PS7_SCUTIMER_0_BASEADDR);
		//fermat_sw_get_results();
		us_transcurridos=(int)(timer_ini-timer_fin)*1e6/(XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ/2);
		xil_printf("Tiempo transcurrido SW: %d us\r\n",us_transcurridos);
	*/


	xil_printf("dbg_function end\r\n");
	return 0;

}

int rst_sys(u8 *comando)
{
	int i;
	struct tcp_pcb *tpcb = Connected_pcb;
	tcp_close(tpcb);
	xil_printf("\r\nReiniciando el sistema...\r\n");
	for(i=0;i<10000000;i++); //Un retardo para que se vea el último mensaje
	Xil_Out32(0XF8000008,0x0000DF0D);	//Desbloqueo Registros slcr
	Xil_Out32(0xF8000200,1);			//Restart todo el sistema!!
	return 0;
}

int read_qspi_flash(u8 *command)
{
	u32 flash_address;
	u32 bytecount;
	u32 image_addr;

	memcpy(&image_addr,command,sizeof(u32));
	command+=sizeof(u32);

	if(image_addr==(u32)0xFFFFFFFF)
		image_addr = (u32)buffer_imagen;

	memcpy(&flash_address,command,sizeof(u32));
	command+=sizeof(u32);

	memcpy(&bytecount,command,sizeof(u32));
	command+=sizeof(u32);
    Xil_ExceptionDisableMask(XIL_EXCEPTION_IRQ);
	FlashRead_simple(flash_address,(u8*)image_addr, bytecount);
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	return 12;
}

int write_qspi_flash(u8 *command)
{
	u32 flash_address;
	u32 bytecount;
	u32 image_addr;
	int ret;


	memcpy(&image_addr,command,sizeof(u32));
	command+=sizeof(u32);

	if(image_addr==(u32)0xFFFFFFFF)
		image_addr = (u32)buffer_imagen;

	memcpy(&flash_address,command,sizeof(u32));
	command+=sizeof(u32);

	memcpy(&bytecount,command,sizeof(u32));
	command+=sizeof(u32);

	xil_printf("Borrando %d bytes desde la posición 0x%08X la flash...\r\n",bytecount,flash_address);
	ret=FlashErase_simple(flash_address, bytecount);
	xil_printf("Borrado terminado con código %d.\r\n",ret);
	xil_printf("Escribiendo %d bytes desde la posición 0x%08X la flash...\r\n",bytecount,flash_address);
	ret=FlashWrite_simple(flash_address, (u8*)image_addr ,bytecount);
	xil_printf("Escritura terminada con código %d.\r\n",ret);

	return 12;
}

int load_n_run_image(u8 *command)
{
	u8 num_apps;
	struct tcp_pcb *tpcb = Connected_pcb;
	u32 image_addr;
    //tcp_close(tpcb);
    Xil_ExceptionDisableMask(XIL_EXCEPTION_IRQ);

	memcpy(&image_addr,command,sizeof(u32));
	command+=sizeof(u32);

	if(image_addr==(u32)0xFFFFFFFF)
		image_addr = (u32)buffer_imagen;

    xil_printf("\r\ntcpcom: Desconexión por parte del servidor");
    num_apps=ssbl_ddr(image_addr);
    Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
    xil_printf("%d aplicaciones encontradas en la imagen, ninguna cargada en CPU0.\r\n",num_apps);
    Xil_DCacheEnable();
    Xil_ICacheEnable();
    if(num_apps)
    {
    	cons_prod_init_structs();
    }
	//SHR_CPU0_Status(UCI_LOADED);
	return 4;
}

int tcpcom_send(u8* ptr, u32 bytes,u32 id)
{
	int abtt;
	err_t err;
	struct tcp_pcb *tpcb = Connected_pcb;
	u32 respuesta[4];
	int more_data_flags = 3;
	int send_now_flags = 1;

	if(Tx_large_left>0)
	{
		xil_printf("get_mem: Llamada a get_mem habiendo un envío en trámite, operación no realizada.\r\n");
		return -1;
	}
	//xil_printf("Llamada get_mem\r\n");
	if(Tx_large_left>0)
	{
		xil_printf("get_mem: Llamada a get_mem habiendo un envío en trámite, operación no realizada.\r\n");
		return 0;
	}

	if((u32)ptr==(u32)0xFFFFFFFF)
		Tx_large_left_curr_addr = (u8*)buffer_imagen;
	else
		Tx_large_left_curr_addr = (u8*)ptr;

	Tx_large_left = bytes;

	abtt = tcp_sndbuf(tpcb);
	if(abtt>20) // Se envia una pequeña cabecera con la ID que nos han enviado para que el emisor pueda reconocer el envío
	{
		int flags;
		if(abtt>=20+Tx_large_left)
			flags=more_data_flags;
		else
			flags=send_now_flags;

		respuesta[0] = (u32)CMD_HEADER;
		respuesta[1] = (u32)16;
		respuesta[2] = (u32)id;
		respuesta[3] = (u32)bytes;
		err = tcp_write(tpcb, respuesta, 16, flags);
		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_write: %d\r\n", err);
			Connected_pcb = NULL;
			return -1;
		}

		err = tcp_output(tpcb);
		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_output: %d\r\n",err);
		}
	}
	else
	{
		xil_printf("TCP no está listo para enviar la cabecera, se aborta el envio.\r\n");
		return -1;
	}
	// Protección por si se accede a un recurso AXI_LITE (si no esta en ddr se copia a un buffer) Mejor no usar esto
//	if((Tx_large_left_curr_addr<XPAR_PS7_DDR_0_S_AXI_BASEADDR) || ((Tx_large_left_curr_addr+Tx_large_left)>XPAR_PS7_AFI_0_S_AXI_HIGHADDR))
//	{
//		memcpy(ddr_buffer,Tx_large_left_curr_addr,Tx_large_left);
//		Tx_large_left_curr_addr = ddr_buffer;
//	}
	// Si lo que se quiere enviar cabe de una tacada se envía directamente (para poder hacer varios envíos rápidos)
	abtt = tcp_sndbuf(tpcb);
	if (abtt >= Tx_large_left)
	{
		err = tcp_write(tpcb, Tx_large_left_curr_addr, Tx_large_left, send_now_flags);
		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_write g: %d\r\n", err);
			close_tcpcom_and_disconnect();
			return -1;
		}
		Tx_large_left = 0;
		err = tcp_output(tpcb);
		if (err != ERR_OK)
		{
			xil_printf("tcpcom: Error on tcp_output: %d\r\n",err);
		}
		return -1;
	}
	return 0;
}


u16 get_mem(u8*command,u16 bytes_left)
{
	u32 u32_tmp;
	u8 *data_ptr;
	u32 bytes;
	u32 id;

	memcpy(&u32_tmp,command,sizeof(u32));
	data_ptr=(u8*)u32_tmp;
	command+=sizeof(u32);

	if((u32)data_ptr==(u32)0xFFFFFFFF)
		data_ptr = (u8*)buffer_imagen;

	memcpy(&bytes,command,sizeof(u32));
	command+=sizeof(u32);

	memcpy(&id,command,sizeof(u32));
	command+=sizeof(u32);

	tcpcom_send(data_ptr,bytes,id);

	return 12;
}

u16 get_shared_mem(u8 *command)
{
	u32 id;
	u8 d1, d2, d3, d4;
	u8 *ptr = (u8 *)SHRD_MEM_HEADER;

	d1=ptr[0];
	d2=ptr[1];
	d3=ptr[2];
	d4=ptr[3];
	memcpy(&id,command,sizeof(u32));
	tcpcom_send(ptr,LAST_SHRD_KB_SIZE,id);
	SHRD_ResetLog();
	//xil_printf("\nRead Status: CPU1_LOADED(%d)",gb_shared->cpu1.flags.BIT.CPU1_LOADED);
	return 4;
}

int tcpcom_recv(u8* data_dst,u8* data_org, u32 bytes,u32 bytes_left_on_buffer)
{
	if(Rx_large_left>0 || Rx_host2uci_total_data_left>0)
	{
		xil_printf("Llamada a tcpcom_recv habiendo un envío en trámite.\r\n");
		for(;;);
	}

	Rx_large_left_curr_addr=data_dst;
	Rx_large_left = bytes;

	if(Rx_large_left>bytes_left_on_buffer)	// En este caso consumimos el buffer y la transacción no está completa
	{
		memcpy(Rx_large_left_curr_addr,data_org,bytes_left_on_buffer);
		Rx_large_left_curr_addr=&Rx_large_left_curr_addr[bytes_left_on_buffer];
		Rx_large_left -= bytes_left_on_buffer;
		return bytes_left_on_buffer;
	}
	else	// En este caso la transacción está completa y pueden quedar otros comandos en el buffer
	{
		memcpy(Rx_large_left_curr_addr,data_org,Rx_large_left);
		Rx_large_left = 0;
		//xil_printf("Recepción larga realizada con éxito.\r\n");
		return bytes;
	}
}


u16 put_mem(u8*command,u32 bytes_left_on_buffer)
{
	u32 u32_tmp;
	u8 *data_dst;
	u32 bytes;

	memcpy(&u32_tmp,command,sizeof(u32));
	data_dst=(u8*)u32_tmp;
	command+=sizeof(u32);

	if((u32)data_dst==(u32)0xFFFFFFFF)
		data_dst = (u8*)buffer_imagen;

	memcpy(&bytes,command,sizeof(u32));
	command+=sizeof(u32);

	return 8+tcpcom_recv(data_dst, command, bytes,bytes_left_on_buffer-8);

}
int current_net_params(u8*command)
{
	u32 id;

	memcpy(&id,command,sizeof(u32));
	command+=sizeof(u32);

	tcpcom_send((u8*)&Net_param,sizeof(Net_param),id);

	return 4;
}

int save_net_params(u8*command,u32 bytes_left_on_buffer)
{
	int bytes_read_from_buffer;
	int bytes;
	struct network_parameters_s net_param_tmp;

	memcpy(&bytes,command,sizeof(u32));
	command+=sizeof(u32);

	if(bytes!=sizeof(struct network_parameters_s))
	{
		xil_printf("Error de tamaño de estructura de parámetros de red\r\n");
		return 4+bytes;
	}
	bytes_read_from_buffer=tcpcom_recv((u8*)&net_param_tmp, command, bytes,bytes_left_on_buffer-4);
	FlashErase_simple(NETWORK_PARAMETERS_FLASH_ADDR, sizeof(net_param_tmp));
	FlashWrite_simple(NETWORK_PARAMETERS_FLASH_ADDR,(void*) &net_param_tmp, sizeof(net_param_tmp));

	return 4+bytes_read_from_buffer;
}
int flash_net_params(u8*command)
{
	struct network_parameters_s net_param_tmp;
	u32 id;

	memcpy(&id,command,sizeof(u32));
	command+=sizeof(u32);
	FlashRead_simple(NETWORK_PARAMETERS_FLASH_ADDR,(void*) &net_param_tmp, sizeof(net_param_tmp));
	tcpcom_send((u8*)&net_param_tmp,sizeof(net_param_tmp),id);

	return 4;
}
int uci_clear()
{
	u32 bytes;


	bytes=cons_num_data_to_read(cons_prod_UCI2HOST_p);

	cons_refresh_read(cons_prod_UCI2HOST_p,bytes);

	return 0;
}



u16 uci_send(u8*command,u32 bytes_left_on_buffer)
{
	u32 u32_tmp,num_datos_max_2_uci;
	u8 *data_dst;
	u32 bytes_to_copy;
	u32 total_bytes_to_send;


	if(Rx_large_left || Rx_host2uci_total_data_left)
	{
		xil_printf("Llamada a uci_send habiendo un envío en trámite.\r\n");
		for(;;);
	}

	memcpy(&total_bytes_to_send,command,sizeof(u32));
	command+=sizeof(u32);
	bytes_left_on_buffer-=4;

	//xil_printf("Llamada a uci_send: bytes_left_on_buffer = %d,total_bytes_to_send = %d\r\n",bytes_left_on_buffer,total_bytes_to_send);


	num_datos_max_2_uci=prod_num_data_to_write(cons_prod_HOST2UCI_p);
	while(num_datos_max_2_uci<total_bytes_to_send)
	{
		//Si pasa esto es que estamos generando más datos de los que puede generar la UCI, se puede considerar una situación de error.
		xil_printf("\r\nError, en comando UCI_SEND, no hay sitio en el buffer hacia la UCI:\r\nnum_datos_max_2_uci = %d, total_bytes_to_send=%d",num_datos_max_2_uci,total_bytes_to_send);
		WFI();	//Espero sin hacer nada. Como tarde, llega la interrupción de la red, cada 250 ms
		num_datos_max_2_uci=prod_num_data_to_write(cons_prod_HOST2UCI_p);
	}
	if(total_bytes_to_send<=bytes_left_on_buffer)
		bytes_to_copy=total_bytes_to_send;
	else
		bytes_to_copy=bytes_left_on_buffer;

	if(bytes_to_copy)
	{
		prod_memcpy(cons_prod_HOST2UCI_p,bytes_to_copy,command);
		prod_refresh_write(cons_prod_HOST2UCI_p,bytes_to_copy);
	}

	Rx_host2uci_total_data_left=total_bytes_to_send-bytes_to_copy;


	return 4+bytes_to_copy;

}


int uci_recv(u8* command)
{
	u32 n_data_uci_2_host;
	u32 max_n_data_uci_2_host;
	u32 data_sent_to_host,data_to_send_now,data_left;

	memcpy(&n_data_uci_2_host,command,sizeof(u32));
	command+=sizeof(u32);

	data_sent_to_host=0;
	while(data_sent_to_host==n_data_uci_2_host)
	{
		uci2host_data_available=0; //Pongo a 0 porque lo voy a leer ahora mismo.
		max_n_data_uci_2_host=cons_num_data_to_read(cons_prod_UCI2HOST_p);
		if(max_n_data_uci_2_host==0)
			while(!uci2host_data_available); //AQUÍ SE ESPERA A LA INTERRUPCIÓN!!
		else
		{
			data_left=n_data_uci_2_host-data_sent_to_host;
			data_to_send_now=(max_n_data_uci_2_host>data_left?data_left:max_n_data_uci_2_host);
			cons_memcpy(cons_prod_UCI2HOST_p,data_to_send_now,&buffer_imagen[data_sent_to_host]);
			cons_refresh_read(cons_prod_UCI2HOST_p,data_to_send_now);
			data_sent_to_host+=data_to_send_now;
		}
	}
	return 4;

}
int uci_clear_send_fast_recv(u8* command)
{
	u32 n_data_host_2_uci,n_data_uci_2_host;
	u32 max_n_data_host_2_uci,max_n_data_uci_2_host;
	u32 data_sent_to_host,data_to_send_now,data_left;
	u32 id;


	memcpy(&n_data_host_2_uci,command,sizeof(u32));
	command+=sizeof(u32);

	memcpy(&n_data_uci_2_host,command,sizeof(u32));
	command+=sizeof(u32);

	memcpy(&id,command,sizeof(u32));
	command+=sizeof(u32);

	// Primero borro lo que se haya podido recibir hasta ahora
	max_n_data_uci_2_host=cons_num_data_to_read(cons_prod_UCI2HOST_p);
	cons_refresh_read(cons_prod_UCI2HOST_p,max_n_data_uci_2_host);

	// ahora envío a la UCI
	max_n_data_host_2_uci=prod_num_data_to_write(cons_prod_HOST2UCI_p);

	while(max_n_data_host_2_uci<n_data_host_2_uci)
	{
		dbg_printf("Error, en comando UCI_C_S_B_R, no hay sitio en el buffer hacia la UCI:\r\nnum_datos_max_2_uci = %d, bytes=%d\r\n",max_n_data_host_2_uci,n_data_host_2_uci);
		//usleep(2);
		max_n_data_host_2_uci=prod_num_data_to_write(cons_prod_HOST2UCI_p);
	}
	// Se lo envío (los datos se han enviado previamente al buffer)
	prod_memcpy(cons_prod_HOST2UCI_p,n_data_host_2_uci,buffer_imagen);
	// Lo hago efectivo
	prod_refresh_write(cons_prod_HOST2UCI_p,n_data_host_2_uci);
	// Y ahora esperamos recibir los datos de vuelta
	data_sent_to_host=0;
	while(data_sent_to_host<n_data_uci_2_host)
	{
		uci2host_data_available=0; //Pongo a 0 porque lo voy a leer ahora mismo.
		max_n_data_uci_2_host=cons_num_data_to_read(cons_prod_UCI2HOST_p);
		if(max_n_data_uci_2_host==0)
			while(!uci2host_data_available); //AQUÍ SE ESPERA A LA INTERRUPCIÓN!!
		else
		{
			data_left=n_data_uci_2_host-data_sent_to_host;
			data_to_send_now=(max_n_data_uci_2_host>data_left?data_left:max_n_data_uci_2_host);

			cons_memcpy(cons_prod_UCI2HOST_p,data_to_send_now,&buffer_imagen[data_sent_to_host]);
			cons_refresh_read(cons_prod_UCI2HOST_p,data_to_send_now);
			data_sent_to_host+=data_to_send_now;
		}
	}

	if(0)
	{
		int i;
		xil_printf("\r\nRespuesta: ");
		for(i=0;i<n_data_uci_2_host&&i<100;i++)
			xil_printf("%02X ",buffer_imagen[i]);
		xil_printf("\r\n");
	}

	tcpcom_send(buffer_imagen,n_data_uci_2_host,id);

	return 12;
}
int uci_uci2host_n_data(u8* command)
{
	u32 max_n_data_uci_2_host;
	u32 id;

	memcpy(&id,command,sizeof(u32));
	command+=sizeof(u32);
	max_n_data_uci_2_host=cons_num_data_to_read(cons_prod_UCI2HOST_p);
	tcpcom_send((u8*)&max_n_data_uci_2_host,sizeof(max_n_data_uci_2_host),id);
	return 4;
}
int SetCacheEnabled(void)
{
	gb_cache_enabled = 1;
	cpu0_net_mmu_config();
	SHRD_CPU0_CacheEnabled(1);
	xil_printf("\r\nCACHE ENABLED");
	return 0;
}
int SetCacheDisabled(void)
{
	gb_cache_enabled = 0;
	cpu0_net_mmu_config_RED();
	SHRD_CPU0_CacheEnabled(0);
	xil_printf("\r\nCACHE DISABLED");
	return 0;
}

u32 decode_cmd(u8* command,u32 bytes_left)
{
	u32 u32_tmp;
	u32 cmd_header;
	u32 cmd_length;
	u32 cmd_code;
	static u32 num_command=0;

	if(bytes_left>0xFFFF)
		xil_printf("DEBUG: bytes_left = %u\r\n",bytes_left);
	if(bytes_left<8)
	{
		xil_printf("DEBUG: Tamaño insuficiente para la cabecera y la cantidad de datos bytes_left = %u\r\n",bytes_left);
		return 0;
	}
#if DEBUG_PROBLEMA_HEADER
	static u32 cmd_header_anterior;
	static u32 cmd_length_anterior;
	static u32 cmd_code_anterior;

#endif

	memcpy(&u32_tmp,command,sizeof(u32));
	cmd_header=u32_tmp;
	command+=sizeof(u32);

	memcpy(&u32_tmp,command,sizeof(u32));
	cmd_length=u32_tmp;
	command+=sizeof(u32);

	memcpy(&u32_tmp,command,sizeof(u32));
	cmd_code=u32_tmp;
	command+=sizeof(u32);


	if(cmd_header!=CMD_HEADER)
	{
		u8* temp;

		xil_printf("ERROR, no se encuentra cabecera de comando:\r\n");
		xil_printf("\r\n");
		xil_printf("bytes_left = %08X\r\n",bytes_left);
		xil_printf("\r\n");
#if DEBUG_PROBLEMA_HEADER
		{
			int i;
			static int num_prints=0;
			num_prints++;
			if(num_prints<10)
			{
				xil_printf("\r\n");
				xil_printf("cmd_header_anterior 0x%08X\r\n",cmd_header_anterior);
				xil_printf("cmd_length_anterior 0x%08X\r\n",cmd_length_anterior);
				xil_printf("cmd_code_anterior   0x%08X\r\n",cmd_code_anterior);
				xil_printf("\r\n");
				xil_printf("cmd_header 0x%08X\r\n",cmd_header);
				xil_printf("cmd_length 0x%08X\r\n",cmd_length);
				xil_printf("cmd_code   0x%08X\r\n",cmd_code);
				xil_printf("\r\n");
				xil_printf("PUNTEROS:\r\n");
				xil_printf("recv_buf: 0x%08X\r\n",(u32)recv_buf);
				xil_printf("command : 0x%08X\r\n",(u32)command);
				xil_printf("\r\n");
				xil_printf("VOLCADO:\r\n");
				xil_printf("recv_buf:\r\n");
				temp=recv_buf;
				for(i=-NUM_VOLCADO;i<NUM_VOLCADO;i++)
					xil_printf("%02X ",*(temp+i));
				xil_printf("\r\n");
				xil_printf("command : \r\n");
				temp=command;
				for(i=-NUM_VOLCADO;i<NUM_VOLCADO;i++)
					xil_printf("%02X ",*(temp+i));
				xil_printf("\r\n");
			}
			else
				xil_printf("Pausa aquí\r\n");
#if DEBUG_PROBLEMA_HEADER
	cmd_header_anterior = cmd_header;
	cmd_length_anterior = cmd_length;
	cmd_code_anterior	= cmd_code;
#endif
		}
#endif
		return bytes_left;
	}

	if(bytes_left<cmd_length)
	{
		xil_printf("DEBUG: Comando incompleto bytes_left = %u < cmd_length = %u, comando=0x%08X, num_command:%d\r\n",bytes_left,cmd_length,cmd_code, num_command);
		return 0;
	}

#if DEBUG_PROBLEMA_HEADER
	cmd_header_anterior = cmd_header;
	cmd_length_anterior = cmd_length;
	cmd_code_anterior	= cmd_code;
#endif
	num_command++;
	switch(cmd_code)
	{
	case PUT_MEM:
		return COM_MIN_SIZE+put_mem(command,bytes_left-COM_MIN_SIZE);
		break;
	case DBG_FUNCTION:
		return COM_MIN_SIZE+dbg_function();
		break;
	case GET_MEM:
		return COM_MIN_SIZE+get_mem(command,bytes_left-COM_MIN_SIZE);
		break;
	case LOAD_N_RUN:
		return COM_MIN_SIZE+load_n_run_image(command);
		break;
	case WRITE_QSPI:
		return COM_MIN_SIZE+write_qspi_flash(command);
		break;
	case READ_QSPI:
		return COM_MIN_SIZE+read_qspi_flash(command);
		break;
	case RST_SYS:
		return COM_MIN_SIZE+rst_sys(command);
		break;
	case CURR_NET_PARAM:
		return COM_MIN_SIZE + current_net_params(command);
		break;
	case SAVE_NET_PARAM:
		return COM_MIN_SIZE+save_net_params(command,bytes_left-COM_MIN_SIZE);
		break;
	case FLSH_NET_PARAM:
		return COM_MIN_SIZE+flash_net_params(command);
		break;
	case UCI_SEND:
		//xil_printf("UCI_SEND\r\n");
		return COM_MIN_SIZE+uci_send(command,bytes_left-COM_MIN_SIZE);
		break;
	case UCI_RECV:
		return COM_MIN_SIZE+uci_recv(command);
		break;
	case UCI_C_S_F_R:
		return COM_MIN_SIZE+uci_clear_send_fast_recv(command);
		break;
	case UCI_C_S_B_R:
		uci_clear();
		return COM_MIN_SIZE+uci_send(command,bytes_left-COM_MIN_SIZE);// NO ES UN ERROR, ESTE COMANDO ES IGUAL QUE UCI_SEND, PERO ANTES BORRA EL BUFFER UCI2HOST
		break;
	case UCI_UCI2HOST_N:
		return COM_MIN_SIZE+uci_uci2host_n_data(command);
		break;
	case IMAGE_CLEAR_BUF:
		//return COM_MIN_SIZE;
		return COM_MIN_SIZE+tcp_image_clear_buffer();
		break;
    case CACHE_ENABLED:
        return COM_MIN_SIZE + SetCacheEnabled();
        break;      
    case CACHE_DISABLED:
        return COM_MIN_SIZE + SetCacheDisabled();
        break;
    case IMAGE_DISABLED:
        return COM_MIN_SIZE + tcp_image_buffer_disabled();
        break;      
    case IMAGE_ENABLED:
        return COM_MIN_SIZE + tcp_image_buffer_enabled();
        break;
    case GET_SHARED_MEM:
        return COM_MIN_SIZE + get_shared_mem(command);
        break;
	}
	xil_printf("Comando desconocido.\r\n");

	return cmd_length;

}
void flush_pbuf(struct pbuf *p_base)
{
	struct pbuf *pbuf_ptr=p_base;

	while(pbuf_ptr != NULL)
	{
		/*Xil_DCacheFlushRange(pbuf_ptr,sizeof(struct pbuf));
		Xil_DCacheFlushRange(pbuf_ptr->payload,pbuf_ptr->len);
		Xil_DCacheFlushLine(pbuf_ptr+sizeof(struct pbuf));
		Xil_DCacheFlushLine(pbuf_ptr->payload+pbuf_ptr->len);*/
		Xil_DCacheInvalidateRange(pbuf_ptr,sizeof(struct pbuf));
		Xil_DCacheInvalidateRange(pbuf_ptr->payload,pbuf_ptr->len);
		Xil_DCacheInvalidateLine(pbuf_ptr+sizeof(struct pbuf));
		Xil_DCacheInvalidateLine(pbuf_ptr->payload+pbuf_ptr->len);
		pbuf_ptr=pbuf_ptr->next;
	}
}
static err_t tcpcom_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	u16 total_bytes;
	u16 copy_len;
	struct pbuf buf;
	u32 next = (u32) 1;
	u8 *rx_buffer_ptr_temp;
	u8 *ptr_temp;
	u32 cmd_data_length=0;
	u32 bytes_left_on_buffer;
	u32 decode_cmd_ret,offset=0;
	static u32 bytes_mal=0;
	u32 total_data_sent_to_UCI=0;
	static u32 previous_bytes_left=0;
	struct pbuf *p_buf_ptr;


    /* close socket if the peer has sent the FIN packet  */
    if (p == NULL)
    {
        tcp_close(tpcb);
        xil_printf("\r\ntcpcom: Desconexión por parte del cliente");
        return ERR_OK;
    }
    if(err!=ERR_OK)
    	xil_printf("\r\nERROR (tcpcom_recv_callback)!!");

    //tcp_recved(tpcb, p->tot_len);
    flush_pbuf(p);

    p_buf_ptr = p;
    total_bytes = p_buf_ptr->tot_len;
    rx_buffer_ptr_temp = (u8*) &recv_buf[previous_bytes_left];

    //xil_printf("Rx_large_left = %d,total_bytes = %d\r\n",Rx_large_left,total_bytes);
    if(Rx_large_left) // Si estábamos a mitad de una recepción, seguimos con ello sin pasar por buffer intermedio
    {
    	do
    	{
    		copy_len = (p_buf_ptr->len>Rx_large_left?Rx_large_left:p_buf_ptr->len);


    		memcpy(Rx_large_left_curr_addr, p_buf_ptr->payload, copy_len);


    		if(p_buf_ptr->len>Rx_large_left) // En este caso, detrás del envío largo, había algo más, se pone en el buffer de recepción
    		{
    			ptr_temp = (u8*)p_buf_ptr->payload;
    			ptr_temp = &ptr_temp[copy_len];
    			memcpy(rx_buffer_ptr_temp, ptr_temp, p_buf_ptr->len-copy_len);
    			rx_buffer_ptr_temp+=p_buf_ptr->len-copy_len;
    			cmd_data_length += (u32)p_buf_ptr->len-copy_len;
    		}
    		Rx_large_left-=copy_len;
    		Rx_large_left_curr_addr=&Rx_large_left_curr_addr[copy_len];
    		next = (u32) p_buf_ptr->next;
    		p_buf_ptr = p_buf_ptr->next;

    		if(Rx_large_left==0)
    		{
    			xil_printf("FIN RECEPCIÓN LARGA.\r\n");
    			break;
    		}
    	}while (next != (u32) NULL);

    	if(Rx_large_left==0)
    	{
    		xil_printf("Rx_large_left = 0\r\n");
    		Rx_rqt_ack=1;
    	}
    }
    else if(Rx_host2uci_total_data_left)
    {
    	total_data_sent_to_UCI = 0;
    	do
    	{
    		copy_len = (p_buf_ptr->len>Rx_host2uci_total_data_left?Rx_host2uci_total_data_left:p_buf_ptr->len);

    		prod_memcpy(cons_prod_HOST2UCI_p,copy_len,p_buf_ptr->payload);
    		prod_refresh_write(cons_prod_HOST2UCI_p,copy_len);

    		total_data_sent_to_UCI+=copy_len;

    		if(p_buf_ptr->len>Rx_host2uci_total_data_left) // En este caso, detrás del envío largo, había algo más, se pone en el buffer de recepción
    		{
    			ptr_temp = (u8*)p_buf_ptr->payload;
    			ptr_temp = &ptr_temp[copy_len];
    			memcpy(rx_buffer_ptr_temp, ptr_temp, p_buf_ptr->len-copy_len);
    			rx_buffer_ptr_temp+=p_buf_ptr->len-copy_len;
    			cmd_data_length += (u32)p_buf_ptr->len-copy_len;
    		}

    		Rx_host2uci_total_data_left-=copy_len;
    		next = (u32) p_buf_ptr->next;
    		p_buf_ptr = p_buf_ptr->next;

    		if(Rx_host2uci_total_data_left==0)
    		{
    			//xil_printf("Rx_host2uci_total_data_left = 0.\r\n");
    			break;
    		}
    	}while (next != (u32) NULL);
    }

    while (next != (u32) NULL)
	{
 		memcpy(rx_buffer_ptr_temp, p_buf_ptr->payload, p_buf_ptr->len);
		rx_buffer_ptr_temp+=p_buf_ptr->len;
		cmd_data_length += (u32)p_buf_ptr->len;
		next = (u32) p_buf_ptr->next;
		p_buf_ptr = p_buf_ptr->next;
	}

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    bytes_left_on_buffer=cmd_data_length+previous_bytes_left;
    while(bytes_left_on_buffer)
    {
    	decode_cmd_ret=decode_cmd(&recv_buf[offset],bytes_left_on_buffer);

    	if(decode_cmd_ret==0)
    	{
    		previous_bytes_left=bytes_left_on_buffer;
    		if(bytes_left_on_buffer!=0 && offset!=0)
    		{
    			int pos;
    			xil_printf("Alineando %d bytes sobrantes al principio del buffer...\r\n",bytes_left_on_buffer);
    			for(pos=0;pos<bytes_left_on_buffer;pos++)
    				recv_buf[pos]=recv_buf[pos+offset];
    		}
    		break;
    	}
    	if(bytes_left_on_buffer<decode_cmd_ret)
    	{
    		xil_printf("Error, tratando de consumir más datos de los que hay en el buffer de lectura rx_buffer: \r\n");
    		xil_printf("bytes_left_on_buffer = %u\r\n",bytes_left_on_buffer);
    		xil_printf("decode_cmd_ret = %u\r\n",decode_cmd_ret);
    		previous_bytes_left=0;
    		break;
    	}
    	bytes_left_on_buffer-=decode_cmd_ret;
    	offset+=decode_cmd_ret;
    	previous_bytes_left=0;
    }
    //xil_printf("Rx_large_left=%d\r\n",Rx_large_left);

    //tcp_recved(tpcb, p->tot_len);
    //pbuf_free(p);
    return ERR_OK;
}

err_t tcpcom_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    xil_printf("\r\ntcpcom: Connection Accepted");
    tcp_recv(newpcb, tcpcom_recv_callback);
	tcp_sent(newpcb, tcpcom_sent_callback);
	Connected_pcb = newpcb;
	Connected = 1;
	Direccion_pcb_comandos=(u8*)newpcb;
	xil_printf("Connected_pcb = 0x%08X\r\n",(u32)Connected_pcb);
	xil_printf("Connected_pcb->local_port = %d\r\n",(u32)Connected_pcb->local_port);
	xil_printf("Connected_pcb->remote_port = %d\r\n",(u32)Connected_pcb->remote_port);
    return ERR_OK;
}
int start_tcpcom_application()
{
    struct tcp_pcb *pcb;
    err_t err;
    // Creo los buffer de emisión y recepción de comandos.

    cons_prod_HOST2UCI_p=prod_init(HOST2UCI_BUF_ID);
    cons_prod_UCI2HOST_p=cons_init(UCI2HOST_BUF_ID,uci2host_buffer_interrupt_function);

    /* create new TCP PCB structure */
    pcb = tcp_new();
    if (!pcb)
    {
    	xil_printf("tcpcom: Error creating PCB. Out of Memory\r\n");
    	return -1;
    }

    /* bind to iperf @port */
    err = tcp_bind(pcb, IP_ADDR_ANY, Net_param.cmmd_port);
    if (err != ERR_OK)
    {
    	xil_printf("tcpcom: Unable to bind to port %d: err = %d\r\n", Net_param.cmmd_port, err);
    	return -2;
    }

    /* we do not need any arguments to callback functions :) */
    tcp_arg(pcb, NULL);

    /* listen for connections */
    pcb = tcp_listen(pcb);
    if (!pcb) {
    	xil_printf("tcpcom: Out of memory while tcp_listen\r\n");
    	return -3;
    }

    /* specify callback to use for incoming connections */
    tcp_accept(pcb, tcpcom_accept_callback);

    tcpcom_server_running = 1;

    return 0;
}

void print_tcpcom_app_header()
{
    xil_printf("Servidor de COMANDOS en puerto %d iniciado, esperando conexión.\r\n",Net_param.cmmd_port);
}
