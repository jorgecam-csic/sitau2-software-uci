// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

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

#define DATOS_TEST	0
#ifdef DATOS_TEST
u32 datos_test[2<<21];
#endif



// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================



int UCI_Acquire_and_beamform(TVCH *vch)
{
//	ACQ_FMC_set_up(vch);
//	ACQ_FMC_start_trigger();
//	return 0;
int result,i,idx_avr;
u32 l;
int prf_acq_number = 1;
u64 timstamp_ini,timstamp_end;
float us_acq;
u16 flags;
u16 emi_prom_rpt;
u32 activacion_salida=0;
int interrupt_mode=0;
volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

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
	if ((result = VCH_PRG_BeamformerFocalLaws(vch)) < 0) return RLOG(result);
    if ((result = VCH_PRG_Registers_PA_DMA(vch)) < 0) return RLOG(result);

    //n_averages = 1<<vch->base[0].afe.AFE_BUSSAR_UT.log2_promediados;
    if(0)//(n_averages>1)
    {
    	prog_ini_promediado_abs(&vch->pulser, vch->base[0].bcc_addr, &Global_commnd_buf);
		if (gb_hw_sitau_enabled == 1)
		{
			if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
				ELOG("ERROR_BCC_SEND\n\r",-1);
		}
		Global_commnd_buf.total_len = 0;
    }
	if(vch->processing_type==PROCESSING_TYPE_NONE) emi_prom_rpt = 1;
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

	TRIG_set_interrupt_source(TRIGGER_PRFTIMER1_MASK);
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

    TIMER_Start(TIMER_SCAN_PRF_1);
	if(interrupt_mode==0)
	{

		for (i=0; i<prf_acq_number;i++)
		{
			for (l=0; l < vch->n_efl; l++)
			{
				for (idx_avr=0; idx_avr<emi_prom_rpt; idx_avr++)
				{
					//if(gb_log_acquiring == 1)
					//  LOG_ACQUIRING("\r\nACQUIRING FP_VCH %d - Num Prom %02d, Sub-Image %03d (%3d A-Scan) (%d Samples)... ",
					// vch->id,
					// idx_avr,
					// l,
					// vch->focal_law[l].n_ascan,
					// vch->focal_law[l].image_size * 2);

					if(gb_log_acquiring == 1)
					{
						u32 registro;
						BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
						LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
						BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
						LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
						LOG_ACQUIRING("\r\n");
					}
					//UCI_SW2HW_trig();
					if(i>=activacion_salida)
						UCI_Alarm2(1);

					if ((result = BCC_wait_PRO_rise(1000000.0)) < 0)
					{
						bcc_print_all_busy_flags(vch->n_bases);
						ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);
					}
					if ((result = BCC_wait_PRO_fall(1000000.0)) < 0)
					{
						bcc_print_all_busy_flags(vch->n_bases);
						ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);
					}

					if(gb_test_remote_fifo_control)
					{
						if(fifo_control_get_or_flag(vch->base[0].bcc_addr))
						{
							fifo_control_print_all(vch->base[0].bcc_addr);
							fifo_control_clear_all(vch->base[0].bcc_addr);
						}
					}

					if (gb_log_beamforming || gb_test_remotedatamover)
					{
						status_t status_datamov;
						status_datamov = get_afe2mem_datamover_status(vch->base[0].bcc_addr);
						if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
							LOG_ACQUIRING("STATUS: 0x%08X\n\r",status_datamov.status_32);
					}
				}
			}
		}
	}
	else
	{
		while(ACQ->acquisition_ended==0);
	}

      TIMER_Stop(TIMER_SCAN_PRF_1);
      TRIG_remove_interrupt_source(TRIGGER_PRFTIMER1_MASK);

      MCBCC_TRIG2PRO(0);
      MCBCC_set_PRO(0);
      UCI_Sync(0);

      timstamp_end=timestamp_get_ticks_64();

      UCI_Alarm1(0);
      UCI_Alarm2(0);

      us_acq=timestamp_get_us_float(timstamp_ini, timstamp_end);
      if(gb_log_acquiring)
      {
		  LOG_ACQUIRING("\n\r");
		  LOG_ACQUIRING("Fin de adquisición en los módulos, tiempo total: %d us\n\r",(int)us_acq);
		  LOG_ACQUIRING("Número de disparos %d, tiempo entre disparos: %d us\n\r",prf_acq_number * vch->n_efl,(int)us_acq/(prf_acq_number * vch->n_efl));
      }

      //MCBCC_print_regs();
 	  //Reset direcciones remotas

      //if (vch->processing_type!=PROCESSING_TYPE_NONE) return EUCI_NONE;
      MCBCC_TRIG2PRO(0);
      MCBCC_set_PRO(0);

  	TRIG_remove_interrupt_source(TRIGGER_SW2HW_MASK);
  	vch->remote_addr_bf_cur = vch->remote_addr_bf_ini;

  	for (int image=0; image<gb_uci.n_acquisitions;image++)
  	{

		if (gb_log_beamforming)
		  LOG_BEAMFORMING("\r\nSENDING DATA, IMAGE %d... ",image);

		if ((result = UCI_Acquire_beamformed(vch)) < 0) return RLOG(result);
  	}


   return EUCI_NONE;
}


// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_Acquire_FocalLaw(TVCH *vch)
{
//	ACQ_FMC_set_up(vch);
//	ACQ_FMC_start_trigger();
//	return 0;
int result,i,idx_avr;
u32 l;
int prf_acq_number = 1;
u64 timstamp_ini,timstamp_end;
float us_acq;
u16 flags;
u16 emi_prom_rpt;
u32 activacion_salida=0;
int interrupt_mode=1;
volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

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

    //n_averages = 1<<vch->base[0].afe.AFE_BUSSAR_UT.log2_promediados;
    if(0)//(n_averages>1)
    {
    	prog_ini_promediado_abs(&vch->pulser, vch->base[0].bcc_addr, &Global_commnd_buf);
		if (gb_hw_sitau_enabled == 1)
		{
			if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
				ELOG("ERROR_BCC_SEND\n\r",-1);
		}
		Global_commnd_buf.total_len = 0;
    }
	if(vch->processing_type==PROCESSING_TYPE_NONE) emi_prom_rpt = 1;
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

	TRIG_set_interrupt_source(TRIGGER_PRFTIMER1_MASK);
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

    TIMER_Start(TIMER_SCAN_PRF_1);
	if(interrupt_mode==0)
	{

		for (i=0; i<prf_acq_number;i++)
		{
			for (l=0; l < vch->n_efl; l++)
			{
				for (idx_avr=0; idx_avr<emi_prom_rpt; idx_avr++)
				{
					//if(gb_log_acquiring == 1)
					//  LOG_ACQUIRING("\r\nACQUIRING FP_VCH %d - Num Prom %02d, Sub-Image %03d (%3d A-Scan) (%d Samples)... ",
					// vch->id,
					// idx_avr,
					// l,
					// vch->focal_law[l].n_ascan,
					// vch->focal_law[l].image_size * 2);

					if(gb_log_acquiring == 1)
					{
						u32 registro;
						BCC_read_reg(1,3,DELAY_MEM_PROG_R,&registro);
						LOG_ACQUIRING("\r\nDELAY_MEM_PROG_R: %08X",registro);
						BCC_read_reg(1,3,DELAY_MEM_ADDR_R,&registro);
						LOG_ACQUIRING("\r\nDELAY_MEM_ADDR_R: %08X",registro);
						LOG_ACQUIRING("\r\n");
					}
					//UCI_SW2HW_trig();
					if(i>=activacion_salida)
						UCI_Alarm2(1);

					if (gb_hw_sitau_enabled == 1)
					{
						if ((result = BCC_wait_flag(PORF_MASK, 1000000.0)) < 0)
							ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);

						if ((result = BCC_wait_flag(POFF_MASK,1000000.0)) < 0)
							ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);
					}
					if((gb_log_beamforming || gb_test_remote_fifo_control)  && fifo_control_get_or_flag(vch->base[0].bcc_addr))
					{
						fifo_control_print_all(vch->base[0].bcc_addr);
						fifo_control_clear_all(vch->base[0].bcc_addr);
					}

					if (gb_log_beamforming || gb_test_remotedatamover)
					{
						status_t status_datamov;
						status_datamov = get_afe2mem_datamover_status(vch->base[0].bcc_addr);
						if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
							LOG_ACQUIRING("STATUS: 0x%08X\n\r",status_datamov.status_32);
					}
				}
			}
		}
	}
	else
	{
		while(ACQ->acquisition_ended==0);
	}

      TIMER_Stop(TIMER_SCAN_PRF_1);
      TRIG_remove_interrupt_source(TRIGGER_PRFTIMER1_MASK);

      MCBCC_TRIG2PRO(0);
      MCBCC_set_PRO(0);
      UCI_Sync(0);

      timstamp_end=timestamp_get_ticks_64();

      UCI_Alarm1(0);
      UCI_Alarm2(0);

      us_acq=timestamp_get_us_float(timstamp_ini, timstamp_end);
      if(gb_log_acquiring)
      {
		  LOG_ACQUIRING("\n\r");
		  LOG_ACQUIRING("Fin de adquisición en los módulos, tiempo total: %d us\n\r",(int)us_acq);
		  LOG_ACQUIRING("Número de disparos %d, tiempo entre disparos: %d us\n\r",prf_acq_number * vch->n_efl,(int)us_acq/(prf_acq_number * vch->n_efl));
      }

      //MCBCC_print_regs();
 	  //Reset direcciones remotas

      //if (vch->processing_type!=PROCESSING_TYPE_NONE) return EUCI_NONE;

      for (i=0; i<prf_acq_number; i++)
      {
         for (l=0; l<vch->n_efl; l++)
         {
//            if (gb_log_acquiring == 1)
        	 //               LOG_ACQUIRING("\r\nSENDING DATA FP_VCH %d - Sub-Image %03d (%3d A-Scan) (%d Samples)... ",
        	 //   vch->id,
        	 //   l,
        	 //   vch->focal_law[l].n_ascan,
        	 //   vch->focal_law[l].image_size * 2);

        	 if ((result = UCI_Acquire(vch, l)) < 0) return RLOG(result);
         }
      }

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_Beamforming(TVCH *vch)
{
int result, image;
u16 flags;
int prf_acq_number = 1;
int ley_focal_emi;
beamformer_ut_struct_t bf_ut_struct;
u16 emi_prom_idx=0,emi_prom_rpt=0;
//u64 timstamp_ini;

//gb_log_beamforming=1;
//gb_test_remotedatamover=1;
	// Configuracion HW
	if ((result = VCH_PRG_BeamformerFocalLaws(vch)) < 0) return RLOG(result);

	if ((result = VCH_PRG_BeamformerRegisters(vch)) < 0) return RLOG(result);

	if(gb_log_beamforming == 1) VCH_PrintConfig(vch);
	// ---------------------------------------------------------------------------
	flags=MCBCC_get_flags(PORF_MASK|POFF_MASK);

	TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);
	MCBCC_POAF(1);
	MCBCC_TRIG2PRO(1);
	UCI_set_Sync_is_PRO();

	if(gb_log_beamforming)
	{
		LOG_BEAMFORMING("\n\rIniciando la conformación...\n\r");
		MCBCC_print_regs();

		flags=MCBCC_get_flags(0);
		LOG_BEAMFORMING("FLAGS 0x%04X\n\r",flags);
	}
	else
		flags=MCBCC_get_flags(0);


	if(vch->processing_type==PROCESSING_TYPE_NONE)
		emi_prom_rpt = 1;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_B)
		emi_prom_rpt = vch->n_signals_processing_avr;
	else if(vch->processing_type==PROCESSING_TYPE_EMI_B)
		emi_prom_rpt = vch->n_signals_processing_emi;
	else if(vch->processing_type==PROCESSING_TYPE_PROM_A)
		emi_prom_rpt = 1;
	//timstamp_ini=timestamp_get_ticks_64();


	UCI_Alarm1(1);

	if(gb_log_beamforming)
	{
		LOG_BEAMFORMING("\n\rRegistros actuales BEAMFORMER:\n\r");
		beamformer_get_regs(&bf_ut_struct,vch->base[0].bcc_addr);
		beamformer_print_regs(&bf_ut_struct);
		LOG_BEAMFORMING("\n\rRegistros actuales DATAMOVER REMOTO:\n\r");

		beamformer_get_regs(&bf_ut_struct,vch->base[0].bcc_addr);
		beamformer_print_regs(&bf_ut_struct);
	}
	if (vch->n_efl > 0)
	{
		prf_acq_number = gb_uci.n_acquisitions;

		if (prf_acq_number < 1) prf_acq_number = 1;

		for (image=0; image<gb_uci.n_acquisitions;image++)
		{
			for (ley_focal_emi=0; ley_focal_emi<vch->n_efl; ley_focal_emi++)
			{
				for(emi_prom_idx=0;emi_prom_idx<emi_prom_rpt;emi_prom_idx++)
				{
					if(gb_log_beamforming)
						LOG_BEAMFORMING("\n\rBEAMFORMING FP_VCH %d, Image %03d (%3d images) (%3d lines) (%3d emi_prom_rpt) (%d Samples)... ",
						 vch->id,
						 image,
						 gb_uci.n_acquisitions,
						 vch->bf_regs.image_lines.BITS.image_lines+1,
						 emi_prom_rpt,
						 vch->pa_image.n_beamformed_samples);

					if (gb_hw_sitau_enabled)
					{
						if(gb_log_beamforming) LOG_BEAMFORMING("\n\rCiclo de conformación: Subiendo PRO!\n\r");
						/////////TRIGGER!!!!////////
						UCI_SW2HW_trig();
						////////////////////////////
						/*
						if (result = BCC_wait_flag(PORF_MASK,1000000.0))
							ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);
						if (result = BCC_wait_flag(POFF_MASK,1000000.0))
							ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);
						*/
						if ((result = BCC_wait_PRO_rise(1000000.0)) < 0)
						{
							bcc_print_all_busy_flags(vch->n_bases);
							ELOG(charEUCI_BCC_PRO_RISE,EUCI_BCC_PRO_RISE);
						}
						if ((result = BCC_wait_PRO_fall(1000000.0)) < 0)
						{
							bcc_print_all_busy_flags(vch->n_bases);
							ELOG(charEUCI_BCC_PRO_FALL,EUCI_BCC_PRO_FALL);
						}
						if(gb_log_beamforming || gb_test_remote_fifo_control)
						{
							if(fifo_control_get_or_flag(vch->base[0].bcc_addr))
							{
								fifo_control_print_all(vch->base[0].bcc_addr);
								fifo_control_clear_all(vch->base[0].bcc_addr);
							}
						}

						if(gb_log_beamforming || gb_test_remotedatamover)
						{
							status_t status_datamov;
							if (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 0 || (vch->bf_regs.bf_gen_ctrl.BITS.serial_bf_start==1 && vch->bf_regs.bf_gen_ctrl.BITS.serial_bf_start==0))
							{
								status_datamov = get_beamformer2mem_datamover_status(vch->base[0].bcc_addr);
								if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
								  LOG_BEAMFORMING("\n\rSTATUS BF0_2_MEM: 0x%08X\n\r",status_datamov.status_32);
							}
//							if (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 0 || (vch->bf_regs.bf_gen_ctrl.BITS.serial_bf_start==1 && vch->bf_regs.bf_gen_ctrl.BITS.serial_bf_start==1))
//							{
//								status_datamov = get_col1beamformer2mem_datamover_status(vch->base[0].bcc_addr);
//								if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
//								  LOG_BEAMFORMING("\n\rSTATUS BF1_2_MEM: 0x%08X\n\r",status_datamov.status_32);
//							}

							status_datamov = get_mem2beamformer_datamover_status(vch->base[0].bcc_addr);
							if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
							  LOG_BEAMFORMING("\n\rSTATUS DATAMOVER 256 MEM 2 BF: 0x%08X\n\r",status_datamov.status_32);
							status_datamov = get_mem2emi_prom_beamformer_datamover_status(vch->base[0].bcc_addr);
							//if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
							  //LOG_BEAMFORMING("\n\rSTATUS DATAMOVER 256 MEM 2 BF (PROM): 0x%08X\n\r",status_datamov.status_32);
							status_datamov = get_beamformer2mem_datamover_status(vch->base[0].bcc_addr);
							if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
							  LOG_BEAMFORMING("\n\rSTATUS DATAMOVER 32  MEM 2 BF: 0x%08X\n\r",status_datamov.status_32);
							if(gb_log_beamforming)
							{
								LOG_BEAMFORMING("\n\rRegistros actuales BEAMFORMER:\n\r");
								beamformer_get_regs(&bf_ut_struct,vch->base[0].bcc_addr);
								beamformer_print_regs(&bf_ut_struct);
							}
						}
					}
				}
			}
		}
	}

    MCBCC_TRIG2PRO(0);
    MCBCC_set_PRO(0);

	TRIG_remove_interrupt_source(TRIGGER_SW2HW_MASK);
	vch->remote_addr_bf_cur = vch->remote_addr_bf_ini;
	for (image=0; image<gb_uci.n_acquisitions;image++)
	{

          if (gb_log_beamforming)
        	  LOG_BEAMFORMING("\r\nSENDING DATA, IMAGE %d... ",image);

          if ((result = UCI_Acquire_beamformed(vch)) < 0) return RLOG(result);
    }
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------

int UCI_Acquire_beamformed(TVCH *vch)
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

   // HEADER
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xCEB0... ");
   if ((result = UCI_MemoryWrite_uint32(0x0001CEB0)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // ID DEL CANAL VIRTUAL
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nId. Virtual Channel(%d)... ", vch->id);
   if ((result = UCI_MemoryWrite_uint32(vch->id)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // NUMERO TOTAL DE DATOS DE LA TRAMA
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Data(%d)... ", n_words_32_image);
   if ((result = UCI_MemoryWrite_uint32(n_words_32_image)) < 0) return RLOG(result);
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

	   minibase = vch->base[2].bcc_addr;

	   remote_addr = vch->remote_addr_bf_cur;

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
#if HILBERT
		   config_HIL_coeficients(hilbert_coef);

#endif
		   config_interleaving(calc_interleaving(vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1),HILBERT);
	   }
	   for(i=0;i<vch->pa_image.n_blocks;i++)
	   {


		   if (gb_log_acquiring_frame == 1)
		   {
			   LOG_ACQUIRING_FRAME("Adquiriendo bloque %d:\n\r",i);
			   LOG_ACQUIRING_FRAME("Direccion  minibase: %d\n\r",minibase);
			   LOG_ACQUIRING_FRAME("Lineas del bloque (lineas bloque completo): %d (%d)\n\r",lista_lineas_por_bloque[i],lineas_bloque_completo);
			   LOG_ACQUIRING_FRAME("Direccion remota: 0x%08X\n\r",remote_addr);
			   LOG_ACQUIRING_FRAME("Conteo de lineas (lineas totales): %d (%d)\n\r",conteo_lineas,vch->pa_image.n_lines);
		   }

		   if(lista_lineas_por_bloque[i]==lineas_bloque_completo)
		   {

			   if(paso_por_filtro)
			   {

#if HILBERT
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_ENVOLV_IDX,0);
#else
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,1);
#endif
				   DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_completo,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);

				   if ((result = DMA_Write32_from_BCC_wait_end(1000000.0)) < 0)
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
				   if ((result = DMA_Write32_from_BCC_wait_end(1000000.0)) < 0)
					 xil_printf("ERROR, TIMEOUT ESPERANDO DMA_memcpy_remote_base_int_no_block(n_data,minibase,addr)\n\r");
			   }

		   }
		   else if (lista_lineas_por_bloque[i]!=0)//TEORICAMENTE SOLO EN EL BLOQUE FINAL! (si es incompleto)
		   {
			   n_data_bloque_extraido = RoundSup(((float)lista_lineas_por_bloque[i] * vch->pa_image.n_beamformed_samples)/2.0);

			   if(paso_por_filtro)
			   {
#if HILBERT
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_ENVOLV_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_ENVOLV_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);
#else
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_FILTER_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_FILTER_IDX,0);
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_CHASEL_IDX,0);
#endif
				   channel_extract_configure_basic(lista_lineas_por_bloque[i],(vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1),vch->pa_image.n_beamformed_samples);

				   DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(n_data_bloque_extraido,n_data_bloque_completo,remote_addr,LAST_DONT_TOUCH);

				   if ((result = DMA_Write32_from_BCC_wait_end(1000000.0)) < 0)
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

				   if ((result = DMA_Write32_from_BCC_wait_end(1000000.0)) < 0)
					 xil_printf("ERROR, TIMEOUT ESPERANDO DMA_asymmetrical_memcpy_remote_base_int_no_block(n_data_DMA,n_data_remote,minibase,remote_addr)\n\r");

				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_CHASEL_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,1);//desactivamos
				   switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);//activamos el camino normal
			   }

		   }
		   else //lista_lineas_por_bloque[i]==0
			   continue;

		   remote_addr += n_data_bloque_completo*4;

		   conteo_lineas+=lista_lineas_por_bloque[i];
	   }

	   if (conteo_lineas!=vch->pa_image.n_lines)
		 xil_printf("ERROR, el número de líneas escritas en la trama no coincide con el número de lineas de la imagen.\n\r");
   }
   vch->remote_addr_bf_cur = remote_addr;

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

// -----------------------------------------------------------------------------
int UCI_Acquire(TVCH *vch, u32 idx_fl)
{
int result, ch, n_ascan;
u32 n_words_32_image, n_buffer_data;
u32 addr, n_data;
u8 minibase;
u8 mem2gtx=0,mem2uci=1,header_gtx=0;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	if (idx_fl >= MOD_PUL_MAX_FOCAL_LAW) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	n_ascan = MOD_MAX_CH * vch->n_bases;
	//first_base	= &vch->base[0];

// ____________________________________________________________________________
// CABECERA DE LA TRAMA
// ============================================================================

   n_words_32_image = vch->size32_fp_frame;
   if (gb_log_acquiring_frame == 1)
      LOG_ACQUIRING_FRAME("\r\nDMA_Set_Datamover_Write16((%d + %d) * %d + %d + 1 = %d)... ",
         vch->size_header_line_fp,
         vch->size32_fp_image,
		 (int)MOD_MAX_CH,
         vch->size_header_img_fp,
         n_words_32_image);

   //if ((result = DMA_Set_Datamover_Write32(n_words_32_image, &n_buffer_data)) < 0) return RLOG(result);
   if ((result = DMA_Set_Datamover_Write32(n_words_32_image, &n_buffer_data)) < 0)
   {
	   int tries=1,max_tries=100;

	   gb_log_acquiring_frame=1;
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

   // ESCRIBE LA PALABRA DE INICIO DE LA IMAGEN
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\n0xCEB0... ");
   if ((result = UCI_MemoryWrite_uint32(0x0000CEB0)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // ESCRIBE EL ID DEL CANAL VIRTUAL
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nId. Virtual Channel(%d)... ", vch->id);
   if ((result = UCI_MemoryWrite_uint32(vch->id)) < 0) return RLOG(result);
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("OK");

   // ESCRIBE EL NUMERO TOTAL DE DATOS DE LA TRAMA
   if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("\r\nN. Data(%d)... ", n_words_32_image);
   if ((result = UCI_MemoryWrite_uint32(n_words_32_image)) < 0) return RLOG(result);
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

// _____________________________________________________________________________
// ------ INICIA PROCESAMIENTO
// =============================================================================

   //if (gb_log_acquiring_frame == 1)  LOG_ACQUIRING_FRAME("\r\n   AMPLIA_Process()... ");

   //if (gb_hw_sitau_enabled == 1)



      //if ((result = AMPLIA_Process()) < 0) return RLOG(result);
      //if ((result = AMPLIA_wait_end_Process(1000000.0)) < 0) return RLOG(result);

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
	addr = vch->remote_addr_cur;
	n_data = vch->afe.AFE_BUSSAR_UT.num_samples * 32 / 2;

   if(DATOS_TEST)
   {
	   for(u8 idx_minibase=0;idx_minibase<vch->n_bases;idx_minibase++)
	   {
		   u8 minibase_test=vch->base[idx_minibase].bcc_addr;
		   for(int i = 0; i < n_data; i++)
			   datos_test[i] = 0xCAFE0000 + i;

		   bcc_copy_uci2mem(datos_test,n_data,addr,minibase_test,&Global_commnd_buf);
	   }
   }
   if(mem2uci)
	{

		//for(int idx_minibase=vch->n_bases-1;idx_minibase>=0;idx_minibase--)
	   for(u8 idx_minibase=0;idx_minibase<vch->n_bases;idx_minibase++)
		{
			minibase =  vch->base[idx_minibase].bcc_addr;

			DMA_memcpy_remote_base_int_no_block(n_data,minibase,addr);

			if ((result = DMA_Write32_from_BCC_wait_end(1000000.0)) < 0)
				xil_printf("ERROR, TIMEOUT ESPERANDO DMA_memcpy_remote_base_int_no_block(n_data,minibase,addr)\n\r");

			{
				u32* sswitch;

				sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

				switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BCC_IDX, SSWITCH032_SLAVE_FROM_DDR_IDX,0);

				switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

				MCBCC_deactivate_emu();
			}
		}
	}
   if(mem2gtx)
   {
		u8 minibase_origen;
		int lecturas_estado_datamover=0;

		if(header_gtx)
		{
			u32 datos32[8]={0xC51CDA5E,0x01234567,0x22222222,0x33333333,0x44444444,0x55555555,0x66666666,0xDA5EC51C};
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[ZYNQGTX_SWITCH_TABLE_IDX],SSWITCHGTX_UCI_MASTER_TO_SUTXDN_IDX, SSWITCHGTX_SLAVE_FROM_MEM_IDX,0);
			swgtx_remote_down_pass_throw_to_host(0);
			set_axi4_2_gtx(0);
			axi2gtx_fifo_send(datos32, 8);

		}

		sw256_remote_mem_to_512_switch(0,1);// Ponemos todos los switches de 128 hacia el conformador DESACTIVADOS

		for(u8 idx_minibase_org=0;idx_minibase_org<vch->n_bases;idx_minibase_org++)
		{
			u32 btt;
			status_t datamover_status;

			minibase_origen =  vch->base[idx_minibase_org].bcc_addr;
			gtx_remote_from_minibase_mem_down_to_host(minibase_origen,vch->n_bases);

			datamover_status=get_mem2beamformer_datamover_status(minibase_origen);
			btt = n_data*4;

			config_mem2beamformer(addr,btt,0,0,1,minibase_origen,&Global_commnd_buf);
			BCC_SendBuffer(&Global_commnd_buf);
			Global_commnd_buf.total_len=0;

			lecturas_estado_datamover=0;
			datamover_status.status_32=0;
			do{
				datamover_status=get_mem2beamformer_datamover_status(minibase_origen);
				if(lecturas_estado_datamover>1000)
				{
					xil_printf("ERROR MINIBASE %d: %d lecturas_estado_datamover sin respuesta\n\r",minibase_origen,lecturas_estado_datamover);
					break;
				}
				lecturas_estado_datamover++;
			}while(datamover_status.BIT.transfer_flag==0);

			if(datamover_status.BIT.status_response.BIT.okey!=1)
				xil_printf("ERROR MINIBASE %d: datamover devuelve %08X\n\r",minibase_origen,datamover_status.status_32);

			sw256_remote_mem_to_gtx_switch(minibase_origen,1); //Desactivamos el SW de 128 hacia el SW GTX en el módulo que acaba de ser origen de datos

		}
		sw256_remote_mem_to_512_switch(0,0);// Ponemos todos los switches de 128 hacia el conformador ACTIVADOS
   }

   vch->remote_addr_cur=addr+n_data*4;

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

