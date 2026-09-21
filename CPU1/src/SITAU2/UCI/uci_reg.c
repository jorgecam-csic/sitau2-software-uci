// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "uci_reg.h"
#include "trigger.h"

// ____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ============================================================================

//T_StartElement gb_start_element;
T_SubSystem gb_pa_sys[UCI_N_MAX_SUBSYSTEM];
T_UCI gb_uci;
float gb_delay_muxei_32_128 = 0.000050; // seg.
float gb_delay_muxei_32_512 = 0.000150; // seg.
float gb_delay_muxinc = 0.000010; // seg.
float gb_delay_watchdog = 0.000500; // seg.

// Registros de la UCI
unsigned long gb_acq_counter = 0;
u32 gb_time_stamp = 0x0071E57A;
long gb_encoder[UCI_N_ENCODER];
int gb_triggered_encoder[UCI_N_ENCODER];
//int gb_pa_processing = 0;
T_SITAUStatus gb_sitau_status = ST_None;

s32 gb_encoder_trigger_value[UCI_N_ENCODER];
s32 gb_encoder_trigger_value_offset = 0;
s32 gb_encoder_channel_a_valid_edges_counter[UCI_N_ENCODER];
s32 gb_encoder_channel_b_valid_edges_counter[UCI_N_ENCODER];
s32 gb_encoder_channel_a_filtered_glitches_counter[UCI_N_ENCODER];
s32 gb_encoder_channel_b_filtered_glitches_counter[UCI_N_ENCODER];
s32 gb_encoder_sign_changes_counter[UCI_N_ENCODER];
s32 gb_encoder_error[UCI_N_ENCODER];
u64 Timstamp_ini=0,Timstamp_acq=0,Timstamp_end=0;



int gb_log_prg_base_wmpar = 0;
int gb_log_prg_mod_wmpar = 0;
int gb_log_amplia_send_command = 0;
int gb_log_prg_reg = 0;
int gb_log_acquiring = 0;
int gb_log_acquiring_frame = 0;
int gb_log_protocol = 0;
int gb_log_beamforming = 0;
int gb_test_remotedatamover = 0;
int gb_test_remote_fifo_control = 0;

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void UCI_SetupDefault(void)
{
int result, i;

   for(i=0; i<UCI_N_ENCODER; i++)
   {
      if ((result = ENC_config(i,1,1,0,1,0)) < 0) {RLOG(result);}
      if ((result = ENC_filter_config(i,800,63)) < 0) {RLOG(result);}
   }
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void UCI_Stop(void)
{
   TRIG_reset_all_interrupt_sources();
   //UCI_datapath_clear();
   gb_sitau_status = ST_None;
   xil_printf("\r\nStopped");
   SHRD_CPU1_Status(gb_sitau_status);
	UCI_Alarm1(0);
	UCI_Alarm2(0);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void UCI_ResetConfig(void)
{
int i;

   for (i=0; i<UCI_N_MAX_SUBSYSTEM; i++)
   {
      gb_pa_sys[i].exists = 0;
      gb_pa_sys[i].enabled = 0;
      gb_pa_sys[i].multiplexed = 0;
      gb_pa_sys[i].id_base_first = 0;
      gb_pa_sys[i].id_base_last = 0;
      gb_pa_sys[i].actual_start_element = 0;
   }
   gb_uci.encoder_trigger = 0;
   gb_uci.trigger_source = 0;
	gb_uci.encoder_trigger_steps  = 1;
   //gb_uci.trigger_encoder;
   gb_uci.trigger_prf_time = 1000;
   gb_uci.n_sub_systems = 1;
   gb_uci.active_virtual_channel=0;
   gb_uci.pause_after_burst = 0;
   gb_uci.status_enabled = 1;
   gb_uci.status_period_ms = 0;
   gb_uci.status_pcie = 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveCommand(void)
{
   return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_TriggeredEncoder(void)
{
int result = 0;

   if (gb_uci.encoder_trigger < UCI_N_ENCODER) 
         result = gb_triggered_encoder[gb_uci.encoder_trigger];
   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_TriggeredPRF(void)
{
int result = 0;

   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_TriggeredExternal(void)
{
int result = 0;

   return result;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
// int UCI_LoadFlashFirmware(T_MSG_LoadFlashFirmware *msg)
// {
// unsigned long n_data;
   
   // if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
   // n_data = msg->n_data;
   // for (i=0; i<n_data; i++)
   // {
      
   // }   
// }

