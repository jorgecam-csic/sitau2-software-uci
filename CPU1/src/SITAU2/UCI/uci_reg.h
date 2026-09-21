#ifndef uci_regH
#define uci_regH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "uci_define.h"
#include "global.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef enum T_SITAUStatus
{
	ST_None					= 0,
	ST_ConfigAcquisition	= 1,
	ST_SystemArmed			= 2,
	ST_TransferData			= 3,
	ST_Trigger 				= 4,
	ST_WaitStop				= 5,
} T_SITAUStatus;

typedef struct T_SubSystem {
   unsigned char exists;
   unsigned char enabled;
   unsigned char n_bases;
   unsigned char id_base_first;
   unsigned char id_base_last;
   unsigned char n_modules;
   unsigned char id_mod_first;
   unsigned char id_mod_last;
   unsigned short id_fl_first;
   unsigned short id_fl_last;
   unsigned char multiplexed;
   unsigned short n_active_channels;
   unsigned short n_multiplexed_channels;
   unsigned short n_total_channels;   
   unsigned char actual_start_element;
   unsigned char actual_focal_law;
} T_SubSystem;

typedef struct T_UCI {
   unsigned char encoder_trigger;
   long encoder_trigger_steps;
   unsigned char trigger_source;
   unsigned short trigger_prf_time;
   unsigned short n_sub_systems;
   unsigned short n_focal_laws;
   
   unsigned long n_acquisitions;
   unsigned long n_images_burst;
   unsigned short delay_sync_n_acq;
   unsigned long ind_acquisitions;
   unsigned short acquisition_mode;
   unsigned short data_link_mode;
   unsigned char active_virtual_channel;
   unsigned char pause_after_burst;

   unsigned char status_enabled;
   unsigned char status_pcie;
   unsigned long status_period_ms;
} T_UCI;

// Acquisition Modes
#define AQUISITION_MODE_FIXED			0
#define AQUISITION_MODE_CONTINOUS		1
//Estos modos ya no existen como caso aislado. Si no hay burst, n_images_burst = 1
//#define AQUISITION_MODE_FIXED_BURST		2
//#define AQUISITION_MODE_CONTINOUS_BURST	3

// Data Link Modes
#define DATA_LINK_MODE_GE		0
#define DATA_LINK_MODE_GTX		1
#define DATA_LINK_MODE_GE_GTX	2

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

//extern int gb_terminate;
extern T_SubSystem gb_pa_sys[UCI_N_MAX_SUBSYSTEM];
extern T_UCI gb_uci;
extern float gb_delay_muxei_32_128;
extern float gb_delay_muxei_32_512;
extern float gb_delay_muxinc;
extern float gb_delay_watchdog;

extern unsigned long gb_time_stamp;
extern long gb_encoder[UCI_N_ENCODER];

// Registros de la UCI
extern unsigned long gb_acq_counter;
extern unsigned long gb_time_stamp;
extern long gb_encoder[UCI_N_ENCODER];
extern int gb_triggered_encoder[UCI_N_ENCODER];
extern T_SITAUStatus gb_sitau_status;

extern s32 gb_encoder_trigger_value[UCI_N_ENCODER];
extern s32 gb_encoder_trigger_value_offset;
extern s32 gb_encoder_channel_a_valid_edges_counter[UCI_N_ENCODER];
extern s32 gb_encoder_channel_b_valid_edges_counter[UCI_N_ENCODER];
extern s32 gb_encoder_channel_a_filtered_glitches_counter[UCI_N_ENCODER];
extern s32 gb_encoder_channel_b_filtered_glitches_counter[UCI_N_ENCODER];
extern s32 gb_encoder_sign_changes_counter[UCI_N_ENCODER];

extern int gb_log_prg_base_wmpar;
extern int gb_log_prg_mod_wmpar;
extern int gb_log_amplia_send_command;
extern int gb_log_prg_reg;
extern int gb_log_acquiring;
extern int gb_log_acquiring_frame;
extern int gb_log_protocol;
extern int gb_log_beamforming;

extern int gb_test_remotedatamover;
extern int gb_test_remote_fifo_control;

extern u64 Timstamp_ini,Timstamp_acq,Timstamp_end;

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================
 
void UCI_SetupDefault(void);

void UCI_Stop(void);

int UCI_ReceiveCommand(void);

int UCI_TriggeredEncoder(void);

int UCI_TriggeredPRF(void);

int UCI_TriggeredExternal(void);

void UCI_ResetConfig(void);

// int UCI_LoadFlashFirmware(T_MSG_LoadFlashFirmware *msg);

#endif
