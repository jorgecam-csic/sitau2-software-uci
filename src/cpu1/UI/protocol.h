#ifndef protocolH
#define protocolH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include "xil_types.h"

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "vch_tad.h"
#include "shared_mem.h"
#include "beamformer_bussar.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define TAC_E_S			0x0 // Escritura Única
#define TAC_E_G			0x1 // Escritura Global
#define TAC_E_R			0x2 // Escritura Ráfagas
#define TAC_LEC			0x3 // Léctura (ráfagas o global)

#define DELAY_LOADFIRMWARE	3000 // 2 ms
#define DELAY_GORDO			3000 // 2 ms
#define DELAY_CORTO			100

#define UCI_REG_UCIv2		0x3E

#define MAX_N_BASES_MSG	4
#define TFM_FMC_FORWARD_MEMORY_SIZE_MSG	(BEAMFORMER_SIZE_FMC_FWD_WORD32*MAX_LINE*MAX_PROCESSING_ELEM*MAX_N_BASES_MSG)
#define TFM_PWI_FORWARD_MEMORY_SIZE_MSG	(BEAMFORMER_SIZE_PWI_FWD_WORD32*MAX_PWI_FL*MAX_LINE)
#define TFM_PWI_COS_ANG_MEMORY_SIZE_MSG (MAX_PWI_FL)
// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef enum T_CMD_Type
{
	CMD_NOP                             = 0,
	CMD_STOP                            = 1,
	CMD_TERMINAL_LOG                    = 2,
	CMD_STATUS_v0                       = 3,
	CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0	= 4,
	CMD_FP_VIRTUAL_CHANNEL_TGC          = 5,
	CMD_FP_VIRTUAL_CHANNEL_FIR          = 6,
	CMD_FP_PULSE_AMPLITUDE              = 7,
	CMD_FP_PRINT_VCH_CONFIG             = 8,
	CMD_FP_ACQUIRE                      = 9,
	CMD_VCH_EMISSION_FOCAL_LAW    		= 10,
	CMD_FP_TRIGGER_SOURCE_v0			= 11,
	CMD_FP_LOAD_FIRMWARE				= 12,   
	CMD_VIRTUAL_CHANNEL_BEAMFORMER 		= 13,
    CMD_VERSION                         = 14,
	CMD_RESET_ACQ_COUNTER               = 15,	
	CMD_RESET_ENCODER					= 16,
	CMD_FP_TRIGGER_SOURCE_v1		    = 17,
	CMD_FP_TRIGGER_SOURCE_v2			= 18,
	CMD_STATUS                          = 19,
	CMD_FP_VIRTUAL_CHANNEL_CONFIG_v1	= 20,
	CMD_FP_TRIGGER_SOURCE_v3			= 21,
	CMD_PAUSE							= 22,
	CMD_CONTINUE						= 23,
	CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2	= 24,
	CMD_TFM_FMC_FORWARD					= 25,
	CMD_HW_SITAU_ENABLED				= 26,
	CMD_FP_TRIGGER_SOURCE				= 27,
	CMD_STATUS_CONFIG					= 28,
	CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3	= 29,
	CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT	= 30,
	CMD_VCH_EMISSION_FOCAL_LAW_EXT 		= 31,
	CMD_TFM_PWI_FORWARD 				= 32,
	CMD_FP_VIRTUAL_CHANNEL_CONFIG 		= 33,
} T_CMD_Type;

typedef union T_AMPLIACommand {
   unsigned long CMD;         // 32Bits Access
   struct {                   // BIT  Access
      unsigned long REG : 6;  // Bit[0:5]
      unsigned long TAC : 2;  // Bit[6:7]
      unsigned long DIR : 8;  // Bit[8:15]
      unsigned long DAT : 16; // Bit[16:31]
   } BIT;
} T_AMPLIACommand;

typedef struct T_MSG_SITAUTemperature {
   unsigned char temp_1;
   unsigned char temp_2;
   unsigned char hyst;
} T_MSG_SITAUTemperature;

typedef struct T_MSG_PulseAmplitude {
   unsigned char data;
} T_MSG_PulseAmplitude;

typedef struct TMSG_STRUCT_Pulser {
    unsigned char enabled;
    unsigned char n_pulses;
    unsigned char high_z;
    unsigned char mode;
    unsigned long pulse_width;
    unsigned long delay;
} TMSG_STRUCT_Pulser;

typedef struct TMSG_BASE {
	unsigned char enabled;
	unsigned char bcc_addr;
} TMSG_BASE;

typedef struct TMSG_Config_v0 {
   int id_vch;
   unsigned char enabled;
   unsigned char n_bases;
   unsigned short start_element;
   unsigned short n_acq_samples;
   unsigned short n_beamformed_samples;
   unsigned short gain_da;
   unsigned short gain_hi;
   float prf_time_line;
   unsigned char n_mux_inc;

   unsigned char processing_type;
   unsigned char wait_prf_signals_processing;
   unsigned short n_signals_processing_avr;
   unsigned short n_signals_processing_emi;

   unsigned char enabled_timestamp;
   unsigned char enabled_buffer_dma;
   unsigned char enabled_temperature;
   unsigned char enabled_encoder_1;
   unsigned char enabled_encoder_2;
   unsigned char enabled_encoder_3;
   unsigned char enabled_encoder_4;

	u8 external_trigger;
	u8 decimation_factor;
	u32 water_delay;
	u8 log2_promediados;

   AFE_SPI_UT_t	afe_spi;
   TMSG_STRUCT_Pulser pulser;

   TMSG_BASE base[MAX_N_BASES_MSG]; //!< Estructura con los registros de las Bases
} TMSG_Config_v0;

typedef struct TMSG_Config_v1 {
   int id_vch;
   unsigned char enabled;   
   unsigned char n_bases;
   unsigned char acquisition_type;
   unsigned long gtx_n_focal_law;
   unsigned short start_element;
   unsigned short n_acq_samples;   
   unsigned short n_beamformed_samples;   
   unsigned short gain_da;
   unsigned short gain_hi;      
   float prf_time_line;   
   unsigned char n_mux_inc;   

   unsigned char processing_type;
   unsigned char wait_prf_signals_processing;
   unsigned short n_signals_processing_avr;
   unsigned short n_signals_processing_emi;

   unsigned char enabled_timestamp;
   unsigned char enabled_buffer_dma;
   unsigned char enabled_temperature;
   unsigned char enabled_encoder_1;
   unsigned char enabled_encoder_2;
   unsigned char enabled_encoder_3;
   unsigned char enabled_encoder_4;

	u8 external_trigger;
	u8 decimation_factor;
	u32 water_delay;
	u8 log2_promediados;

   AFE_SPI_UT_t	afe_spi;
   TMSG_STRUCT_Pulser pulser;

   TMSG_BASE base[MAX_N_BASES_MSG]; //!< Estructura con los registros de las Bases
} TMSG_Config_v1;

typedef struct TMSG_Config_v2 {
   int id_vch;
   unsigned char enabled;
   unsigned char n_bases;
   unsigned char acquisition_type;
   unsigned long gtx_n_focal_law;
   unsigned short start_element;
   unsigned short n_acq_samples;
   unsigned short n_beamformed_samples;
   unsigned short gain_da;
   unsigned short gain_hi;
   float prf_time_line;
   unsigned char n_mux_inc;
   unsigned char signal_mode;

   unsigned char processing_type;
   unsigned char wait_prf_signals_processing;
   unsigned short n_signals_processing_avr;
   unsigned short n_signals_processing_emi;

   unsigned char enabled_timestamp;
   unsigned char enabled_buffer_dma;
   unsigned char enabled_temperature;
   unsigned char enabled_encoder_1;
   unsigned char enabled_encoder_2;
   unsigned char enabled_encoder_3;
   unsigned char enabled_encoder_4;

	u8 external_trigger;
	u8 decimation_factor;
	u32 water_delay;
	u8 log2_promediados;

   AFE_SPI_UT_t	afe_spi;
   TMSG_STRUCT_Pulser pulser;

   TMSG_BASE base[MAX_N_BASES_MSG]; //!< Estructura con los registros de las Bases
} TMSG_Config_v2;

typedef struct TMSG_Config_v3 {
   int id_vch;
   unsigned char enabled;
   unsigned char n_bases;
   unsigned char acquisition_type;
   unsigned long gtx_n_focal_law;
   unsigned short start_element;
   unsigned short n_acq_samples;
   unsigned short n_beamformed_samples;
   unsigned short gain_da;
   unsigned short gain_hi;
   float prf_time_line;
   unsigned char n_mux_inc;
   unsigned char signal_mode;

   unsigned char processing_type;
   unsigned char wait_prf_signals_processing;
   unsigned short n_signals_processing_avr;
   unsigned short n_signals_processing_emi;

   unsigned char enabled_timestamp;
   unsigned char enabled_buffer_dma;
   unsigned char enabled_temperature;
   unsigned char enabled_encoder_1;
   unsigned char enabled_encoder_2;
   unsigned char enabled_encoder_3;
   unsigned char enabled_encoder_4;

	u8 external_trigger;
	u8 decimation_factor;
	u32 water_delay;
	u8 log2_promediados;

   AFE_SPI_UT_t	afe_spi;
   TMSG_STRUCT_Pulser pulser;

   TMSG_BASE base[MAX_N_BASES]; //!< Estructura con los registros de las Bases
} TMSG_Config_v3;

typedef struct TMSG_Config {
   int id_vch;
   unsigned char enabled;
   unsigned char n_bases;
   unsigned char acquisition_type;
   unsigned long gtx_n_focal_law;
   unsigned short start_element;
   unsigned short n_acq_samples;
   unsigned short n_beamformed_samples;
   unsigned short gain_da;
   unsigned short gain_hi;
   float prf_time_line;
   float prf_time_burst;
   unsigned char n_mux_inc;
   unsigned char signal_mode;

   unsigned char processing_type;
   unsigned char wait_prf_signals_processing;
   unsigned short n_signals_processing_avr;
   unsigned short n_signals_processing_emi;

   unsigned char enabled_timestamp;
   unsigned char enabled_buffer_dma;
   unsigned char enabled_temperature;
   unsigned char enabled_encoder_1;
   unsigned char enabled_encoder_2;
   unsigned char enabled_encoder_3;
   unsigned char enabled_encoder_4;

	u8 external_trigger;
	u8 decimation_factor;
	u32 water_delay;
	u8 log2_promediados;

   AFE_SPI_UT_t	afe_spi;
   TMSG_STRUCT_Pulser pulser;

   TMSG_BASE base[MAX_N_BASES]; //!< Estructura con los registros de las Bases
} TMSG_Config;

typedef struct TMSG_TGC {
	unsigned short id_vch;
	unsigned short n_points;
	unsigned char curve[TGC_MAX_SIZE];
} TMSG_TGC;

typedef struct TMSG_FIR {
	unsigned short id_vch;
	unsigned char enabled;
	unsigned short n_coefficients;
	short int coefficients[FIR_N_COEFFICIENTS];
} TMSG_FIR;

 typedef struct TMSG_STRUCT_Beamformer {
   unsigned long bf_memory[MOD_BEAMFORMER_MEMORY_SIZE]; // size = n_fl * 32
} TMSG_STRUCT_Beamformer;

typedef struct TMSG_Beamformer {
	int id_vch;
	unsigned long n_acq_samples; // (PA) n_afe_samples = n_samples_beamformed + aperture_length
								   // // (TFM) n_afe_samples = n_samples_diagonal_ida_vuelta;
	unsigned long n_beamformed_samples;
	unsigned short n_fl;
	unsigned short n_fl_parallel; // 1, 8, 16, 32
	unsigned short t0_max;
	int n_lword;
	int shift_bits;
	int beamforming_type;
	TMSG_STRUCT_Beamformer base[MAX_N_BASES_MSG];
} TMSG_Beamformer;

typedef struct TMSG_BeamformerExtended {
	unsigned short id_vch;
	unsigned short id_first_base;
	unsigned long n_acq_samples; // (PA) n_afe_samples = n_samples_beamformed + aperture_length
								   // // (TFM) n_afe_samples = n_samples_diagonal_ida_vuelta;
	unsigned long n_beamformed_samples;
	unsigned short n_fl;
	unsigned short n_fl_parallel; // 1, 8, 16, 32
	unsigned short t0_max;
	int n_lword;
	int shift_bits;
	int beamforming_type;
	TMSG_STRUCT_Beamformer base[MAX_N_BASES_MSG];
} TMSG_BeamformerExtended;

typedef struct TMSG_TFM_FMC_Forward {
	int id_vch;
    unsigned long size32_memory;
    unsigned long size32_forward_focal_law;
    unsigned long n_forward_focal_law;
	unsigned long memory[TFM_FMC_FORWARD_MEMORY_SIZE_MSG];
} TMSG_TFM_FMC_Forward;

typedef struct TMSG_TFM_PWI_Forward {
    int id_vch;
    unsigned long size32_memory;
    unsigned long size32_forward_focal_law;
    unsigned long n_forward_focal_law;
    unsigned long forwardMemory[TFM_PWI_FORWARD_MEMORY_SIZE_MSG];
    unsigned short anglesMemory[MAX_PWI_FL];
} TMSG_TFM_PWI_Forward;

typedef struct TMSG_PWI_Forward {
	int id_vch;
    unsigned long size32_memory;
    //unsigned long size32_forward_focal_law;
    unsigned long n_forward_focal_law;
	unsigned long memory_fwd[TFM_PWI_FORWARD_MEMORY_SIZE_MSG];  //Aqu� hay n_forward_focal_law*n_lineas_conformadas palabras de 32
	unsigned short memory_pwi_cos_ang[TFM_PWI_COS_ANG_MEMORY_SIZE_MSG];//registro_cos_ang = round((0.5*cos(ang_rad)-0.25)*2^17); Aqu� hay n_forward_focal_law palabras de 16 bits
} TMSG_PWI_Forward;

typedef struct TMSG_STRUCT_EmissionFocalLaw {
   unsigned short efl[MOD_MAX_CH * MOD_PUL_MAX_FOCAL_LAW];
} TMSG_STRUCT_EmissionFocalLaw;

typedef struct TMSG_EmissionFocalLaws {
	unsigned short id_vch;
	unsigned short n_bases;
	unsigned short n_fl;
	TMSG_STRUCT_EmissionFocalLaw base[MAX_N_BASES_MSG];
} TMSG_EmissionFocalLaws;

typedef struct TMSG_EmissionFocalLawsExtended {
	unsigned short id_vch;
	unsigned short id_first_base;
	unsigned short n_bases;
	unsigned short n_fl;
	TMSG_STRUCT_EmissionFocalLaw base[MAX_N_BASES_MSG];
} TMSG_EmissionFocalLawsExtended;

typedef union TMSG_STRUCT_TriggerSource {
	unsigned long REG;
	struct {
		unsigned long SW2HW	    :1;  // Bit[0]: Software UCI
		unsigned long EXT        :1;  // Bit[1]: External Signal
		unsigned long PRF_1		 :1;  // Bit[2]: PRF Timer 1
		unsigned long PRF_2		 :1;  // Bit[3]: PRF Timer 2
		unsigned long TIME_STAMP :1;  // Bit[4]: Time Stamp
		unsigned long ENC_1		 :1;  // Bit[5]: Encoder 1
		unsigned long ENC_2		 :1;  // Bit[6]: Encoder 2
		unsigned long ENC_3		 :1;  // Bit[7]: Encoder 3
		unsigned long ENC_4		 :1;  // Bit[8]: Encoder 4
      unsigned long NOP        :23; // Bit[9:31]: Reserved
	} BIT;
} TMSG_STRUCT_TriggerSource;

typedef struct TMSG_TriggerSource_v0 {
   TMSG_STRUCT_TriggerSource trigger_source;
   float prf_time;
   unsigned long encoder_steps;
   unsigned long n_acquisitions;
   unsigned short delay_sync_n_acq;
} TMSG_TriggerSource_v0;

typedef struct TMSG_TriggerSource_v1 {
   TMSG_STRUCT_TriggerSource trigger_source;
   float prf_time;
   unsigned long encoder_steps;
   unsigned long n_acquisitions;
   unsigned short delay_sync_n_acq;
   unsigned short acquisition_mode;
   unsigned long n_acquisitions_interval;
} TMSG_TriggerSource_v1;

typedef struct TMSG_TriggerSource_v2 {
   TMSG_STRUCT_TriggerSource trigger_source;
   float prf_time;
   unsigned long encoder_steps;
   unsigned long n_acquisitions;
   unsigned short delay_sync_n_acq;
   unsigned short acquisition_mode;
   unsigned short data_link_mode;
   unsigned long n_acquisitions_interval;
} TMSG_TriggerSource_v2;

typedef struct TMSG_TriggerSource_v3 {
   TMSG_STRUCT_TriggerSource trigger_source;
   float prf_time;
   unsigned long encoder_steps;
   unsigned long n_acquisitions;
   unsigned long n_images_burst;
   unsigned short delay_sync_n_acq;
   unsigned short acquisition_mode;
   unsigned short data_link_mode;
} TMSG_TriggerSource_v3;

typedef struct TMSG_TriggerSource {
   TMSG_STRUCT_TriggerSource trigger_source;
   float prf_time;
   unsigned long encoder_steps;
   unsigned long n_acquisitions;
   unsigned long n_images_burst;
   unsigned short delay_sync_n_acq;
   unsigned short acquisition_mode;
   unsigned short data_link_mode;
   unsigned short pause_after_burst;
} TMSG_TriggerSource;

typedef struct TMSG_StatusConfig {
   unsigned char enabled;
   unsigned char pcie;
   unsigned long period_ms;
} TMSG_StatusConfig;

typedef struct TMSG_Pause {
    unsigned long n_acquisitions;
} TMSG_Pause;

typedef struct TMSG_LoadFirmware {
   unsigned long size_file;
} TMSG_LoadFirmware;

typedef struct T_CMD_Version {
   unsigned char branch;
   unsigned char major;
   unsigned char minor;
   char date[VERSION_DATE_SIZE];
   char device[VERSION_DESCRIPTION_SIZE];
} T_CMD_Version;

typedef struct TMSG_Version {
   u16 checksum;
   T_CMD_Version cpu0;
   T_CMD_Version cpu1;
   u32 hw;
   u32 mod;
} TMSG_Version;

typedef struct TMSG_SITAUStatus_v0 {
   u16 checksum;
   unsigned char sitau_status;
   unsigned char trigger_source;
   unsigned long n_acquisitions;
   unsigned long ind_acquisitions;
   unsigned long size_buffer_command;
   TMSG_Version version;
   TSHRD_LOG_ErrorRegister log_error;
} TMSG_SITAUStatus_v0;

typedef struct TMSG_SITAUStatus {
   u16 checksum;
   u64 status_code;
   u32 rd_address;
   u32 wr_address;
   u32 n_overflows;
   unsigned char sitau_status;
   unsigned char trigger_source;
   u32 n_acquisitions;
   u32 ind_acquisitions;
   u32 size_buffer_command;
   TSHRD_LOG_ErrorRegister log_error;
} TMSG_SITAUStatus;

typedef union TMSG_TerminalLog {
   unsigned short REG;          // 32Bits Access
   struct {                    // BIT  Access
      unsigned short PRG_BASE_WMPAR          : 1;  // Bit[0]
      unsigned short PRG_MOD_WMPAR           : 1;  // Bit[1]
      unsigned short AMPLIA_SEND_COMMAND     : 1;  // Bit[2]
      unsigned short PRG_REG                 : 1;  // Bit[3]
      unsigned short ACQUIRING               : 1;  // Bit[4]
      unsigned short ACQUIRING_FRAME         : 1;  // Bit[5]
      unsigned short PROTOCOL                : 1;  // Bit[6]
      unsigned short BEAMFORMING             : 1;  // Bit[7]
      unsigned short NOP                     : 8;  // Bit[8:15]
   } BIT;
} TMSG_TerminalLog;

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================

int main_prog(void);
int UCI_Receive (void);
int UCI_Send_MSG_STATUS(void);
int UCI_Send (unsigned char *data_out, int n_bytes);

#endif

