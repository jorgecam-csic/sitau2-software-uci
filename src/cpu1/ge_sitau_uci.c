/*
 * ge_sitau_uci.c
 *
 *  Created on: 21/11/2016
 *      Author: csic
 */

#include "ge_sitau_uci.h"
#include "log.h"

typedef enum estados_uci_e
{
	NOP,
	PROG_SERIAL_OSC,
	PROG_SERIAL_32B
}estados_uci_t;

#define TAC_E_S			0x0
#define TAC_E_G			0x1
#define TAC_E_R			0x2
#define TAC_LEC			0x3

#define UCI_NUM_REGS	64

#define REG_REDIR		0
#define REG_PROC		19
#define REG_SERIAL_OSC	39
#define REG_SERIAL_32B	40
#define REG_SLEEPUS		41

u16 Mid_buffer[1<<16]; // 64K u16 word buffer

volatile u16 registros_uci[UCI_NUM_REGS];
estados_uci_t estado_uci=NOP;
u8 retry_read = 1;
float timeout_read = 0.3;
float timeout_acq = 1;

u32 num_muestras_scan_capturado;


int test_prepara_adquisicion_sitau_DMA_MCBCC_CONSPRODBUFF(u32 total_samples_to_buffer)
{
	u32 num_bytes_avilable_buffer;
	u32 total_bytes_to_buffer;
	u32 bytes_first_blow,bytes_second_blow;
	u8* dst_addr;
	u8* image_buff_start_addr=IMAGE_prod_p->buffer_start_address;

	total_bytes_to_buffer=total_samples_to_buffer<<1;

	num_bytes_avilable_buffer=prod_num_data_to_write(IMAGE_prod_p, NULL);

	if(total_bytes_to_buffer>num_bytes_avilable_buffer)
	{
		dbg_printf("Sin espacio en el buffer de im�genes num_bytes_avilable_buffer=%d, total_bytes_to_buffer=%d\r\n",num_bytes_avilable_buffer,total_bytes_to_buffer);
		return (num_bytes_avilable_buffer-total_bytes_to_buffer); //Si no hay espacio suficiente en el buffer, devuelvo negativo (las muestras que faltan)
	}
	if(total_samples_to_buffer>((1<<23)-4))
	{
		dbg_printf("total_bytes_to_buffer=%d>%d: funci�n no soporta m�s de ((2^23)-4) bytes.\r\n",total_bytes_to_buffer,((1<<23)-4));
		return (-total_bytes_to_buffer); //Si no hay espacio suficiente en el buffer, devuelvo negativo (las muestras que faltan)
	}

	bytes_first_blow=prod_memref(IMAGE_prod_p,total_bytes_to_buffer,&dst_addr);
	if(bytes_first_blow<total_bytes_to_buffer)
		bytes_second_blow=total_bytes_to_buffer-bytes_first_blow;
	else
		bytes_second_blow=0;

	if(bytes_second_blow)
	{
		DATAMOV_put_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],dst_addr,bytes_first_blow,0x1,0);
		DATAMOV_put_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],image_buff_start_addr,bytes_second_blow,0xF,1);
		dbg_printf("DMA da la vuelta al buffer!\r\n");
	}
	else
		DATAMOV_put_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],dst_addr,bytes_first_blow,0xF,1);

	MCBCC_amplia_2_stream_after_0_words_no_int(total_samples_to_buffer);

	return total_bytes_to_buffer;
}
int test_wait_to_end_acq()
{
	timer_set_one_count(timeout_acq,TIMEOUT_TIMER);
	timer_start(TIMEOUT_TIMER);
	while(Datamover_instances[LOCAL_DATAMOVER_ID].datamov_flags->DATAMOV_bit_flags.S2MM_end==0)
	{
		if(Timer_flag[TIMEOUT_TIMER])
		{
			dbg_printf("ACQ timeout!");
			return -1;
		}
		WFI_dma();
	}
	dbg_printf("Datamover termin�, Datamover_instances[LOCAL_DATAMOVER_ID].S2MM_status_reg_int = %08X\r\n",Datamover_instances[LOCAL_DATAMOVER_ID].S2MM_status_reg_int);
	Datamover_instances[LOCAL_DATAMOVER_ID].datamov_flags->DATAMOV_bit_flags.S2MM_end=0;
	if(MCBCC_check_rd_stream_flag()==0)
	{
		dbg_printf("Datamover ends but MCBCC didnt ended!");
		return -1;
	}
	return 0;
}


