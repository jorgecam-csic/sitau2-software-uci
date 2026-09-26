// -----------------------------------------------------------------------------
/**
@file uci_dma.c

@brief Este archivo contiene la implementaciï¿½n de las funciones de la clase \c FL_Aperture.<br>

Esta clase tienen la funcionalidad necesaria para definir las aperturas que componen
un barrido.

@author (rg) Ricardo Gonzï¿½lez

<pre>
MODIFICATION HISTORY:

Ver   Who  Date       Changes
----- ---- ---------- -----------------------------------------------------------
1.00a (rg) 16/01/2017 First release
</pre>
*/

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "uci_dma.h"
#include "uci_set.h"
#include "ttimer.h"
#include "uci_error_code.h"
#include "bcc_bussar.h"
#include "switch_driver.h"
#include "datamover_remote.h"
//int DMA_Datamover_poll_end();
//int DMA_Datamover_wait_until_end();

#define SUM_MODO_SEGURO 1

int word16_last=0;
int word32_last=0;
u8 DMA_busy=0;
u32 Bytes_to_transfer_sent_to_datamover=0;
datamov_instance_t *DMA_datamover_p=NULL;

#ifdef CODIGO_A_ELIMINAR
int DMA_Set_Datamover_Write16(int n_data, int *n_buffer_data)
{
	u32 num_bytes_avilable_buffer;
	u32 total_bytes_to_buffer;
	u32 bytes_first_blow,bytes_second_blow;
	u8* dst_addr;
//	u32 total_samples_to_buffer;
	u8* image_buff_start_addr=IMAGE_prod_p->buffer_start_address;

	if(DMA_busy)
   {
		UCI_datapath_clear();
      return ELOG(charEUCI_Start_DMA_not_end, EUCI_Start_DMA_not_end);
   }

	if(word16_last)
   {
		UCI_datapath_clear();
		return ELOG(charEUCI_Start_DMA_not_end, EUCI_Start_DMA_not_end);
   }

	total_bytes_to_buffer=n_data<<1;

	num_bytes_avilable_buffer=prod_num_data_to_write(IMAGE_prod_p, n_buffer_data);

	if(total_bytes_to_buffer>num_bytes_avilable_buffer)
   {
//		UCI_datapath_clear();
		return ELOG(charEUCI_no_mem_img_buffer, EUCI_no_mem_img_buffer);
   }

	if(total_bytes_to_buffer>((1<<23)-2))
   {
		UCI_datapath_clear();
		return ELOG(charEUCI_max_bytes_datamover, EUCI_max_bytes_datamover);
   }

	//Aquï¿½ se da por buena la situaciï¿½n del sistema
	bytes_first_blow=prod_memref(IMAGE_prod_p,total_bytes_to_buffer,&dst_addr);

	if((u32)dst_addr<0x20000000)
		xil_printf("UYUYUYUUUUYYYUYUYUYUYUY!!!!!!!!!");

	if(bytes_first_blow<total_bytes_to_buffer)
		bytes_second_blow=total_bytes_to_buffer-bytes_first_blow;
	else
		bytes_second_blow=0;

	if(bytes_second_blow)
	{
		DATAMOV_put_mem_int(DMA_datamover_p,dst_addr,bytes_first_blow,0x1,0);
		DATAMOV_put_mem_int(DMA_datamover_p,image_buff_start_addr,bytes_second_blow,0xF,1);
//		xil_printf("DMA da la vuelta al buffer! (Comentar esta lï¿½nea, ya que no es ningï¿½n problema)\r\n");
//		xil_printf("DMA (first): dst_addr = 0x%08X, btt = %d\r\n",(u32)dst_addr,bytes_first_blow);
//		xil_printf("DMA (secnd): dst_addr = 0x%08X, btt = %d\r\n",(u32)image_buff_start_addr,bytes_second_blow);

	}
	else
	{
		DATAMOV_put_mem_int(&Datamover_instances[0],dst_addr,bytes_first_blow,0xF,1);
		//xil_printf("DMA: dst_addr = 0x%08X, btt = %d\r\n",(u32)dst_addr,bytes_first_blow);
	}

	word16_last=n_data;

	DMA_busy=1;

	Bytes_to_transfer_sent_to_datamover=total_bytes_to_buffer;

	return EUCI_NONE;
}
int DMA_Write16(u16 data)
{
	u8 last_behavior;
	int result;
	if(!DMA_busy)
   {
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
   }

	if(word16_last<=0)
   {
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
   }

	word16_last--;

	if(word16_last==0)
		last_behavior=1;
	else
		last_behavior=0;

	result=MCBCC_write_word32_to_stream((u32)data,last_behavior);

	if(result==MCBCC_MST_ERR_READ_OP_BUSY) 
   {
		UCI_datapath_clear();
      return ELOG(charEUCI_DMA_MCBCC_Write_word16_busy, EUCI_DMA_MCBCC_Write_word16_busy);
   }
	if(result==MCBCC_MST_ERR_READ_OP_STREAM_NRDY) 
   {
		UCI_datapath_clear();
      return ELOG(charEUCI_DMA_Write_word16_Stream_nrdy, EUCI_DMA_Write_word16_Stream_nrdy);
   }

	return EUCI_NONE;
}
int DMA_Write16_from_Amplia_wait_end(float us_timeout)
{
   if (gb_hw_sitau_enabled == 1)
   {
      int result;
      u32 flag_end=0;
      u32 words_left_err;

      if(us_timeout>0)
      {
         if ((result = TIMER_set_one_count(us_timeout,TIMER_SLEEP)) < 0)
         {
            UCI_datapath_clear();
            return RLOG(result);
         }
      }
      if(us_timeout>0)
      {
         if ((result = TIMER_Start(TIMER_SLEEP)) < 0)
         {
            UCI_datapath_clear();
            return RLOG(result);
         }
      }

      flag_end=MCBCC_get_flags(RSEF_MASK);
      while(!flag_end)
      {
         if(us_timeout==0)
            break;
         WFI_dma();
         flag_end=MCBCC_get_flags(RSEF_MASK);
         if(us_timeout>0&&TIMER_Timeout(TIMER_SLEEP)&&flag_end==0)
         {
            words_left_err = MCBCC_get_rd_stream_words_left();
            xil_printf("\r\nMCBCC_get_rd_stream_words_left(): %d", words_left_err);              
            MCBCC_print_regs();
            UCI_datapath_clear();         
            return ELOG(charEUCI_MCBCC_MST_rd_stream_timeout, EUCI_MCBCC_MST_rd_stream_timeout);
         }

      }
      if(us_timeout==0 && flag_end==0)
      {
         UCI_datapath_clear(); 
         return ELOG(charEUCI_MCBCC_MST_rd_stream_not_end, EUCI_MCBCC_MST_rd_stream_not_end);
      }

      if(us_timeout>0)
      {
         if ((result = TIMER_Stop(TIMER_SLEEP)) < 0)
         {
            UCI_datapath_clear(); 
            return RLOG(result);
         }
      }

      if(MCBCC_get_rd_vld_in_flag())
      {
         UCI_datapath_clear(); 
         return ELOG(charEUCI_MCBCC_MST_rd_stream_more_data_on_stream, EUCI_MCBCC_MST_rd_stream_more_data_on_stream);
      }
   }
// #endif
	return EUCI_NONE;
}
int DMA_Prepare_Write16_from_Amplia(int n_data)
{
char str[100];
// #if !defined(_NO_HW_SITAU) 
   if (gb_hw_sitau_enabled == 1)
   {
      u8 last_behavior;
      if (!DMA_busy)
      {
         UCI_datapath_clear();
         return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
      }

      if (word16_last < n_data)
      {
         sprintf(str, "\r\n(word16_last(%d) < n_data(%lu)",word16_last, n_data);
         ELOG(str, -1);
         UCI_datapath_clear();
         return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
      }

      if (word16_last == n_data) last_behavior=LAST_FORCE_ONE;
      else last_behavior = LAST_FORCE_ZERO;

      word16_last-=n_data;

      MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data,0,last_behavior);
   }
// #endif
	return EUCI_NONE;
}
#endif //CODIGO_A_ELIMINAR
// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**
Configura el acceso a la escritura DMA estableciendo el nï¿½mero total de datos que se 
van a escribir en diferentes accesos.

@param[in] n_data   Nï¿½mero de datos que se van a escribir.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int DMA_init(datamov_instance_t *datamover_instance_pointer)
{

	DMA_datamover_p=datamover_instance_pointer;
	word16_last=0;
	word32_last=0;
	DMA_busy=0;
	Bytes_to_transfer_sent_to_datamover=0;
	return EUCI_NONE;
}

u32 DMA_Get_bytes_available(void)
{
	return prod_num_data_to_write(IMAGE_prod_p, NULL);
}

int DMA_Set_Datamover_Write32(u32 n_data, u32 *n_buffer_data)
{
	u32 num_bytes_avilable_buffer;
	u32 total_bytes_to_buffer;
	u32 bytes_first_blow,bytes_second_blow;
	u8* dst_addr;
//	u32 total_samples_to_buffer;
	u8* image_buff_start_addr=IMAGE_prod_p->buffer_start_address;

	if(DMA_busy)
   {
		UCI_datapath_clear();
      return ELOG(charEUCI_Start_DMA_not_end, EUCI_Start_DMA_not_end);
   }

	if(word32_last)
   {
		UCI_datapath_clear();
		return ELOG(charEUCI_Start_DMA_not_end, EUCI_Start_DMA_not_end);
   }

	total_bytes_to_buffer=n_data<<2;

	num_bytes_avilable_buffer=prod_num_data_to_write(IMAGE_prod_p, n_buffer_data);

	if(total_bytes_to_buffer>num_bytes_avilable_buffer)
   {
//		UCI_datapath_clear();
		return ELOG(charEUCI_no_mem_img_buffer, EUCI_no_mem_img_buffer);
   }

	if(total_bytes_to_buffer>((1<<23)-4))
   {
		//UCI_datapath_clear();
		return ELOG(charEUCI_max_bytes_datamover, EUCI_max_bytes_datamover);
   }

	//Aquï¿½ se da por buena la situaciï¿½n del sistema
	bytes_first_blow=prod_memref(IMAGE_prod_p,total_bytes_to_buffer,&dst_addr);

	if((u32)dst_addr<0x20000000)
		xil_printf("UYUYUYUUUUYYYUYUYUYUYUY!!!!!!!!!");

	if(bytes_first_blow<total_bytes_to_buffer)
		bytes_second_blow=total_bytes_to_buffer-bytes_first_blow;
	else
		bytes_second_blow=0;

	if(bytes_second_blow)
	{
		DATAMOV_put_mem_int(DMA_datamover_p,dst_addr,bytes_first_blow,0x1,0);
		DATAMOV_put_mem_int(DMA_datamover_p,image_buff_start_addr,bytes_second_blow,0xF,1);
//		xil_printf("DMA da la vuelta al buffer! (Comentar esta lï¿½nea, ya que no es ningï¿½n problema)\r\n");
//		xil_printf("DMA (first): dst_addr = 0x%08X, btt = %d\r\n",(u32)dst_addr,bytes_first_blow);
//		xil_printf("DMA (secnd): dst_addr = 0x%08X, btt = %d\r\n",(u32)image_buff_start_addr,bytes_second_blow);

	}
	else
	{
		DATAMOV_put_mem_int(&Datamover_instances[LOCAL_DATAMOVER_ID],dst_addr,bytes_first_blow,0xF,1);
		//xil_printf("DMA: dst_addr = 0x%08X, btt = %d\r\n",(u32)dst_addr,bytes_first_blow);
	}

	word32_last=n_data;

	DMA_busy=1;

	Bytes_to_transfer_sent_to_datamover=total_bytes_to_buffer;

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Escribe en memoria un dato a traves de DMA

@param[in] data   Dato que se escribe en memoria.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------


int DMA_Write32(u32 data)
{
	u8 last_behavior;
	int result;
	if(!DMA_busy)
   {
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
   }

	if(word32_last<=0)
   {
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
   }

	word32_last--;

	if(word32_last==0)
		last_behavior=1;
	else
		last_behavior=0;

	result=MCBCC_write_word32_to_stream(data,last_behavior);

	if(result==MCBCC_MST_ERR_READ_OP_BUSY) 
   {
		UCI_datapath_clear();
      return ELOG(charEUCI_DMA_MCBCC_Write_word16_busy, EUCI_DMA_MCBCC_Write_word16_busy);
   }
	if(result==MCBCC_MST_ERR_READ_OP_STREAM_NRDY) 
   {
		UCI_datapath_clear();
      return ELOG(charEUCI_DMA_Write_word16_Stream_nrdy, EUCI_DMA_Write_word16_Stream_nrdy);
   }

	return EUCI_NONE;
}

int DMA_Write32_from_BCC_wait_end(float us_timeout)
{
	int result;
	u32 flag_end=0;
	u32 words_left_err;

	if(us_timeout>0)
	{
		if ((result = TIMER_set_one_count(us_timeout,TIMER_SLEEP)) < 0)
		{
			UCI_datapath_clear();
			return RLOG(result);
		}
	}
	if(us_timeout>0)
	{
		if ((result = TIMER_Start(TIMER_SLEEP)) < 0)
		{
			UCI_datapath_clear();
			return RLOG(result);
		}
	}

	flag_end=MCBCC_get_flags(RSEF_MASK);
	while(!flag_end)
	{
		if(us_timeout==0)
			break;
		WFI_dma();
		flag_end=MCBCC_get_flags(RSEF_MASK);
		if(us_timeout>0&&TIMER_Timeout(TIMER_SLEEP)&&flag_end==0)
		{
			if(MCBCC_get_read_stream_end())
			{
				if (0)
				{
				u32 flags;

				flags=MCBCC_get_reg_flags(0);
				xil_printf("FLAGS REALES:\n\r");
				MCBCC_print_int_flags(flags);

				flags=MCBCC_get_flags(0);
				xil_printf("FLAGS COPIA:\n\r");
				MCBCC_print_int_flags(flags);
				}
				xil_printf("\r\nTIMEOUT ENDED!!! Pero todo OK\r\n");
				break;
			}
			words_left_err = MCBCC_get_rd_stream_words_left();
			xil_printf("\r\nMCBCC_get_rd_stream_words_left(): %d", words_left_err);
			MCBCC_print_regs();
			UCI_datapath_clear();
			MCBCC_print_regs();
			return ELOG(charEUCI_MCBCC_MST_rd_stream_timeout, EUCI_MCBCC_MST_rd_stream_timeout);
		}
	}
	if(us_timeout==0 && flag_end==0&&MCBCC_get_read_stream_end()==0)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_MCBCC_MST_rd_stream_not_end, EUCI_MCBCC_MST_rd_stream_not_end);
	}

	if(us_timeout>0)
	{
		if ((result = TIMER_Stop(TIMER_SLEEP)) < 0)
		{
			//UCI_datapath_clear();
			return RLOG(result);
		}
	}

	if(MCBCC_get_rd_vld_in_flag())
	{
	 UCI_datapath_clear();
	 return ELOG(charEUCI_MCBCC_MST_rd_stream_more_data_on_stream, EUCI_MCBCC_MST_rd_stream_more_data_on_stream);
	}

// #endif
	return EUCI_NONE;
}


int DMA_asymmetrical_memcpy_remote_base_int_no_block_last_ctrl(u32 n_data_DMA,u32 n_data_remote,u8 minibase,u32 remote_addr,u8 last_behavior)
{

	char str[100];
	bcc_header_t comando_lectura_stream;
	bcc_header_t comando_extesion_longitud;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	status_t status_datamov;

	if (!DMA_busy)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
	}

	if (word32_last < n_data_DMA)
	{
		sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data_DMA);
		ELOG(str, -1);
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
	}

	word32_last-=n_data_DMA;

	LSW=(u16)(n_data_remote&0xFFFF);
	MSW=(u16)(n_data_remote>>16);
	btt=n_data_remote<<2;

	comando_lectura_stream.HEADER = 0x00000000;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.RD  = 1;
	comando_lectura_stream.STREAM_CMD.MODULE  = minibase;
	comando_lectura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;

	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber si la transmision es de más de 16 bits (transacciones 32 bit)
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	Global_commnd_buf.total_len=0;

	status_datamov = get_minibase2uci_buffer_datamover_status(minibase); //Leo el estado del datamover. Es por si acaso luego falla, para saber si lo que ha fallado es el datamover.

	MCBCC_clear_counters(); //borramos contadores por si falla tener alguna idea de lo que pasa.
	{
		u32* sswitch;

		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}
	//Preparamos DATAMOVER remoto
	config_minibase2uci_buffer(remote_addr,btt,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	//Preparamos MCBCC_mst
	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data_remote,prev_words,last_behavior);

	//Ejecutamos STREAM
	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
	MCBCC_send_HEAD(comando_lectura_stream.HEADER);


	return EUCI_NONE;
}
int DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(u32 n_data_DMA,u32 n_data_remote,u32 remote_addr,u8 last_behavior)
{
	char str[100];
	bcc_header_t comando_lectura_escritura_stream;
	bcc_header_t comando_extesion_longitud;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	status_t status_datamov;
	u32 word32_index;

	u8 minibase=0;

	if (!DMA_busy)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
	}

	if (word32_last < n_data_DMA)
	{
		sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data_DMA);
		ELOG(str, -1);
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
	}

//	if (word32_last == n_data_DMA) last_behavior=LAST_FORCE_ONE;
//	else last_behavior = LAST_FORCE_ZERO;

	word32_last-=n_data_DMA;

	LSW=(u16)(n_data_remote&0xFFFF);
	MSW=(u16)(n_data_remote>>16);
	btt=n_data_remote<<2;

	comando_lectura_escritura_stream.HEADER = 0x00000000;
	comando_lectura_escritura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_escritura_stream.STREAM_CMD.CTRL.BITS.RD  = 1;
	comando_lectura_escritura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
	comando_lectura_escritura_stream.STREAM_CMD.MODULE  = minibase;
	comando_lectura_escritura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;

	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber si la transmision es de más de 16 bits (transacciones 32 bit)
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	Global_commnd_buf.total_len=0;

	status_datamov = get_minibase2uci_buffer_datamover_status(1); //Leo el estado del datamover. Es por si acaso luego falla, para saber si lo que ha fallado es el datamover.

	MCBCC_clear_counters(); //borramos contadores por si falla tener alguna idea de lo que pasa.


	if(0)
	{
		u32* sswitch;

		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		//Configuración para que sume
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_SMT_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_SMP_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0);
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_SUM_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}

	{
		BCC_write_reg_inmediate(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_LINK_REG,MISC_PRO_LINK_LVDS_MASK|MISC_PRO_LINK_DISABLE_TRIG_SEC_MASK);
		BCC_write_reg_inmediate(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_ADDR_CHAIN_REG,0);
	}

	//Preparamos DATAMOVER remoto
	config_minibase2uci_buffer(remote_addr,btt,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	MCBCC_int_enable(RSEE_MASK);

	//Preparamos MCBCC_mst
	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data_remote,prev_words,last_behavior);

	//Ejecutamos STREAM
	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
	MCBCC_send_HEAD(comando_lectura_escritura_stream.HEADER);

	gtx_set_link_pro_out(1);
	//
	if(0)
	{
		for(int word32_index=0;word32_index<(n_data_remote-1);word32_index++)
		{
			MCBCC_send_HEAD((u32)0x0);
		}
		MCBCC_send_DATA(0x0);
	}
	else
	{
		MCBCC_zeros_2_lvds(n_data_remote);
	}


	return EUCI_NONE;
}
int DMA_asymmetrical_memcpy_remote_base_int_no_block_sum(u32 n_data_DMA,u32 n_data_remote,u32 remote_addr)
{
	char str[100];
	u8 last_behavior;
	bcc_header_t comando_lectura_escritura_stream;
	bcc_header_t comando_extesion_longitud;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	status_t status_datamov;
	u32 word32_index;

	u8 minibase=0;

	if (!DMA_busy)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
	}

	if (word32_last < n_data_DMA)
	{
		sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data_DMA);
		ELOG(str, -1);
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
	}

	if (word32_last == n_data_DMA) last_behavior=LAST_FORCE_ONE;
	else last_behavior = LAST_FORCE_ZERO;

	word32_last-=n_data_DMA;

	LSW=(u16)(n_data_remote&0xFFFF);
	MSW=(u16)(n_data_remote>>16);
	btt=n_data_remote<<2;

	comando_lectura_escritura_stream.HEADER = 0x00000000;
	comando_lectura_escritura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_escritura_stream.STREAM_CMD.CTRL.BITS.RD  = 1;
	comando_lectura_escritura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
	comando_lectura_escritura_stream.STREAM_CMD.MODULE  = minibase;
	comando_lectura_escritura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;

	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber si la transmision es de más de 16 bits (transacciones 32 bit)
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	Global_commnd_buf.total_len=0;

	status_datamov = get_minibase2uci_buffer_datamover_status(1); //Leo el estado del datamover. Es por si acaso luego falla, para saber si lo que ha fallado es el datamover.

	MCBCC_clear_counters(); //borramos contadores por si falla tener alguna idea de lo que pasa.


	{
		u32* sswitch;

		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		//Configuración para que sume
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_SMT_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_SMP_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0);
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_SUM_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}

	{
		BCC_write_reg_inmediate(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_LINK_REG,MISC_PRO_LINK_LVDS_MASK|MISC_PRO_LINK_DISABLE_TRIG_SEC_MASK);
		BCC_write_reg_inmediate(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_ADDR_CHAIN_REG,0);
	}

	//Preparamos DATAMOVER remoto
	config_minibase2uci_buffer(remote_addr,btt,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	MCBCC_int_enable(RSEE_MASK);

	//Preparamos MCBCC_mst
	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data_remote,prev_words,last_behavior);

	//Ejecutamos STREAM
	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
	MCBCC_send_HEAD(comando_lectura_escritura_stream.HEADER);

	gtx_set_link_pro_out(1);
	//
	if(0)
	{
		for(int word32_index=0;word32_index<(n_data_remote-1);word32_index++)
		{
			MCBCC_send_HEAD((u32)0x0);
		}
		MCBCC_send_DATA(0x0);
	}
	else
	{
		MCBCC_zeros_2_lvds(n_data_remote);
	}


	return EUCI_NONE;
}

int DMA_asymmetrical_memcpy_remote_base_int_no_block(u32 n_data_DMA,u32 n_data_remote,u8 minibase,u32 remote_addr)
{

	char str[100];
	u8 last_behavior;
	bcc_header_t comando_lectura_stream;
	bcc_header_t comando_extesion_longitud;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	status_t status_datamov;

	if (!DMA_busy)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
	}

	if (word32_last < n_data_DMA)
	{
		sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data_DMA);
		ELOG(str, -1);
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
	}

	if (word32_last == n_data_DMA) last_behavior=LAST_FORCE_ONE;
	else last_behavior = LAST_FORCE_ZERO;

	word32_last-=n_data_DMA;

	LSW=(u16)(n_data_remote&0xFFFF);
	MSW=(u16)(n_data_remote>>16);
	btt=n_data_remote<<2;

	comando_lectura_stream.HEADER = 0x00000000;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.RD  = 1;
	comando_lectura_stream.STREAM_CMD.MODULE  = minibase;
	comando_lectura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;

	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber si la transmision es de más de 16 bits (transacciones 32 bit)
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	Global_commnd_buf.total_len=0;

	status_datamov = get_minibase2uci_buffer_datamover_status(minibase); //Leo el estado del datamover. Es por si acaso luego falla, para saber si lo que ha fallado es el datamover.

	MCBCC_clear_counters(); //borramos contadores por si falla tener alguna idea de lo que pasa.
	{
		u32* sswitch;

		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}
	//Preparamos DATAMOVER remoto
	config_minibase2uci_buffer(remote_addr,btt,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	MCBCC_int_enable(RSEE_MASK);

	//Preparamos MCBCC_mst
	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data_remote,prev_words,last_behavior);

	//Ejecutamos STREAM
	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
	MCBCC_send_HEAD(comando_lectura_stream.HEADER);


	return EUCI_NONE;
}
int DMA_asymmetrical_memcpy_remote_base_int_no_block_add(u32 n_data_DMA,u32 n_data_remote,u8 minibase,u32 remote_addr)
{

	char str[100];
	u8 last_behavior;
	bcc_header_t comando_lectura_stream;
	bcc_header_t comando_extesion_longitud;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	status_t status_datamov;

	if (!DMA_busy)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
	}

	if (word32_last < n_data_DMA)
	{
		sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data_DMA);
		ELOG(str, -1);
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
	}

	if (word32_last == n_data_DMA) last_behavior=LAST_FORCE_ONE;
	else last_behavior = LAST_FORCE_ZERO;

	word32_last-=n_data_DMA;

	LSW=(u16)(n_data_remote&0xFFFF);
	MSW=(u16)(n_data_remote>>16);
	btt=n_data_remote<<2;

	comando_lectura_stream.HEADER = 0x00000000;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.RD  = 1;
	comando_lectura_stream.STREAM_CMD.MODULE  = minibase;
	comando_lectura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;

	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber si la transmision es de más de 16 bits (transacciones 32 bit)
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	Global_commnd_buf.total_len=0;

	status_datamov = get_minibase2uci_buffer_datamover_status(minibase); //Leo el estado del datamover. Es por si acaso luego falla, para saber si lo que ha fallado es el datamover.

	MCBCC_clear_counters(); //borramos contadores por si falla tener alguna idea de lo que pasa.
	{
		u32* sswitch;

		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}
	//Preparamos DATAMOVER remoto
	config_minibase2uci_buffer(remote_addr,btt,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	MCBCC_int_enable(RSEE_MASK);

	//Preparamos MCBCC_mst
	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data_remote,prev_words,last_behavior);

	//Ejecutamos STREAM
	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
	MCBCC_send_HEAD(comando_lectura_stream.HEADER);


	return EUCI_NONE;
}
int DMA_memcpy_remote_base_int_no_block_last_ctrl(u32 n_data,u8 minibase,u32 remote_addr,u8 last_behavior)
{
	//if (gb_hw_sitau_enabled == 1)

	char str[100];

	bcc_header_t comando_lectura_stream;
	bcc_header_t comando_extesion_longitud;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	status_t status_datamov;

	if (!DMA_busy)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
	}

	if (word32_last < n_data)
	{
		sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data);
		ELOG(str, -1);
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
	}

	word32_last-=n_data;

	LSW=(u16)(n_data&0xFFFF);
	MSW=(u16)(n_data>>16);
	btt=n_data<<2;

	comando_lectura_stream.HEADER = 0x00000000;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.RD  = 1;
	comando_lectura_stream.STREAM_CMD.MODULE  = minibase;
	comando_lectura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;
	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber si la transmision es de más de 16 bits (transacciones 32 bit)
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	Global_commnd_buf.total_len=0;

	status_datamov = get_minibase2uci_buffer_datamover_status(minibase); //Leo el estado del datamover. Es por si acaso luego falla, para saber si lo que ha fallado es el datamover.

	MCBCC_clear_counters(); //borramos contadores por si falla tener alguna idea de lo que pasa.
	{
		u32* sswitch;

		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}
	//Preparamos DATAMOVER remoto
	config_minibase2uci_buffer(remote_addr,btt,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	//Preparamos MCBCC_mst
	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data,prev_words,last_behavior);

	//Ejecutamos STREAM
	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
	MCBCC_send_HEAD(comando_lectura_stream.HEADER);



	return EUCI_NONE;
}
int DMA_memcpy_remote_base_int_no_block(u32 n_data,u8 minibase,u32 remote_addr)
{
	//if (gb_hw_sitau_enabled == 1)

	char str[100];
	u8 last_behavior;
	bcc_header_t comando_lectura_stream;
	bcc_header_t comando_extesion_longitud;
	bcc_header_t comando_ack;
	u16 LSW,MSW;
	u32 btt;
	u32 prev_words;
	volatile u32 temp;
	status_t status_datamov;

	if (!DMA_busy)
	{
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
	}

	if (word32_last < n_data)
	{
		sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data);
		ELOG(str, -1);
		UCI_datapath_clear();
		return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
	}

	if (word32_last == n_data) last_behavior=LAST_FORCE_ONE;
	else last_behavior = LAST_FORCE_ZERO;

	word32_last-=n_data;

	LSW=(u16)(n_data&0xFFFF);
	MSW=(u16)(n_data>>16);
	btt=n_data<<2;

	comando_lectura_stream.HEADER = 0x00000000;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
	comando_lectura_stream.STREAM_CMD.CTRL.BITS.RD  = 1;
	comando_lectura_stream.STREAM_CMD.MODULE  = minibase;
	comando_lectura_stream.STREAM_CMD.LENGHT = LSW;
	prev_words=1;
	if(MSW)
	{
		comando_extesion_longitud.HEADER = 0x00000000;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
		comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
		comando_extesion_longitud.SPECIAL_CMD.MODULE = 0;//minibase;//Es el módulo 0 porque todos tienen que saber si la transmision es de más de 16 bits (transacciones 32 bit)
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
		comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		prev_words=2;
	}
	Global_commnd_buf.total_len=0;
	//Preparamos DATAMOVER remoto

	MCBCC_int_enable(RSEE_MASK);

	status_datamov = get_minibase2uci_buffer_datamover_status(minibase); //Leo el estado del datamover. Es por si acaso luego falla, para saber si lo que ha fallado es el datamover.

	MCBCC_clear_counters(); //borramos contadores por si falla tener alguna idea de lo que pasa.
	{
		u32* sswitch;

		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}
	config_minibase2uci_buffer(remote_addr,btt,minibase,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	//Preparamos MCBCC_mst
	MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data,prev_words,last_behavior);

	//Ejecutamos STREAM
	if(MSW)
		MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
	MCBCC_send_HEAD(comando_lectura_stream.HEADER);

	comando_ack=comando_lectura_stream;
	comando_ack.STREAM_CMD.CTRL.BITS.ACK = 1;

	for(int i=0;i<1000;i++)
	{
		temp=MCBCC_get_rd_HEAD();
		if(temp==comando_ack.HEADER) return EUCI_NONE;
	}

	if(temp!=comando_ack.HEADER)
		xil_printf("\n\rWARNING: el comando de lectura no ha sido confirmado con ACK valor devuelto: %08Xn\r",temp);

	return EUCI_NONE;
}

int DMA_Prepare_Write32_from_BCC_after_n_words(u32 n_data,u32 n_words_to_wait)
{
char str[100];

   //if (gb_hw_sitau_enabled == 1)
   {
      u8 last_behavior;
      if (!DMA_busy)
      {
         UCI_datapath_clear();
         return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
      }

      if (word32_last < n_data)
      {
         sprintf(str, "\r\n(word32_last(%d) < n_data(%lu)",word32_last, n_data);
         ELOG(str, -1);
         UCI_datapath_clear();
         return ELOG(charEUCI_DMA_too_much_data, EUCI_DMA_too_much_data);
      }

      if (word32_last == n_data) last_behavior=LAST_FORCE_ONE;
      else last_behavior = LAST_FORCE_ZERO;

      word32_last-=n_data;

      MCBCC_lvds_2_stream_after_n_words_int_no_block(n_data,n_words_to_wait,last_behavior);
   }

	return EUCI_NONE;
}

int DMA_close_and_send_to_host(float us_timeout)
{
	int result;
	u32 flag_end=0;
//	u32 words_left_err;
	u8 datamover_status;

	if(!DMA_busy)
   {
      UCI_datapath_clear(); 
		return ELOG(charEUCI_DMA_not_set, EUCI_DMA_not_set);
   }

	if(word32_last)
   {
		char str[100];
      sprintf(str, "\r\nword32_last = %d\r\n",word32_last);
      ELOG(str, -1);
      UCI_datapath_clear(); 
      return ELOG(charEUCI_DMA_Closing_not_zero_word16_left, EUCI_DMA_Closing_not_zero_word16_left);
   }

	if(us_timeout>0)
   {
		if ((result=TIMER_set_one_count(us_timeout,TIMER_SLEEP)) < 0)
      {
         UCI_datapath_clear(); 
			return RLOG(result);
      }
   }
   
	if(us_timeout>0)
   {
		if ((result=TIMER_Start(TIMER_SLEEP)) < 0)
      {
         UCI_datapath_clear(); 
			return RLOG(result);
      }
   }

	flag_end=DATAMOV_poll_S2MM_end(DMA_datamover_p);
	while(!flag_end)
	{
		if(us_timeout==0)
			break;
		WFI_dma();
		flag_end=DATAMOV_poll_S2MM_end(DMA_datamover_p);
		if(us_timeout>0&&TIMER_Timeout(TIMER_SLEEP)&&flag_end==0)
		{
			UCI_datapath_clear(); 
         return ELOG(charEUCI_DMA_Closing_Datamover_S2MM_not_end, EUCI_DMA_Closing_Datamover_S2MM_not_end);
		}
	}
	if(us_timeout==0 && flag_end==0)
   {
      UCI_datapath_clear(); 
		return ELOG(charEUCI_DMA_Closing_Datamover_S2MM_not_end, EUCI_DMA_Closing_Datamover_S2MM_not_end);
   }

	if((us_timeout>0) && ((result=TIMER_Stop(TIMER_SLEEP)) < 0) )
   {
      UCI_datapath_clear(); 
      return RLOG(result);
   }

	datamover_status=DATAMOV_S2MM_end_status(DMA_datamover_p);

	if((datamover_status&0x80)==0)
	{
		if(datamover_status&0x10)
		{
			//UCI_datapath_clear();
			return ELOG(charEUCI_DMA_Closing_Datamover_S2MM_end_with_error, EUCI_DMA_Closing_Datamover_S2MM_end_with_error);
		}
		else
		{
			UCI_datapath_clear(); 
         return ELOG(charEUCI_DMA_Closing_Datamover_S2MM_end_with_unknown_error,EUCI_DMA_Closing_Datamover_S2MM_end_with_unknown_error);
		}
	}

	prod_refresh_write(IMAGE_prod_p,Bytes_to_transfer_sent_to_datamover);

	Bytes_to_transfer_sent_to_datamover=0;

	DMA_busy=0;

	return EUCI_NONE;

}

