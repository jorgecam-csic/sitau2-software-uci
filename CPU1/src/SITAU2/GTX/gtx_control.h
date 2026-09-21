/*
 * gtx_control.h
 *
 *  Created on: 24 ago. 2022
 *      Author: csic
 */

#ifndef SRC_SITAU2_GTX_GTX_CONTROL_H_
#define SRC_SITAU2_GTX_GTX_CONTROL_H_

#include "xil_types.h"
#include "bcc_bussar.h"

#define MGT_CLK_GPIO_UCI_GTX_PWRDOWN_BITNUM		5


#define MGT_ADV_DNCLK_GPIO_UCI_GTX_PWRDOWN_BITNUM		32

typedef struct gtx_misc_fsm_ctrl_reg_s
{
	union
	{
		u32 gtx_misc_fsm_ctrl_reg_32;
		struct
		{
			u32 gtx_pwdn_up : 1;
			u32 gtx_pwdn_dn : 1;
			u32 gtx_rdy_up : 1;
			u32 gtx_rdy_dn : 1;

			u32 reserved1 : 4;

			u32 lnk_dn_force_nothing : 1;
			u32 lnk_dn_force_memory  : 1;
			u32 lnk_dn_force_header  : 1;
			u32 lnk_dn_force_bypass  : 1;

			u32 lnk_dn_reset_fsm	 : 1;

			u32 reserved2 : 3;

			u32 reset_tx_dn : 1;
			u32 reset_rx_dn : 1;
			u32 reset_tx_up : 1;
			u32 reset_rx_up : 1;

			u32 reserved3   : 12;
		}BITS;
	};
}gtx_misc_fsm_ctrl_reg_t;

typedef struct gtx_misc_adv_ctrl_reg_s
{
	union
	{
		u32 gtx_misc_adv_ctrl_reg_32;
		struct
		{
			u32 tx_diff    : 4;
			u32 lpm        : 1;
			u32 pma_tx_rst : 1;
			u32 reserved1  : 2;
			u32 precursor  : 5;
			u32 reserved2  : 3;
			u32 postcursor : 5;
			u32 reserved3  : 9;
		}BITS;
	};
}gtx_misc_adv_ctrl_reg_t;

typedef struct mem2gtx_speed_ctrl_reg_s
{
	union
	{
		u32 mem2gtx_speed_ctrl_reg_32;
		struct
		{
			u32 active_cycles : 8;
			u32 total_cycles  : 8;

			u32 reserved1 : 15;

			u32 speed_control_enable : 1;
		}BITS;
	};
}mem2gtx_speed_ctrl_reg_t;

typedef struct gtx_status_counters_reg_s
{
	union
	{
		u32 gtx_status_counters_reg_32;
		struct
		{
			u8 channel_not_ready;
			u8 soft_errors;
			u8 hard_errors;

			u8 reserved1 : 7;
			u8 reset_counters : 1;
		}BITS;
	};
}gtx_status_counters_reg_t;


void gtx_set_adv_regs(u8 minibase,u8 tx_diff,u8 lpm,u8 pma_tx_rst,u8 precursor,u8 postcursor,u8 dn_0_up_1);
void enable_all_UCI_gtx(int ena);
void gtx_print_all_errors(u8 num_bases);
void mem2gtx_set_speed(u8 enable,u8 active_cycles,u8 total_cycles,u8 minibase);
gtx_status_counters_reg_t gtx_get_link_errors_dn(u8 minibase);
gtx_status_counters_reg_t gtx_get_link_errors_up(u8 minibase);
void gtx_print_errors(gtx_status_counters_reg_t errors);
gtx_misc_fsm_ctrl_reg_t gtx_get_ctrl_reg(u8 minibase);

void enable_all_remote_gtx(int ena);
int config_gtx(gtx_misc_fsm_ctrl_reg_t gtx_reg,u8 minibase,commnd_buffer_t *commnd_buf_p);
void swgtx_remote_up_pass_throw_to_uci(int disable);
//void gtx_from_minibase_mem_to_host(u8 minibase_origen);
void sw256_remote_mem_to_512_switch(u8 minibase,int disable);
void sw256_remote_mem_to_gtx_switch(u8 minibase,int disable);
void swgtx_remote_down_pass_throw_to_host(int disable);
void gtx_remote_from_minibase_mem_up_to_uci(u8 minibase_origen,u8 n_minibases);
void gtx_remote_from_minibase_mem_down_to_host(u8 minibase_origen,u8 n_minibases);
void gtx_remote_from_minibase_hea_down_to_host(u8 minibase_origen,u8 n_minibases);

void gtx_send_remote_header(u32* header_p,u32 header_size,u8 minibase_header,u8 last_behaviour);
void gtx_send_remote_repeat_header(u8 minibase_header,u8 last_behaviour);

void gtx_set_link_pro_out(u8 valor);
u32 gtx_get_link_pro_in();
void gtx_set_pro_link_to_gtx_control();
void gtx_set_pro_link_to_lvds_control();

#endif /* SRC_SITAU2_GTX_GTX_CONTROL_H_ */
