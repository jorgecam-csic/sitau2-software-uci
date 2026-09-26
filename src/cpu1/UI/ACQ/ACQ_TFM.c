#include "ACQ_TFM.h"
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
#include "ACQ_FMC.h"
#include "datamover_remote.h"

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

volatile ACQ_hndlr_t TFM_handler;
extern volatile int new_host_data;


void TFM_interrupt_callback_function(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	state_TFM_t next_state = ACQ->state_TFM;


	if (ACQ->state_TFM == IDLE)
	{


	}
	else if (ACQ->state_TFM == AQUIRING)
	{
		ACQ_FMC_continuous_interrupt_callback_function();
		if (ACQ->acquisition_ended==1)
			next_state = BEAMFORMING;
	}
	else if (ACQ->state_TFM == BEAMFORMING)
	{


	}
	else if (ACQ->state_TFM == RECONFIG)
	{


	}
	else if (ACQ->state_TFM == TRANSFER_TO_UCI)
	{


	}
	else if (ACQ->state_TFM == WAITING_UNPAUSE)
	{
		if(ACQ->pause_continous==0)
			next_state = AQUIRING;

	}
	ACQ->state_TFM = next_state;
}


void TFM_restart_for_new_burst()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;


	ACQ->n_images_count=0;
	ACQ->n_images_beamformed=0;
	ACQ->n_images_sent=0;

	ACQ->n_focal_laws_count=0;

	ACQ->n_lines_bf_count=0;


	ACQ->write_addr_index = ACQ->addr_ini;
	ACQ->read_addr_index = ACQ->addr_ini;


	ACQ->n_images_burst_count=0;
	ACQ->n_bursts_count=0;
	ACQ->n_bursts_beamfomed=0;
	ACQ->n_bursts_sent=0;

	MCBCC_int_enable(POFE_MASK);

	MCBCC_asign_callback(ACQ_FMC_errorPRO,TRE_NBIT);
	MCBCC_int_enable(TREE_MASK);

	MCBCC_asign_callback(TFM_interrupt_callback_function,POF_NBIT);//	PRO Fall flank interrupt Enable asignado a la interrupción

	MCBCC_asign_callback(ACQ_FMC_trig_in_interrupt_callback_function,TRG_NBIT);   //	TRIGGER rise flank interrupt Enable asignado a la interrupción
	MCBCC_int_disable(TRGE_MASK);
}

void TFM_FSM(void)
{

	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	//state_TFM_t next_state = ACQ->state_TFM;
	const u8 bcc_addr_broadcast = 0;


	if (ACQ->state_TFM == IDLE)
	{
		TFM_conf_mode_ACQ();
		TRIG_reset_all_interrupt_sources();
		switch(gb_uci.trigger_source)
		{
			case TRG_SCAN_PRF: TRIG_set_interrupt_source(TRIGGER_PRFTIMER1_MASK); break;
			case TRG_SCAN_EXT: TRIG_set_interrupt_source(TRIGGER_EXT_MASK); break;
		}
		ACQ->state_TFM = AQUIRING;
		ACQ_FMC_start_trigger();
	}
	if (ACQ->state_TFM == AQUIRING)
	{
		if (ACQ->acquisition_ended==1 || ACQ->pause_continous==1)
		{
			ACQ->n_bursts_count++;
			ACQ->state_TFM = BEAMFORMING;
		}
	}
	if (ACQ->state_TFM == BEAMFORMING)
	{
		TRIG_reset_all_interrupt_sources();
		TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);
		MCBCC_POAF(1);
		MCBCC_TRIG2PRO(1);
		UCI_Sync(0);
		UCI_set_Sync_is_PRO();
		MCBCC_remove_callback(POF_NBIT);
		MCBCC_remove_callback(POR_NBIT);
		MCBCC_int_enable(PORE_MASK);
		MCBCC_int_enable(POFE_MASK);

		TFM_process_burst();

		ACQ->n_bursts_beamfomed++;
		ACQ->state_TFM = TRANSFER_TO_UCI;
	}
	if (ACQ->state_TFM == TRANSFER_TO_UCI)
	{
		ACQ->read_addr_index = ACQ->tfm_image_addr_index;
		TFM_transfer_fsm(ACQ->virtual_channel_p);
		ACQ->n_bursts_sent++;

		if (ACQ->acquisition_mode == AQUISITION_MODE_CONTINOUS)
		{
			if(ACQ->end_continous==1)
				ACQ->state_TFM = ENDED;
			else
			{

				TRIG_reset_all_interrupt_sources();
				switch(gb_uci.trigger_source)
				{
					case TRG_SCAN_PRF: TRIG_set_interrupt_source(TRIGGER_PRFTIMER1_MASK); break;
					case TRG_SCAN_EXT: TRIG_set_interrupt_source(TRIGGER_EXT_MASK); break;
				}

				TFM_restart_for_new_burst();
				set_afe2mem_256bit_datamover_next_addr(ACQ->write_addr_index, bcc_addr_broadcast);

				TFM_conf_mode_ACQ();

				ACQ->state_TFM = WAITING_UNPAUSE;

			}

		}
		else if(ACQ->n_images_sent==ACQ->n_images_total)
			ACQ->state_TFM = ENDED;
	}
	if (ACQ->state_TFM == WAITING_UNPAUSE)
	{
		if(ACQ->pause_continous==0)
			ACQ->state_TFM = AQUIRING;
	}

	if (ACQ->state_TFM == ENDED)
	{
		gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquiring = 0;
	}

}

void TFM_stop_continous()
{
	//PA_end_continous = 1;
}

void TFM_stop_trigger(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	TIMER_Stop(TIMER_SCAN_PRF_1);
	MCBCC_remove_callback(POF_NBIT);
	MCBCC_remove_callback(TRE_NBIT);
	TRIG_remove_interrupt_source(TRIGGER_PRFTIMER1_MASK);
	UCI_Alarm1(0);
	UCI_Alarm2(0);
}

void TFM_pause_trigger(void)
{
	TIMER_Stop(TIMER_SCAN_PRF_1);
	UCI_Alarm1(0);
	UCI_Alarm2(0);
}

void TFM_conf_mode_ACQ()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	const u8 bcc_addr_broadcast = 0;
	u32 pro_mask_word;
	pul_general_ctrl_t pul_ctrl;
	u32 pulser_delay_mem_index;


	// Datamover 256 lo activan los AFEs
	pro_mask_word = (1<<MISC_PROENA_BIT_AFES) | (1<<MISC_PROENA_BIT_TGC) | (1<<MISC_PROENA_BIT_PULSER);
	bcc_set_pro_mask_word(bcc_addr_broadcast,pro_mask_word);
	bcc_set_busy_or_word(bcc_addr_broadcast,~pro_mask_word);

	pulser_delay_mem_index=32*ACQ->n_focal_laws_count;

	BCC_write_reg_inmediate(bcc_addr_broadcast,PULSER_BUSSAR_SUBMOD_ADDR,DELAY_MEM_ADDR_R,pulser_delay_mem_index);
	pul_ctrl.pul_general_ctrl_u32=0x0;
	pul_ctrl.BITS.external_trigger=1;
	pul_ctrl.BITS.pulser_load     =1;
	pul_ctrl.BITS.pulser_auto_load=1;
	pul_ctrl.BITS.pul_enable      =1;
	pul_ctrl.BITS.n_pulses        =ACQ->virtual_channel_p->pulser.pul_general_ctrl.BITS.n_pulses;
	pul_ctrl.BITS.pulser_current  =ACQ->virtual_channel_p->pulser.pul_general_ctrl.BITS.pulser_current;

	BCC_write_reg_inmediate(bcc_addr_broadcast,PULSER_BUSSAR_SUBMOD_ADDR,PULSER_GEN_R,pul_ctrl.pul_general_ctrl_u32);


	//BCC_write_reg_inmediate(bcc_addr_broadcast,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,ACQ->tfm_image_addr_index);
	//BCC_write_reg_inmediate(bcc_addr_broadcast,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,ACQ->scratch_addr_index);

}


void TFM_conf_mode_BF_one_acq()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	const u8 bcc_addr_broadcast = 0;
	u32 pro_mask_word;
	emi_prom_config_t emi_config;
	u8 auto_inc;

	//
	pro_mask_word = (1<<MISC_PROENA_BIT_DMVR_256) | (1<<MISC_PROENA_BIT_BFMR) | (1<<MISC_PROENA_BIT_DMVR_BFMR);
	bcc_set_pro_mask_word(bcc_addr_broadcast,pro_mask_word);
	bcc_set_busy_or_word(bcc_addr_broadcast,~pro_mask_word);

	emi_config.emi_prom_config_u32 = 0x0;
	emi_config.BITS.ce_bits = (1<<EMI_PROM_MAN_CS_BFDATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROMBF0);
	emi_config.BITS.enable = 0;
	emi_config.BITS.first_rpt = 1;
	emi_config.BITS.last_rpt = 1;
	emi_prom_prog_inmediat(&emi_config,bcc_addr_broadcast);

	auto_inc=0;
	mem2beamformer_inmediate_command(ACQ->bytes_per_fl_remote,0,auto_inc,1,0,1,1,bcc_addr_broadcast);

}
void TFM_conf_mode_BF_one_acq_old()
{
	const u8 bcc_addr_broadcast = 0;
	u32 pro_mask_word;
	emi_prom_config_t emi_config;

	//
	pro_mask_word = (1<<MISC_PROENA_BIT_DMVR_256) | (1<<MISC_PROENA_BIT_BFMR) | (1<<MISC_PROENA_BIT_DMVR_BFMR);
	bcc_set_pro_mask_word(bcc_addr_broadcast,pro_mask_word);
	bcc_set_busy_or_word(bcc_addr_broadcast,~pro_mask_word);

	emi_config.emi_prom_config_u32 = 0x0;
	emi_config.BITS.ce_bits = (1<<EMI_PROM_MAN_CS_BFDATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROMBF0);
	emi_config.BITS.enable = 0;

	emi_prom_prog_inmediat(&emi_config,bcc_addr_broadcast);

	emi_config.emi_prom_config_u32 = 0x0;
	emi_config.BITS.ce_bits = 0;
	emi_config.BITS.last_rpt = 1;
	emi_prom_prog_inmediat(&emi_config,bcc_addr_broadcast);
}

void TFM_conf_mode_BF_first_acq()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	const u8 bcc_addr_broadcast = 0;
	u32 pro_mask_word;
	emi_prom_config_t emi_config;

	pro_mask_word = (1<<MISC_PROENA_BIT_DMVR_256) | (1<<MISC_PROENA_BIT_BFMR) | (1<<MISC_PROENA_BIT_DMVR_BFMR);
	bcc_set_pro_mask_word(bcc_addr_broadcast,pro_mask_word);
	bcc_set_busy_or_word(bcc_addr_broadcast,~pro_mask_word);

	emi_config.emi_prom_config_u32 = 0x0;
	emi_config.BITS.ce_bits = (1<<EMI_PROM_MAN_CS_BFDATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROMBF0);
	emi_config.BITS.first_rpt = 1;
	emi_config.BITS.last_rpt = 0;
	emi_config.BITS.prom = 1;
	emi_config.BITS.dsr = ACQ->dsr_shift_bits;
	emi_prom_prog_inmediat(&emi_config,bcc_addr_broadcast);

	BF0_datamover_remote_write_inmediate_MM2S_addr(ACQ->scratch_addr_index,0);

}

void TFM_conf_mode_BF_mid_acq()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	const u8 bcc_addr_broadcast = 0;
	u32 pro_mask_word;
	emi_prom_config_t emi_config;

	pro_mask_word = (1<<MISC_PROENA_BIT_DMVR_256) | (1<<MISC_PROENA_BIT_BFMR) | (1<<MISC_PROENA_BIT_DMVR_BFMR);
	bcc_set_pro_mask_word(bcc_addr_broadcast,pro_mask_word);
	bcc_set_busy_or_word(bcc_addr_broadcast,~pro_mask_word);

	emi_config.emi_prom_config_u32 = 0x0;
	emi_config.BITS.ce_bits = (1<<EMI_PROM_MAN_CS_BFDATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROMBF0);
	emi_config.BITS.first_rpt = 0;
	emi_config.BITS.last_rpt = 0;
	emi_config.BITS.prom = 1;
	emi_config.BITS.dsr = ACQ->dsr_shift_bits;
	emi_prom_prog_inmediat(&emi_config,bcc_addr_broadcast);

	BF0_datamover_remote_write_inmediate_MM2S_addr(ACQ->scratch_addr_index,0);

}

void TFM_conf_mode_BF_last_acq()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	const u8 bcc_addr_broadcast = 0;
	u32 pro_mask_word;
	emi_prom_config_t emi_config;

	pro_mask_word = (1<<MISC_PROENA_BIT_DMVR_256) | (1<<MISC_PROENA_BIT_BFMR) | (1<<MISC_PROENA_BIT_DMVR_BFMR);
	bcc_set_pro_mask_word(bcc_addr_broadcast,pro_mask_word);
	bcc_set_busy_or_word(bcc_addr_broadcast,~pro_mask_word);

	emi_config.emi_prom_config_u32 = 0x0;
	emi_config.BITS.ce_bits = (1<<EMI_PROM_MAN_CS_BFDATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROMBF0);
	emi_config.BITS.first_rpt = 0;
	emi_config.BITS.last_rpt = 1;
	emi_config.BITS.prom = 1;
	emi_config.BITS.dsr = ACQ->dsr_shift_bits;
	emi_prom_prog_inmediat(&emi_config,bcc_addr_broadcast);

	BF0_datamover_remote_write_inmediate_MM2S_addr(ACQ->scratch_addr_index,0);

}

void TFM_DEBUG_void_forward_focal_law(int focal_law,TVCH *vch)
{
	u32 *focal_law_ptr;
	u32 focal_law_size_word32;

	//const u32 disable_mask_const=0x20000;
	u32 repeat_byte,format_word;


	int i;

	repeat_byte=(1<<(focal_law+1))-1;
	format_word=(repeat_byte<<24)|(repeat_byte<<16)|(repeat_byte<<8)|repeat_byte;

	focal_law_size_word32=BEAMFORMER_SIZE_FMC_FWD_WORD32*vch->pa_image.n_lines;
	focal_law_ptr=&vch->tfm_fmc_forward.memory[focal_law_size_word32*focal_law];

	//for(i=0;i<ACQ->n_lines_bf_total;i++)
		//focal_law_ptr[BEAMFORMER_SIZE_FMC_FWD_WORD32*i+3]|=disable_mask_const;

	for(i=0;i<vch->pa_image.n_lines*BEAMFORMER_SIZE_FMC_FWD_WORD32;i++)
		focal_law_ptr[i]=format_word;

}

int TFM_beamformer_prog_new_fw_focal_law(int focal_law)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	TVCH *vch;
	vch=ACQ->virtual_channel_p;

	u32 *focal_law_ptr;
	u32 focal_law_size_word32;


	if(ACQ->MBF_image_type==TFM)
	{
		focal_law_size_word32=BEAMFORMER_SIZE_FMC_FWD_WORD32*ACQ->n_lines_bf_total;
		focal_law_ptr=&vch->tfm_fmc_forward.memory[focal_law_size_word32*focal_law];
	}
	else if(ACQ->MBF_image_type==PWI)
	{
		focal_law_size_word32=BEAMFORMER_SIZE_PWI_FWD_WORD32*ACQ->n_lines_bf_total;
		focal_law_ptr=&vch->tfm_pwi_forward.forwardMemory[focal_law_size_word32*focal_law];
	}
	else //esto no debería suceder, pero mejor dejar algo inicializado
		focal_law_size_word32=0;


	vch->bf_regs.prog_addr_ini_end_ida.BITS.prog_addr_ini_ida=ACQ->next_fwd_fl_offset_addr;

	beamformer_fwrd_params(focal_law_ptr, focal_law_size_word32, &vch->bf_regs, 0, &Global_commnd_buf);

	ACQ->next_fwd_fl_offset_is_prog=1;

	ACQ->current_fwd_fl=focal_law;

	return 0;
}

void TFM_beamformer_refresh_offset_fw_focal_law(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	TVCH *vch;
	vch=ACQ->virtual_channel_p;

	if(ACQ->next_fwd_fl_offset_is_prog==0)
		return;

	vch->bf_regs.ln_planner_reg.BITS.ln_plan_ida_offset=ACQ->next_fwd_fl_offset_addr;

	beamformer_set_ln_plan_reg(vch->bf_regs.ln_planner_reg.ln_planner_reg_u32,0);


	//if(ACQ->next_fwd_fl_offset_addr+3*(ACQ->n_lines_bf_total)>BEAMFORMER_MAX_FW_FOCAL_LAWS)//esto no es lo correcto, es apaño de lo siguiente, pero creo que ya está arreglado.
	if(ACQ->next_fwd_fl_offset_addr+2*(ACQ->n_lines_bf_total)>BEAMFORMER_MAX_FW_FOCAL_LAWS)
		ACQ->next_fwd_fl_offset_addr =0;
	else
		ACQ->next_fwd_fl_offset_addr+=ACQ->n_lines_bf_total;

	ACQ->next_fwd_fl_offset_is_prog=0;
	if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\r NEXT FWD_FL_OFFSET:  %08X",ACQ->next_fwd_fl_offset_is_prog);
}
void TFM_process_burst()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	int result;
	u8 bcc_addr_broadcast=0;
	u8 auto_inc;

	beamformer_ut_struct_t bf_ut_struct_print_pre;
	beamformer_ut_struct_t bf_ut_struct_print_pos;
	beamformer_ut_struct_t bf_ut_struct_print_ps1;

	ACQ->read_addr_index = ACQ->addr_ini;

	for(ACQ->n_images_beamformed=0;ACQ->n_images_beamformed<ACQ->n_images_total;ACQ->n_images_beamformed++)
	{
		if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\r--------Nueva IMAGEN: %d/%d----------\n\r",ACQ->n_images_beamformed,ACQ->n_images_total);
		for(ACQ->n_focal_laws_count=0;ACQ->n_focal_laws_count<ACQ->n_focal_laws_total;ACQ->n_focal_laws_count++)
		{

			if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\r--------Nueva FL: %d/%d----------\n\r",ACQ->n_focal_laws_count,ACQ->n_focal_laws_total);

			/*
			if(DEBUG_BEAMFORMER)
			{
				u32 registro;
				BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
				BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
				LOG_ACQUIRING("\r\n");
			}
			*/


			if(ACQ->MBF_image_type==PWI)
				beamformer_prog_cos_pwi_reg(&ACQ->virtual_channel_p->bf_regs,ACQ->virtual_channel_p->tfm_pwi_forward.angleMemory,ACQ->n_focal_laws_count,bcc_addr_broadcast);


			if(DEBUG_BEAMFORMER)
			{
				u32 registro;
				BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
				BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
				LOG_ACQUIRING("\r\n");
			}

			if(ACQ->n_focal_laws_total==1)
				TFM_conf_mode_BF_one_acq();
			else if((ACQ->n_focal_laws_count+1)==ACQ->n_focal_laws_total)
				TFM_conf_mode_BF_last_acq();
			else if(ACQ->n_focal_laws_count==0)
				TFM_conf_mode_BF_first_acq();
			else// if(ACQ->n_focal_laws_count==1)
				TFM_conf_mode_BF_mid_acq();

			if(ACQ->n_focal_laws_total>1)
			{
				if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pre,1);

				if(ACQ->n_focal_laws_count==(ACQ->n_focal_laws_total-1))
					TFM_beamformer_prog_new_fw_focal_law(0);
				else
					TFM_beamformer_prog_new_fw_focal_law(ACQ->n_focal_laws_count+1);


				if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pos,1);


				if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPRE PROG FL\n\r");
				if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pre);
				if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPOS PROG FL \n\r");
				if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pos);

			}

			set_mem2beamformer_256bit_datamover_next_addr(ACQ->read_addr_index, bcc_addr_broadcast);

			for(ACQ->n_lines_bf_count=0;ACQ->n_lines_bf_count<ACQ->n_lines_bf_total;ACQ->n_lines_bf_count+=ACQ->n_lines_bf_parallel)
			{
				if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\r--------Nuevo grupo lineas: %d/%d----------\n\r",ACQ->n_lines_bf_count,ACQ->n_lines_bf_total);

				if(ACQ->n_lines_bf_count+ACQ->n_lines_bf_parallel>=ACQ->n_lines_bf_total)
					auto_inc=1;
				else
					auto_inc=0;


				if((ACQ->n_focal_laws_total>1) && (ACQ->n_lines_bf_count+ACQ->n_lines_bf_parallel>=ACQ->n_lines_bf_total))
				{
					if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pre,1);

					TFM_beamformer_refresh_offset_fw_focal_law();

					if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pos,1);


					if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPRE REFRESH FW FL\n\r");
					if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pre);
					if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPOS REFRESH FW FL \n\r");
					if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pos);

				}


				if(DEBUG_BEAMFORMER)
				{
					u32 read_address;
					read_address =  get_mem2beamformer_256bit_datamover_next_addr(3),

					LOG_ACQUIRING("\r\nDATAMOVER_START_ADDR: %08X\n\r",read_address);
				}
				/////////TRIGGER!!!!////////
				if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\r----------- TRIGGER (beamform TFM)! ----------- \n\r");
				UCI_SW2HW_trig();
				////////////////////////////
				if(DEBUG_BEAMFORMER)
				{
					u32 registro;
					BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
					LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
					BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
					LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
					LOG_ACQUIRING("\r\n");
				}

				result = BCC_wait_flag(PORF_MASK,1000000.0);
				if (result)
					ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);


				result = BCC_wait_flag(POFF_MASK,1000000.0);
				if (result)
				{
					ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);
					bcc_print_all_busy_flags(gb_uci.n_sub_systems);
				}


				if(DEBUG_BEAMFORMER)
				{
					beamformer_get_regs(&bf_ut_struct_print_ps1,1);
					xil_printf("\n\r\n\r                                              POS TRIGGER FL \n\r");
					beamformer_print_regs(&bf_ut_struct_print_ps1);
				}
				if((ACQ->n_focal_laws_total>1) && (ACQ->n_lines_bf_count+ACQ->n_lines_bf_parallel>=ACQ->n_lines_bf_total))
					ACQ->read_addr_index+=ACQ->bytes_per_fl_remote;

			}
		}
	}

}

void TFM_process_no_int(void)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	int result;
	u8 bcc_addr_broadcast=0;

	beamformer_ut_struct_t bf_ut_struct_print_pre;
	beamformer_ut_struct_t bf_ut_struct_print_pos;
	beamformer_ut_struct_t bf_ut_struct_print_ps1;

	for(ACQ->n_images_count=0;ACQ->n_images_count<ACQ->n_images_total;)
	{
		for(ACQ->n_focal_laws_count=0;ACQ->n_focal_laws_count<ACQ->n_focal_laws_total;ACQ->n_focal_laws_count++)
		{

			if(DEBUG_BEAMFORMER)
			{
				u32 registro;
				BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
				BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
				LOG_ACQUIRING("\r\n");
			}

			TFM_conf_mode_ACQ();
			if(ACQ->MBF_image_type==PWI)
				beamformer_prog_cos_pwi_reg(&ACQ->virtual_channel_p->bf_regs,ACQ->virtual_channel_p->tfm_pwi_forward.angleMemory,ACQ->n_focal_laws_count,bcc_addr_broadcast);


			if(DEBUG_BEAMFORMER)
			{
				u32 registro;
				BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
				BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
				LOG_ACQUIRING("\r\n");
			}

			/////////TRIGGER!!!!////////
			UCI_SW2HW_trig();
			////////////////////////////

			if(DEBUG_BEAMFORMER)
			{
				u32 registro;
				BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
				BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
				LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
				LOG_ACQUIRING("\r\n");
			}

			if (result = BCC_wait_flag(PORF_MASK,1000000.0))
				ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);
			if (result = BCC_wait_flag(POFF_MASK,1000000.0))
				ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);

			if(ACQ->n_focal_laws_total==1)
				TFM_conf_mode_BF_one_acq();
			else if((ACQ->n_focal_laws_count+1)==ACQ->n_focal_laws_total)
				TFM_conf_mode_BF_last_acq();
			else if(ACQ->n_focal_laws_count==0)
				TFM_conf_mode_BF_first_acq();
			else// if(ACQ->n_focal_laws_count==1)
				TFM_conf_mode_BF_mid_acq();

			if(ACQ->n_focal_laws_total>1)
			{
				if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pre,1);

				if(ACQ->n_focal_laws_count==(ACQ->n_focal_laws_total-1))
					TFM_beamformer_prog_new_fw_focal_law(0);
				else
					TFM_beamformer_prog_new_fw_focal_law(ACQ->n_focal_laws_count+1);


				if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pos,1);


				if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPRE PROG FL\n\r");
				if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pre);
				if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPOS PROG FL \n\r");
				if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pos);

			}

			for(ACQ->n_lines_bf_count=0;ACQ->n_lines_bf_count<ACQ->n_lines_bf_total;ACQ->n_lines_bf_count+=ACQ->n_lines_bf_parallel)
			{



				if((ACQ->n_focal_laws_total>1) && (ACQ->n_lines_bf_count+ACQ->n_lines_bf_parallel>=ACQ->n_lines_bf_total))
				{
					if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pre,1);

					TFM_beamformer_refresh_offset_fw_focal_law();

					if(DEBUG_BEAMFORMER) beamformer_get_regs(&bf_ut_struct_print_pos,1);


					if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPRE REFRESH FL\n\r");
					if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pre);
					if(DEBUG_BEAMFORMER) xil_printf("\n\r\n\rPOS REFRESH FL \n\r");
					if(DEBUG_BEAMFORMER) beamformer_print_regs(&bf_ut_struct_print_pos);

				}


				if(DEBUG_BEAMFORMER)
				{
					u32 registro;
					BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
					LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
					BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
					LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
					LOG_ACQUIRING("\r\n");
				}
				/////////TRIGGER!!!!////////
				UCI_SW2HW_trig();
				////////////////////////////
				if(DEBUG_BEAMFORMER)
				{
					u32 registro;
					BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
					LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
					BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
					LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
					LOG_ACQUIRING("\r\n");
				}
				if (result = BCC_wait_flag(PORF_MASK,1000000.0))
					ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);


				if (result = BCC_wait_flag(POFF_MASK,1000000.0))
					ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);


				if(DEBUG_BEAMFORMER)
				{
					beamformer_get_regs(&bf_ut_struct_print_ps1,1);
					xil_printf("\n\r\n\r                                              POS DISPARO FL \n\r");
					beamformer_print_regs(&bf_ut_struct_print_ps1);
				}

			}
		}
		ACQ->n_images_count++;

		TFM_transfer(ACQ->virtual_channel_p);

	}

}

void TFM_process_no_int_one_elem()
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
	int result;


	for(ACQ->n_images_count=0;ACQ->n_images_count<ACQ->n_images_total;)
	{
		for(ACQ->n_focal_laws_count=0;ACQ->n_focal_laws_count<ACQ->n_focal_laws_total;ACQ->n_focal_laws_count++)
		{
			TFM_conf_mode_ACQ();
			/////////TRIGGER!!!!////////
			UCI_SW2HW_trig();
			////////////////////////////

			if (result = BCC_wait_flag(PORF_MASK,1000000.0))
				ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);
			if (result = BCC_wait_flag(POFF_MASK,1000000.0))
				ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);

			TFM_conf_mode_BF_one_acq();

			for(ACQ->n_lines_bf_count=0;ACQ->n_lines_bf_count<ACQ->n_lines_bf_total;ACQ->n_lines_bf_count+=ACQ->n_lines_bf_parallel)
			{

				/////////TRIGGER!!!!////////
				UCI_SW2HW_trig();
				////////////////////////////

				if (result = BCC_wait_flag(PORF_MASK,1000000.0))
					ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);
				if (result = BCC_wait_flag(POFF_MASK,1000000.0))
					ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);

			}
		}
		ACQ->n_images_count++;

		TFM_transfer(ACQ->virtual_channel_p);
	}
}

void TFM_start_trigger(void)
{
	Timstamp_ini=timestamp_get_ticks_64();
	TFM_process_no_int();
    UCI_Alarm1(1);
    UCI_Alarm2(1);	
}

//(unsigned int images,unsigned int n_images_burst,unsigned int focal_laws_per_block,unsigned int focal_laws,unsigned int emi_prom,char acquisition_mode,u32 addr_ini,u32 addr_end,u32 acq_size)

void TFM_interrupt_startup    (TVCH *vch,unsigned int images,unsigned int n_emission_focal_laws,unsigned int n_lines_image,unsigned int n_parallel_bf,unsigned int acquisition_addr,unsigned int scratch_addr,unsigned int tfm_image_addr,MBF_image_type_t MBF_image_type, char pause_after_burst)
{
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	ACQ->end_continous = 0;
	ACQ->pause_continous = 0;
	ACQ->acquisition_ended=0;
	ACQ->transmission_ended=0;
	ACQ->n_overflows=0;
	ACQ->n_pros=0;
	ACQ->n_emi_prom_count=0;
	ACQ->n_focal_laws_per_block_count=0;
	ACQ->n_focal_laws_per_block_total=vch->gtx_n_focal_law;
	ACQ->block_counter_header=0;
	ACQ->image_counter_header=0;
	ACQ->focal_law_index_header=0;

	ACQ->n_pros_delay_sync =0;

	ACQ->state_TFM = IDLE;
	ACQ->virtual_channel_p = vch;
	ACQ->pause_after_burst=1;

	MCBCC_int_enable(POFE_MASK);
	//MCBCC_asign_callback(TFM_end_pro_interrupt_callback_function,POF_NBIT);//	PRO Fall flank interrupt Enable asignado a la interrupción

	MCBCC_asign_callback(TFM_errorPRO,TRE_NBIT);
	MCBCC_int_enable(TREE_MASK);

	ACQ->n_images_count=0;
	ACQ->n_images_beamformed=0;
	ACQ->n_images_sent=0;
	ACQ->n_images_total=images;

	ACQ->n_focal_laws_count=0;
	ACQ->n_focal_laws_total=n_emission_focal_laws;

	ACQ->n_lines_bf_parallel=n_parallel_bf;
	ACQ->n_lines_bf_count=0;
	ACQ->n_lines_bf_total=n_lines_image;

	ACQ->acquisition_addr_index=acquisition_addr;
	ACQ->scratch_addr_index=scratch_addr;
	ACQ->tfm_image_addr_index=tfm_image_addr;

	ACQ->write_addr_index = acquisition_addr;
	ACQ->read_addr_index = acquisition_addr;
	ACQ->bytes_per_fl_remote = vch->afe.AFE_BUSSAR_UT.num_samples * 32 * 2;
	ACQ->addr_ini = acquisition_addr;
	ACQ->addr_end = tfm_image_addr-1;

	ACQ->current_fwd_fl=0;
	ACQ->next_fwd_fl_offset_is_prog=0;
	ACQ->next_fwd_fl_offset_addr=ACQ->n_lines_bf_total;
	ACQ->num_fwd_fl_max=BEAMFORMER_MAX_FW_FOCAL_LAWS;

	ACQ->dsr_shift_bits=ceil_Log2(ACQ->n_focal_laws_total);
	//ACQ->dsr_shift_bits=ceil_Log2(ACQ->n_focal_laws_total)-1;
	//ACQ->dsr_shift_bits=1;
	ACQ->MBF_image_type=MBF_image_type;


	ACQ->acquisition_mode = gb_uci.acquisition_mode;

	if(ACQ->acquisition_mode==AQUISITION_MODE_FIXED)
		ACQ->n_images_total=images;
	else if(ACQ->acquisition_mode == AQUISITION_MODE_CONTINOUS)
		ACQ->n_images_total=images;

	//ACQ->n_images_burst_total=gb_uci.n_images_burst;
	ACQ->n_images_burst_total=images;

	ACQ->n_images_burst_count=0;
	ACQ->n_bursts_count=0;
	ACQ->n_bursts_beamfomed=0;
	ACQ->n_bursts_sent=0;

	MCBCC_int_enable(POFE_MASK);

	MCBCC_asign_callback(ACQ_FMC_errorPRO,TRE_NBIT);
	MCBCC_int_enable(TREE_MASK);

	MCBCC_asign_callback(TFM_interrupt_callback_function,POF_NBIT);//	PRO Fall flank interrupt Enable asignado a la interrupción

	MCBCC_asign_callback(ACQ_FMC_trig_in_interrupt_callback_function,TRG_NBIT);   //	TRIGGER rise flank interrupt Enable asignado a la interrupción
	MCBCC_int_disable(TRGE_MASK);

	ACQ->store_timestamp_on_int=1; //En el primer bloque mandamos timestamp


}


void TFM_errorPRO(void)
{
	u32 busy_word;
	status_t status_datamov;
	u32 wr_addr_hw;

	xil_printf("\n\rError en el PRO! Se intentó elevar el PRO cuando el PRO estaba en alto!\n\r");

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

u8 pruebas_T0=1;


int TFM_set_up(TVCH *vch)
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
	u32 acquisition_addr,scratch_addr,tfm_image_addr;
	MBF_image_type_t MBF_image_type;


	u32 remote_image_size;

	Current_ACQ_hndlr_ptr = &TFM_handler;

	//TRIG_reset_all_interrupt_sources();
	TIMER_Stop(TIMER_SCAN_PRF_1);
	MCBCC_TRIG2PRO(0);
	MCBCC_set_PRO(0);
	while(MCBCC_get_PROI());



	//TRIG_reset_all_interrupt_sources();

	if(vch->bf_regs.bits_ida.BITS.tfm)
		MBF_image_type = TFM;
	else if (vch->bf_regs.bits_ida.BITS.pwi)
		MBF_image_type = PWI;
	else if (vch->bf_regs.bits_ida.BITS.ipa)
		MBF_image_type = IPA;
	else
		return -1; //Aquí lo suyo sería reportar algo, pero no debería llegar aquí

	vch->remote_addr_bf_cur = vch->remote_addr_bf_ini;
	// Configiracion HW
	// ---------------------------------------------------------------------------





	if(pruebas_T0)
	{
		vch->bf_regs.T0_BF.T0[0] = 550; // esto es para hacer pruebas
		vch->afe.AFE_BUSSAR_UT.num_samples = vch->bf_regs.n_foci + vch->bf_regs.T0_BF.T0[0]+30;
	}

	// Se programa en HW sólo una vez

	if(DEBUG_BEAMFORMER)
	{
		beamformer_ut_struct_t bf_ut_struct_print;
		xil_printf("\n\r--------------antes pulser focal laws----------------\n\r");
		beamformer_get_regs(&bf_ut_struct_print,1);
		beamformer_print_regs(&bf_ut_struct_print);
	}

	if ((result = VCH_PRG_EmissionFocalLaws(vch)) < 0) return RLOG(result);

	if(DEBUG_BEAMFORMER)
	{
		beamformer_ut_struct_t bf_ut_struct_print;
		xil_printf("\n\r--------------antes forward focal laws----------------\n\r");
		beamformer_get_regs(&bf_ut_struct_print,1);
		beamformer_print_regs(&bf_ut_struct_print);
	}
/*
	TFM_DEBUG_void_forward_focal_law(0,vch);
	TFM_DEBUG_void_forward_focal_law(1,vch);
	TFM_DEBUG_void_forward_focal_law(2,vch);
	TFM_DEBUG_void_forward_focal_law(3,vch);
*/
	if ((result = VCH_PRG_ForwardFocalLaws(vch)) < 0) return RLOG(result);

	if(DEBUG_BEAMFORMER)
	{
		beamformer_ut_struct_t bf_ut_struct_print;
		xil_printf("\n\r--------------antes backward focal laws----------------\n\r");
		beamformer_get_regs(&bf_ut_struct_print,1);
		beamformer_print_regs(&bf_ut_struct_print);
	}

	if ((result = VCH_PRG_BeamformerFocalLaws(vch)) < 0) return RLOG(result);

//	if(DEBUG_BEAMFORMER)
//	{
//		beamformer_ut_struct_t bf_ut_struct_print;
//		xil_printf("\n\r--------------antes forward focal laws----------------\n\r");
//		beamformer_get_regs(&bf_ut_struct_print,1);
//		beamformer_print_regs(&bf_ut_struct_print);
//	}
//
//	if ((result = VCH_PRG_ForwardFocalLaws(vch)) < 0) return RLOG(result);

	if(DEBUG_BEAMFORMER)
	{
		beamformer_ut_struct_t bf_ut_struct_print;
		xil_printf("\n\r--------------antes registers----------------\n\r");
		beamformer_get_regs(&bf_ut_struct_print,1);
		beamformer_print_regs(&bf_ut_struct_print);
	}
	if ((result = VCH_PRG_Registers_TFM(vch)) < 0) return RLOG(result);


	if(DEBUG_BEAMFORMER)
	{
		beamformer_ut_struct_t bf_ut_struct_print;
		xil_printf("\n\r--------------despues registers----------------\n\r");
		beamformer_get_regs(&bf_ut_struct_print,1);
		beamformer_print_regs(&bf_ut_struct_print);
	}

	/*
	if(vch->processing_type==PROCESSING_TYPE_NONE) emi_prom_rpt = 1;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_B) emi_prom_rpt = vch->n_signals_processing_avr;
	else if(vch->processing_type==PROCESSING_TYPE_EMI_B)  emi_prom_rpt = vch->n_signals_processing_emi;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_A) emi_prom_rpt = 1<<vch->afe.AFE_BUSSAR_UT.log2_promediados;
	*/
	emi_prom_rpt = 1;



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
	TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);
	MCBCC_POAF(1);
	MCBCC_TRIG2PRO(1);
	UCI_Sync(0);
	UCI_set_Sync_is_PRO();

	fifo_control_clear_all(0);

	//TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);
	if (vch->n_efl == 0)
		return 0;

	if(gb_log_acquiring)
		VCH_PrintConfig(vch);

	if(gb_log_acquiring)
	{
		AFE_SPI_UT_t regs_afe;
		get_spi_afe_regs(&regs_afe, 2);
		print_spi_afe_regs(regs_afe);
	}

	// ---------------------------------------------------------------------------


	remote_image_size = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;
//	TFM_interrupt_startup(unsigned int images  ,unsigned int n_images_burst,unsigned int n_lines_image,unsigned int emi_prom     ,unsigned short acquisition_mode,u32 addr_ini           ,u32 addr_end           ,u32 acq_size                                              );
//	TFM_interrupt_startup(gb_uci.n_acquisitions,gb_uci.n_images_burst      ,vch->n_efl     ,(unsigned int)emi_prom_rpt,gb_uci.acquisition_mode        ,vch->remote_addr_bf_ini,vch->remote_addr_bf_end,remote_image_size);

	acquisition_addr=vch->remote_addr_ini;
	tfm_image_addr=vch->remote_addr_bf_ini;
	scratch_addr=vch->emi_prom_scratch_addr;




//	TFM_interrupt_startup(TVCH *vch,unsigned int images  ,unsigned int n_emission_focal_laws,unsigned int n_lines_image,unsigned int n_parallel_bf                    ,unsigned int acquisition_addr,unsigned int scratch_addr,unsigned int tfm_image_addr);
	TFM_interrupt_startup(vch, gb_uci.n_acquisitions,vch->n_efl, vch->pa_image.n_lines, vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1,acquisition_addr, scratch_addr, tfm_image_addr, MBF_image_type, gb_uci.pause_after_burst);



	//mem2gtx_set_speed(1,7,8,0);
	TIMER_set_periodic_count(vch->prf_time_line,TIMER_SCAN_PRF_1);

	vch->hardware_set_up = 1;

	return 0;
}


int TFM_transfer_fsm(TVCH *vch)
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
			TFM_send_image(vch);
			//xil_printf("ACQ->n_images_sent = %d\r",ACQ->n_images_sent);
			ACQ->read_addr_index=vch->remote_addr_bf_cur;
		}

		if( (ACQ->acquisition_mode == AQUISITION_MODE_FIXED && ACQ->n_images_sent>=ACQ->n_images_total) ||
			(ACQ->acquisition_mode == AQUISITION_MODE_CONTINOUS && ACQ->end_continous))
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


int TFM_transfer(TVCH *vch)
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
			TFM_send_image(vch);
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


int TFM_write_header(TVCH *vch)
{
int result,i;

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

}

int TFM_send_image(TVCH *vch)
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

   TFM_write_header(vch);

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
		if(1)
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


		   if (lista_lineas_por_bloque[i]!=0)//TEORICAMENTE SOLO EN EL BLOQUE FINAL! (si es incompleto)
		   {
			   n_data_bloque_extraido = RoundSup(((float)lista_lineas_por_bloque[i] * vch->pa_image.n_beamformed_samples)/2.0);


			   channel_extract_configure_basic_no_test(lista_lineas_por_bloque[i],(vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1),vch->pa_image.n_beamformed_samples);

			   DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_extraido,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);

			   if ((result = DMA_Write32_from_BCC_wait_end(TFM_LINE_RECV_TIMEOUT)) < 0)
				 xil_printf("ERROR, TIMEOUT ESPERANDO DMA_asymmetrical_memcpy_remote_base_int_no_block(n_data_DMA,n_data_remote,minibase,remote_addr)\n\r");

			   usleep_A9(3);


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

   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);//desactivamos
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);//desactivamos
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);
   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);//activamos el camino normal

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

