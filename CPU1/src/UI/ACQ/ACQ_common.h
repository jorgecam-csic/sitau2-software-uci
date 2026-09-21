/*
 * acq_common.h
 *
 *  Created on: 24 oct. 2023
 *      Author: kokor
 */

#ifndef SRC_UI_ACQ_ACQ_COMMON_H_
#define SRC_UI_ACQ_ACQ_COMMON_H_

#include "xil_types.h"
#include "vch_tad.h"

#define DEBUG_ADDR_WRITE_FMC_INI	0

extern int Debug_addr_write;

typedef enum state_TFM_e
{
	IDLE,
	AQUIRING,
	BEAMFORMING,
	AQUIRING_AND_TRANSFER_TO_UCI,
	TRANSFER_TO_UCI,
	WAITING_UNPAUSE,
	RECONFIG,
	ENDED
} state_TFM_t;

typedef enum MBF_image_type_e
{
	TFM,
	PWI,
	IPA,
	OTHER
} MBF_image_type_t;

typedef struct ACQ_hndlr_s
{
	char acquisition_ended;
	char transmission_ended;
	char overflow_flag;
	char pause_continous;
	//char continue_continous;
	char end_continous;
	char acquisition_mode;

	char store_timestamp_on_int;
	char stored_timestamp;
	char pause_after_burst;

	char trig2pro_disabled;

	unsigned int n_pros_delay_sync;

	unsigned int n_pros;
	unsigned int n_emi_prom_count;
	unsigned int n_emi_prom_total;
	unsigned int n_focal_laws_per_block_count;
	unsigned int n_focal_laws_per_block_total;

	unsigned int n_focal_laws_count;
	unsigned int n_focal_laws_beamformed;
	unsigned int n_focal_laws_total;
	unsigned int n_images_count;
	unsigned int n_images_beamformed;
	unsigned int n_images_sent;
	unsigned int n_images_total;
	unsigned int n_images_burst_count;
	unsigned int n_images_burst_beamformed;
	unsigned int n_images_burst_total;
	unsigned int n_bursts_count;
	unsigned int n_bursts_beamfomed;
	unsigned int n_bursts_sent;
	unsigned int n_overflows;

	unsigned int write_addr_index;
	unsigned int read_addr_index;
	unsigned int addr_ini;
	unsigned int addr_end;
	unsigned int bytes_per_fl_remote;

	unsigned int n_blocks_sent;
	unsigned int n_blocks_acquired;
	unsigned int n_blocks_to_send;

	unsigned int block_counter_header;
	unsigned int image_counter_header;
	unsigned short int focal_law_index_header;

	u64 timestamp_value;
	unsigned int bloq_index_timestamped;

	unsigned int scratch_addr_index;
	unsigned int tfm_image_addr_index;
	unsigned int acquisition_addr_index;

	unsigned int n_lines_bf_parallel;
	unsigned int n_lines_bf_count;
	unsigned int n_lines_bf_total;

	unsigned int current_fwd_fl;

	unsigned int next_fwd_fl_offset_is_prog;
	unsigned int num_fwd_fl_max;

	MBF_image_type_t MBF_image_type;

	u16 next_fwd_fl_offset_addr;
	s8 dsr_shift_bits;

	state_TFM_t state_TFM;

	TVCH *virtual_channel_p;

}ACQ_hndlr_t;

extern volatile ACQ_hndlr_t *Current_ACQ_hndlr_ptr;
extern int DEBUG_BEAMFORMER;
void ACQ_debug_beamformer(void);

#endif /* SRC_UI_ACQ_ACQ_COMMON_H_ */
