/*
 * fppa_uci_acq.c
 *
 *  Created on: 14 nov. 2019
 *      Author: csic
 */


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
#include "timestamp.h"
// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================
#if 0
int BF_UCI_Beamform_FP_PA(TFP_VirtualChannel *vch)
{
int result,i,idx_avr, n_averages;
u32 l;
u8 Test_remoteDatamover_end=0;
int prf_acq_number = 1;
u64 timstamp_ini,timstamp_end;
int lineas_idx,n_lineas_imagen,n_lineas_paralelo;
int adquisicion_idx,n_imagenes;
float us_acq;
int delay;
u16 flags;

	int lineas_idx,n_lineas_imagen,n_lineas_paralelo;
	int imagen_idx,n_imagenes;
	//vch->base[0].afe.AFE_BUSSAR_UT.num_samples;
	//vch->base[0].afe.AFE_BUSSAR_UT.num_samples

	//TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);
	TRIG_reset_all_interrupt_sources();
	TIMER_Stop(TIMER_SCAN_PRF_1);
	MCBCC_TRIG2PRO(0);
	MCBCC_set_PRO(0);
	while(MCBCC_get_PROI());

	TRIG_reset_all_interrupt_sources();

   // Configiracion HW
   // ---------------------------------------------------------------------------
   if ((result = BF_VCH_BASE_PRGFocalLaws(&vch->base[0])) < 0) return RLOG(result);
   if ((result = BF_VCH_BASE_PRGRegisters(&vch->base[0])) < 0) return RLOG(result);

   n_images = 1<<vch->base[0].afe.AFE_BUSSAR_UT.log2_promediados;
   if(0)//(n_averages>1)
   {
   	prog_ini_promediado_abs(&vch->base[0].pulser,vch->base[0].bcc_addr,&Global_commnd_buf);
		if (gb_hw_sitau_enabled == 1)
	   {
	      if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
	         xil_printf("ERROR_BCC_SEND\n\r");

	   }
		Global_commnd_buf.total_len = 0;
   }

   vch->base[0].remote_addr_ini = FP_REMOTE_START_ADDR;
   vch->base[0].remote_addr_cur = FP_REMOTE_START_ADDR;
   vch->base[0].remote_addr_end = FP_REMOTE_LAST_ADDR;

   FP_VCH_BASE_PrintRegisters(&vch->base[0]);
   // ---------------------------------------------------------------------------
    flags=MCBCC_get_flags(PORF_MASK|POFF_MASK);
    xil_printf("FLAGS 0x%04X\n\r",flags);

	TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);
	MCBCC_POAF(1);
	MCBCC_TRIG2PRO(1);
	UCI_set_Sync_is_PRO();

    xil_printf("\n\rantes de disparar a la red:\n\r");
    MCBCC_print_regs();

    flags=MCBCC_get_flags(0);
    xil_printf("FLAGS 0x%04X\n\r",flags);

    timstamp_ini=timestamp_get_ticks_64();

   UCI_Alarm1(1);

	//TRIG_set_interrupt_source(TRIGGER_SW2HW_MASK);

	if (vch->n_fl > 0)
	{

	  for (adquisicon_idx=0; adquisicon_idx<n_imagenes;adquisicon_idx++)
	  {
		for (linea_idx=0; linea_idx<n_lineas_imagen;linea_idx+=lineas_paralelo)
		{
			if(gb_log_acquiring == 1)
			  LOG_ACQUIRING("\r\nACQUIRING FP_VCH %d - Num Prom %02d, Sub-Image %03d (%3d A-Scan) (%d Samples)... ",
				 vch->id,
				 idx_avr,
				 l,
				 vch->focal_law[l].n_ascan,
				 vch->focal_law[l].image_size * 2);

			UCI_SW2HW_trig();

			if (gb_hw_sitau_enabled == 1)
			{
			  if (result = BCC_wait_flag(PORF_MASK,1000000.0))
				 xil_printf("ERROR, TIMEOUT DE DISPARO!\n\r"); //TERMINADO!!!

			  if (result = BCC_wait_flag(POFF_MASK,1000000.0))
				 xil_printf("ERROR, TIMEOUT DE PROCESAMIENTO!\n\r"); //TERMINADO!!!
			}

			if (Test_remoteDatamover_end)
			{
				status_t status_datamov;
				status_datamov = get_afe2mem_datamover_status(vch->base[0].bcc_addr);
				if (status_datamov.BIT.status_response.BIT.okey == 0 || status_datamov.BIT.transfer_flag == 0)
				  xil_printf("STATUS: 0x%08X\n\r",status_datamov.status_32);
			}
		}
	  }

      MCBCC_TRIG2PRO(0);
      MCBCC_set_PRO(0);

      timstamp_end=timestamp_get_ticks_64();

      UCI_Alarm1(0);

      us_acq=timestamp_get_us_64(timstamp_ini, timstamp_end);
      xil_printf("\n\r");
      xil_printf("Fin de adquisición en los módulos, tiempo total: %d us\n\r",(int)us_acq);
      xil_printf("Número de disparos %d, tiempo entre disparos: %d us\n\r",prf_acq_number*vch->n_fl,(int)us_acq/(prf_acq_number*vch->n_fl));
      xil_printf("\n\rantes de mandar a la red:\n\r");

      //MCBCC_print_regs();
 	  //Reset direcciones remotas
      vch->base[0].remote_addr_cur = vch->base[0].remote_addr_ini;

      for (i=0; i<prf_acq_number; i++)
      {
         for (l=0; l<vch->n_fl; l++)
         {
            if (gb_log_acquiring == 1)
               LOG_ACQUIRING("\r\nSENDING DATA FP_VCH %d - Sub-Image %03d (%3d A-Scan) (%d Samples)... ",
                  vch->id,
                  l,
                  vch->focal_law[l].n_ascan,
                  vch->focal_law[l].image_size * 2);

            if ((result = FP_UCI_Acquire(vch, l)) < 0) return RLOG(result);
         }
      }
   }
   return EUCI_NONE;
}
#endif
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
