/*
 * tcpimage.c
 *
 *  Created on: 22 de feb. de 2017
 *      Author: csic
 */
#include "tcpimage.h"

cons_prod_t *cons_IMAGE_p;

volatile u8 image_data_available=0;
volatile u8 image_data_disabled=0;

u8 tcpimage_server_running=0;

static u32	Tx_large_left = 0;
static u8*	Tx_large_left_curr_addr = NULL;
static u32	Tranfering_image_num_data = 0;
static u8	Image_connected = 0;


#if (USE_JUMBO_FRAMES==1)
#define SEND_BUFSIZE (9000)
#else
#define SEND_BUFSIZE (1400)
#endif

#define SEND_SIZE	1400

#define RECV_BUFSIZE	(8*1024*1024)
static struct tcp_pcb *Image_pcb = NULL;
volatile extern int TxPerfConnMonCntr;

#define USE_SEND_IMAGE_MID_BUFFER 	0

#if (USE_SEND_IMAGE_MID_BUFFER==1)
	#define SEND_IMAGE_MID_BUFFER_SIZE	(8*1024*1024)
	u8 Send_image_mid_buffer[SEND_IMAGE_MID_BUFFER_SIZE];
	#define MAX_BURST_SEND (SEND_IMAGE_MID_BUFFER_SIZE)
#else
	#define MAX_BURST_SEND (16*1024*1024)
#endif


u32 Max_burst_send=MAX_BURST_SEND;//8*1024*1024;


void image_buffer_interrupt_function(void)
{
	//xil_printf("INT CPU1 to CPU0 (image)\r\n");
	image_data_available=1;
}
int tcp_image_clear_buffer(void)
{
	u32 num_data_to_clear;
    //Xil_ExceptionDisableMask(XIL_EXCEPTION_IRQ);
	image_data_available=0;
	Tx_large_left=0;
	Tranfering_image_num_data = 0;
	num_data_to_clear = cons_num_data_to_read(cons_IMAGE_p);
	cons_refresh_read(cons_IMAGE_p,num_data_to_clear);
	//Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
    //xil_printf("\r\ntcpimage: tcp_image_clear_buffer()");
	return 0;
}
int tcp_image_buffer_enabled(void)
{
	image_data_disabled = 0;
	tcp_image_clear_buffer();
    xil_printf("\r\ntcpimage: tcp_image_buffer_enabled()");
	return 0;
}
int tcp_image_buffer_disabled(void)
{
	image_data_disabled = 1;
	tcp_image_clear_buffer();
    xil_printf("\r\ntcpimage: tcp_image_buffer_disabled()");
	return 0;
}
clear_image_buffer_and_disconnect()
{
	tcp_image_clear_buffer();
	Image_pcb = NULL;
}

int transfer_tcpimage_data()
{
#if __arm__
	u8 copy = 1;
	u8 more = 0;
#else
	int copy = 0;
	int more = 0;
#endif
	err_t err;
	struct tcp_pcb *tpcb = Image_pcb;
	int btt,abtt;

	if (!Image_pcb)
		return ERR_OK;

	if(image_data_disabled)
	{
		//tcp_image_clear_buffer();
		return 0;
	}

	if(Tx_large_left==0)
	{
		if (image_data_available==0)
			return ERR_OK;
		else if (image_data_available!=0)
		{
			int num_datos_to_read;
			image_data_available=0;
			num_datos_to_read=cons_num_data_to_read(cons_IMAGE_p);
			if(num_datos_to_read==0)
				return ERR_OK;

#if (USE_SEND_IMAGE_MID_BUFFER==1)
			Tranfering_image_num_data=(num_datos_to_read>Max_burst_send?Max_burst_send:num_datos_to_read);
			cons_memcpy(cons_IMAGE_p,Tranfering_image_num_data,Send_image_mid_buffer);
			Tx_large_left_curr_addr = Send_image_mid_buffer;
#else
			if(Max_burst_send)
				Tranfering_image_num_data=(num_datos_to_read>Max_burst_send?Max_burst_send:num_datos_to_read);
			else
				Tranfering_image_num_data=num_datos_to_read;

			Tranfering_image_num_data = cons_memref(cons_IMAGE_p,Tranfering_image_num_data,&Tx_large_left_curr_addr);
			if(Max_burst_send)
				Tranfering_image_num_data=(Tranfering_image_num_data>Max_burst_send?Max_burst_send:Tranfering_image_num_data);
#endif

			if(Tranfering_image_num_data!=num_datos_to_read)
				image_data_available = 1;
			Tx_large_left=Tranfering_image_num_data;
		}
	}

	abtt=tcp_sndbuf(tpcb);
	//xil_printf("abtt = %d\r\n",abtt);
	if(abtt>Tx_large_left)
	{
		err = tcp_write(tpcb, Tx_large_left_curr_addr, Tx_large_left, copy|more);

		if (err != ERR_OK)
		{
			xil_printf("\r\ntcpimage: Error on tcp_write a: %d", err);
			xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
			xil_printf("\r\nbtt: %d", btt);
			xil_printf("\r\ntpcb->state: %d", tpcb->state);
			xil_printf("\r\ntpcb->snd_wnd: %d", tpcb->snd_wnd);
			xil_printf("\r\ntpcb->snd_max: %d", tpcb->snd_wnd_max);
			xil_printf("\r\ntpcb->snd_buf: %d", tpcb->snd_buf);
			xil_printf("\r\ntpcb->unsend_oversize: %d", tpcb->unsent_oversize);
			//clear_image_buffer_and_disconnect();
         tcp_image_clear_buffer();
			return -1;
		}

		err = tcp_output(tpcb);
		if (err != ERR_OK)
		{
			xil_printf("\r\ntcpimage: Error on tcp_output b: %d",err);
			xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
			xil_printf("\r\nbtt: %d", btt);
			xil_printf("\r\ntpcb->state: %d", tpcb->state);
			xil_printf("\r\ntpcb->snd_wnd: %d", tpcb->snd_wnd);
			xil_printf("\r\ntpcb->snd_max: %d", tpcb->snd_wnd_max);
			xil_printf("\r\ntpcb->snd_buf: %d", tpcb->snd_buf);
			xil_printf("\r\ntpcb->unsend_oversize: %d", tpcb->unsent_oversize);
			//clear_image_buffer_and_disconnect();
         //tcp_image_clear_buffer();
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
				xil_printf("\r\ntcpimage: Error on tcp_write c: %d", err);
				xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
				xil_printf("\r\nbtt: %d", btt);
				xil_printf("\r\ntpcb->state: %d", tpcb->state);
				xil_printf("\r\ntpcb->snd_wnd: %d", tpcb->snd_wnd);
				xil_printf("\r\ntpcb->snd_max: %d", tpcb->snd_wnd_max);
				xil_printf("\r\ntpcb->snd_buf: %d", tpcb->snd_buf);
				xil_printf("\r\ntpcb->unsend_oversize: %d", tpcb->unsent_oversize);
				//clear_image_buffer_and_disconnect();

				tcp_image_clear_buffer();
				return -1;
			}

			err = tcp_output(tpcb);
			if (err != ERR_OK)
			{
				xil_printf("\r\ntcpimage: Error on tcp_output d: %d",err);
				xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
				xil_printf("\r\nbtt: %d", btt);
				xil_printf("\r\ntpcb->state: %d", tpcb->state);
				xil_printf("\r\ntpcb->snd_wnd: %d", tpcb->snd_wnd);
				xil_printf("\r\ntpcb->snd_max: %d", tpcb->snd_wnd_max);
				xil_printf("\r\ntpcb->snd_buf: %d", tpcb->snd_buf);
				xil_printf("\r\ntpcb->unsend_oversize: %d", tpcb->unsent_oversize);
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
			xil_printf("\r\ntcpimage: Error on tcp_write e: %d", err);
			xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
			xil_printf("\r\nbtt: %d", btt);
			xil_printf("\r\ntpcb->state: %d", tpcb->state);
			xil_printf("\r\ntpcb->snd_wnd: %d", tpcb->snd_wnd);
			xil_printf("\r\ntpcb->snd_max: %d", tpcb->snd_wnd_max);
			xil_printf("\r\ntpcb->snd_buf: %d", tpcb->snd_buf);
			xil_printf("\r\ntpcb->unsend_oversize: %d", tpcb->unsent_oversize);
			//clear_image_buffer_and_disconnect();
         //tcp_image_clear_buffer();
			return -1;
		}

		err = tcp_output(tpcb);
		if (err != ERR_OK)
		{
			xil_printf("\r\ntcpimage: Error on tcp_output f: %d",err);
			xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
			xil_printf("\r\nbtt: %d", btt);
			xil_printf("\r\ntpcb->state: %d", tpcb->state);
			xil_printf("\r\ntpcb->snd_wnd: %d", tpcb->snd_wnd);
			xil_printf("\r\ntpcb->snd_max: %d", tpcb->snd_wnd_max);
			xil_printf("\r\ntpcb->snd_buf: %d", tpcb->snd_buf);
			xil_printf("\r\ntpcb->unsend_oversize: %d", tpcb->unsent_oversize);
		}
		Tx_large_left-=btt;
		Tx_large_left_curr_addr=&Tx_large_left_curr_addr[btt];
	}

	//xil_printf("Tx_large_left=%d\r\n",Tx_large_left);
	if(Tx_large_left==0)
	{
		//xil_printf("Envío largo realizado con éxito: Tranfering_image_num_data = %d.\r\n",Tranfering_image_num_data);
		if(Tranfering_image_num_data)
		{
			cons_refresh_read(cons_IMAGE_p,Tranfering_image_num_data);
			Tranfering_image_num_data=0;
		}
	}
    return ERR_OK;
}

static err_t tcpimage_sent_callback(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	TxPerfConnMonCntr = 0;
	transfer_tcpimage_data();
	return ERR_OK;
}


static err_t tcpimage_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
int count_line;

    /* close socket if the peer has sent the FIN packet  */
    if (p == NULL)
    {
    	Image_pcb = NULL;
        tcp_close(tpcb);
        xil_printf("\r\ntcpimage: Desconexión por parte del cliente");
        return ERR_OK;
    }



	tcp_recved(tpcb, p->tot_len);

	{
		int i;
		u8* temp;
		temp=(u8*)p->payload;
		xil_printf("\r\n");
		xil_printf("\r\tcpimage_recv_callback:");
		xil_printf("\r\ntpcb = 0x%08X",(u32)tpcb);
		xil_printf("\r\ntpcb->local_port = %d",(u32)tpcb->local_port);
		xil_printf("\r\ntpcb->remote_port = %d",(u32)tpcb->remote_port);
		xil_printf("\r\n");
		xil_printf("\r\ntcpimage: Error on tcpimage_recv_callback: %d",err);
		xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
		xil_printf("\r\ntpcb->state: %d", tpcb->state);
		xil_printf("\r\ntpcb->snd_wnd: %d", tpcb->snd_wnd);
		xil_printf("\r\ntpcb->snd_max: %d", tpcb->snd_wnd_max);
		xil_printf("\r\ntpcb->snd_buf: %d", tpcb->snd_buf);
		xil_printf("\r\ntpcb->unsend_oversize: %d", tpcb->unsent_oversize);
		xil_printf("\r\n");
		xil_printf("PUNTEROS:\r\n");
		xil_printf("p->payload: 0x%08X\r\n",(u32)p->payload);
		xil_printf("\r\n");
		xil_printf("VOLCADO:\r\n");
		xil_printf("p->payload:\r\n");

		for (	count_line = 0,
				i = -16;
					i < p->tot_len;
						i++,
						count_line++)
		{
			if (count_line > 32) 
			{
				xil_printf("\r\n");
				count_line = 0;
			}
//			if((!(i%4)) && i<=32) xil_printf("\r\n");
			xil_printf("%02X ",*(temp+i));
		}
		xil_printf("\r\n");

	}

	xil_printf("tcpimage: Descartando %d bytes por el puerto de imágenes.\r\n",p->tot_len);

    pbuf_free(p);

    return ERR_OK;
}

err_t tcpimage_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    xil_printf("\r\ntcpimage: Connection Accepted");
    tcp_recv(newpcb, tcpimage_recv_callback);
	tcp_sent(newpcb, tcpimage_sent_callback);
	Image_pcb = newpcb;
	Image_connected = 1;
	{
		xil_printf("\r\ntcpimage_accept_callback:");
		xil_printf("\r\ntcpimage: Error on tcp_output f: %d",err);
		xil_printf("\r\nTx_large_left_curr_addr: %08X", Tx_large_left_curr_addr);
		xil_printf("\r\ntpcb->state: %d", Image_pcb->state);
		xil_printf("\r\ntpcb->snd_wnd: %d", Image_pcb->snd_wnd);
		xil_printf("\r\ntpcb->snd_max: %d", Image_pcb->snd_wnd_max);
		xil_printf("\r\ntpcb->snd_buf: %d", Image_pcb->snd_buf);
		xil_printf("\r\ntpcb->unsend_oversize: %d", Image_pcb->unsent_oversize);
	}
	xil_printf("Image_pcb = 0x%08X\r\n",(u32)Image_pcb);
	xil_printf("Image_pcb->local_port = %d\r\n",(u32)Image_pcb->local_port);
	xil_printf("Image_pcb->remote_port = %d\r\n",(u32)Image_pcb->remote_port);
    return ERR_OK;
}

int start_tcpimage_application()
{
    struct tcp_pcb *pcb;
    err_t err;
    // Creo los buffer de emisión y recepción de comandos.

    cons_IMAGE_p=cons_init(IMAGE_BUF_ID,image_buffer_interrupt_function);

    /* create new TCP PCB structure */
    pcb = tcp_new();
    if (!pcb)
    {
    	xil_printf("tcpimage: Error creating PCB. Out of Memory\r\n");
    	return -1;
    }

    /* bind to iperf @port */
    err = tcp_bind(pcb, IP_ADDR_ANY, Net_param.data_port);
    if (err != ERR_OK)
    {
    	xil_printf("tcpimage: Unable to bind to port %d: err = %d\r\n", Net_param.data_port, err);
    	return -2;
    }

    /* we do not need any arguments to callback functions :) */
    tcp_arg(pcb, NULL);

    /* listen for connections */
    pcb = tcp_listen(pcb);
    if (!pcb) {
    	xil_printf("tcpimage: Out of memory while tcp_listen\r\n");
    	return -3;
    }

    /* specify callback to use for incoming connections */
    tcp_accept(pcb, tcpimage_accept_callback);

    tcpimage_server_running = 1;

    return 0;
}

void print_tcpimage_app_header()
{
    xil_printf("Servidor de IMÁGENES en puerto %d iniciado, esperando conexión.\r\n",Net_param.data_port);
}
