/*
 * pulser_bussar.h
 *
 *  Created on: 21 mar. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_BEAMFORM_BUSSAR_H_
#define SRC_SITAU2_BEAMFORM_BUSSAR_H_

#include "xil_types.h"
#include "bcc_bussar.h"

#define NUM_BEAMFORMER_COLUMNS 1
#define NUM_BEAMFORMER_DATAMOVERS NUM_BEAMFORMER_COLUMNS

typedef struct bf_general_ctrl_s
{
	union
	{
		u32 bf_general_ctrl_u32;
		struct
		{
			u32 unarmed : 1;
			u32 init_start : 1;
			u32 external_trigger : 1;
			u32 param_rdy : 1;

			u32 end_beamform : 1;
			u32 acquiring : 1;
			u32 trigger_software : 1;
			u32 param_stream_err : 1;

			u32 start_param_deser : 1;
			u32 busy_param_deser : 1;
			u32 end_param_deser : 1;
			u32 serial_param_prog : 1;

			u32 serial_bf_start : 4;

			u32 break_line_planner : 1;
			u32 enable_line_planner : 1;
			u32 end_beamformer_cycle : 1;
			u32 end_shot_cycle : 1;

			u32 end_image_cycle : 1;
			u32 reserved0 : 1;
			u32 auto_sequence : 1;
			u32 go_idle : 1;

			u32 dec_beamformer : 5;
			u32 reserved1 : 2;
			u32 not_busy : 1;
		}BITS;
	};
}bf_general_ctrl_t;

typedef union parallel_lines_u
{
	u32 parallel_lines_u32;
	struct
	{
		u16 parallel_lines;
		u16 current_parallel_lines;
	}BITS;
}parallel_lines_t;

typedef union image_lines_u
{
	u32 image_lines_u32;
	struct
	{
		u16 image_lines;
		u16 current_image_lines;
	}BITS;
}image_lines_t;

typedef union prog_addr_ini_end_u
{
	u32 prog_addr_ini_end_u32;
	struct
	{
		u16 prog_addr_ini;
		u16 prog_addr_end;
	}BITS;
}prog_addr_ini_end_t;

typedef union T0_BF_u
{
	u32 T0[NUM_BEAMFORMER_COLUMNS];
}T0_BF_t;

typedef struct bits_ida_s
{
	union
	{
		u32 bits_ida_u32;
		struct
		{
			u32 tfm : 1;
			u32 pwi : 1;
			u32 ipa : 1;
			u32 reserved : 4;

			u32 param_stream_err_ida : 1;

			u32 start_param_deser_ida : 1;
			u32 busy_param_deser_ida : 1;
			u32 end_param_deser_ida : 1;
			u32 reserved1 : 21;
		}BITS;
	};
}bits_ida_t;

typedef union prog_addr_ini_end_ida_u
{
	u32 prog_addr_ini_end_ida_u32;
	struct
	{
		u16 prog_addr_ini_ida;
		u16 prog_addr_end_ida;
	}BITS;
}prog_addr_ini_end_ida_t;

typedef union ln_planner_reg_u
{
	u32 ln_planner_reg_u32;
	struct
	{
		u16 ln_plan_start_addr;
		u16 ln_plan_ida_offset;
	}BITS;
}ln_planner_reg_t;

typedef struct beamformer_ut_struct_s
{
	u32 n_foci;
	parallel_lines_t parallel_lines;
	image_lines_t image_lines;
	prog_addr_ini_end_t prog_addr_ini_end;
	prog_addr_ini_end_ida_t prog_addr_ini_end_ida;
	s32 shft_bits;
	ln_planner_reg_t ln_planner_reg;
	bits_ida_t bits_ida;
	u32 cos_pwi;
	bf_general_ctrl_t bf_gen_ctrl;
	T0_BF_t T0_BF;
	u32 images_fl_loaded;
	u32 fw_fl_loaded;
}beamformer_ut_struct_t;

#define BEAMFORMER_CTRL_BITS_REG		0
#define BEAMFORMER_SAMPLES_REG 			1
#define BEAMFORMER_PAR_LINES_REG 		2
#define BEAMFORMER_IMG_LINES_REG 		3
#define BEAMFORMER_PROG_ADDR_REG		4

#define BEAMFORMER_BITS_REG_IDA_REG		8
#define BEAMFORMER_PROG_ADDR_IDA_REG	9
#define BEAMFORMER_LINE_PLAN_ADDR_REG	10
#define BEAMFORMER_SHIFT_BITS_REG		11
#define BEAMFORMER_IMAGES_FL_LOADED_REG	12
#define BEAMFORMER_FWD_FL_LOADED_REG	13
#define BEAMFORMER_COS_PWI_REG			14

#define BEAMFORMER_T0_BF_OFFSET_REG		16

#define BEAMFORMER_MAX_FW_FOCAL_LAWS	512
#define BEAMFORMER_MAX_BW_FOCAL_LAWS	512
#define BEAMFORMER_SIZE_FMC_FWD_WORD32	4
#define BEAMFORMER_SIZE_PWI_FWD_WORD32	1
#define MAX_PWI_FL						128

void beamformer_prog_cos_pwi_reg(beamformer_ut_struct_t *bf_ut_struct,u16 *cos_pwi_mem, u32 focal_law,u8 minibase);

int beamformer_print_ida_reg(bits_ida_t *ida_reg);

int beamformer_print_ctrl_reg(bf_general_ctrl_t *ctrl_reg);

int beamformer_print_regs(beamformer_ut_struct_t *bf_ut_struct);

int beamformer_get_regs(beamformer_ut_struct_t *bf_ut_struct,u8 minibase);

int beamformer_Init_start(beamformer_ut_struct_t *bf_ut_struct,u8 minibase);

int beamformer_SW_start(beamformer_ut_struct_t *bf_ut_struct,u8 minibase);

int beamformer_prog_registros(beamformer_ut_struct_t *bf_ut_struct,u8 minibase,commnd_buffer_t *commnd_buf_p);

int beamformer_prog_params(u32 *focal_laws_p,int focal_laws_words32,beamformer_ut_struct_t *bf_ut_struct,u8 minibase,commnd_buffer_t *commnd_buf_p);

int beamformer_fwrd_params(u32 *focal_laws_p,int focal_laws_words32,beamformer_ut_struct_t *bf_ut_struct,u8 minibase,commnd_buffer_t *commnd_buf_p);

int beamformer_prog_one_fwrd_focal_law(u32 *focal_laws_p,int n_lines,u32 offset,beamformer_ut_struct_t *bf_ut_struct,commnd_buffer_t *commnd_buf_p);

int beamformer_Stop_auto_sequence(u8 minibase);

int beamformer_set_ln_plan_reg(u32 ln_planner_reg_u32,u8 minibase);

#endif /* SRC_SITAU2_BEAMFORM_BUSSAR_H_ */
