#include "ACQ_FMC.h"
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
#include "ACQ_FMC.h"
#include "dspdma_func.h"
#include "bussar_addr.h"
#include "bcc_bussar.h"
#include "timer_util.h"
#include "uci_fsm.h"

extern volatile int new_host_data;


#define USE_GTX_LINK_PRO			1

#define MINIBASE_BROADCAST			0

volatile ACQ_hndlr_t ACQ_FMC_handler;


void ACQ_FMC_continuous_interrupt_callback_function(void)
{
	u32 write_addr_index_new;

	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	if (ACQ->acquisition_ended==1) return; //No debería ser necesario, pero por si acaso.

	if(ACQ->store_timestamp_on_int)
	{
		ACQ->timestamp_value = timestamp_get_ticks_64();
		ACQ->store_timestamp_on_int = 0;
		ACQ->stored_timestamp = 1;
		ACQ->bloq_index_timestamped = ACQ->n_blocks_acquired;
	}

	ACQ->n_pros++;

	ACQ->n_emi_prom_count++;

	if(ACQ->n_emi_prom_count>=ACQ->n_emi_prom_total)
	{
		// RICARDO: AQUI ES DONDE SE PUEDE PARAR PORQUE YA HA ACABADO EL PROMEDIADO
		if(ACQ->end_continous)
		{
			ACQ->acquisition_ended=1;
			ACQ_FMC_stop_trigger();
			Timstamp_acq=timestamp_get_ticks_64();
			xil_printf("\n\rFin de adquisición 1 OK! (AVR %d/%d)\n\r", ACQ->n_emi_prom_count, ACQ->n_emi_prom_total);
		}
		else
		{
			ACQ->n_emi_prom_count=0;
			ACQ->n_focal_laws_count++;

			if(++ACQ->n_focal_laws_per_block_count==ACQ->n_focal_laws_per_block_total)
			{
				ACQ->n_focal_laws_per_block_count=0;
				ACQ->n_blocks_acquired++;
			}

			write_addr_index_new = (ACQ->addr_end-ACQ->write_addr_index>2*ACQ->bytes_per_fl_remote?ACQ->write_addr_index+ACQ->bytes_per_fl_remote:ACQ->addr_ini);
			if(write_addr_index_new>=ACQ->read_addr_index && ACQ->write_addr_index<ACQ->read_addr_index)
			{
				ACQ->n_overflows++;
				ACQ->overflow_flag=1;
				xil_printf("\r\nWARNINIG: Overflow Acquisition Buffer (%d)",ACQ->n_overflows);
			}
			ACQ->write_addr_index = write_addr_index_new;
			if(Debug_addr_write)
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
	}

	if(ACQ->n_focal_laws_count>=ACQ->n_focal_laws_total)
	{
		// RICARDO: AQUI ES DONDE SE PUEDE PARAR PORQUE YA HA ACABADO UNA IMAGEN COMPLETA
		ACQ->n_focal_laws_count = 0;
		ACQ->n_images_count++;
		if (ACQ->write_addr_index < ACQ->read_addr_index)
		{
			if ((ACQ->bytes_per_fl_remote * ACQ->n_focal_laws_total * ACQ->n_images_burst_total) >= ACQ->read_addr_index - ACQ->write_addr_index)
			{
				UCI_Alarm2(0);
				xil_printf("\n\rTrigger Disabled Type 1!");
				MCBCC_TRIG2PRO(0);
				ACQ->trig2pro_disabled = 1;
				MCBCC_int_enable(TRGE_MASK);
			}
		}
		else
		{
			if ((ACQ->bytes_per_fl_remote * ((ACQ->n_focal_laws_total * ACQ->n_images_burst_total) + 1))>= ((ACQ->addr_end - ACQ->write_addr_index) + (ACQ->read_addr_index- ACQ->addr_ini)))
			{
				UCI_Alarm2(0);
				xil_printf("\n\rTrigger Disabled Type 2!");
				MCBCC_TRIG2PRO(0);
				ACQ->trig2pro_disabled = 1;
				MCBCC_int_enable(TRGE_MASK);
			}
		}


		//Fin de imagen
		//if(ACQ->acquisition_mode==AQUISITION_MODE_CONTINOUS_BURST || ACQ->acquisition_mode==AQUISITION_MODE_FIXED_BURST)
		{
			ACQ->n_images_burst_count++;

			if (ACQ->n_images_burst_count>=ACQ->n_images_burst_total)
			{

				//Fin de burst
				ACQ->n_images_burst_count=0;
				ACQ->n_bursts_count++;
				//Esta comparación es por un tema de dejarlo listo para nueva adquisición
				if(ACQ->end_continous && (ACQ->n_emi_prom_total <= 1 || (ACQ->n_emi_prom_total > 1 && ACQ->n_emi_prom_count>=ACQ->n_emi_prom_total)))
				{
					ACQ->acquisition_ended=1;
					ACQ_FMC_stop_trigger();
					Timstamp_acq=timestamp_get_ticks_64();
					xil_printf("\n\rFin de adquisición 2 OK! (AVR %d/%d)\n\r", ACQ->n_emi_prom_count, ACQ->n_emi_prom_total);
					gb_sitau_status = ST_WaitStop;
				}
				else if(ACQ->pause_continous||ACQ->pause_after_burst)
				{
					ACQ_FMC_pause_trigger();
					ACQ->store_timestamp_on_int=1;
					xil_printf("\n\rPAUSA!\n\r");
				}
			}
		}

		if (ACQ->end_continous && ACQ->acquisition_mode==AQUISITION_MODE_CONTINOUS && (ACQ->n_emi_prom_total <= 1 || (ACQ->n_emi_prom_total > 1 && ACQ->n_emi_prom_count>=ACQ->n_emi_prom_total)))
		{
			ACQ->acquisition_ended=1;
			ACQ_FMC_stop_trigger();
			Timstamp_acq=timestamp_get_ticks_64();
			xil_printf("\n\rFin de adquisición 3 OK! (AVR %d/%d)\n\r", ACQ->n_emi_prom_count, ACQ->n_emi_prom_total);
			gb_sitau_status = ST_WaitStop;
		}
	}
	if(ACQ->n_images_count>=ACQ->n_images_total && (ACQ->acquisition_mode==AQUISITION_MODE_FIXED))
	{
		ACQ->acquisition_ended=1;
		ACQ_FMC_stop_trigger();
		Timstamp_acq=timestamp_get_ticks_64();
		xil_printf("\n\rFin de adquisición fija!\n\r");
	}
	//ESTO ES IMPORTANTE: Al terminar la adquisición, el último bloque puede quedar a la mitad. Esto es importante determinarlo. Este bloque irá marcado con LAST y tiene que llevar marcado en la cabecera el número de leyes focales válidas que tiene (ACQ->n_focal_laws_per_block_count)
	if(ACQ->acquisition_ended==1 && ACQ->n_focal_laws_per_block_count!=0)
		ACQ->n_blocks_acquired++;

	gb_uci.ind_acquisitions = ACQ->n_images_count;
//	if(ACQ->n_pros_delay_sync==0)
//		UCI_Alarm2(1);
//	else
//		ACQ->n_pros_delay_sync--;
}

void ACQ_FMC_trig_in_interrupt_callback_function(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;


	if (ACQ->trig2pro_disabled == 1)
	{
		if(Debug_addr_write)
		{
			u32 wr_addr_hw;
			wr_addr_hw=get_afe2mem_datamover_next_addr(1);
			if(wr_addr_hw!=ACQ->write_addr_index)
			{
				xil_printf("\n\rwr_addr_hw=0x%08X, ACQ->write_addr_index=0x%08X\n\r",wr_addr_hw,ACQ->write_addr_index);
			}
		}
		if (ACQ->write_addr_index < ACQ->read_addr_index)
		{
			if ((ACQ->bytes_per_fl_remote * ACQ->n_focal_laws_total * ACQ->n_images_burst_total) >= ACQ->read_addr_index - ACQ->write_addr_index)
			{
				return;
			}
			else
			{
				UCI_Alarm2(1);
				xil_printf("\n\rTrigger Enabled Type 1!");
				MCBCC_TRIG2PRO(1);
				ACQ->trig2pro_disabled = 0;
				MCBCC_int_disable(TRGE_MASK);
			}
		}
		else
		{
			if ((ACQ->bytes_per_fl_remote * ((ACQ->n_focal_laws_total * ACQ->n_images_burst_total) + 1))>= ((ACQ->addr_end - ACQ->write_addr_index) + (ACQ->read_addr_index- ACQ->addr_ini)))
			{
				return;
			}
			else
			{
				UCI_Alarm2(1);
				xil_printf("\n\rTrigger Enabled Type 2!");
				MCBCC_TRIG2PRO(1);
				ACQ->trig2pro_disabled = 0;
				MCBCC_int_disable(TRGE_MASK);
			}
		}
	}
}

void ACQ_FMC_hard_break(TVCH *vch)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	ACQ_FMC_pause_trigger();
	FMC_end_transmision(vch);
	xil_printf("\n\rEnvío finalizado de manera abrupta\n\r");
}
void ACQ_FMC_stop_continous(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	ACQ_FMC_continue_continous();
	if(ACQ->pause_continous)
		ACQ->acquisition_ended=1;
	ACQ->end_continous = 1;
	UCI_Alarm1(0);
	UCI_Alarm2(0);
}
int ACQ_FMC_pause_continous(TMSG_Pause *msg)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	ACQ->pause_continous = 1;
	if((ACQ->acquisition_mode==AQUISITION_MODE_FIXED || ACQ->acquisition_mode==AQUISITION_MODE_CONTINOUS) && ACQ->end_continous == 0)
	{
		UCI_Alarm1(0);
		ACQ_FMC_pause_trigger();
		ACQ->store_timestamp_on_int=1;
	}
	xil_printf("\n\rPAUSE! (%d acquisitions)\n\r",msg->n_acquisitions);
	return 0;
}
int ACQ_FMC_continue_continous()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	//ACQ->continue_continous = 1;
	ACQ->pause_continous = 0;
	TIMER_Start(TIMER_SCAN_PRF_1);
	UCI_Alarm1(1);
	UCI_Alarm2(1);
	xil_printf("\n\rCONTINUE!\n\r");
	return 0;
}


void ACQ_FMC_pause_trigger(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	TIMER_Stop(TIMER_SCAN_PRF_1);
	ACQ->pause_continous=1; //ojo con esto... espero que no joda nada :D
	UCI_Alarm1(0);
	UCI_Alarm2(0);
}

void ACQ_FMC_stop_trigger(void)
{
	MCBCC_TRIG2PRO(0);
	TIMER_Stop(TIMER_SCAN_PRF_1);
	MCBCC_remove_callback(POF_NBIT);
	MCBCC_remove_callback(TRE_NBIT);
	//TRIG_remove_interrupt_source(TRIGGER_PRFTIMER1_MASK);
	TRIG_reset_all_interrupt_sources();
	UCI_Alarm1(0);
	UCI_Alarm2(0);
}
void ACQ_FMC_start_trigger(void)
{
	Timstamp_ini=timestamp_get_ticks_64();
    TIMER_Start(TIMER_SCAN_PRF_1);
    UCI_Alarm1(1);
    UCI_Alarm2(1);
}

void ACQ_FMC_interrupt_startup(unsigned int images,unsigned int n_images_burst,unsigned int focal_laws_per_block,unsigned int focal_laws,unsigned int emi_prom,char acquisition_mode,u32 addr_ini,u32 addr_end,u32 acq_size,u32 n_pros_delay_sync, char pause_after_burst)
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
	ACQ->n_focal_laws_total=focal_laws;
	ACQ->n_emi_prom_total=emi_prom;
	ACQ->n_focal_laws_per_block_count=0;
	ACQ->n_focal_laws_per_block_total=focal_laws_per_block;
	ACQ->n_blocks_sent=0;
	ACQ->n_blocks_acquired=0;
	ACQ->n_blocks_to_send=CEILING(images*focal_laws,focal_laws_per_block);
	ACQ->block_counter_header=0;
	ACQ->image_counter_header=0;
	ACQ->focal_law_index_header=0;
	ACQ->acquisition_mode=acquisition_mode;
	ACQ->n_pros_delay_sync=n_pros_delay_sync;
	ACQ->pause_after_burst=pause_after_burst;

	if(ACQ->acquisition_mode==AQUISITION_MODE_FIXED)
		ACQ->n_images_total=images;
	else if(ACQ->acquisition_mode == AQUISITION_MODE_CONTINOUS)
		ACQ->n_images_total=0;

	ACQ->n_images_burst_total=n_images_burst;
	ACQ->n_images_burst_count=0;
	ACQ->n_bursts_count=0;
	ACQ->n_bursts_sent=0;

	ACQ->write_addr_index = addr_ini;
	ACQ->read_addr_index = addr_ini;
	ACQ->bytes_per_fl_remote = acq_size;
	ACQ->addr_ini = addr_ini;
	ACQ->addr_end = addr_end;

	ACQ->trig2pro_disabled = 0;

	MCBCC_int_enable(POFE_MASK);

	MCBCC_asign_callback(ACQ_FMC_errorPRO,TRE_NBIT);
	MCBCC_int_enable(TREE_MASK);

	MCBCC_asign_callback(ACQ_FMC_continuous_interrupt_callback_function,POF_NBIT);//	PRO Fall flank interrupt Enable asignado a la interrupción

	MCBCC_asign_callback(ACQ_FMC_trig_in_interrupt_callback_function,TRG_NBIT);   //	TRIGGER rise flank interrupt Enable asignado a la interrupción
	MCBCC_int_disable(TRGE_MASK);

	ACQ->store_timestamp_on_int=1; //En el primer bloque mandamos timestamp

}

void ACQ_FMC_errorPRO(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	u32 busy_word;
	status_t status_datamov;
	u32 wr_addr_hw;
	static u32 Errores_pro=0;

	// SI SE PRODUCEN 100 ERRORES DE PRO SEGUIDOS SIN QUE SE HAYAN PRODUCIDO pros DESCENCENTES --> ERROR GRAVE
	// SE IMPRIMEN LOS ERRORES Y SE PARA LA UCI CON UN STOP


	if((Errores_pro % (1024*20))==0)
		xil_printf("\n\rError en el PRO = %d! Se intentó elevar el PRO cuando el PRO estaba en alto!\n\r",Errores_pro);

	Errores_pro++;

	if (0)
	{
		for(u8 base_index=0;base_index<gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_bases;base_index++)
		{

			u8 base_bcc_addr = gb_fp_virtual_channel[gb_uci.active_virtual_channel].base[base_index].bcc_addr;
			xil_printf("\n\rBASE %d\n\r",base_bcc_addr);
			fifo_control_print_all(base_bcc_addr);
			fifo_control_clear_all(base_bcc_addr);
			busy_word=bcc_get_busy_in_word(base_bcc_addr);
			xil_printf("MISC_NOT_BUSY_IN_REG: 0x%02X\n\r",busy_word);
			busy_word=bcc_get_busy_or_word(base_bcc_addr);
			xil_printf("MISC_NOT_BUSY_OR_REG: 0x%02X\n\r",busy_word);

			status_datamov = get_afe2mem_datamover_status(base_bcc_addr);
			//if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
			xil_printf("STATUS: 0x%08X\n\r",status_datamov.status_32);
			wr_addr_hw=get_afe2mem_datamover_next_addr(base_bcc_addr);
			xil_printf("\n\rwr_addr_hw=0x%08X, ACQ_write_addr_index=0x%08X\n\r",wr_addr_hw,ACQ->write_addr_index);
		}
		bcc_print_all_busy_flags(4);
	}

}

void ACQ_FMC_errorPRO_link(u32 rd_addr)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	u32 busy_word;
	status_t status_datamov;
	u32 wr_addr_hw;
	u8 base_QSFP=1;
	gtx_misc_fsm_ctrl_reg_t valor_reg_control_GTX;

	xil_printf("\n\rError en el PRO LINK. El pro_link_in de la UCI no llega a 0\n\r");

	for(u8 base_index=0;base_index<gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_bases;base_index++)
	{

		u8 base_bcc_addr = gb_fp_virtual_channel[gb_uci.active_virtual_channel].base[base_index].bcc_addr;
		xil_printf("\n\rBASE %d\n\r",base_bcc_addr);
		fifo_control_print_all(base_bcc_addr);
		fifo_control_clear_all(base_bcc_addr);
		busy_word=bcc_get_busy_in_word(base_bcc_addr);
		xil_printf("MISC_NOT_BUSY_IN_REG: 0x%02X\n\r",busy_word);
		busy_word=bcc_get_busy_or_word(base_bcc_addr);
		xil_printf("MISC_NOT_BUSY_OR_REG: 0x%02X\n\r",busy_word);

		status_datamov = get_mem2beamformer_datamover_status(base_bcc_addr);
		//if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
		xil_printf("STATUS lectura: 0x%08X\n\r",status_datamov.status_32);
		wr_addr_hw=get_afe2mem_datamover_next_addr(base_bcc_addr);
		xil_printf("\n\rrd_addr_hw=0x%08X, ACQ_write_addr_index=0x%08X\n\r",wr_addr_hw,ACQ->write_addr_index);
	}

	gtx_print_all_errors(gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_bases);
	xil_printf("\n\rLeyendo regsitro de control GTX de la base conectada al QSFP:\n\r");
	if(BCC_read_reg(base_QSFP,MISC_BUSSAR_SUBMOD_ADDR,MISC_GTX_FSM_CTRL_REG,&valor_reg_control_GTX.gtx_misc_fsm_ctrl_reg_32))
	{
		xil_printf("No se ha podido leer el registro\n\r");
	}
	else
	{
		xil_printf("El enlace QSFP está ");
		if(valor_reg_control_GTX.BITS.gtx_rdy_dn)
			xil_printf("El enlace QSFP está ENCENDIDO (CHANNEL UP = 1)\n\n");
		else
			xil_printf("El enlace QSFP está APAGADO (CHANNEL UP = 0)\n\n");
	}
}

int ACQ_FMC_set_up(TVCH *vch)
{
int result,i,idx_avr;
u32 l;
int prf_acq_number = 1;
u64 timstamp_ini,timstamp_end;
float us_acq;
u16 flags;
u16 emi_prom_rpt;
u32 activacion_salida=0;

gtx_misc_fsm_ctrl_reg_t ctrl_gtx_reg;

	Current_ACQ_hndlr_ptr = &ACQ_FMC_handler;

	TRIG_reset_all_interrupt_sources();
	TIMER_Stop(TIMER_SCAN_PRF_1);
	MCBCC_TRIG2PRO(0);
	MCBCC_set_PRO(0);
	while(MCBCC_get_PROI());

	TRIG_reset_all_interrupt_sources();

	vch->remote_addr_cur = vch->remote_addr_ini;
	// Configiracion HW
	// ---------------------------------------------------------------------------

	// Se programa en HW sólo una vez
	if ((result = VCH_PRG_EmissionFocalLaws(vch)) < 0) return RLOG(result);

	// Se genera el buffer de commandos una sola vez, pero se envía al HW en cada disparo
	if ((result = VCH_PRG_Registers_FMC(vch)) < 0) return RLOG(result);

	if (vch->processing_type==PROCESSING_TYPE_NONE) emi_prom_rpt = 1;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_B) emi_prom_rpt = vch->n_signals_processing_avr;
	else if(vch->processing_type==PROCESSING_TYPE_EMI_B)  emi_prom_rpt = vch->n_signals_processing_emi;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_A) emi_prom_rpt = 1<<vch->afe.AFE_BUSSAR_UT.log2_promediados;

	if(gb_log_acquiring) VCH_PrintConfig(vch);
	if(gb_log_acquiring)
	{
		AFE_SPI_UT_t regs_afe;
		get_spi_afe_regs(&regs_afe, 2);
		print_spi_afe_regs(regs_afe);
	}
	// ---------------------------------------------------------------------------
	flags=MCBCC_get_flags(PORF_MASK|POFF_MASK);
	if(gb_log_acquiring) LOG_ACQUIRING("FLAGS 0x%04X\n\r",flags);

	TRIG_reset_all_interrupt_sources();
	switch(gb_uci.trigger_source)
	{
	case TRG_SCAN_PRF: TRIG_set_interrupt_source(TRIGGER_PRFTIMER1_MASK); break;
	case TRG_SCAN_EXT: TRIG_set_interrupt_source(TRIGGER_EXT_MASK); break;
	}

	MCBCC_POAF(1);		// Pro Out Auto Fall (pone el PRO a 0 automaticamente
	MCBCC_TRIG2PRO(1);
	UCI_Sync(0);
	UCI_set_Sync_is_PRO();
	//UCI_set_Sync_is_SW();
	//UCI_Sync(0);

	//if(gb_log_acquiring) LOG_ACQUIRING("\n\rIniciando adquisición...\n\r");
	//if(gb_log_acquiring) MCBCC_print_regs();

	flags=MCBCC_get_flags(0);
	if(gb_log_acquiring) LOG_ACQUIRING("FLAGS 0x%04X\n\r",flags);

	activacion_salida=gb_uci.delay_sync_n_acq;


	ACQ_FMC_interrupt_startup(gb_uci.n_acquisitions,gb_uci.n_images_burst,vch->gtx_n_focal_law,vch->n_efl,emi_prom_rpt,(char)gb_uci.acquisition_mode,vch->remote_addr_ini,vch->remote_addr_end,vch->afe.AFE_BUSSAR_UT.num_samples * 32 * 2,activacion_salida,gb_uci.pause_after_burst);

	//timstamp_ini=timestamp_get_ticks_64();
	//TIMER_set_periodic_count(1000.0,TIMER_SCAN_PRF_1);
	TIMER_set_periodic_count(vch->prf_time_line,TIMER_SCAN_PRF_1);

	UCI_Alarm2(0);
	fifo_control_clear_all(0);

	gtx_set_pro_link_to_gtx_control();

	Global_commnd_buf.total_len=0;
	ctrl_gtx_reg.gtx_misc_fsm_ctrl_reg_32 = 0x0;
	ctrl_gtx_reg.BITS.lnk_dn_reset_fsm = 1;
	config_gtx(ctrl_gtx_reg,MINIBASE_BROADCAST,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	//Global_commnd_buf.total_len=0;

	gtx_set_pro_link_to_gtx_control();

	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len=0;

	mem2gtx_set_speed(1,7,8,0);
//	mem2gtx_set_speed(1,6,8,0);
//	xil_printf("\n\r---------------------------ESTO ES UNA PRUEBA, ELIMINAR!!!! LO CORRECTO ES mem2gtx_set_speed(1,7,8,0);\n\r");

	vch->hardware_set_up = 1;
	return 0;
}


int FMC_transfer_GTX_remote_repeat_header(TVCH *vch,u8 minibase_gtx_header,u8 last_behaviour)
{
	gtx_send_remote_repeat_header(0,last_behaviour);
	return 0;
}

int FMC_transfer_GTX_header(TVCH *vch,u8 minibase_gtx_header,u32 gtx_block_count,u32 image_index,u16 focal_law_index,u16 valid_focal_laws_in_last_block,u8 last_block,u8 last_behaviour,u8 timestamp_present, u32 oob_data)
{
	u32 cabecera32[GTX_HEADER_SIZE];

	u16 MSB_cabecera32_5 =(last_block?valid_focal_laws_in_last_block:vch->n_efl);

	//El bit más alto identifica si es el último bloque de la adquisición y por tanto, no se deberían recibir más (sería otra adquisición que podría tener otros parámetros). Los 8 bits más bajos indican el canal virtual (ahora mismo en principio el 0). No debería cambiar entre bloques de la misma adquisición
	u32 cabecera32_6 = (last_block?(1<<31):0)|(timestamp_present?(1<<30):0)|(0xFF&vch->id);

	cabecera32[0]= 0x45535546;										//"FUSE"
	cabecera32[1]= gtx_block_count;									//Identificación de bloque, aumenta de uno en uno
	cabecera32[2]= vch->afe.AFE_BUSSAR_UT.num_samples*32*2/32-1;	//Número fijo: Muestras*Ncanales*2byte/muestra/número de bytes por "beat"(32)
	cabecera32[3]= vch->gtx_n_focal_law-1;							//Número fijo: Número de leyes focales que hay en cada bloque
	cabecera32[4]= image_index;										//Número de imagen por la que va este bloque. Puede haber varias iágenes en el mismo bloque, por lo que aumentará en más de uno entre bloques o puede haber varios bloques por imagen por lo que puede no aumentar nada entre bloques consecutivos
	cabecera32[5]= (MSB_cabecera32_5<<16)| (focal_law_index);		//Número de la primera ley focal que se envía en este bloque
	cabecera32[6]= cabecera32_6;									//Varias opciones
	cabecera32[7]= oob_data;										//Out Of Band DATA

	if(minibase_gtx_header==0)
	{
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[ZYNQGTX_SWITCH_TABLE_IDX],SSWITCHGTX_UCI_MASTER_TO_SUTXDN_IDX, SSWITCHGTX_SLAVE_FROM_MEM_IDX,0);
		swgtx_remote_down_pass_throw_to_host(0);
		set_axi4_2_gtx(0);
		axi2gtx_fifo_send(cabecera32, GTX_HEADER_SIZE);
	}
	else if (minibase_gtx_header)
	{
		//gtx_remote_from_minibase_hea_down_to_host(minibase_gtx_header,vch->n_bases);
		gtx_send_remote_header(cabecera32,GTX_HEADER_SIZE,minibase_gtx_header,last_behaviour);
	}
	return 0;
}

int FMC_transfer(TVCH *vch)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	int result;
	u32 room_avilable_on_GETH_buffer;
	u32 total_image_size;
	int tries,max_eth_tries;
	max_eth_tries=5;
	tries=0;
	total_image_size = vch->size32_fp_frame*vch->n_efl*4;
	u8 last_block=0;

	if(gb_uci.data_link_mode==DATA_LINK_MODE_GTX)
	{

		for (ACQ->n_blocks_sent; (ACQ->n_blocks_sent<ACQ->n_blocks_acquired); ACQ->n_blocks_sent++)
		{

			if(ACQ->acquisition_ended==1)
				if((ACQ->n_blocks_sent+1)==ACQ->n_blocks_acquired)
					last_block=1;


			FMC_transfer_block_GTX_link_pro(vch,last_block);
			ACQ->read_addr_index=vch->remote_addr_cur;
			if(last_block)
				break;
		}
		if(last_block || (ACQ->acquisition_ended && (ACQ->n_blocks_sent>=ACQ->n_blocks_acquired)))
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

			xil_printf("\n\rFin de envío! FMC_transfer(1) (AVR %d/%d)\n\r", ACQ->n_emi_prom_count, ACQ->n_emi_prom_total);
			gb_sitau_status = ST_WaitStop;
		}
	}
	else if(gb_uci.data_link_mode==DATA_LINK_MODE_GE)
	{
		if (ACQ->end_continous && (ACQ->n_emi_prom_total <= 1 || (ACQ->n_emi_prom_total > 1 && ACQ->n_emi_prom_count>=ACQ->n_emi_prom_total)))
		{
			FMC_end_transmision(vch);
			xil_printf("\n\rFin de envío! FMC_transfer(2.1) (AVR %d/%d)\n\r", ACQ->n_emi_prom_count, ACQ->n_emi_prom_total);
			gb_sitau_status = ST_WaitStop;
		}
		else
		{
			for (ACQ->n_images_sent; ACQ->end_continous == 0 && new_host_data == 0 && ACQ->n_images_sent<ACQ->n_images_count; ACQ->n_images_sent++)
			{
				room_avilable_on_GETH_buffer=DMA_Get_bytes_available();

				if (ACQ->end_continous && (ACQ->n_emi_prom_total <= 1 || (ACQ->n_emi_prom_total > 1 && ACQ->n_emi_prom_count>=ACQ->n_emi_prom_total)))
				{
					FMC_end_transmision(vch);
					xil_printf("\n\rFin de envío! FMC_transfer(2) (AVR %d/%d)\n\r", ACQ->n_emi_prom_count, ACQ->n_emi_prom_total);
					gb_sitau_status = ST_WaitStop;
				}
				//while(room_avilable_on_GETH_buffer<total_image_size)
				else
				{
					if (room_avilable_on_GETH_buffer<total_image_size)
					{
						//RICARDO
						return 0;
					}

	//				if((ACQ->acquisition_ended==0 && ACQ->end_continous==0) || new_host_data == 1)
	//					return 0; 				//La memoria está a tope, pero no he acabado la adquisición, que se salga sin más-
	//				else
	//				{
	//					if(tries>max_eth_tries)
	//					{
	//						return ELOG(charEUCI_ETH_STUCK, EUCI_ETH_STUCK);
	//					}
	//					if(tries==0)
	//						xil_printf("\n\rEsperando vacíado buffer ethernet");
	//					xil_printf(".");
	//					TIMER_Sleep(50000.0); 	// Si ya he acabado de disparar, simplemente dejo que termine
	//					room_avilable_on_GETH_buffer=DMA_Get_bytes_available();
	//					tries++;
	//				}

					// En este punto, sabemos que hay espacio para una imagen entera
					for (int focal_law=0; focal_law<vch->n_efl; focal_law++)
					{
					   if ((result = FMC_transfer_focal_law_ETH(vch, focal_law)) < 0)
						   return RLOG(result);
					}
					ACQ->read_addr_index=vch->remote_addr_cur;
				}
			}
			ACQ->read_addr_index=vch->remote_addr_cur;

			if( (ACQ->n_images_sent>=ACQ->n_images_total && ACQ->n_images_total!= 0) ||
				(ACQ->end_continous && ACQ->n_images_total ==0 && (ACQ->n_emi_prom_total <= 1 || (ACQ->n_emi_prom_total > 1 && ACQ->n_emi_prom_count>=ACQ->n_emi_prom_total))))
			{
				FMC_end_transmision(vch);
				xil_printf("\n\rFin de envío! FMC_transfer(3) (AVR %d/%d)\n\r", ACQ->n_emi_prom_count, ACQ->n_emi_prom_total);
				gb_sitau_status = ST_WaitStop;
			}
		}

	}

	return 0;
}
int FMC_end_transmision(TVCH *vch)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	MCBCC_TRIG2PRO(0);
	MCBCC_int_disable(TRGE_MASK);
	ACQ_FMC_stop_trigger();
	ACQ->acquisition_ended =1;
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

	xil_printf("\n\rFin de envío! FMC_end_transmision()\n\r");
}



int FMC_transfer_focal_law_ETH_old(TVCH *vch, u32 focal_law)
{
int result;
u32 n_words_32_focal_law, n_buffer_data;
u32 addr, n_data_per_minibase_word32;
u8 minibase;
//u8 mem2gtx=0,mem2uci=1,header_gtx=0;


	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	if (focal_law >= MOD_PUL_MAX_FOCAL_LAW) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	//first_base	= &vch->base[0];

// ____________________________________________________________________________
// CABECERA DE LA TRAMA
// ============================================================================

   n_words_32_focal_law = vch->size32_fp_frame;
   if (gb_log_acquiring_frame == 1)
      LOG_ACQUIRING_FRAME("\r\nDMA_Set_Datamover_Write16((%d + %d) * %d + %d + 1 = %d)... ",
         vch->size_header_line_fp,
         vch->size32_fp_image,
		 (int)MOD_MAX_CH,
         vch->size_header_img_fp,
         n_words_32_focal_law);

   if ((result = DMA_Set_Datamover_Write32(n_words_32_focal_law, &n_buffer_data)) < 0) return RLOG(result);

   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   if ((result = FMC_transfer_write_header(vch, focal_law,n_buffer_data)) < 0) return RLOG(result);

// _____________________________________________________________________________
// ------ LECTURA DE DATOS
// =============================================================================


   //AQUI SE VE SI HAY QUE FILTRAR. En tal caso, se coloca el switch como convenga.



/*	if (gb_hw_sitau_enabled == 0)
	{
		// Recorre los canales habilitados para la recepcion
		if ((result = VCH_Process_NoHardware_sitau2(vch->afe.AFE_BUSSAR_UT.num_samples,idx_fl*100)) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}*/

   n_data_per_minibase_word32 = vch->afe.AFE_BUSSAR_UT.num_samples * 32 / 2;
	if((vch->remote_addr_end - vch->remote_addr_cur)<(n_data_per_minibase_word32*4))
		vch->remote_addr_cur = vch->remote_addr_ini;


	addr = vch->remote_addr_cur;
#define DEBUG_MONTALDO_FILTERS 0

	{
	   if(!DEBUG_MONTALDO_FILTERS)
		   config_FIR_coeficients(vch->fir.coefficients);
	   else
		   config_FIR_coeficients(filtro_unidad_casi);

	   if (vch->signal_mode == VIDEO)
		   config_HIL_coeficients(hilbert_coef);
	   config_interleaving(calc_interleaving(32),(vch->signal_mode == VIDEO));
	}


	//for(int idx_minibase=vch->n_bases-1;idx_minibase>=0;idx_minibase--)
   for(u8 idx_minibase=0;idx_minibase<vch->n_bases;idx_minibase++)
	{
		minibase =  vch->base[idx_minibase].bcc_addr;


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
			   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
			   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
			   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);
		   }
		   channel_extract_configure_basic(32,32,vch->afe.AFE_BUSSAR_UT.num_samples);

			u32* sswitch;

			sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

			switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

			MCBCC_deactivate_emu();
		}

		//DMA_memcpy_remote_base_int_no_block(n_data_per_minibase_word32,minibase,addr);
		DMA_memcpy_remote_base_int_no_block_last_ctrl(n_data_per_minibase_word32,minibase,addr,LAST_DONT_TOUCH);
		//DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_completo,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);
		//xil_printf("addr = %08X, n_data_per_minibase_word32 = %6d, minibase %d\n\r",addr,n_data_per_minibase_word32,minibase);

		if((result = DMA_Write32_from_BCC_wait_end(FMC_LINE_RECV_TIMEOUT)) < 0)
		{
			xil_printf("ERROR, TIMEOUT ESPERANDO DMA_memcpy_remote_base_int_no_block(n_data_per_minibase_word32,minibase,addr)\n\r");
			{
				status_t status_datamov;
				status_datamov = get_minibase2uci_buffer_datamover_status(minibase);
				if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
					LOG_ACQUIRING("STATUS: 0x%08X\n\r",status_datamov.status_32);
			}
		}

		usleep_A9(100);

		{
			u32* sswitch;

			sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

			MCBCC_deactivate_emu();
		}
	}


   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
	//No hace falta que comprobemos que la siguiente posición es buena, ya lo comprobamos al leer!
	vch->remote_addr_cur+=n_data_per_minibase_word32*4;

	if(0)
	{
		u32 direccion;
		u32 status;
		u32 comando;
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,&direccion);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STATUS   ,&status);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND  ,&comando);
		xil_printf("\n\rwrite_addr = 0x%08X, read_addr = 0x%08X, status = 0x%08X, comando = 0x%08X\n\r",direccion,vch->remote_addr_cur,status,comando);
	}
// _____________________________________________________________________________
// ------ FIN DEL BUCLE DE ADQUISICION DE LINEAS
// =============================================================================

   // ESCRIBE LA PALABRA DE FIN DE LA IMAGEN
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xC0C0... ");
   if ((result = UCI_MemoryWrite_uint32(0x0000C0C0)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   if ((result = DMA_close_and_send_to_host(2)) < 0) return RLOG(result);

	// if ((result = UCI_SetNextVirtualChannel_ConfigHardware (vch)) < 0) return RLOG(result);

	// if ((result = VCH_AcquisitionData_Disabled(vch)) < 0) return RLOG(result);

	// Salida SYNC = 0
	//if ((result = UCI_Sync(0)) < 0) return RLOG(result);

// _____________________________________________________________________________
// ------ FIN DEL BUCLE DE PRINCIPAL
// =============================================================================

   if (gb_log_acquiring == 1) LOG_ACQUIRING(" OK");

   return EUCI_NONE;
}

int FMC_transfer_focal_law_ETH(TVCH *vch, u32 focal_law)
{
int result;
u32 n_words_32_focal_law, n_buffer_data;
u32 addr, n_data_per_minibase_word32;
u8 minibase;
//u8 mem2gtx=0,mem2uci=1,header_gtx=0;


	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	if (focal_law >= MOD_PUL_MAX_FOCAL_LAW) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	//first_base	= &vch->base[0];

// ____________________________________________________________________________
// CABECERA DE LA TRAMA
// ============================================================================

   n_words_32_focal_law = vch->size32_fp_frame;
   if (gb_log_acquiring_frame == 1)
      LOG_ACQUIRING_FRAME("\r\nDMA_Set_Datamover_Write16((%d + %d) * %d + %d + 1 = %d)... ",
         vch->size_header_line_fp,
         vch->size32_fp_image,
		 (int)MOD_MAX_CH,
         vch->size_header_img_fp,
         n_words_32_focal_law);

   if ((result = DMA_Set_Datamover_Write32(n_words_32_focal_law, &n_buffer_data)) < 0) return RLOG(result);

   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   if ((result = FMC_transfer_write_header(vch, focal_law,n_buffer_data)) < 0) return RLOG(result);

// _____________________________________________________________________________
// ------ LECTURA DE DATOS
// =============================================================================


   //AQUI SE VE SI HAY QUE FILTRAR. En tal caso, se coloca el switch como convenga.



/*	if (gb_hw_sitau_enabled == 0)
	{
		// Recorre los canales habilitados para la recepcion
		if ((result = VCH_Process_NoHardware_sitau2(vch->afe.AFE_BUSSAR_UT.num_samples,idx_fl*100)) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}*/

   n_data_per_minibase_word32 = vch->afe.AFE_BUSSAR_UT.num_samples * 32 / 2;
	//if(gb_uci.acquisition_mode == AQUISITION_MODE_CIRCULAR_BUFFER)
	if((vch->remote_addr_end - vch->remote_addr_cur)<(n_data_per_minibase_word32*4))
		vch->remote_addr_cur = vch->remote_addr_ini;


	addr = vch->remote_addr_cur;

	{
		static int forzar_filtro_unidad=1;

	   if(!forzar_filtro_unidad)
		   config_FIR_coeficients(vch->fir.coefficients);
	   else
		   config_FIR_coeficients(filtro_unidad_casi);

	   if (vch->signal_mode == VIDEO)
		   config_HIL_coeficients(hilbert_coef);
	   config_interleaving(calc_interleaving(32),(vch->signal_mode == VIDEO));
	}

   if (vch->signal_mode == VIDEO)
   {
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_ENVOLV_IDX,0);
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);
   }
   else
   {
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
	   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);
   }
	//for(int idx_minibase=vch->n_bases-1;idx_minibase>=0;idx_minibase--)
   for(u8 idx_minibase=0;idx_minibase<vch->n_bases;idx_minibase++)
	{
		minibase =  vch->base[idx_minibase].bcc_addr;


		channel_extract_configure_basic(32,32,vch->afe.AFE_BUSSAR_UT.num_samples);

		{


			u32* sswitch;

			sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

			switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

			MCBCC_deactivate_emu();
		}

		//DMA_memcpy_remote_base_int_no_block(n_data_per_minibase_word32,minibase,addr);
		DMA_memcpy_remote_base_int_no_block_last_ctrl(n_data_per_minibase_word32,minibase,addr,LAST_DONT_TOUCH);
		//DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_completo,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);
		//xil_printf("addr = %08X, n_data_per_minibase_word32 = %6d, minibase %d\n\r",addr,n_data_per_minibase_word32,minibase);

		if((result = DMA_Write32_from_BCC_wait_end(FMC_LINE_RECV_TIMEOUT)) < 0)
		{
			xil_printf("ERROR, TIMEOUT ESPERANDO DMA_memcpy_remote_base_int_no_block(n_data_per_minibase_word32,minibase,addr)\n\r");
			{
				status_t status_datamov;
				status_datamov = get_minibase2uci_buffer_datamover_status(minibase);
				if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
					LOG_ACQUIRING("STATUS: 0x%08X\n\r",status_datamov.status_32);
			}
		}

		//usleep_A9(3);


		{
			u32* sswitch;

			sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

			MCBCC_deactivate_emu();
		}
	}
   usleep_A9(3);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
	//No hace falta que comprobemos que la siguiente posición es buena, ya lo comprobamos al leer!
	vch->remote_addr_cur+=n_data_per_minibase_word32*4;

	if(0)
	{
		u32 direccion;
		u32 status;
		u32 comando;
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,&direccion);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STATUS   ,&status);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND  ,&comando);
		xil_printf("\n\rwrite_addr = 0x%08X, read_addr = 0x%08X, status = 0x%08X, comando = 0x%08X\n\r",direccion,vch->remote_addr_cur,status,comando);
	}
// _____________________________________________________________________________
// ------ FIN DEL BUCLE DE ADQUISICION DE LINEAS
// =============================================================================

   // ESCRIBE LA PALABRA DE FIN DE LA IMAGEN
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xC0C0... ");
   if ((result = UCI_MemoryWrite_uint32(0x0000C0C0)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   if ((result = DMA_close_and_send_to_host(2)) < 0) return RLOG(result);

	// if ((result = UCI_SetNextVirtualChannel_ConfigHardware (vch)) < 0) return RLOG(result);

	// if ((result = VCH_AcquisitionData_Disabled(vch)) < 0) return RLOG(result);

	// Salida SYNC = 0
	//if ((result = UCI_Sync(0)) < 0) return RLOG(result);

// _____________________________________________________________________________
// ------ FIN DEL BUCLE DE PRINCIPAL
// =============================================================================

   if (gb_log_acquiring == 1) LOG_ACQUIRING(" OK");

   return EUCI_NONE;
}

int FMC_transfer_focal_law_ETH_sin_filtro(TVCH *vch, u32 focal_law)
{
int result;
u32 n_words_32_focal_law, n_buffer_data;
u32 addr, n_data_per_minibase_word32;
u8 minibase;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	if (focal_law >= MOD_PUL_MAX_FOCAL_LAW) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	//first_base	= &vch->base[0];

// ____________________________________________________________________________
// CABECERA DE LA TRAMA
// ============================================================================

   n_words_32_focal_law = vch->size32_fp_frame;
   if (gb_log_acquiring_frame == 1)
      LOG_ACQUIRING_FRAME("\r\nDMA_Set_Datamover_Write16((%d + %d) * %d + %d + 1 = %d)... ",
         vch->size_header_line_fp,
         vch->size32_fp_image,
		 (int)MOD_MAX_CH,
         vch->size_header_img_fp,
         n_words_32_focal_law);

   if ((result = DMA_Set_Datamover_Write32(n_words_32_focal_law, &n_buffer_data)) < 0) return RLOG(result);

   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   if ((result = FMC_transfer_write_header(vch, focal_law,n_buffer_data)) < 0) return RLOG(result);

// _____________________________________________________________________________
// ------ LECTURA DE DATOS
// =============================================================================


   //AQUI SE VE SI HAY QUE FILTRAR. En tal caso, se coloca el switch como convenga.



/*	if (gb_hw_sitau_enabled == 0)
	{
		// Recorre los canales habilitados para la recepcion
		if ((result = VCH_Process_NoHardware_sitau2(vch->afe.AFE_BUSSAR_UT.num_samples,idx_fl*100)) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}*/

   n_data_per_minibase_word32 = vch->afe.AFE_BUSSAR_UT.num_samples * 32 / 2;
	if((vch->remote_addr_end - vch->remote_addr_cur)<(n_data_per_minibase_word32*4))
		vch->remote_addr_cur = vch->remote_addr_ini;


	addr = vch->remote_addr_cur;

	//for(int idx_minibase=vch->n_bases-1;idx_minibase>=0;idx_minibase--)
   for(u8 idx_minibase=0;idx_minibase<vch->n_bases;idx_minibase++)
	{
		minibase =  vch->base[idx_minibase].bcc_addr;
		{
			u32* sswitch;

			sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

			switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

			MCBCC_deactivate_emu();
		}
		DMA_memcpy_remote_base_int_no_block(n_data_per_minibase_word32,minibase,addr);
		//xil_printf("addr = %08X, n_data_per_minibase_word32 = %6d, minibase %d\n\r",addr,n_data_per_minibase_word32,minibase);

		if((result = DMA_Write32_from_BCC_wait_end(20000.0)) < 0)
		{
			xil_printf("ERROR, TIMEOUT ESPERANDO DMA_memcpy_remote_base_int_no_block(n_data_per_minibase_word32,minibase,addr)\n\r");
			{
				status_t status_datamov;
				status_datamov = get_minibase2uci_buffer_datamover_status(minibase);
				if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
					LOG_ACQUIRING("STATUS: 0x%08X\n\r",status_datamov.status_32);
			}
		}

		{
			u32* sswitch;

			sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

			MCBCC_deactivate_emu();
		}
	}


	//No hace falta que comprobemos que la siguiente posición es buena, ya lo comprobamos al leer!
	vch->remote_addr_cur+=n_data_per_minibase_word32*4;

	if(0)
	{
		u32 direccion;
		u32 status;
		u32 comando;
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,&direccion);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STATUS   ,&status);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND  ,&comando);
		xil_printf("\n\rwrite_addr = 0x%08X, read_addr = 0x%08X, status = 0x%08X, comando = 0x%08X\n\r",direccion,vch->remote_addr_cur,status,comando);
	}
// _____________________________________________________________________________
// ------ FIN DEL BUCLE DE ADQUISICION DE LINEAS
// =============================================================================

   // ESCRIBE LA PALABRA DE FIN DE LA IMAGEN
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xC0C0... ");
   if ((result = UCI_MemoryWrite_uint32(0x0000C0C0)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
   if ((result = DMA_close_and_send_to_host(2)) < 0) return RLOG(result);

	// if ((result = UCI_SetNextVirtualChannel_ConfigHardware (vch)) < 0) return RLOG(result);

	// if ((result = VCH_AcquisitionData_Disabled(vch)) < 0) return RLOG(result);

	// Salida SYNC = 0
	//if ((result = UCI_Sync(0)) < 0) return RLOG(result);

// _____________________________________________________________________________
// ------ FIN DEL BUCLE DE PRINCIPAL
// =============================================================================

   if (gb_log_acquiring == 1) LOG_ACQUIRING(" OK");

   return EUCI_NONE;
}

int FMC_transfer_write_header(TVCH *vch, u32 idx_fl,u32 n_buffer_data)
{
	int result, n_ascan;
	u32 n_words_32_focal_law;

	n_words_32_focal_law = vch->size32_fp_frame;
	n_ascan = MOD_MAX_CH * vch->n_bases;

	// ESCRIBE LA PALABRA DE INICIO DE LA IMAGEN
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xCEB0... ");
	if ((result = UCI_MemoryWrite_uint32(0x0000CEB0)) < 0) return RLOG(result);
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

	// ESCRIBE EL ID DEL CANAL VIRTUAL
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nId. Virtual Channel(%d)... ", vch->id);
	if ((result = UCI_MemoryWrite_uint32(vch->id)) < 0) return RLOG(result);
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

	// ESCRIBE EL NUMERO TOTAL DE DATOS DE LA TRAMA
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Data(%d)... ", n_words_32_focal_law);
	if ((result = UCI_MemoryWrite_uint32(n_words_32_focal_law)) < 0) return RLOG(result);
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

	// ESCRIBE EL ID DE LA LEY FOCAL
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nId. Focal Law(%d)... ", idx_fl);
	if ((result = UCI_MemoryWrite_uint32(idx_fl)) < 0) return RLOG(result);
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

	// ESCRIBE EL NUMERO DE SEÑALES A-SCAN (Canales habilitados para la recepción)
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. AScan(%d)... ", n_ascan);
	if ((result = UCI_MemoryWrite_uint32((u32)n_ascan)) < 0) return RLOG(result);
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

	// ESCRIBE EL NUMERO DE DATOS DE CADA ASCAN (Canal)
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Samples(%d)... ", vch->afe.AFE_BUSSAR_UT.num_samples);
	if ((result = UCI_MemoryWrite_uint32(vch->afe.AFE_BUSSAR_UT.num_samples)) < 0) return RLOG(result);
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

	// ESCRIBE EL CONTADOR DE ADQUISICIONES
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nAcqCounter... ");
	vch->acq_counter++;

	if ((result = UCI_MemoryWrite_uint32(vch->acq_counter)) < 0) return RLOG(result);
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

	// ESCRIBE EL TIME STAMP
	if (vch->enabled_timestamp == 1)
	{
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nTimeStamp... ");
		if ((result = UCI_MemoryWrite_TimeStamp()) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}

	// ESCRIBE LA POSICION DEL ENCODER
	// Encoder 0
	if (vch->enabled_encoder_1 == 1)
	{
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[0])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel A Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[0])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel B Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[0])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel A Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[0])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Channel B Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[0])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_0 Sign Changes... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[0])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}
	// Encoder 1
	if (vch->enabled_encoder_2 == 1)
	{
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[1])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel A Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[1])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel B Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[1])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel A Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[1])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Channel B Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[1])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_1 Sign Changes... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[1])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}
	// Encoder 2
	if (vch->enabled_encoder_3 == 1)
	{
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[2])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel A Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[2])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel B Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[2])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel A Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[2])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Channel B Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[2])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_2 Sign Changes... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[2])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}
	// Encoder 3
	if (vch->enabled_encoder_4 == 1)
	{
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_trigger_value[3])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel A Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_valid_edges_counter[3])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel B Edges... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_valid_edges_counter[3])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel A Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_a_filtered_glitches_counter[3])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Channel B Filtered Glitches... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_channel_b_filtered_glitches_counter[3])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nEncoder_3 Sign Changes... ");
		if ((result = UCI_MemoryWrite_EncoderValue(gb_encoder_sign_changes_counter[3])) < 0) return RLOG(result);
		if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");
	}

	// ESCRIBE EL VALOR DEL OFFSET APLICADO AL ENCODER DEL DISPARO
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


	// ESCRIBE EL NUMERO DE DATOS EN EL BUFFER DMA
	if (vch->enabled_buffer_dma == 1)
	{
		if ((result = DMA_Write32(n_buffer_data)) < 0) return RLOG(result);
	}
}


int FMC_transfer_block_GTX_link_pro(TVCH *vch,u8 last_block)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	u32 addr, n_data_per_minibase_word32;
	u32 btt;
	u64 tic=0,toc=0;
	int i;
	float us_acq,velocidad_del_canal_MBps;
	gtx_misc_fsm_ctrl_reg_t ctrl_gtx_reg;

	u8 timestamp_flag=0;
	u32 oob_data=0x77777778;
	u8 hard_break_flag=0;
	u16 num_valid_focal_laws_on_last_block;

	if(ACQ->stored_timestamp && (ACQ->block_counter_header==ACQ->bloq_index_timestamped))
	{
		timestamp_flag = 1;
		oob_data=timestamp_get_ms_32(Timstamp_ini, ACQ->timestamp_value);
		ACQ->stored_timestamp=0;
	}


	tic=timestamp_get_ticks_64();

	n_data_per_minibase_word32 = vch->afe.AFE_BUSSAR_UT.num_samples * 32 / 2;
	btt = n_data_per_minibase_word32*4;

	sw256_remote_mem_to_512_switch(MINIBASE_BROADCAST,1);
	sw256_remote_mem_to_gtx_switch(MINIBASE_BROADCAST,0);

	if(last_block)
		last_block=1; //esto es para hacer breakpoints, se puede quitar

	if(ACQ->n_focal_laws_per_block_count==0)
	{
		num_valid_focal_laws_on_last_block=ACQ->n_focal_laws_per_block_total;
	}
	else
		num_valid_focal_laws_on_last_block=ACQ->n_focal_laws_per_block_count;

	for (u16 block_focal_law_cnt=0; block_focal_law_cnt<vch->gtx_n_focal_law; block_focal_law_cnt++)
	{


		//if(gb_uci.acquisition_mode == AQUISITION_MODE_CIRCULAR_BUFFER)// A partir de ahora siempre buffer circular
		if(1)
		{
			if((vch->remote_addr_end - vch->remote_addr_cur)<(n_data_per_minibase_word32*4))
				vch->remote_addr_cur = vch->remote_addr_ini;
		}
		addr = vch->remote_addr_cur;


		Global_commnd_buf.total_len=0;
		ctrl_gtx_reg.gtx_misc_fsm_ctrl_reg_32 = 0x0;
		ctrl_gtx_reg.BITS.lnk_dn_reset_fsm = 1;


		config_gtx(ctrl_gtx_reg,MINIBASE_BROADCAST,&Global_commnd_buf);
		config_mem2beamformer(addr,btt,0,0,1,MINIBASE_BROADCAST,&Global_commnd_buf);
		BCC_SendBuffer(&Global_commnd_buf);
		Global_commnd_buf.total_len=0;

		if(block_focal_law_cnt==0)
		{
			gtx_set_link_pro_out(1);
			while(gtx_get_link_pro_in()==0);
			gtx_set_link_pro_out(0);
			FMC_transfer_GTX_header(vch,4,ACQ->block_counter_header,ACQ->image_counter_header,ACQ->focal_law_index_header,num_valid_focal_laws_on_last_block,last_block,1,timestamp_flag,oob_data);
		}
		else
		{
			FMC_transfer_GTX_header(vch,4,ACQ->block_counter_header,ACQ->image_counter_header,ACQ->focal_law_index_header,num_valid_focal_laws_on_last_block,last_block,0,timestamp_flag,oob_data);
			//FMC_transfer_GTX_remote_repeat_header(vch,4,0);
			gtx_set_link_pro_out(1);
			while(gtx_get_link_pro_in()==0);
			gtx_set_link_pro_out(0);
		}

		i=0;
		while(gtx_get_link_pro_in())
		{
			int j;
			i++;
			if(i>5000)
			{
				ACQ_FMC_errorPRO_link(addr);
				LOG_ACQUIRING("\n\r");
				for(j=0;j<20;j++)
				{
					LOG_ACQUIRING("Esperando... %d\n\r",(int)j);
					usleep_timer(20000);
					if(gtx_get_link_pro_in()==0)
						break;
				}
				if(gtx_get_link_pro_in()==0)
				{
					LOG_ACQUIRING("PRO_link tardó en bajar, pero bajó.\n\r",(int)j);
					break;
				}
				else
				{
					LOG_ACQUIRING("PROBLEMA EN LAS COMUNICACIONES: ciclo PRO_link timed out.\n\r");
					hard_break_flag=1;
					break;
				}
			}
		}
		if(hard_break_flag)
		{
			ACQ_FMC_hard_break(vch);
			break;
		}

		if(++ACQ->focal_law_index_header==vch->n_efl)
		{
			ACQ->focal_law_index_header=0;
			ACQ->image_counter_header++;
		}


		vch->remote_addr_cur+=n_data_per_minibase_word32*4;

	}
	ACQ->block_counter_header++;

	toc=timestamp_get_ticks_64();
	us_acq=timestamp_get_us_float(tic, toc);

	velocidad_del_canal_MBps=btt*vch->n_efl*4/us_acq;

	if(velocidad_del_canal_MBps<100)
	{
		u32 direccion;
		u32 status;
		u32 comando;
		LOG_ACQUIRING("\n\ri=\n\r",(int)i);
		LOG_ACQUIRING("\n\rVelocidad del canal %d MBps\n\r",(int)velocidad_del_canal_MBps);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,&direccion);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STATUS   ,&status);
		BCC_read_reg(2,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND  ,&comando);
		LOG_ACQUIRING("\n\rwrite_addr = 0x%08X, read_addr = 0x%08X, status = 0x%08X, comando = 0x%08X\n\r",direccion,vch->remote_addr_cur,status,comando);
	}

	/*
	LOG_ACQUIRING("\n\r");
	LOG_ACQUIRING("Tiempo ACQ: %d us\n\r",(int)us_acq);
	*/

	if (gb_log_acquiring == 1) LOG_ACQUIRING(" OK");

	return EUCI_NONE;
}
