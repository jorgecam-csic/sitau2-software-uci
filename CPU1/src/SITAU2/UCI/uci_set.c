// -----------------------------------------------------------------------------
/**
@file uci_set.c

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

#include "uci_set.h"
#include "uci_reg.h"
#include "uci_dma.h"
#include "uci_error_code.h"
#include "log.h"
#include "protocol.h"
#include "trigger.h"
#include "timer_util.h"
#include "datamover_driver.h"
#include "vch.h"
#include "TLV5626.h"

static u32 GPIO_value=0;


// -----------------------------------------------------------------------------
/**
Output Sync 1
*/
// -----------------------------------------------------------------------------
int UCI_Alarm1(int data)
{
	u32 *base_gpio = (u32 *)ALARM1_GPIO_BASEADDRESS;

	if(data)
		GPIO_value|=ALARM1_MASK;
	else
		GPIO_value&=~((u32)ALARM1_MASK);

	base_gpio[ALARM1_DATAVAL_REG]=GPIO_value;

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Led ACQ
*/
// -----------------------------------------------------------------------------
int UCI_Alarm2(int data)
{
	u32 *base_gpio = (u32 *)ALARM2_GPIO_BASEADDRESS;

	if(data)
		GPIO_value|=ALARM2_MASK;
	else
		GPIO_value&=~((u32)ALARM2_MASK);

	base_gpio[ALARM2_DATAVAL_REG]=GPIO_value;

	return EUCI_NONE;
}

int UCI_SW2HW_trig(void)
{
	u32 *base_gpio = (u32 *)SW2HWTRG_GPIO_BASEADDRESS;

	GPIO_value|=SW2HWTRG_MASK;
	base_gpio[SW2HWTRG_DATAVAL_REG]=GPIO_value;

	GPIO_value&=~((u32)SW2HWTRG_MASK);
	base_gpio[SW2HWTRG_DATAVAL_REG]=GPIO_value;

	return EUCI_NONE;
}
// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
/**
Pone la salida de sincronismo al valor indicado por el parï¿½metro 'data'.

@param[in] data   0 -> La salida de sincronismo se pone a nivel bajo.
                  1 -> La salida de sincronismo se pone a nivel alto.
*/
// -----------------------------------------------------------------------------
int UCI_Sync (int data)
{
	u32 *base_gpio = (u32 *)SYNC_GPIO_BASEADDRESS;

	if(data)
		GPIO_value|=SYNC_MASK;
	else
		GPIO_value&=~((u32)SYNC_MASK);

	base_gpio[SYNC_DATAVAL_REG]=GPIO_value;

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura la salida de sincronismo como la se�al PRO.

*/
// -----------------------------------------------------------------------------
int UCI_set_Sync_is_PRO (void)
{
	u32 *base_gpio = (u32 *)SYNC_MUX_SEL_GPIO_BASEADDRESS;

	GPIO_value |= SYNC_MUX_SEL_MASK;

	base_gpio[SYNC_MUX_SEL_DATAVAL_REG]=GPIO_value;

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura la salida de sincronismo como la se�al SYNQ gestionada por SW.

*/
// -----------------------------------------------------------------------------
int UCI_set_Sync_is_SW (void)
{

	u32 *base_gpio = (u32 *)SYNC_MUX_SEL_GPIO_BASEADDRESS;

	GPIO_value &= ~((u32)SYNC_MUX_SEL_MASK);

	base_gpio[SYNC_MUX_SEL_DATAVAL_REG]=GPIO_value;

	return EUCI_NONE;
}

// -----------------------------------------------------------------------------
/**
Resetea mï¿½dulo amplia, mï¿½dulo MCBCC, control de datamover, datamover y todos los fifos asociados.

*/
// -----------------------------------------------------------------------------
int UCI_datapath_clear(void)
{
	u32 *base_gpio = (u32 *)RST_DATAPATH_GPIO_BASEADDRESS;
	u32 i;


	GPIO_value|=RST_DATAPATH_MASK;
	base_gpio[RST_DATAPATH_DATAVAL_REG]=GPIO_value;
	for(i=0;i<100;i++); //pequeï¿½o delay, probablemente no muy necesario
	GPIO_value&=~((u32)RST_DATAPATH_MASK);
	base_gpio[RST_DATAPATH_DATAVAL_REG]=GPIO_value;
   DMA_init(&Datamover_instances[LOCAL_DATAMOVER_ID]);
   MCBCC_init();
   for(i=0;i<100000;i++); //delay, probablemente no muy necesario
   xil_printf("\r\nUCI_datapath_clear()");
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Inicializa el valor de un encoder a cero.

@param[in] id_encoder   Identificador del encoder.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_SetEncoderZero (unsigned char id_encoder)
{
   if (id_encoder >= UCI_N_ENCODER) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
   gb_encoder[id_encoder] = 0;
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Inicializa el valor de todos los encoders a cero.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_SetAllEncoderZero (void)
{
int i;

   for (i=0; i<UCI_N_ENCODER; i++) gb_encoder[i] = 0;
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Inicializa el valor del contador de adquisiciones.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_SetAcqCounterZero (void)
{
   gb_acq_counter = 0;
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Inicializa el valor del contador de adquisiciones.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_IncAcqCounter (void)
{
   gb_acq_counter++;
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Carga los valores por defecto en las estructuras de memoria internas.
S?lo escribe los registros de escritura.
*/
// -----------------------------------------------------------------------------
int UCI_SetPulseAmplitude (unsigned char data)
{
	if (gb_sitau_status != ST_None) return ELOG(charEUCI_IsRunning, EUCI_IsRunning);
	if (verifica_codigo_pulser() == 0)
	{
		TLV5626_value((u32 *)XPAR_TLV5626_AXI_LITE_TOP_0_BASEADDR, data);
		xil_printf("Fuente programada OK.\r\n");
	}
	else xil_printf("Fuente No programada\r\n");

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura el evento de disparo.

@return Ver cÃ¯Â¿Â½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Set_TriggerSource_v0 (TMSG_TriggerSource_v0 *msg)
{
T_Trigger trg;
int result;

   if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	TRIG_reset_all_interrupt_sources();

   trg.REG = 0;
   gb_uci.ind_acquisitions = 0;
   gb_uci.n_acquisitions = msg->n_acquisitions;
   gb_uci.delay_sync_n_acq = msg->delay_sync_n_acq;
   //gb_uci.n_acquisitions_interval = 1;
   gb_uci.acquisition_mode = AQUISITION_MODE_FIXED;
   gb_uci.data_link_mode = DATA_LINK_MODE_GE;

   if (msg->trigger_source.BIT.EXT == 1)
   {
      trg.BIT.EXT = 1;
      gb_uci.trigger_source = TRG_SCAN_EXT;
   }
   else if (msg->trigger_source.BIT.PRF_1 == 1)
   {
      trg.BIT.PRF_1 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_1);
      timer_start(TIMER_SCAN_PRF_1);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.PRF_2 == 1)
   {
      trg.BIT.PRF_2 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_2);
      timer_start(TIMER_SCAN_PRF_2);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.ENC_1 == 1)
   {
      if ((result = ENC_config(0,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_1 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 0;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_2 == 1)
   {
      if ((result = ENC_config(1,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_2 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 1;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_3 == 1)
   {
      if ((result = ENC_config(2,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_3 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 2;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_4 == 1)
   {
      if ((result = ENC_config(3,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_4 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 3;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else
   {
      gb_uci.trigger_source = TRG_SCAN_PRF;
      gb_sitau_status = ST_None;
      return EUCI_NONE;
   }
	gb_encoder_trigger_value_offset = 0;
   TRIG_set_interrupt_source(trg.REG);

	//if ((result = VCH_PRG(&gb_fp_virtual_channel[0])) < 0) return RLOG(result);
	gb_sitau_status = ST_ConfigAcquisition;
	SHRD_CPU1_Status(gb_sitau_status);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura el evento de disparo.

@return Ver cÃ¯Â¿Â½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Set_TriggerSource_v1 (TMSG_TriggerSource_v1 *msg)
{
T_Trigger trg;
int result;

   if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	TRIG_reset_all_interrupt_sources();

   trg.REG = 0;
   gb_uci.ind_acquisitions = 0;
   gb_uci.n_acquisitions = msg->n_acquisitions;
   gb_uci.delay_sync_n_acq = msg->delay_sync_n_acq;
   //gb_uci.n_acquisitions_interval = msg->n_acquisitions_interval;
   gb_uci.acquisition_mode = msg->acquisition_mode;
   gb_uci.data_link_mode = DATA_LINK_MODE_GE;

   if (msg->trigger_source.BIT.EXT == 1)
   {
      trg.BIT.EXT = 1;
      gb_uci.trigger_source = TRG_SCAN_EXT;
   }
   else if (msg->trigger_source.BIT.PRF_1 == 1)
   {
      trg.BIT.PRF_1 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_1);
      timer_start(TIMER_SCAN_PRF_1);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.PRF_2 == 1)
   {
      trg.BIT.PRF_2 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_2);
      timer_start(TIMER_SCAN_PRF_2);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.ENC_1 == 1)
   {
      if ((result = ENC_config(0,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_1 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 0;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_2 == 1)
   {
      if ((result = ENC_config(1,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_2 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 1;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_3 == 1)
   {
      if ((result = ENC_config(2,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_3 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 2;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_4 == 1)
   {
      if ((result = ENC_config(3,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_4 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 3;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else
   {
      gb_uci.trigger_source = TRG_SCAN_PRF;
      gb_sitau_status = ST_None;
      SHRD_CPU1_Status(gb_sitau_status);
      return EUCI_NONE;
   }
	gb_encoder_trigger_value_offset = 0;

	TRIG_set_interrupt_source(trg.REG);

	if (gb_fp_virtual_channel[gb_uci.active_virtual_channel].enabled == 1)
	{
		if (gb_log_acquiring == 1)
			LOG_ACQUIRING("\r\nSET_UP FP_VCH %d (%3d Lines) (%d Samples)... ",
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].id,
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_efl,
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].afe.AFE_BUSSAR_UT.num_samples);

		ACQ_FMC_set_up(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]); // Est� ser� m�s complicado m�s tarde

	}


   //TRIG_set_interrupt_source(trg.REG);
   //if ((result = VCH_PRG(&gb_fp_virtual_channel[0])) < 0) return RLOG(result);
   gb_sitau_status = ST_ConfigAcquisition;
   SHRD_CPU1_Status(gb_sitau_status);
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura el evento de disparo.

@return Ver cÃ¯Â¿Â½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Set_TriggerSource_v2 (TMSG_TriggerSource_v2 *msg)
{
T_Trigger trg;
int result;

   if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	TRIG_reset_all_interrupt_sources();

   trg.REG = 0;
   gb_uci.ind_acquisitions = 0;
   gb_uci.n_acquisitions = msg->n_acquisitions;
   gb_uci.delay_sync_n_acq = msg->delay_sync_n_acq;
   //gb_uci.n_acquisitions_interval = msg->n_acquisitions_interval;
   gb_uci.acquisition_mode = msg->acquisition_mode;
   gb_uci.data_link_mode = msg->data_link_mode;

   if (msg->trigger_source.BIT.EXT == 1)
   {
      trg.BIT.EXT = 1;
      gb_uci.trigger_source = TRG_SCAN_EXT;
   }
   else if (msg->trigger_source.BIT.PRF_1 == 1)
   {
      trg.BIT.PRF_1 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_1);
      timer_start(TIMER_SCAN_PRF_1);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.PRF_2 == 1)
   {
      trg.BIT.PRF_2 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_2);
      timer_start(TIMER_SCAN_PRF_2);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.ENC_1 == 1)
   {
      if ((result = ENC_config(0,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_1 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 0;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_2 == 1)
   {
      if ((result = ENC_config(1,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_2 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 1;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_3 == 1)
   {
      if ((result = ENC_config(2,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_3 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 2;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_4 == 1)
   {
      if ((result = ENC_config(3,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_4 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 3;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else
   {
      gb_uci.trigger_source = TRG_SCAN_PRF;
      gb_sitau_status = ST_None;
      SHRD_CPU1_Status(gb_sitau_status);
      return EUCI_NONE;
   }
	gb_encoder_trigger_value_offset = 0;

	TRIG_set_interrupt_source(trg.REG);

	if (gb_fp_virtual_channel[gb_uci.active_virtual_channel].enabled == 1)
	{
		if (gb_log_acquiring == 1)
			LOG_ACQUIRING("\r\nSET_UP FP_VCH %d (%3d Lines) (%d Samples)... ",
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].id,
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_efl,
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].afe.AFE_BUSSAR_UT.num_samples);

		ACQ_FMC_set_up(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]); // Est� ser� m�s complicado m�s tarde

	}


   //TRIG_set_interrupt_source(trg.REG);
   //if ((result = VCH_PRG(&gb_fp_virtual_channel[0])) < 0) return RLOG(result);
   gb_sitau_status = ST_ConfigAcquisition;
   SHRD_CPU1_Status(gb_sitau_status);
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura el evento de disparo.

@return Ver cÃ¯Â¿Â½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Set_TriggerSource_v3 (TMSG_TriggerSource_v3 *msg)
{
T_Trigger trg;
int result;

	if (gb_sitau_status != ST_None) return ELOG(charEUCI_IsRunning, EUCI_IsRunning);
	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	TRIG_reset_all_interrupt_sources();
	trg.REG = 0;
	gb_uci.ind_acquisitions = 0;
	gb_uci.n_acquisitions = msg->n_acquisitions;
	gb_uci.n_images_burst = msg->n_images_burst;
	gb_uci.delay_sync_n_acq = msg->delay_sync_n_acq;
	gb_uci.acquisition_mode = msg->acquisition_mode;
	gb_uci.data_link_mode = msg->data_link_mode;

//	if(1)
//	{
//		trg.REG = 0;
//		xil_printf("\n\rSaltando funcion UCI_Set_TriggerSource()...\n\r");
//	}
//	else
	if (msg->trigger_source.BIT.EXT == 1)
	{
		trg.BIT.EXT = 1;
		gb_uci.trigger_source = TRG_SCAN_EXT;
	}
	else if (msg->trigger_source.BIT.PRF_1 == 1)
	{
		trg.BIT.PRF_1 = 1;
		timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_1);
		timer_start(TIMER_SCAN_PRF_1);
		gb_uci.trigger_source = TRG_SCAN_PRF;
	}
	else if (msg->trigger_source.BIT.PRF_2 == 1)
	{
		trg.BIT.PRF_2 = 1;
		timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_2);
		timer_start(TIMER_SCAN_PRF_2);
		gb_uci.trigger_source = TRG_SCAN_PRF;
	}
	else if (msg->trigger_source.BIT.ENC_1 == 1)
	{
		if ((result = ENC_config(0,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_1 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 0;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else if (msg->trigger_source.BIT.ENC_2 == 1)
	{
		if ((result = ENC_config(1,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_2 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 1;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else if (msg->trigger_source.BIT.ENC_3 == 1)
	{
		if ((result = ENC_config(2,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_3 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 2;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else if (msg->trigger_source.BIT.ENC_4 == 1)
	{
		if ((result = ENC_config(3,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_4 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 3;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else
	{
		gb_uci.trigger_source = TRG_SCAN_PRF;
		gb_sitau_status = ST_None;
		SHRD_CPU1_Status(gb_sitau_status);
		return EUCI_NONE;
	}
	gb_encoder_trigger_value_offset = 0;
	//TRIG_set_interrupt_source(trg.REG);
	gb_sitau_status = ST_ConfigAcquisition;
	SHRD_CPU1_Status(gb_sitau_status);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura el evento de disparo.

@return Ver cÃ¯Â¿Â½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Set_TriggerSource (TMSG_TriggerSource *msg)
{
T_Trigger trg;
int result;

	if (gb_sitau_status != ST_None)
		{ELOG(charEUCI_IsRunning, EUCI_IsRunning);}
	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	TRIG_reset_all_interrupt_sources();
	trg.REG = 0;
	gb_uci.ind_acquisitions = 0;
	gb_uci.n_acquisitions = msg->n_acquisitions;
	gb_uci.n_images_burst = msg->n_images_burst;
	gb_uci.delay_sync_n_acq = msg->delay_sync_n_acq;
	gb_uci.acquisition_mode = msg->acquisition_mode;
	gb_uci.data_link_mode = msg->data_link_mode;
	gb_uci.pause_after_burst = msg->pause_after_burst;

//	if(1)
//	{
//		trg.REG = 0;
//		xil_printf("\n\rSaltando funcion UCI_Set_TriggerSource()...\n\r");
//	}
//	else
	if (msg->trigger_source.BIT.EXT == 1)
	{
		trg.BIT.EXT = 1;
		gb_uci.trigger_source = TRG_SCAN_EXT;
	}
	else if (msg->trigger_source.BIT.PRF_1 == 1)
	{
		trg.BIT.PRF_1 = 1;
		timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_1);
		timer_start(TIMER_SCAN_PRF_1);
		gb_uci.trigger_source = TRG_SCAN_PRF;
	}
	else if (msg->trigger_source.BIT.PRF_2 == 1)
	{
		trg.BIT.PRF_2 = 1;
		timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_2);
		//timer_start(TIMER_SCAN_PRF_2);
		gb_uci.trigger_source = TRG_SCAN_PRF;
	}
	else if (msg->trigger_source.BIT.ENC_1 == 1)
	{
		if ((result = ENC_config(0,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_1 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 0;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else if (msg->trigger_source.BIT.ENC_2 == 1)
	{
		if ((result = ENC_config(1,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_2 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 1;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else if (msg->trigger_source.BIT.ENC_3 == 1)
	{
		if ((result = ENC_config(2,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_3 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 2;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else if (msg->trigger_source.BIT.ENC_4 == 1)
	{
		if ((result = ENC_config(3,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
		trg.BIT.ENC_4 = 1;
		gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 3;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
	}
	else
	{
		gb_uci.trigger_source = TRG_SCAN_PRF;
		gb_sitau_status = ST_None;
		SHRD_CPU1_Status(gb_sitau_status);
		return EUCI_NONE;
	}
	gb_encoder_trigger_value_offset = 0;
	//TRIG_set_interrupt_source(trg.REG);
	gb_sitau_status = ST_ConfigAcquisition;
	SHRD_CPU1_Status(gb_sitau_status);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura el evento de disparo.

@return Ver cÃ¯Â¿Â½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Set_StatusConfig (TMSG_StatusConfig *msg)
{
int result;

	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	gb_uci.status_enabled = msg->enabled;
	gb_uci.status_period_ms = msg->period_ms;
	gb_uci.status_pcie = msg->pcie;
	SHRD_CPU1_StatusEnabled(gb_uci.status_enabled);
	if (gb_uci.status_enabled == 1) xil_printf("\nStatus Enabled\r\n");
	else xil_printf("\nStatus Disabled\r\n");
	return EUCI_NONE;
}

