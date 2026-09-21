#ifndef vch_tadH
#define vch_tadH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "global.h"
#include "afe_bussar.h"
#include "pulser_bussar.h"
#include "beamformer_bussar.h"
#include "afe_bussar.h"
#include "tgc_bussar.h"
#include "prom_emi.h"

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

#define MAX_VIRTUAL_CHANNELS    10

#define FIRMWARE_PHASED_ARRAY    0
#define FIRMWARE_FULL_PARALLEL   1

#define VERSION_DATE_SIZE           24
#define VERSION_DESCRIPTION_SIZE    24

#define MAX_N_BASES           8     //!< Número máximo de módulos ��MAX_MINIBASES??
#define MOD_MAX_CH            32    //!< Número máximo de canales por módulo
#define MOD_PUL_MAX_FOCAL_LAW 1024  //!< Número máximo de leyes focales en emision
#define MOD_BFM_MAX_FOCAL_LAW 512  //!< Número máximo de leyes focales en recepcion
#define TGC_MAX_SIZE          2048  //!< Número máximo de puntos d ela curva TGC
#define FIR_N_COEFFICIENTS    63    //!< Número de coeficientes del filtro FIR

#define MAX_BLOCKS_32b           255
#define MAX_BLOCKS               MAX_BLOCKS_32b * 4

// Acquisition Types
#define AQUISITION_TYPE_MC		0
#define AQUISITION_TYPE_PA		1
#define AQUISITION_TYPE_TFM		2
#define AQUISITION_TYPE_PA_MC	3

// Beamforming Types
#define BEAMFORMING_PA		0
#define BEAMFORMING_TFM		1
#define BEAMFORMING_PWI		2
#define BEAMFORMING_IPA		3
//Visualizaciones de onda
#define RF    0
#define VIDEO 1
#define HWp   2
#define HWn   3
#define FW    4

#define PROCESSING_TYPE_NONE	0
#define PROCESSING_TYPE_PROM_B	1
#define PROCESSING_TYPE_EMI_B 	2
#define PROCESSING_TYPE_PROM_A	3

#define C_MINIMUM_BRAM_DEEP   512
#define C_NUM_BEAMFORMERS	   1

#define LOG_2_NU	               2
#define C_WRITE_INTERFACE_WIDTH  32

#define C_T_INT_WIDTH	14
#define C_A_INT_WIDTH	(C_T_INT_WIDTH - 1)
#define C_A_D_PRECISION	6
#define C_Z_WIDTH		   11
#define C_EXTRA			3

#define D_0_WIDTH	(C_T_INT_WIDTH+C_A_D_PRECISION)
#define A_0_WIDTH	(C_A_INT_WIDTH+C_A_D_PRECISION)
#define	R_0_WIDTH	(C_T_INT_WIDTH+LOG_2_NU-1)
#define Z_0_WIDTH	(C_Z_WIDTH)
#define T_0_WIDTH	(C_T_INT_WIDTH+LOG_2_NU)
#define E_0_WIDTH	(C_EXTRA)

#define D_0_OFFSET	0
#define A_0_OFFSET	(D_0_WIDTH)
#define	R_0_OFFSET	(A_0_WIDTH+D_0_WIDTH)
#define Z_0_OFFSET	(R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)
#define T_0_OFFSET	(Z_0_WIDTH+R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)
#define E_0_OFFSET	(T_0_WIDTH+Z_0_WIDTH+R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)

#define C_INIT_WORD_WIDTH			(E_0_WIDTH+T_0_WIDTH+C_Z_WIDTH+R_0_WIDTH+A_0_WIDTH+D_0_WIDTH)

#define MAX_PROCESSING_ELEM			32
#define MAX_LINE					512

#define C_LINES_PER_BEAMFORMER		   (MOD_BFM_MAX_FOCAL_LAW/C_NUM_BEAMFORMERS)
#define C_N_INIT_SHARED_CH			      (C_MINIMUM_BRAM_DEEP/C_LINES_PER_BEAMFORMER)
#define C_N_PARALLEL_INIT			      (MOD_MAX_CH/C_N_INIT_SHARED_CH)
#define C_USED_BITS					      (C_N_PARALLEL_INIT*C_INIT_WORD_WIDTH)
#define	FLOOR_N_BRAMS_BEAMFORMER	   ((int)(C_USED_BITS/C_PARAM_BRAM_DATA_WIDTH))
#define ZERO_BRAM_WASTE_BITS		      ((FLOOR_N_BRAMS_BEAMFORMER*C_PARAM_BRAM_DATA_WIDTH)==C_USED_BITS?1:0)
#define C_N_BRAMS_BEAMFORMER		      (ZERO_BRAM_WASTE_BITS?FLOOR_N_BRAMS_BEAMFORMER:FLOOR_N_BRAMS_BEAMFORMER+1)
#define C_WASTED_BITS				      ((C_N_BRAMS_BEAMFORMER*C_PARAM_BRAM_DATA_WIDTH)-C_USED_BITS)
#define FLOOR_TRANSF_PER_USED_BITS	   ((int)(C_USED_BITS/C_WRITE_INTERFACE_WIDTH))
#define ZERO_TRANSF_WASTE_BITS		   ((FLOOR_TRANSF_PER_USED_BITS*C_WRITE_INTERFACE_WIDTH)==C_USED_BITS?1:0)
#define C_N_TRANSF					      (ZERO_TRANSF_WASTE_BITS?FLOOR_TRANSF_PER_USED_BITS:FLOOR_TRANSF_PER_USED_BITS+1)
#define C_TRANSF_BITS				      (C_N_TRANSF*C_WRITE_INTERFACE_WIDTH)
#define C_WASTED_TRANSF				      ((C_TRANSF_BITS)-C_USED_BITS)
#define C_TRANSF_WORDS_BASE			   (C_N_TRANSF*MOD_BFM_MAX_FOCAL_LAW)
//#define C_TOTAL_TRANSF_WORDS		      (C_TRANSF_WORDS_BASE*FP_MAX_N_BASES)
#define MAX_BITS_SEND_PER_WORD		   ((C_INIT_WORD_WIDTH+C_WRITE_INTERFACE_WIDTH-1))
#define FLOOR_MAX_TRANSF_PER_WORD	   ((int)MAX_BITS_SEND_PER_WORD/C_WRITE_INTERFACE_WIDTH)
#define CEIL_MAX_TRANSF_PER_WORD	      ((FLOOR_MAX_TRANSF_PER_WORD*C_WRITE_INTERFACE_WIDTH)==MAX_BITS_SEND_PER_WORD?FLOOR_MAX_TRANSF_PER_WORD:FLOOR_MAX_TRANSF_PER_WORD+1)
#define MAX_PARAM_WORDS				      (CEILING(C_INIT_WORD_WIDTH,32)*MOD_MAX_FOCAL_LAW*MAX_ELEM)
#define INIT_WORD_32BIT_WORDS_NUMBER	CEILING(C_INIT_WORD_WIDTH,32)
#define MOD_BEAMFORMER_MEMORY_SIZE     C_TRANSF_WORDS_BASE

// ---------------------------------------------------

//#define C_TOTAL_BRAMS_BIT_WIDTH_FWD	CEILING(C_INIT_WORD_WIDTH, C_PARAM_BRAM_DATA_WIDTH))
//#define C_PARAM_NUM_BRAMS_FWD		(C_TOTAL_BRAMS_BIT_WIDTH_FWD/C_PARAM_BRAM_DATA_WIDTH)
//#define C_N_TRANSF_FWD				CEILING(C_INIT_WORD_WIDTH,C_WRITE_INTERFACE_WIDTH)
//#define C_N_TRANSF_PWI				(1)

//Se dimensiona el tama�o para los tiempos de ida para una imagen TFM de todos los canales (al menos activos) y MAX_LINE lineas. Para un sistema multiplexado probablemente habr�a que ampliarlo.
//#define FP_BASE_FORWARD_MEMORY_SIZE	(C_N_TRANSF_FWD*MAX_LINE*MAX_PROCESSING_ELEM*MAX_N_BASES)

#define TFM_FMC_FORWARD_MEMORY_SIZE	(BEAMFORMER_SIZE_FMC_FWD_WORD32*MAX_LINE*MAX_PROCESSING_ELEM*MAX_N_BASES)
#define TFM_PWI_FORWARD_MEMORY_SIZE	(BEAMFORMER_SIZE_PWI_FWD_WORD32*MAX_PWI_FL*MAX_LINE)
#define TFM_PWI_COS_ANG_MEMORY_SIZE (MAX_PWI_FL)

// ---------------------------------------------------

#define FP_REMOTE_START_ADDR		DDR_MINIBASE_START_ADDR
#define FP_REMOTE_LAST_ADDR			DDR_MINIBASE_LAST_ADDR
#define FP_REMOTE_ALIGN_SIZE		DDR_MINIBASE_ALIGN_SIZE
#define FP_REMOTE_SIZE				DDR_MINIBASE_SIZE
#define FP_REMOTE_HALF_ADDR 		(DDR_MINIBASE_START_ADDR+(DDR_MINIBASE_SIZE/2))
#define FP_REMOTE_SCRATCH_SIZE 		(1<<23)
#define FP_REMOTE_SCRATCH_ADDR 		(FP_REMOTE_LAST_ADDR-FP_REMOTE_SCRATCH_SIZE+1)

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct TFP_VCH_PA_Image {
	u32 n_lines;
	u32 n_beamformed_samples;
	u32 n_blocks;
	u32 n_blocks_32b;
	u32 n_blocks_frame;
	u32 block_32b[MAX_BLOCKS_32b];
	u32 image_size_32b;
	u32 frame_size_32b;
} TFP_VCH_PA_Image;

//typedef struct THW_FP_ReceiverFocalLaw {
//   unsigned short n_ascan;
//   unsigned long image_size;
//   unsigned long n_words_32_image;
//   unsigned char ch[MOD_MAX_CH];
//} THW_FP_ReceiverFocalLaw;
//
//typedef struct THW_FP_ReceiverFocalLaws {
//   unsigned short n_fl;
//   THW_FP_ReceiverFocalLaw enabled[MOD_MAX_FOCAL_LAW];
//} THW_FP_ReceiverFocalLaws;
//
//typedef struct THW_FP_ReceptionFocalLaw {
//   unsigned long n_ascan;
//   unsigned long image_size;
//   unsigned long n_words_32_image;
//   unsigned char ch[MOD_MAX_CH];
//} THW_FP_ReceptionFocalLaw;

typedef struct TVCH_TGC
{
	unsigned short n_points;
   unsigned short curve[TGC_MAX_SIZE];
   TGC_UT_struct_t cfg;
} TVCH_TGC;

typedef struct TVCH_FIR {
   u32 enabled;
   u32 n_coefficients;
   short int coefficients[FIR_N_COEFFICIENTS];
} TVCH_FIR;

typedef struct TVCH_BASE {
	unsigned char bcc_addr;
	unsigned long bf_memory[MOD_BEAMFORMER_MEMORY_SIZE];
	unsigned short efl[MOD_MAX_CH * MOD_PUL_MAX_FOCAL_LAW];
} TVCH_BASE;

typedef struct TVCH_TFM_FMC_FORWARD {
    unsigned long size32_memory;
    unsigned long size32_forward_focal_law;
    unsigned long n_forward_focal_law;
    unsigned long current_forward_focal_law;

	u32 memory[TFM_FMC_FORWARD_MEMORY_SIZE];

} TVCH_TFM_FMC_FORWARD;

typedef struct TVCH_TFM_PWI_FORWARD {
    unsigned long size32_memory;
    unsigned long size32_forward_focal_law;
    unsigned long n_forward_focal_law;
    unsigned long current_forward_focal_law;

	u32 forwardMemory[TFM_PWI_FORWARD_MEMORY_SIZE];
	u16 angleMemory[MAX_PWI_FL];

} TVCH_TFM_PWI_FORWARD;

typedef struct TVCH {
	u32 id;

	unsigned char enabled;
	unsigned char acquiring;
	unsigned char hardware_set_up;
	u8 acquisition_type;
	u8 signal_mode;
	unsigned long gtx_n_focal_law;

	unsigned short gain_da;
	unsigned short gain_hi;

	unsigned short start_element;

	unsigned long acquisition_size_fp;


	unsigned char processing_type;
	unsigned char wait_prf_signals_processing;
	unsigned short n_signals_processing_avr;
	u8 log2_promediados;
	unsigned short n_signals_processing_emi;

	u8 decimation_factor; //!< Factor de diezmado
	u32 acq_delay;

	TFP_VCH_PA_Image pa_image;

	u32 acq_counter;
	float prf_time_line;
	float prf_time_burst;

	unsigned char enabled_timestamp;
	unsigned char enabled_buffer_dma;
	unsigned char enabled_temperature;
	unsigned char enabled_encoder_1;
	unsigned char enabled_encoder_2;
	unsigned char enabled_encoder_3;
	unsigned char enabled_encoder_4;

	emi_prom_config_t emi_prom_config_acquisition;
	emi_prom_config_t emi_prom_config_beamforming;

	unsigned short n_efl;
	//unsigned short n_pa_fl;
	unsigned long size_header_line_fp;
	unsigned long size_header_img_fp;
	unsigned long size_header_line_pa;
	unsigned long size_header_img_pa;
	unsigned long size32_fp_image;
	unsigned long size32_fp_frame;

	beamformer_ut_struct_t bf_regs;
	int size32_bf_memory;

	u32 emi_prom_scratch_addr;

	u32 remote_addr_ini;
	u32 remote_addr_cur;
	u32 remote_addr_end;

	u32 remote_addr_bf_ini;
	u32 remote_addr_bf_cur;
	u32 remote_addr_bf_end;

	AFE_UT_t afe;
	PULSER_UT_struct_t pulser;
	TVCH_TGC tgc;
	TVCH_FIR fir;
	TVCH_TFM_FMC_FORWARD tfm_fmc_forward;
	TVCH_TFM_PWI_FORWARD tfm_pwi_forward;
	unsigned long n_words_32_image;

	unsigned char n_bases;
	TVCH_BASE base[MAX_N_BASES];
} TVCH;

typedef struct THW_Config {
   unsigned char n_bases;
} THW_Config;


//typedef struct T_VirtualChannel {
//	u32 id;
//
//    unsigned char n_bases;
//
//	unsigned char enabled;
//   unsigned char acquiring;
//
//	unsigned short gain_da;
//	unsigned short gain_hi;
//
//   unsigned short start_element;
//
//	unsigned long acquisition_size_fp;
//   u32 n_acq_samples;
////   unsigned short n_efl;
//   //unsigned short n_rfl;
//
//	unsigned char processing_type;
//	unsigned char wait_prf_signals_processing;
//	unsigned short n_signals_processing_avr;
//   u8 log2_promediados;
//	unsigned short n_signals_processing_emi;
//
//   u8 decimation_factor; //!< Factor de diezmado
//	u32 acq_delay;
//   //AFE_SPI_UT_t AFE_SPI_UT;
//   //THW_PULSER pulser;
//   TVCH_TGC tgc;
//   TVCH_FIR fir;
//   //THW_BASE base[MAX_N_BASES];
//   THW hw;
//
//   TFP_VCH_PA_Image pa_image;
//
//   u32 acq_counter;
//   float prf_time_line;
//   unsigned short fp_n_fl;
//   unsigned short pa_n_fl;
//
//   unsigned long size_header_line_fp_;
//   unsigned long size_header_img_fp_;
//
//   unsigned long size_header_line_pa_;
//   unsigned long size_header_img_pa_;
//
//   unsigned char enabled_timestamp;
//   unsigned char enabled_buffer_dma;
//   unsigned char enabled_temperature;
//   unsigned char enabled_encoder_1;
//   unsigned char enabled_encoder_2;
//   unsigned char enabled_encoder_3;
//   unsigned char enabled_encoder_4;
//
//} T_VirtualChannel;

#endif
