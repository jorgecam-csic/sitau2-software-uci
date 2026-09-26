/*
 * bcc_bussar.c
 *
 *  Created on: 25 mar. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_C_
#define SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_C_

#include "bcc_bussar.h"
#include "bcc_bussar_error_code.h"
#include "mcbcc_mst_driver.h"
#include "ttimer.h"
#include "global.h"
#include "log.h"
#include "uci_set.h"
#include "datamover_driver.h"
#include "dynamic_phase.h"
#include "switch_driver.h"
#include "datamover_remote.h"
#include "gtx_control.h"
#include "afe5808a.h"

#define EMAC_ALIGN __attribute__ ((__aligned__(CACHELINE_BYTE_SIZE)))


u32 Global_command_buf_data[MAX_COMMAND_BUFFER_DATA];
commnd_buffer_t Global_commnd_buf = {.data=Global_command_buf_data,.total_len = 0,.max_len=MAX_COMMAND_BUFFER_DATA};
u32 Sent_buff_data[MAX_LVDS_CONF_BUFFER_DATA] EMAC_ALIGN;
commnd_buffer_t Sent_buff = {.data=Sent_buff_data,.total_len = 0,.max_len=MAX_LVDS_CONF_BUFFER_DATA};
u32 Rec_buff_data[MAX_LVDS_CONF_BUFFER_DATA] EMAC_ALIGN;
commnd_buffer_t Rec_buff = {.data=Rec_buff_data,.total_len = 0,.max_len=MAX_LVDS_CONF_BUFFER_DATA};

void bcc_print_all_busy_flags(u8 last_minibase)
{
	u8 minibase;
	u32 flags;

	for(minibase=1;minibase<=last_minibase;minibase++)
	{
		flags=bcc_get_busy_in_word(minibase);
		if(flags!=0x7F)								xil_printf("\n\rMINIMÓDULO %d\n\r",minibase);
		if((flags&(1<<MISC_BUSY_BIT_DMVR_32))==0) 	xil_printf("Datamover 32 no finalizado\n\r");
		if((flags&(1<<MISC_BUSY_BIT_AFES))==0) 		xil_printf("AFEs no finalizados\n\r");
		if((flags&(1<<MISC_BUSY_BIT_DMVR_256))==0) 	xil_printf("Datamover 256 no finalizado\n\r");
		if((flags&(1<<MISC_BUSY_BIT_PULSER))==0) 	xil_printf("Pulser no finalizado\n\r");
		if((flags&(1<<MISC_BUSY_BIT_TGC))==0) 		xil_printf("TGC no finalizado\n\r");
		if((flags&(1<<MISC_BUSY_BIT_BFMR))==0) 		xil_printf("Conformador no finalizado\n\r");
		if((flags&(1<<MISC_BUSY_BIT_DMVR_BFMR))==0)	xil_printf("Datamover del conformador no finalizado\n\r");
	}
}

u32 bcc_set_busy_or_word(u8 minibase,u32 not_busy_word)
{
	volatile u32* base;
	u32 ret_val;

	BCC_write_reg_inmediate(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_NOT_BUSY_OR_REG,not_busy_word);

	return 0;
}

u32 bcc_set_pro_and_busy_mask_buffer(u8 minibase,u32 pro_mask_word,u32 busy_mask,commnd_buffer_t *commnd_buf_p)
{
	int num_words=0;
	int ret_val;

	ret_val=push_bussar_write_reg((u32)pro_mask_word,minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_MASK_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)busy_mask,minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_NOT_BUSY_OR_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	return num_words;
}

u32 bcc_set_pro_mask_word(u8 minibase,u32 pro_mask_word)
{
	volatile u32* base;
	u32 ret_val;

	BCC_write_reg_inmediate(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_MASK_REG,pro_mask_word);

	return 0;
}

u32 bcc_get_pro_mask_word(u8 minibase)
{
	volatile u32* base;
	u32 ret_val;

	base=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);

	ret_val=base[MISC_PRO_MASK_REG];

	MCBCC_deactivate_emu();

	return ret_val;
}
u32 bcc_get_busy_or_word(u8 minibase)
{
	volatile u32* base;
	u32 ret_val;

	base=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);

	ret_val=base[MISC_NOT_BUSY_OR_REG];

	MCBCC_deactivate_emu();

	return ret_val;
}

u32 bcc_get_busy_in_word(u8 minibase)
{
	volatile u32* base;
	u32 ret_val;

	base=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);

	ret_val=base[MISC_NOT_BUSY_IN_REG];

	MCBCC_deactivate_emu();

	return ret_val;
}
int bcc_copy_uci2mem(u32* datos,u32 n_datos,u32 remote_addr,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	u32* sswitch;
	u32 btt;

	btt=n_datos<<2;


	//Configuramos SWITCH32
	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_DDR_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0);

		MCBCC_deactivate_emu();
	}

	//Configuramos datamover remoto para escritura
	config_uci_buffer2minibase(remote_addr,btt,minibase,commnd_buf_p);

	BCC_SendBuffer(commnd_buf_p);
	commnd_buf_p->total_len=0;
	//Configuramos BCC en modo stream escritura
	{
		bcc_header_t comando_escritura_stream;
		bcc_header_t comando_extesion_longitud;
		u16 LSW,MSW;
		u32 n_data;

		n_data = n_datos;

		LSW=(u16)(n_data&0xFFFF);
		MSW=(u16)(n_data>>16);

		comando_escritura_stream.HEADER = 0x00000000;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
		comando_escritura_stream.STREAM_CMD.MODULE  = minibase;
		comando_escritura_stream.STREAM_CMD.LENGHT = LSW;

		if(MSW)
		{
			comando_extesion_longitud.HEADER = 0x00000000;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
			comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		}
		if(MSW)
			MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
		MCBCC_send_HEAD(comando_escritura_stream.HEADER);
	}

	//Enviamos los parámetros
	{
		commnd_buffer_t data_buf;
		data_buf.data = datos;
		data_buf.total_len = n_datos;
		data_buf.max_len = n_datos;

		BCC_SendBuffer(&data_buf);
	}

	{
		status_t status_datamov = get_uci_buffer2minibase_datamover_status(minibase);;
		int i=0;
		while(status_datamov.BIT.status_response.BIT.okey==0)
		{
			i++;
			if(i>10)
			{
				xil_printf("STATUS: 0x%08X\n\r",status_datamov.status_32);
				ELOG(charEBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_END, EBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_END);
				break;
			}

			status_datamov = get_uci_buffer2minibase_datamover_status(minibase);
		}
		if (status_datamov.BIT.transfer_flag == 0)
		{
			xil_printf("STATUS: 0x%08X\n\r",status_datamov.status_32);
			ELOG(charEBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_OK, EBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_OK);
		}
	}



	//Desconfiguramos SWITCH32
	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BFM_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,1);

		MCBCC_deactivate_emu();
	}



	return 0;
}


void init_bcc_and_bussar(void)
{
	int num_bases,retardo_extra;
	int LVDS_deskew_nok;
	volatile u32* base=(u32*)GPIO_CONF_ADDR;
	int AFES_align_nok=1;
	u32 errores_lvds,errores_ddr = 0;
	u32 codigo_pulser_leido;

	LVDS_deskew_nok=BCC_deskew_LVDS(10000.0);


	if(LVDS_deskew_nok)
		xil_printf("\n\rERROR! LVDS no alineados!\n\r");
	else
		xil_printf("\n\rLVDS alineados correctamente.\n\r");


	num_bases=MCBCC_autonum();		//Esto inicia el hardware, mejor hacerlo más tarde

	xil_printf("\n\rNum bases encontradas: %d\n\r",num_bases);


	//set_dynamic_phase_ps(1,-5000);

	retardo_extra=MCBCC_delay_fifo_ini(8,num_bases);
	xil_printf("\n\rRetardo_extra: %d\n\r",retardo_extra);

	errores_lvds = BCC_test_LVDS_configuration();
	//errores_ddr = BCC_test_DDR_remote();


    //BCC_set_dynamic_phase(1, 520, 1000.0);
	//BCC_test_LVDS_configuration();
	//BCC_set_dynamic_phase(2, -720, 1000.0);
	//BCC_test_LVDS_configuration();
	//BCC_set_dynamic_phase(3, -1800, 1000.0);
	//BCC_test_LVDS_configuration();
	//BCC_set_dynamic_phase(4, -3660, 1000.0);
	//BCC_test_LVDS_configuration();


    //BCC_test_TGC_module();

	//for(intentos=0;intentos<5&&align_nok!=0;intentos++)

	gtx_set_adv_regs(0,7,1,0,0,0,0);//configuración para los enlaces DN cercanos
	gtx_set_adv_regs(0,7,1,0,0,0,1);//configuración para los enlaces UP cercanos
	//gtx_set_adv_regs(1,12,0,0,6,0,0);//configuración para los enlace  DN fibra
	if(num_bases==8)
		gtx_set_adv_regs(5,7,0,0,4,0,0);//configuración para los enlace DN fibra base inferior en sistemas de 256


	/***************************PRUEBAS******************/
	if(0)
	{
		u8 dif_tx_dn=7;//7, 3, 12,0 mas alto o igual
		u8 dif_tx_up=0;//7, 3, 12,0
		u8 pre_tx=0;
		u8 pos_tx=0;
		gtx_set_adv_regs(0,dif_tx_dn,1,0,pre_tx,pos_tx,0);//configuración para los enlaces DN cercanos
		gtx_set_adv_regs(0,dif_tx_up,1,0,pre_tx,0,1);//configuración para los enlaces UP cercanos
		//gtx_set_adv_regs(1,12,0,0,6,0,0);//configuración para los enlace  DN fibra
		if(num_bases==8)
			gtx_set_adv_regs(5,7,0,0,4,0,0);//configuración para los enlace DN fibra base inferior en sistemas de 256

		gtx_set_adv_regs(0,dif_tx_dn,0,1,pre_tx,pos_tx,0);//configuración para los enlaces DN cercanos
		gtx_set_adv_regs(0,dif_tx_up,0,1,pre_tx,pos_tx,1);//configuración para los enlaces UP cercanos
		gtx_set_adv_regs(0,dif_tx_dn,0,0,pre_tx,pos_tx,0);//configuración para los enlaces DN cercanos
		gtx_set_adv_regs(0,dif_tx_up,0,0,pre_tx,pos_tx,1);//configuración para los enlaces UP cercanos
	}
	/****************************************************/


	AFES_align_nok=all_afe_remote_align(num_bases);
	enable_all_remote_gtx(1);
	enable_all_UCI_gtx(0);
	//gtx_print_all_errors(num_bases);


	//swgtx_remote_up_pass_throw_to_uci(1);
	//swgtx_remote_down_pass_throw_to_host(1);


	//init_pulser_simple();
	BCC_read_reg(1, MISC_BUSSAR_SUBMOD_ADDR, MISC_VERSION_DATE_REG, &gb_shared->mk32_version);
	BCC_read_reg(1,PULSER_BUSSAR_SUBMOD_ADDR,CODE_CONF_R,&codigo_pulser_leido);

	gb_shared->AFES_align_ok = !AFES_align_nok;
	gb_shared->LVDS_deskew_ok = !LVDS_deskew_nok;
	gb_shared->errores_lvds = errores_lvds;
	gb_shared->errores_ddr = errores_ddr;
	gb_shared->codigo_pulser_leido = codigo_pulser_leido;
	gb_shared->n_bases_bcc = num_bases;
	gb_shared->retardo_extra_bcc = retardo_extra;
}

int BCC_wait_PRO_rise(float us_timeout)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	u32 pro_out=base[PRO_REG]&PROO;
	u32 result;

	volatile u32 flag_end;


	if(us_timeout>0)
	{
		if ((result = TIMER_set_one_count(us_timeout,TIMER_SLEEP)) != 0)
		{
			//UCI_datapath_clear();
			return RLOG(result);
		}
	}

	if(us_timeout>0)
		if ((result = TIMER_Start(TIMER_SLEEP)) != 0) return RLOG(result);

	flag_end=MCBCC_get_flags(PORF_MASK);
	if(flag_end==0&&pro_out)
		return ELOG(charEBCC_INTERRUPT_LOST, EBCC_INTERRUPT_LOST);

	while (!flag_end)
	{
		if (us_timeout==0) break;
		WFI_bcc();
		if (us_timeout>0 && TIMER_Timeout(TIMER_SLEEP))
		{
			pro_out=base[PRO_REG]&PROO;
			//UCI_datapath_clear();
			if(pro_out)
			{
				return ELOG(charEBCC_INTERRUPT_LOST_TIMEOUT, EBCC_INTERRUPT_LOST_TIMEOUT);
			}
			else
			{
				return ELOG(charEBCC_NO_FLAG_IN_TIME, EBCC_NO_FLAG_IN_TIME);
			}
		}
		flag_end=MCBCC_get_flags(PORF_MASK);
	}
	if (us_timeout>0)
		if ((result = TIMER_Stop(TIMER_SLEEP)) < 0) return RLOG(result);

	return EBCCBUSSAR_NONE;
}

int BCC_test_DDR_remote(void) //ESTO ESTA MAL, NO PRUEBA LA DDR REMOTA
{
	int aux;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	volatile u32 temp;
	bcc_header_t comando_lectura_stream;
	bcc_header_t comando_extesion_longitud;
	bcc_header_t comando_ack;
	u32 n_data;


	Xil_DCacheDisable();
	n_data=MAX_LVDS_CONF_BUFFER_DATA;
	btt=n_data<<2;
	Sent_buff.total_len=0;
	//Xil_DCacheDisable();
	for (u32 i=0; i<MAX_LVDS_CONF_BUFFER_DATA; i++){
		Sent_buff.data[i]=i;
		Sent_buff.total_len++;
	}
	Sent_buff.data[0]=0xFFFF;
	Sent_buff.data[1]=0xFFFE;
	Sent_buff.data[2]=0xFFFD;

	BCC_write_reg_inmediate(0,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_LINK_REG,MISC_PRO_LINK_LVDS_MASK);

	Xil_DCacheFlushRange((INTPTR)Sent_buff.data, Sent_buff.total_len*4);
	//Xil_DCacheFlush();
	dmb();

	// Configure AXI switch driver
	// Receive: from LVDS_RX->MCBCC->Datamover
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX ,0);
	// Send: from Datamover-> MCBCC -> LVDS_TX
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_BCCMST_IDX,DSP_DMA_SWITCH_SLAVE_FROM_DDRZYN_IDX ,0);

	// Set up datamover to receive: interrupcion o no interrupción?
	//DATAMOV_put_mem_no_int(datamov_instance_t *datamover_instance_p,void *dst_addr,u32 btt);
	//Solo puedo hacer transacciones de 0xFFFF words, si no da fallo

	DATAMOV_put_mem_no_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Rec_buff.data,btt); // En la doc dice que son bytes to transfer, igual hay que multiplicarlo x 4, esto está en words

	//DATAMOV_put_mem_int(datamov_instance_t *datamover_instance_p,void *dst_addr,       u32 btt,       u8 tag,u8 last) 0x00
	//DATAMOV_put_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Rec_buff.data, MAX_LVDS_CONF_BUFFER_DATA,0x05,1);

	// Set up MCBCC to receive
	//MCBCC_lvds_2_stream_after_n_words_int_no_block(u32 num_words_to_stream,u32 num_words_before_stream, u8 last_behavior)
	//MCBCC_lvds_2_stream_after_n_words_int_no_block(MAX_LVDS_CONF_BUFFER_DATA,1, 0x02);
	//MCBCC_lvds_2_stream_after_n_words(u32 num_words_to_stream,u32 words_to_wait)

	//MCBCC_lvds_2_stream_after_n_words(MAX_LVDS_CONF_BUFFER_DATA, 0);
	// Send a write to register command
	//BCC_write_reg_inmediate(127,5,0,0x80808080);

	// Set up the MCBCC to write
	//MCBCC_stream_2_lvds(MAX_LVDS_CONF_BUFFER_DATA);

	//Setting up the MBCC to receive and send
	LSW=(u16)(n_data&0xFFFF);
	MSW	=(u16)(n_data>>16);
	comando_lectura_stream.HEADER = 0x00000000;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
	comando_lectura_stream.STREAM_CMD.MODULE  = 250;
	comando_lectura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;
	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	MCBCC_int_enable(RSEE_MASK);

	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data,prev_words,LAST_DONT_TOUCH);

	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);

	MCBCC_send_HEAD(comando_lectura_stream.HEADER);

	comando_ack=comando_lectura_stream;
	comando_ack.STREAM_CMD.CTRL.BITS.ACK = 1;

	/*for(int i=0;i<1000;i++)
	{
		temp=MCBCC_get_rd_HEAD();
		if(temp==comando_ack.HEADER) return 1;
	}*/

	//BCC_SendBuffer(&Sent_buff);  // De momento esto funciona ¯\_(:))_/¯ , al hacer bypass del datamover, se escribe directamene desde la memoria de la UCI a la linea LVDS con el mcbcc





	{
		MCBCC_stream_2_lvds(MAX_LVDS_CONF_BUFFER_DATA);
		// Set up data mover to send
		//DATAMOV_get_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Sent_buff.data,btt,0x05,LAST_FORCE_ONE);
		DATAMOV_get_mem_no_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Sent_buff.data,btt);
	}


	//wait_for(10000);
	Xil_DCacheInvalidateRange((INTPTR)Rec_buff.data, Sent_buff.total_len*4);
	dmb();

	aux=BCC_compare_buffers(Sent_buff.data, Rec_buff.data, MAX_LVDS_CONF_BUFFER_DATA);

	Xil_DCacheEnable();
	return aux;
}
int BCC_test_LVDS_configuration(void)
{
	int aux;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	volatile u32 temp;
	bcc_header_t comando_lectura_stream;
	bcc_header_t comando_extesion_longitud;
	bcc_header_t comando_ack;
	u32 n_data;

	n_data=MAX_LVDS_CONF_BUFFER_DATA;
	btt=n_data<<2;
	Sent_buff.total_len=0;
	Rec_buff.total_len=0;

	//Xil_DCacheDisable();
	for (u32 i=0; i<MAX_LVDS_CONF_BUFFER_DATA; i++){
		Rec_buff.data[i]=0xB0B01FE0;
		Rec_buff.total_len++;
	}
	Xil_DCacheFlushRange((INTPTR)Rec_buff.data, Rec_buff.total_len*4);

	for (u32 i=0; i<MAX_LVDS_CONF_BUFFER_DATA; i++){
		Sent_buff.data[i]=i;
		Sent_buff.total_len++;
	}
	Sent_buff.data[0]=0xFFFF;
	Sent_buff.data[1]=0xFFFE;
	Sent_buff.data[2]=0xFFFD;

	BCC_write_reg_inmediate(0,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_LINK_REG,MISC_PRO_LINK_LVDS_MASK|MISC_PRO_LINK_LVDS_MASK);

	Xil_DCacheFlushRange((INTPTR)Sent_buff.data, Sent_buff.total_len*4);
	//Xil_DCacheFlush();
	dmb();

	// Configure AXI switch driver
	// Receive: from LVDS_RX->MCBCC->Datamover
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX ,0);
	// Send: from Datamover-> MCBCC -> LVDS_TX
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_BCCMST_IDX,DSP_DMA_SWITCH_SLAVE_FROM_DDRZYN_IDX ,0);

	// Set up datamover to receive: interrupcion o no interrupción?
	//DATAMOV_put_mem_no_int(datamov_instance_t *datamover_instance_p,void *dst_addr,u32 btt);
	//Solo puedo hacer transacciones de 0xFFFF words, si no da fallo

	DATAMOV_put_mem_no_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Rec_buff.data,btt); // En la doc dice que son bytes to transfer, igual hay que multiplicarlo x 4, esto está en words

	//DATAMOV_put_mem_int(datamov_instance_t *datamover_instance_p,void *dst_addr,       u32 btt,       u8 tag,u8 last) 0x00
	//DATAMOV_put_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Rec_buff.data, MAX_LVDS_CONF_BUFFER_DATA,0x05,1);

	// Set up MCBCC to receive
	//MCBCC_lvds_2_stream_after_n_words_int_no_block(u32 num_words_to_stream,u32 num_words_before_stream, u8 last_behavior)
	//MCBCC_lvds_2_stream_after_n_words_int_no_block(MAX_LVDS_CONF_BUFFER_DATA,1, 0x02);
	//MCBCC_lvds_2_stream_after_n_words(u32 num_words_to_stream,u32 words_to_wait)

	//MCBCC_lvds_2_stream_after_n_words(MAX_LVDS_CONF_BUFFER_DATA, 0);
	// Send a write to register command
	//BCC_write_reg_inmediate(127,5,0,0x80808080);

	// Set up the MCBCC to write
	//MCBCC_stream_2_lvds(MAX_LVDS_CONF_BUFFER_DATA);

	//Setting up the MBCC to receive and send

	LSW=(u16)(n_data&0xFFFF);
	MSW	=(u16)(n_data>>16);
	comando_lectura_stream.HEADER = 0x00000000;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
	comando_lectura_stream.STREAM_CMD.MODULE  = 250;
	comando_lectura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;
	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	MCBCC_int_enable(RSEE_MASK);

	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data,prev_words,LAST_DONT_TOUCH);

	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);

	MCBCC_send_HEAD(comando_lectura_stream.HEADER);

	comando_ack=comando_lectura_stream;
	comando_ack.STREAM_CMD.CTRL.BITS.ACK = 1;

	/*for(int i=0;i<1000;i++)
	{
		temp=MCBCC_get_rd_HEAD();
		if(temp==comando_ack.HEADER) return 1;
	}*/

	//BCC_SendBuffer(&Sent_buff);  // De momento esto funciona ¯\_(:))_/¯ , al hacer bypass del datamover, se escribe directamene desde la memoria de la UCI a la linea LVDS con el mcbcc

	{
		MCBCC_stream_2_lvds(MAX_LVDS_CONF_BUFFER_DATA);
		// Set up data mover to send
		//DATAMOV_get_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Sent_buff.data,btt,0x05,LAST_FORCE_ONE);
		DATAMOV_get_mem_no_int(&Datamover_instances[LOCAL_DATAMOVER_ID],Sent_buff.data,btt);
	}


	//wait_for(100000);
	Xil_DCacheInvalidateRange((INTPTR)Rec_buff.data, Rec_buff.total_len*4);
	dmb();
	aux=BCC_compare_buffers(Sent_buff.data, Rec_buff.data, MAX_LVDS_CONF_BUFFER_DATA);

	//Xil_DCacheEnable();
	return aux;
}
int BCC_deskew_LVDS(float us_timeout)
{
	volatile u32* base=(u32*)GPIO_CONF_ADDR;
	volatile u32* base_input=(u32*)GPIO_CONF_INPUT_ADDR;

	*base= *base | GPIO_CONF_LVDS_UNLOCK_DESKEW_CONF_MASK;
	*base= *base & (~GPIO_CONF_LVDS_UNLOCK_DESKEW_CONF_MASK);

	*base= *base | GPIO_CONF_LVDS_DESKEW_START_MASK;
	while (!(*base_input & GPIO_CONF_LVDS_DESKEW_PAT)); //Espera aquí hasta que el deskew pat se recibe de vuelta
	TIMER_Sleep(us_timeout); // sleep
	*base= *base & (~GPIO_CONF_LVDS_DESKEW_START_MASK); // Al acabar se pone a 0

	*base= *base | GPIO_CONF_LVDS_BITSLIP_START_MASK;
	while (!(*base_input & GPIO_CONF_LVDS_BITSLIP_PAT)); //Espera aquí hasta que el bitslip pat se recibe de vuelta
	TIMER_Sleep(us_timeout); // sleep
	*base= *base & (~GPIO_CONF_LVDS_BITSLIP_START_MASK);

	*base= *base | GPIO_CONF_LVDS_WORDALIGN_MASK;
	while (!(*base_input & GPIO_CONF_LVDS_WORDALIGN_PAT)); //Espera aquí hasta que el wordalign pat se recibe de vuelta
	TIMER_Sleep(us_timeout); // sleep

	*base= *base & (~GPIO_CONF_LVDS_WORDALIGN_MASK);

	TIMER_Sleep(us_timeout); // sleep

	while(!(*base_input & GPIO_CONF_LVDS_CONF_DONE_MASK));


	return (0);
}

int BCC_set_dynamic_phase(int minibase, int delay_ps, float us_timeout)
{	/*
	 * Function: BCC_set_dynamic_phase
	 * ----------------------------
	 *   Hace el retardo o adelanto del reloj de entrada del módulo mmcm,
	 *   seguido de un proceso de deskew para restablecer la comunicación
	 *
	 *	 int minibase, el módulo en el que se desea cambiar la fase
	 *	 int delay_ps, el valor en ps de retardo (>0) o adelanto (<0) en la fase
	 *   float us_timeout, tiempo de espera en us de deskew
	 *
	 *   returns: 0 si no ha habido error de deskew, 1 en caso contrario
	 */
	int deskew_nok;
	volatile u32 data_read;
	volatile u32* base=(u32*)GPIO_CONF_ADDR;

	//Unlock deskew conf in UCI

	//Unlock deskew conf in modules with a write to bit 4 (UDC) of PS_CONTROL_REG in the MISC_REG submodule
	// could be sent only to all the modules that WON'T be phase shifted, in case this fails

	// Es necesario hacer esto, ya que por la logica que hice el cambio de fase tambien pone a 1
	// la señal unlock_deskew, que va a flanco dentro del modulo LVDS_RX
	BCC_write_reg_inmediate(0,MISC_BUSSAR_SUBMOD_ADDR,MISC_PS_CONTROL_REG,0);
	BCC_write_reg_inmediate(0,MISC_BUSSAR_SUBMOD_ADDR,MISC_PS_CONTROL_REG,MISC_PS_CONTROL_UDC_MASK);
	BCC_read_reg(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_PS_CONTROL_REG,&data_read);
	BCC_write_reg_inmediate(0,MISC_BUSSAR_SUBMOD_ADDR,MISC_PS_CONTROL_REG,0);
	BCC_read_reg(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_PS_CONTROL_REG,&data_read);
	TIMER_Sleep(100.0);
	//BCC_read_reg(1,MISC_BUSSAR_SUBMOD_ADDR,MISC_PS_CONTROL_REG,&data_read);

	//set_dynamic_phase_ps(1,-1000);
	set_dynamic_phase_ps(minibase,delay_ps);
	//Desbloqueo de deskew
	*base= *base | GPIO_CONF_LVDS_UNLOCK_DESKEW_CONF_MASK;
	*base= *base & (~GPIO_CONF_LVDS_UNLOCK_DESKEW_CONF_MASK);
	TIMER_Sleep(100.0);
	deskew_nok=BCC_deskew_LVDS(100000.0);

	if(deskew_nok)
		xil_printf("\n\rERROR! LVDS no realineados!\n\r");
	else
		xil_printf("\n\rLVDS realineados correctamente.\n\r");

	return deskew_nok;
}



// Copiada de la función de Cruza en main_uci
// Funcionamiento con SW trigger sin y con prescaler parece OK
int BCC_test_TGC_module()
{

	volatile u32 data_read;
	int num_ptos=0,i,j,valor=0;
	u8 minibase=1;
	u8 tgc_values[500];
	TGC_UT_struct_t tgc_regs;
	//u32 *puntero_remoto=(u32*)(0x43C3041C);

	//BCC_write_reg_inmediate(0,4,0,0x00000001);
	//i=BCC_read_reg(1,4,0,&data_read);
	BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,0x0000FFFF);
	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,&data_read);

	BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,0x00000010);
	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,&data_read);

	BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,0x0000FFFF);
	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,&data_read);

	BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,0x00000020);
	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,&data_read);

	BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,0x000FFFFF); // Si escribo esto me da 1fe de vuelta???
	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,PRESENT_ATTN_R,&data_read);

	for(i=0;num_ptos<128;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor++;

	for(i=0;num_ptos<256;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor--;
	tgc_values[0]=0x40;

	for(i=0;num_ptos<384;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor++;

	for(i=0;num_ptos<500;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor--;
	/*
	for(i=0;num_ptos<768;num_ptos++,i++)
			tgc_values[num_ptos]=255-i;

	for(i=0;num_ptos<1024;num_ptos++,i++)
			tgc_values[num_ptos]=255-i;
*/
	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);

	prog_tgc_curve(tgc_values, num_ptos, 0, minibase, &Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);

	for(i=0;i<40;i++)
	{
		BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,i);
		for(j=0;j<50;j++);
		BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,&data_read);
		xil_printf("\n\rTGC_MEM_PNTR_R: 0x%08X\n\r",data_read);
		BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_DATA_R,&data_read);
		xil_printf("TGC_MEM_DATA_R: 0x%08X\n\r",data_read);
	}

	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);

	Global_commnd_buf.total_len = 0;

	tgc_regs.tgc_ini_delay = 10;
	tgc_regs.tgc_mem_ini_end.BITS.tgc_mem_ini = 0;
	tgc_regs.tgc_mem_ini_end.BITS.tgc_mem_end = num_ptos-1;
	tgc_regs.tgc_general_ctrl.tgc_general_ctrl_u32 = 0;
	tgc_regs.tgc_general_ctrl.BITS.tgc_enable = 0;
	tgc_regs.tgc_general_ctrl.BITS.stop = 1;
	tgc_regs.tgc_general_ctrl.BITS.software_trigger = 0;//En cuanto mandemos esta configuracion se disparara
	tgc_regs.tgc_prescaler = 2;

	prog_tgc_UT_regs(&tgc_regs,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len = 0;

	i=BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);

	//for(i=0;i<4;i++)
		BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_GEN_R,0x000a0001);   //   TGC EN, el a es innecesario
		BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_GEN_R,0x880a0001);	 //	  SW Trigger y TGC EN, el segundo 8 y el a es innecesario
	for(i=0;i<40;i++)
	{
		BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_GEN_R,&data_read);
		xil_printf("\n\rTGC_GEN_R:      0x%08X\n\r",data_read);
		BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,&data_read);
		xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);
	}

	xil_printf("Quitamos bit de test...\n\r");
																//0xF0000001
	BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_GEN_R,0x20000000); //Se escribe en todo: SW TRIG, EXT TRIG MASK, END IGNORE Y STOP,
														// viendo el HW debería tener prioridad el stop
	//Al mandar 0x20000000 se queda en armed
	for(i=0;i<40;i++)
		{
			BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_GEN_R,&data_read);
			xil_printf("\n\rTGC_GEN_R:      0x%08X\n\r",data_read);
			BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,&data_read);
			xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);
		}
	i=BCC_read_reg(minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);




	return(0);
}


int BCC_wait_PRO_fall(float us_timeout)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	u32 pro_in=base[PRO_REG]&PROI;
	u32 result;

	volatile u32 flag_end;


	if(us_timeout>0)
	{
		if ((result = TIMER_set_one_count(us_timeout,TIMER_SLEEP)) != 0)
		{
			//UCI_datapath_clear();
			return RLOG(result);
		}
	}

	if(us_timeout>0)
		if ((result = TIMER_Start(TIMER_SLEEP)) != 0) return RLOG(result);

	flag_end=MCBCC_get_flags(POFF_MASK);

	if(flag_end==0&&pro_in==0)
		return ELOG(charEBCC_INTERRUPT_LOST, EBCC_INTERRUPT_LOST);

	while (!flag_end)
	{
		if (us_timeout==0) break;
		WFI_bcc();
		if (us_timeout>0 && TIMER_Timeout(TIMER_SLEEP))
		{
			pro_in=base[PRO_REG]&PROO;
			//UCI_datapath_clear();
			if(pro_in==0)
			{
				return ELOG(charEBCC_INTERRUPT_LOST_TIMEOUT, EBCC_INTERRUPT_LOST_TIMEOUT);
			}
			else
			{
				return ELOG(charEBCC_NO_FLAG_IN_TIME, EBCC_NO_FLAG_IN_TIME);
			}
		}
		flag_end=MCBCC_get_flags(POFF_MASK);
	}
	if (us_timeout>0)
		if ((result = TIMER_Stop(TIMER_SLEEP)) < 0) return RLOG(result);

	return EBCCBUSSAR_NONE;

}

int BCC_compare_buffers(u32 *Sent_data, u32 *Rec_data, u32 len){

	int n_diff=0;
	for (int i=0; i<len; i++){
		if (Sent_data[i]!=Rec_data[i])
		{
			n_diff++;
			if (n_diff <100)
				xil_printf("\r\n Valor enviado: %08X  , Valor recibido %08X", Sent_data[i],Rec_data[i]);
		}
	}
	xil_printf("\r\n Ha habido %d fallos\r\n", n_diff);

	return n_diff;
}
int BCC_wait_flag(u32 waiting_flags,float us_timeout)
{
	u32 result;
	volatile u32 flag_end;


	if(us_timeout>0)
	{
		if ((result = TIMER_set_one_count(us_timeout,TIMER_SLEEP)) != 0)
		{
			//UCI_datapath_clear();
			return RLOG(result);
		}
	}

	if(us_timeout>0)
		if ((result = TIMER_Start(TIMER_SLEEP)) != 0) return RLOG(result);

	flag_end=MCBCC_get_flags(waiting_flags);
	while (!flag_end)
	{
		if (us_timeout==0) break;
		WFI_bcc();
		if (us_timeout>0 && TIMER_Timeout(TIMER_SLEEP))
		{
			//UCI_datapath_clear();
			if(MCBCC_get_reg_flags(waiting_flags))
			{
				return ELOG(charEBCC_INTERRUPT_LOST, EBCC_INTERRUPT_LOST);

			}
			return ELOG(charEBCC_NO_FLAG_IN_TIME, EBCC_NO_FLAG_IN_TIME);
		}
		flag_end=MCBCC_get_flags(waiting_flags);
	}
	if (us_timeout>0)
		if ((result = TIMER_Stop(TIMER_SLEEP)) < 0) return RLOG(result);

	return EBCCBUSSAR_NONE;
}

void BCC_write_PRO(u16 pro_val, u8 module)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 temp;
	bcc_header_t write_pro_cmd;
	bcc_header_t write_pro_res;
	write_pro_cmd.HEADER = 0x00000000;
	write_pro_cmd.SPECIAL_CMD.CTRL.BITS.WR = 1;
	write_pro_cmd.SPECIAL_CMD.CTRL.BITS.SPC = 1;
	write_pro_cmd.SPECIAL_CMD.MODULE = module;
	write_pro_cmd.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_PRO_REG;
	write_pro_cmd.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = pro_val;
	temp=base[RD_STATUS];// Se lee para borrar los flags
	base[WR_WORD_HEAD]=write_pro_cmd.HEADER;
	do{
		temp = base[RD_STATUS];
	}while(temp&RVHD_MASK);
	write_pro_res.HEADER = base[RD_WORD_HEAD];
}

void BCC_write_reg_inmediate(u8 module,u8 submod,u8 reg,u32 data)
{
	bcc_header_t write_header;

	write_header.HEADER = 0x00000000;
	write_header.BUSSAR_CMD.CTRL.BITS.WR = 1;
	write_header.BUSSAR_CMD.MODULE = module;
	write_header.BUSSAR_CMD.SUBMOD = submod;
	write_header.BUSSAR_CMD.REGIST = reg;
	MCBCC_send_HEAD(write_header.HEADER);
	MCBCC_send_DATA(data);
}

int BCC_read_reg(u8 module,u8 submod,u8 reg,u32 *data_read)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 temp,prev_val;
	bcc_header_t read_header;
	bcc_header_t data;
	int num_words_error=0;


	temp = base[RD_STATUS];
	prev_val = temp;

	while(temp&RVDT_MASK)
	{
		num_words_error++;
		//MCBCC_print_regs();
		base[RD_STATUS]|=RONE_MASK;
		temp = base[RD_STATUS];
	}
	if(num_words_error) WARNING_PRINTF("WARNING se ha intentado una lectura en el BCC, pero ya había datos listos en el BCC (%d).\n\r",num_words_error);
	base[WR_WORD_COUNT] = 0;
	base[RD_WORD_COUNT] = 0;
	base[WR_RD_DIFF] = 0;

	base[RD_STATUS] |= RSAL_MASK | RRDY_MASK;

	read_header.HEADER = 0x00000000;
	read_header.BUSSAR_CMD.CTRL.BITS.RD = 1;
	read_header.BUSSAR_CMD.MODULE = module;
	read_header.BUSSAR_CMD.SUBMOD = submod;
	read_header.BUSSAR_CMD.REGIST = reg;
	data = read_header;
	data.BUSSAR_CMD.CTRL.BITS.ACK=1;
	MCBCC_send_HEAD(read_header.HEADER);
	MCBCC_send_DATA(data.HEADER);

	do{
		temp = base[RD_STATUS];
	}while(temp&RVDT_MASK);

	*data_read=base[RD_WORD_DATA];
	read_header.HEADER=base[RD_WORD_HEAD];

	base[RD_STATUS] = prev_val;
	if(read_header.BUSSAR_CMD.CTRL.BITS.ACK)
	{
		return EBCCBUSSAR_NONE;
	}
	else
	{
		return ELOG(charEBCC_MCBCC_READ_NO_ACK_ERROR, EBCC_MCBCC_READ_NO_ACK_ERROR);
	}
}

u16 BCC_read_PRO(u8 module)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 temp;
	bcc_header_t read_pro_cmd;
	volatile bcc_header_t read_pro_res;

	read_pro_cmd.HEADER = 0x00000000;
	read_pro_cmd.SPECIAL_CMD.CTRL.BITS.RD = 1;
	read_pro_cmd.SPECIAL_CMD.CTRL.BITS.SPC = 1;
	read_pro_cmd.SPECIAL_CMD.MODULE = module;
	read_pro_cmd.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_PRO_REG;
	temp=base[RD_STATUS];// Se lee para borrar los flags
	base[WR_WORD_HEAD]=read_pro_cmd.HEADER;
	do{
		temp = base[RD_STATUS];
	}while(temp&RVHD_MASK);
	read_pro_res.HEADER = base[RD_WORD_HEAD];
	return read_pro_cmd.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL;
}
int push_bussar_write_reg(u32 data,u8 module,u8 submodule,u8 reg,commnd_buffer_t *commnd_buf)
{
	bcc_header_t write_header;
	if(commnd_buf->total_len+2>commnd_buf->max_len)
		return ELOG(charEBCCBUSSAR_COMMNDBUFF_OVF, EBCCBUSSAR_COMMNDBUFF_OVF);

	write_header.HEADER = 0x00000000;
	write_header.BUSSAR_CMD.CTRL.BITS.WR=1;
	write_header.BUSSAR_CMD.MODULE=module;
	write_header.BUSSAR_CMD.SUBMOD=submodule;
	write_header.BUSSAR_CMD.REGIST=reg;

	commnd_buf->data[commnd_buf->total_len++]=write_header.HEADER;
	commnd_buf->data[commnd_buf->total_len++]=data;

	return 2;
}

int push_bussar_u32(u32 data_to_push,commnd_buffer_t *commnd_buf)
{
	if(commnd_buf->total_len+1>MAX_COMMAND_BUFFER_DATA)
		return ELOG(charEBCCBUSSAR_COMMNDBUFF_OVF, EBCCBUSSAR_COMMNDBUFF_OVF);

	commnd_buf->data[commnd_buf->total_len++]=data_to_push;

	return 1;
}

int BCC_SendBuffer(commnd_buffer_t *commnd_buf)
{
	int result;
	u32 *command=commnd_buf->data;
	u32 n_command=commnd_buf->total_len;

	if(commnd_buf->total_len>MAX_COMMAND_BUFFER_DATA)
		return ELOG(charEBCCBUSSAR_COMMNDBUFF_OVF, EBCCBUSSAR_COMMNDBUFF_OVF);

	if (n_command < 100||SEND_BUFFER_NO_DMA)
	{
		while(n_command--)
		{
			if(n_command)
				MCBCC_send_HEAD(*command++);
			else
				MCBCC_send_DATA(*command++);
		}
	}
	else
	{
		Xil_DCacheFlushRange(command,n_command*sizeof(u32));
		dmb();
		MCBCC_lvds_2_stream_after_n_words_int_no_block(n_command,0, 1);
		DATAMOV_get_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],command,n_command*sizeof(u32),0,1);
		if((result=BCC_wait_flag(WSEF_MASK,10000.0))<0)
			return ELOG(charEBCC_MCBCC_MST_WR_ERROR, EBCC_MCBCC_MST_WR_ERROR);
		if(DATAMOV_poll_MM2S_end(&Datamover_instances[LOCAL_DATAMOVER_ID])==0)
			return ELOG(charEBCC_MCBCC_DATAMOVER_ERROR, EBCC_MCBCC_DATAMOVER_ERROR);
	}

	return EBCCBUSSAR_NONE;
}

#endif /* SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_C_ */
