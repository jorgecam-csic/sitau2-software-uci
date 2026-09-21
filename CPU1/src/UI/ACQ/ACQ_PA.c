#include "ACQ_PA.h"
#include "uci_acquire.h"
#include "uci_error_code.h"
#include "uci_set.h"
#include "uci_reg.h"
#include "timer_util.h"
#include "ttimer.h"
#include "uci_dma.h"
#include "uci_data.h"
#include "trigger.h"
#include "mcbcc_mst_driver.h"
#include "bcc_bussar.h"
#include "datamover_structs.h"
#include "datamover_remote.h"
#include "beamformer_bussar.h"
#include "timestamp.h"
#include "fifo_control_driver.h"
#include "uci_error_code.h"
#include "switch_driver.h"
#include "ch_extract.h"
#include "calc.h"
#include "fir_filter.h"
#include "vch_prg.h"
#include "vch.h"
#include "vch_tad.h"
#include <GTX/axi2gtx.h>
#include "gtx_control.h"
//#include "ACQ_FMC.h"
#include "dspdma_func.h"
#include "bussar_addr.h"
#include "bcc_bussar.h"
#include <ACQ_common.h>

/*
unsigned int PA_end_continous = 0;
unsigned int PA_end=0;
unsigned int PA_n_pros=0;
unsigned int PA_n_images_count=0;
unsigned int PA_n_focal_laws_count=0;
unsigned int PA_n_emi_prom_count=0;
unsigned int PA_n_images_total=0;
unsigned int PA_n_focal_laws_total=0;
unsigned int PA_n_emi_prom_total=0;
unsigned int PA_image_size_bytes=0;
*/

volatile ACQ_hndlr_t PA_handler;
extern volatile int new_host_data;

void PA_continuous_interrupt_callback_function(void)
{
	u32 write_addr_index_new;

	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	if (ACQ->acquisition_ended==1) return; //No debería ser necesario, pero por si acaso.

	if(ACQ->store_timestamp_on_int)
	{
		ACQ->timestamp_value = timestamp_get_ticks_64();
		ACQ->store_timestamp_on_int = 0;
		ACQ->stored_timestamp = 1;
		ACQ->bloq_index_timestamped = ACQ->n_images_count;
	}

	ACQ->n_pros++;

	ACQ->n_emi_prom_count++;

	if(ACQ->n_emi_prom_count>=ACQ->n_emi_prom_total)
	{
		ACQ->n_emi_prom_count=0;
		ACQ->n_focal_laws_count++;

		/*
		if(++ACQ->n_focal_laws_per_block_count==ACQ->n_focal_laws_per_block_total)
		{
			ACQ->n_focal_laws_per_block_count=0;
			ACQ->n_blocks_acquired++;
		}
		*/

		write_addr_index_new = (ACQ->addr_end-ACQ->write_addr_index>2*ACQ->bytes_per_fl_remote?ACQ->write_addr_index+ACQ->bytes_per_fl_remote:ACQ->addr_ini);
		if(write_addr_index_new>=ACQ->read_addr_index && ACQ->write_addr_index<ACQ->read_addr_index)
		{
			ACQ->n_overflows++;
			ACQ->overflow_flag=1;
			xil_printf("\n\rOVERFLOW: %d!!!\n\r",ACQ->n_overflows);
		}
		ACQ->write_addr_index = write_addr_index_new;
		if(0)//(DEBUG_ADDR_WRITE_PA)
		{
			u32 wr_addr_hw;
			wr_addr_hw=get_afe2mem_datamover_next_addr(1);
			if(wr_addr_hw!=write_addr_index_new)
			{
				ACQ->write_addr_index = write_addr_index_new;
				xil_printf("\n\rwr_addr_hw=0x%08X, write_addr_index_new=0x%08X\n\r",wr_addr_hw,write_addr_index_new);
			}
		}
	}

	if(ACQ->n_focal_laws_count>=ACQ->n_focal_laws_total)
	{
		ACQ->n_focal_laws_count = 0;
		ACQ->n_images_count++;

		//if(ACQ->acquisition_mode==AQUISITION_MODE_CONTINOUS_BURST || ACQ->acquisition_mode==AQUISITION_MODE_FIXED_BURST)
		{
			ACQ->n_images_burst_count++;
			if(ACQ->n_images_burst_count>=ACQ->n_images_burst_total)
			{
				ACQ->n_images_burst_count=0;
				ACQ->n_bursts_count++;
				if(ACQ->end_continous)
				{
					ACQ->acquisition_ended=1;
					PA_stop_trigger();
					Timstamp_acq=timestamp_get_ticks_64();
					xil_printf("\n\rFin de adquisición continua en ráfagas!\n\r");
				}
				else if(ACQ->pause_continous || ACQ->pause_after_burst)
				{
					PA_pause_trigger();
					ACQ->store_timestamp_on_int=1;
					xil_printf("\n\rPAUSA!\n\r");
				}
			}
		}

		if(ACQ->end_continous && ACQ->acquisition_mode==AQUISITION_MODE_CONTINOUS)
		{
			ACQ->acquisition_ended=1;
			PA_stop_trigger();
			Timstamp_acq=timestamp_get_ticks_64();
			xil_printf("\n\rFin de adquisición continua!\n\r");
		}
	}
	if(ACQ->n_images_count>=ACQ->n_images_total && (ACQ->acquisition_mode==AQUISITION_MODE_FIXED))
	{
		ACQ->acquisition_ended=1;
		PA_stop_trigger();
		Timstamp_acq=timestamp_get_ticks_64();
		xil_printf("\n\rFin de adquisición fija!\n\r");
	}
	/*
	//ESTO ES IMPORTANTE: Al terminar la adquisición, el último bloque puede quedar a la mitad. Esto es importante determinarlo. Este bloque irá marcado con LAST y tiene que llevar marcado en la cabecera el número de leyes focales válidas que tiene (ACQ->n_focal_laws_per_block_count)
	if(ACQ->acquisition_ended==1 && ACQ->n_focal_laws_per_block_count!=0)
		ACQ->n_blocks_acquired++;
	*/

	gb_uci.ind_acquisitions = ACQ->n_images_count;
}

void PA_stop_continous()
{
	//PA_end_continous = 1;
}

void PA_stop_trigger(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	TIMER_Stop(TIMER_SCAN_PRF_1);
	MCBCC_remove_callback(POF_NBIT);
	MCBCC_remove_callback(TRE_NBIT);
	TRIG_remove_interrupt_source(TRIGGER_PRFTIMER1_MASK);
	UCI_Alarm1(0);
	UCI_Alarm2(0);
}

void PA_pause_trigger(void)
{
	TIMER_Stop(TIMER_SCAN_PRF_1);
	UCI_Alarm1(0);
	UCI_Alarm2(0);	
}

void PA_start_trigger(void)
{
	Timstamp_ini=timestamp_get_ticks_64();
    TIMER_Start(TIMER_SCAN_PRF_1);
    UCI_Alarm1(1);
    UCI_Alarm2(1);	
}

//(unsigned int images,unsigned int n_images_burst,unsigned int focal_laws_per_block,unsigned int focal_laws,unsigned int emi_prom,char acquisition_mode,u32 addr_ini,u32 addr_end,u32 acq_size)
void PA_interrupt_startup(unsigned int images,unsigned int n_images_burst,unsigned int n_lines_image,unsigned int emi_prom,unsigned short acquisition_mode,u32 addr_ini,u32 addr_end,u32 acq_size, char pause_after_burst)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	ACQ->end_continous = 0;
	ACQ->pause_continous = 0;
	ACQ->acquisition_ended=0;
	ACQ->transmission_ended=0;
	ACQ->n_images_sent=0;
	ACQ->n_overflows=0;
	ACQ->n_pros=0;
	ACQ->n_images_count=0;
	ACQ->n_focal_laws_count=0;
	ACQ->n_emi_prom_count=0;
	ACQ->n_focal_laws_total=n_lines_image;
	ACQ->n_emi_prom_total=emi_prom;
	ACQ->n_blocks_sent=0;
	ACQ->n_blocks_acquired=0;
	ACQ->n_blocks_to_send=0;
	ACQ->block_counter_header=0;
	ACQ->image_counter_header=0;
	ACQ->focal_law_index_header=0;
	ACQ->acquisition_mode=acquisition_mode;
	ACQ->pause_after_burst=pause_after_burst;	

	if(ACQ->acquisition_mode==AQUISITION_MODE_FIXED)
	{
		ACQ->n_images_total=images;
		ACQ->n_images_burst_total=n_images_burst;
		ACQ->n_images_burst_count=0;
		ACQ->n_bursts_count=0;
		ACQ->n_bursts_sent=0;
	}
	else if(ACQ->acquisition_mode == AQUISITION_MODE_CONTINOUS)
	{
		ACQ->n_images_total=0;
		ACQ->n_images_burst_total=n_images_burst;
		ACQ->n_images_burst_count=0;
		ACQ->n_bursts_count=0;
		ACQ->n_bursts_sent=0;
	}


	ACQ->write_addr_index = addr_ini;
	ACQ->read_addr_index = addr_ini;
	ACQ->bytes_per_fl_remote = acq_size;
	ACQ->addr_ini = addr_ini;
	ACQ->addr_end = addr_end;

	MCBCC_int_enable(POFE_MASK);

	MCBCC_asign_callback(PA_errorPRO,TRE_NBIT);
	MCBCC_int_enable(TREE_MASK);

	MCBCC_asign_callback(PA_continuous_interrupt_callback_function,POF_NBIT);//	PRO Fall flank interrupt Enable asignado a la interrupción

	ACQ->store_timestamp_on_int=1; //En el primer bloque mandamos timestamp

}


void PA_errorPRO(void)
{
	u32 busy_word;
	status_t status_datamov;
	u32 wr_addr_hw;
	static u32 Errores_pro=0;

	if((Errores_pro % (1024*20))==0)
		xil_printf("\n\rError en el PRO = %d! Se intentó elevar el PRO cuando el PRO estaba en alto!\n\r",Errores_pro);

	Errores_pro++;

	if(0)
	{

		for(u8 base_index=0;base_index<gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_bases;base_index++)
		{

			volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

			u8 base_bcc_addr = gb_fp_virtual_channel[gb_uci.active_virtual_channel].base[base_index].bcc_addr;
			xil_printf("\n\rBASE %d\n\r",base_bcc_addr);
			fifo_control_print_all(base_bcc_addr);
			fifo_control_clear_all(base_bcc_addr);
			busy_word=bcc_get_busy_in_word(base_bcc_addr);
			xil_printf("MISC_NOT_BUSY_IN_REG: 0x%02X\n\r",busy_word);
			busy_word=bcc_get_busy_or_word(base_bcc_addr);
			xil_printf("MISC_NOT_BUSY_OR_REG: 0x%02X\n\r",busy_word);

			status_datamov = get_beamformer2mem_datamover_status(base_bcc_addr);
			//if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
			xil_printf("STATUS: 0x%08X\n\r",status_datamov.status_32);
			wr_addr_hw=get_beamformer2mem_next_addr(base_bcc_addr);
			xil_printf("\n\rwr_addr_hw=0x%08X, ACQ_write_addr_index=0x%08X\n\r",wr_addr_hw,ACQ->write_addr_index);
		}
		xil_printf("\n\r--------------------------------------\n\r");
		bcc_print_all_busy_flags(4);
		xil_printf("\n\r--------------------------------------\n\r");
	}
}

int PA_set_up(TVCH *vch)
{
int result,i,idx_avr;
u32 l;
int prf_acq_number = 1;
u64 timstamp_ini,timstamp_end;
float us_acq;
u16 flags;
u16 emi_prom_rpt;
u32 activacion_salida=0;
int interrupt_mode=0;

u32 remote_image_size;

	Current_ACQ_hndlr_ptr = &PA_handler;

	//TRIG_reset_all_interrupt_sources();
	TIMER_Stop(TIMER_SCAN_PRF_1);
	MCBCC_TRIG2PRO(0);
	MCBCC_set_PRO(0);
	while(MCBCC_get_PROI());

	//TRIG_reset_all_interrupt_sources();


	vch->remote_addr_bf_cur = vch->remote_addr_bf_ini;
	// Configiracion HW
	// ---------------------------------------------------------------------------

	// Se programa en HW sólo una vez

	if ((result = VCH_PRG_EmissionFocalLaws(vch)) < 0) return RLOG(result);
	if ((result = VCH_PRG_BeamformerFocalLaws(vch)) < 0) return RLOG(result);
	if ((result = VCH_PRG_Registers_PA_DMA(vch)) < 0) return RLOG(result);

	if     (vch->processing_type==PROCESSING_TYPE_NONE)   emi_prom_rpt = 1;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_B) emi_prom_rpt = vch->n_signals_processing_avr;
	else if(vch->processing_type==PROCESSING_TYPE_EMI_B)  emi_prom_rpt = vch->n_signals_processing_emi;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_A) emi_prom_rpt = 1<<vch->afe.AFE_BUSSAR_UT.log2_promediados;

	if(gb_log_acquiring)
	{
		AFE_SPI_UT_t regs_afe;
		VCH_PrintConfig(vch);
		get_spi_afe_regs(&regs_afe, 2);
		print_spi_afe_regs(regs_afe);
	}
	// ---------------------------------------------------------------------------
	flags=MCBCC_get_flags(PORF_MASK|POFF_MASK);
	if(gb_log_acquiring) LOG_ACQUIRING("FLAGS 0x%04X\n\r",flags);

	//TRIG_set_interrupt_source(TRIGGER_PRFTIMER1_MASK);
	MCBCC_POAF(1);
	MCBCC_TRIG2PRO(1);
	UCI_Sync(0);
	UCI_set_Sync_is_PRO();
	//UCI_set_Sync_is_SW();
	//UCI_Sync(0);

	if(gb_log_acquiring) LOG_ACQUIRING("\n\rIniciando adquisición...\n\r");
	if(gb_log_acquiring) MCBCC_print_regs();

	flags=MCBCC_get_flags(0);
	if(gb_log_acquiring) LOG_ACQUIRING("FLAGS 0x%04X\n\r",flags);

	activacion_salida=gb_uci.delay_sync_n_acq;

	timstamp_ini=timestamp_get_ticks_64();
	//TIMER_set_periodic_count(1000.0,TIMER_SCAN_PRF_1);
	TIMER_set_periodic_count(vch->prf_time_line,TIMER_SCAN_PRF_1);

	UCI_Alarm1(1);
	UCI_Alarm2(0);
	fifo_control_clear_all(0);


	//TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);
	if (vch->n_efl == 0)
		return 0;

	if (gb_uci.trigger_source == TRG_SCAN_PRF)
		prf_acq_number = gb_uci.n_acquisitions;
	if (prf_acq_number < 1)
		prf_acq_number = 1;

	if(gb_log_acquiring)
		VCH_PrintConfig(vch);

	if(gb_log_acquiring)
	{
		AFE_SPI_UT_t regs_afe;
		get_spi_afe_regs(&regs_afe, 2);
		print_spi_afe_regs(regs_afe);
	}

	// ---------------------------------------------------------------------------
	flags=MCBCC_get_flags(PORF_MASK|POFF_MASK);

	if(gb_log_acquiring)
		LOG_ACQUIRING("FLAGS 0x%04X\n\r",flags);

	TRIG_set_interrupt_source(TRIGGER_PRFTIMER1_MASK);
	MCBCC_POAF(1);
	MCBCC_TRIG2PRO(1);
	UCI_Sync(0);
	UCI_set_Sync_is_PRO();
	//UCI_set_Sync_is_SW();
	//UCI_Sync(0);

	//if(gb_log_acquiring) LOG_ACQUIRING("\n\rIniciando adquisición...\n\r");
	//if(gb_log_acquiring) MCBCC_print_regs();

	flags=MCBCC_get_flags(0);

	if(gb_log_acquiring)
		LOG_ACQUIRING("FLAGS 0x%04X\n\r",flags);

	remote_image_size = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;
//	PA_interrupt_startup(unsigned int images  ,unsigned int n_images_burst,unsigned int n_lines_image,unsigned int emi_prom     ,unsigned short acquisition_mode,u32 addr_ini           ,u32 addr_end           ,u32 acq_size                                              );
	PA_interrupt_startup(gb_uci.n_acquisitions,gb_uci.n_images_burst, vch->pa_image.n_lines, (unsigned int)emi_prom_rpt,gb_uci.acquisition_mode, vch->remote_addr_bf_ini, vch->remote_addr_bf_end, remote_image_size, gb_uci.pause_after_burst);
	TIMER_set_periodic_count(vch->prf_time_line,TIMER_SCAN_PRF_1);

	//UCI_Alarm1(1);
	//UCI_Alarm2(0);
	fifo_control_clear_all(0);

	//mem2gtx_set_speed(1,7,8,0);

	vch->hardware_set_up = 1;

	return 0;
}

int PA_transfer(TVCH *vch)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	int result;
	u32 room_avilable_on_GETH_buffer;
	u32 total_image_size;
	int tries,max_eth_tries;
	max_eth_tries=10;
	tries=0;
	total_image_size = vch->size32_fp_frame*vch->n_efl*4;
	u8 last_block=0;

	if(gb_uci.data_link_mode==DATA_LINK_MODE_GE)
	{
		for (ACQ->n_images_sent; new_host_data == 0 && ACQ->n_images_sent<ACQ->n_images_count; ACQ->n_images_sent++)
		{
			room_avilable_on_GETH_buffer=DMA_Get_bytes_available();

			while(room_avilable_on_GETH_buffer<total_image_size)
			{
				if((ACQ->acquisition_ended==0 && ACQ->end_continous==0) || new_host_data == 1)
					return 0; 				//La memoria está a tope, pero no he acabado la adquisición, que se salga sin más-
				else
				{
					if(tries>max_eth_tries)
					{
						return ELOG(charEUCI_ETH_STUCK, EUCI_ETH_STUCK);
					}
					if(tries==0)
						xil_printf("\n\rEsperando vacíado buffer ethernet");
					xil_printf(".");
					TIMER_Sleep(500000.0); 	// Si ya he acabado de disparar, simplemente dejo que termine
					room_avilable_on_GETH_buffer=DMA_Get_bytes_available();
					tries++;
				}
			}

			// En este punto, sabemos que hay espacio para una imagen entera
			PA_send_image(vch);
			//xil_printf("ACQ->n_images_sent = %d\r",ACQ->n_images_sent);
			ACQ->read_addr_index=vch->remote_addr_bf_cur;
		}

		if( (ACQ->n_images_sent>=ACQ->n_images_total && ACQ->n_images_total!= 0) ||
			(ACQ->end_continous                      && ACQ->n_images_total ==0))
		{
			ACQ->transmission_ended=1;
			Timstamp_end=timestamp_get_ticks_64();
			vch->acquiring = 0;
			vch->hardware_set_up = 0;
			if((gb_log_beamforming || gb_test_remote_fifo_control)  && fifo_control_get_or_flag(vch->base[0].bcc_addr))
			{
				for(u8 base_index=0;base_index<vch->n_bases;base_index++)
				{
					fifo_control_print_all(vch->base[base_index].bcc_addr);
					fifo_control_clear_all(vch->base[base_index].bcc_addr);
				}
				gtx_print_all_errors(vch->n_bases);
			}

			xil_printf("\n\rFin de envío!\n\r");
		}
	}
	return 0;
}

int PA_send_header(TVCH *vch)
{
   int result, i;

   // HEADER
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xCEB0... ");
   if ((result = UCI_MemoryWrite_uint32(0x0001CEB0)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // ID DEL CANAL VIRTUAL
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nId. Virtual Channel(%d)... ", vch->id);
   if ((result = UCI_MemoryWrite_uint32(vch->id)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // NUMERO TOTAL DE DATOS DE LA TRAMA
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Data(%d)... ", vch->pa_image.frame_size_32b);
   if ((result = UCI_MemoryWrite_uint32(vch->pa_image.frame_size_32b)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // NUMERO DE LINEAS CONFORMADAS EN PARALELO POR CADA CONFORMADOR
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Parallel Beamformed lines per beamformer line(%d)... ", vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1);
   if ((result = UCI_MemoryWrite_uint32(vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // NUMERO DE SEÑALES A-SCAN (Lineas de la imagen PA)
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. AScan(%d)... ", vch->pa_image.n_lines);
   if ((result = UCI_MemoryWrite_uint32((u32)vch->pa_image.n_lines)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // NUMERO DE DATOS DE CADA A-SCAN (Línea)
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Samples(%d)... ", vch->pa_image.n_beamformed_samples);
   if ((result = UCI_MemoryWrite_uint32(vch->pa_image.n_beamformed_samples)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // NUMERO DE BLOQUES QUE FORMAN LA IMAGEN [N_BLOQUES]
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Blocks(%d)(%d)[%X08]... ", vch->pa_image.n_blocks, vch->pa_image.n_blocks_32b, vch->pa_image.n_blocks_frame);
   if ((result = UCI_MemoryWrite_uint32(vch->pa_image.n_blocks_frame)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // NUMERO DE SEÑALES A-SCAN (LINEAS) DE CADA BLOQUE
   for (i=0; i<vch->pa_image.n_blocks_32b && i<MAX_BLOCKS_32b; i++)
   {
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nLines Block[%d](%d)... ", vch->pa_image.block_32b[i]);
      if ((result = UCI_MemoryWrite_uint32(vch->pa_image.block_32b[i])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   }

   // CONTADOR DE IMAGENES PA
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nAcqCounter... ");
	vch->acq_counter++;
   if ((result = UCI_MemoryWrite_uint32(vch->acq_counter)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // TIME STAMP
   if (vch->enabled_timestamp == 1)
   {
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nTimeStamp... ");
      if ((result = UCI_MemoryWrite_TimeStamp()) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   }

   // ENCODER 0
   if (vch->enabled_encoder_1 == 1)
   {
      // Value
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[0])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel A Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[0])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel B Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[0])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel A Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[0])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel B Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[0])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Sign Changes
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Sign Changes... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[0])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   }
   // ENCODER 1
   if (vch->enabled_encoder_2 == 1)
   {
      // Value
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[1])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel A Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[1])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel B Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[1])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel A Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[1])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel B Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[1])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Sign Changes
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Sign Changes... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[1])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   }
   // ENCODER 2
   if (vch->enabled_encoder_3 == 1)
   {
      // Value
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[2])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel A Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[2])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel B Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[2])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel A Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[2])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel B Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[2])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Sign Changes
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Sign Changes... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[2])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   }
   // ENCODER 3
   if (vch->enabled_encoder_4 == 1)
   {
      // Value
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[3])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel A Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[3])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Edges
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel B Edges... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[3])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel A Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel A Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[3])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Channel B Filtered Glitches
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel B Filtered Glitches... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[3])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

      // Sign Changes
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Sign Changes... ");
      if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[3])) < 0) return RLOG(result);
      if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   }

   // OFFSET APLICADO AL ENCODER DEL DISPARO
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nOffset Trigger Encoder... ");
   if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value_offset)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // ESCRIBE VALORES DE TEMPERATURA
//   if (vch->enabled_temperature == 1)
//   {
//      for (b=0; b<FP_MAX_N_BASES; b++)
//      {
//			if (fp_baseuREG[b].cfg == 1)
//			{
//				if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nTemperature BASE_%d... ",b);
//				if ((result = DMA_Prepare_Write16_from_Amplia(3)) < 0) return RLOG(result);
//				if ((result = BASE_MemoryWrite_Temperature(b)) < 0) return RLOG(result);
//				if ((result = DMA_Write16_from_Amplia_wait_end(500)) < 0) return RLOG(result);
//				if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
//			}
//		}
//   }


   return 0;
}

int PA_send_image(TVCH *vch)
{
   int result, i;
   u32 n_words_32_image, n_buffer_data;
   u32 remote_addr;
   u8 minibase;

   	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
   	// ____________________________________________________________________________
   	// CABECERA DE LA TRAMA
   	// ============================================================================

   	n_words_32_image = vch->pa_image.frame_size_32b;
   //   if (gb_log_acquiring_frame == 1)
   //      LOG_ACQUIRING_FRAME("\r\nDMA_Set_Datamover_Write32((%d + %d) * %d + %d + 1 = %d)... ",
   //         vch->size_header_line_pa_,
   //         (unsigned long)n_words_32_image,
   //         vch->focal_law[idx_fl].n_ascan,
   //         vch->size_header_img_pa_,
   //         n_words_32_image);

	//if ((result = DMA_Set_Datamover_Write32(n_words_32_image, &n_buffer_data)) < 0) return RLOG(result);
	if ((result = DMA_Set_Datamover_Write32(n_words_32_image, &n_buffer_data)) < 0)
	{
	int tries=1,max_tries=100;

	while(tries<=max_tries)
	{
	   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("NO MEMORY ON CIRCULAR BUFFER, WAITING FOR 500ms... (try = %d/%d)\n\r",tries,max_tries);
	   TIMER_Sleep(500000.0);
	   if ((result = DMA_Set_Datamover_Write32(n_words_32_image, &n_buffer_data)) < 0)
		   tries++;
	   else
		   break;
	}
	if(tries>max_tries) return RLOG(result);
	gb_log_acquiring_frame=0;
	}

	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");


	PA_send_header(vch);

	// NUMERO DE DATOS EN EL BUFFER DMA
   if (vch->enabled_buffer_dma == 1)
   {
	  if ((result = DMA_Write32(n_buffer_data)) < 0) return RLOG(result);
   }

// _____________________________________________________________________________
// ------ LECTURA DE DATOS
// =============================================================================


   //
   if (gb_hw_sitau_enabled)
   {
	   u32 conteo_lineas;
	   u8 lineas_bloque_completo;
	   u8 *lista_lineas_por_bloque;
	   u32 n_data_bloque_completo;
	   u32 n_data_bloque_extraido;
	   static int paso_por_filtro=0;
	   int da_la_vuelta=0;

	   minibase = vch->base[2].bcc_addr;



	   lineas_bloque_completo = (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1);
	   lista_lineas_por_bloque = (u8*)vch->pa_image.block_32b;

	   n_data_bloque_completo = ((vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * vch->pa_image.n_beamformed_samples)/2;

	   conteo_lineas=0;
	   //paso_por_filtro=!paso_por_filtro;
	   //config_FIR_coeficients(filtro_unidad);

	   //paso_por_filtro = vch->fir.enabled;
	   paso_por_filtro=1;

	   if(paso_por_filtro)
	   {
		   config_FIR_coeficients(vch->fir.coefficients);
		   if (vch->signal_mode == VIDEO)
			   config_HIL_coeficients(hilbert_coef);
		   config_interleaving(calc_interleaving(vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1),(vch->signal_mode == VIDEO));

	   }
		if(0)
		{
			u32* sswitch;
			u8 minibase=0;

			sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

			//Configuración para que sume
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_SMT_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_SMP_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0);
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_SUM_IDX,0);

			switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

			MCBCC_deactivate_emu();
		}
	   for(i=0;i<vch->pa_image.n_blocks;i++)
	   {
		   //Buffer circular
			if((vch->remote_addr_bf_end - vch->remote_addr_bf_cur)<(n_data_bloque_completo*4))
			{
				u32 addr_write,addr_read;
//				da_la_vuelta=1;
//
				vch->remote_addr_bf_cur = vch->remote_addr_bf_ini;
//				addr_write = get_mem2lvds_next_addr(2);
//				addr_read = get_datamover2mem_next_addr(2);
//				xil_printf("\n\raddr_write = 0x%08X\n\raddr_read  = 0x%08X\n\r",addr_write,addr_read);
//				xil_printf("\n\r");
			}
			remote_addr = vch->remote_addr_bf_cur;

		   if (gb_log_acquiring_frame == 1)
		   {
			   LOG_ACQUIRING_FRAME("Adquiriendo bloque %d:\n\r",i);
			   LOG_ACQUIRING_FRAME("Direccion  minibase: %d\n\r",minibase);
			   LOG_ACQUIRING_FRAME("Lineas del bloque (lineas bloque completo): %d (%d)\n\r",lista_lineas_por_bloque[i],lineas_bloque_completo);
			   LOG_ACQUIRING_FRAME("Direccion remota: 0x%08X\n\r",remote_addr);
			   LOG_ACQUIRING_FRAME("Conteo de lineas (lineas totales): %d (%d)\n\r",conteo_lineas,vch->pa_image.n_lines);
		   }

		   if(0)//(lista_lineas_por_bloque[i]==lineas_bloque_completo)
		   {

			   if(paso_por_filtro)
			   {
				   if (vch->signal_mode == VIDEO)
				   {
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_ENVOLV_IDX,0);
				   }
				   else
				   {
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,1);
				   }
				   DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_completo,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);

				   if ((result = DMA_Write32_from_BCC_wait_end(PA_LINE_RECV_TIMEOUT)) < 0)
					 xil_printf("ERROR, TIMEOUT ESPERANDO DMA_memcpy_remote_base_int_no_block(n_data,minibase,addr)\n\r");

				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);


			   }
			   else
			   {
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
				   //DMA_asymmetrical_memcpy_remote_base_int_no_block_sum(n_data_bloque_completo,n_data_bloque_completo,remote_addr);
				   DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_completo,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);
				   if ((result = DMA_Write32_from_BCC_wait_end(PA_LINE_RECV_TIMEOUT)) < 0)
					 xil_printf("ERROR, TIMEOUT ESPERANDO DMA_memcpy_remote_base_int_no_block(n_data,minibase,addr)\n\r");
			   }

		   }
		   else if (lista_lineas_por_bloque[i]!=0)//TEORICAMENTE SOLO EN EL BLOQUE FINAL! (si es incompleto)
		   {
			   n_data_bloque_extraido = RoundSup(((float)lista_lineas_por_bloque[i] * vch->pa_image.n_beamformed_samples)/2.0);

			   if(paso_por_filtro)
			   {
				   if (vch->signal_mode == VIDEO)
				   {
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_ENVOLV_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);
				   }
				   else
				   {
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_ENVOLV_IDX,1);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,1);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
					   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);
				   }
				   channel_extract_configure_basic(lista_lineas_por_bloque[i],(vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1),vch->pa_image.n_beamformed_samples);

				   DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_extraido,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);

				   if ((result = DMA_Write32_from_BCC_wait_end(PA_LINE_RECV_TIMEOUT)) < 0)
					 xil_printf("ERROR, TIMEOUT ESPERANDO DMA_asymmetrical_memcpy_remote_base_int_no_block(n_data_DMA,n_data_remote,minibase,remote_addr)\n\r");

				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);//desactivamos
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);//desactivamos
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);//activamos el camino normal

			   }
			   else
			   {

				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);

				   channel_extract_configure_basic(lista_lineas_por_bloque[i],(vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1),vch->pa_image.n_beamformed_samples);

				   //DMA_asymmetrical_memcpy_remote_base_int_no_block(n_data_bloque_extraido,n_data_bloque_completo,minibase,remote_addr);
				   //DMA_asymmetrical_memcpy_remote_base_int_no_block_sum(n_data_bloque_extraido,n_data_bloque_completo,remote_addr);

				   DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_extraido,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);

				   if ((result = DMA_Write32_from_BCC_wait_end(PA_LINE_RECV_TIMEOUT)) < 0)
					 xil_printf("ERROR, TIMEOUT ESPERANDO DMA_asymmetrical_memcpy_remote_base_int_no_block(n_data_DMA,n_data_remote,minibase,remote_addr)\n\r");

				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);//desactivamos
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);//activamos el camino normal
			   }

		   }
		   else //lista_lineas_por_bloque[i]==0
			   continue;


		   vch->remote_addr_bf_cur += n_data_bloque_completo*4;
		   if(0){

			u32 addr_write,addr_read;

			//vch->remote_addr_bf_cur = vch->remote_addr_bf_ini;
			addr_write =  get_datamover2mem_next_addr(2);
			addr_read = get_mem2lvds_next_addr(2);
			xil_printf("\n\raddr_write   = 0x%08X\n\raddr_read   = 0x%08X\n\rremote_addr = 0x%08X\n\r",addr_write,addr_read,remote_addr);
			xil_printf("\n\r");
		}
		   conteo_lineas+=lista_lineas_por_bloque[i];
	   }

	   if (conteo_lineas!=vch->pa_image.n_lines)
		 xil_printf("ERROR, el número de líneas escritas en la trama no coincide con el número de lineas de la imagen.\n\r");
   }


   // FOOTER
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xC0C0... ");
   if ((result = UCI_MemoryWrite_uint32(0x0000C0C0)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   if ((result = DMA_close_and_send_to_host(2)) < 0) return RLOG(result);


// _____________________________________________________________________________
// ------ FIN DEL BUCLE DE ADQUISICIÓN DE IMAGEN CONFORMADA
// =============================================================================

   if (gb_log_acquiring == 1) LOG_ACQUIRING(" OK");

   return EUCI_NONE;
}

