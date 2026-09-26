/*
 * ACQ_FMC.h
 *
 *  Created on: 13 sept. 2022
 *      Author: Cruza
 */

#ifndef SRC_UI_ACQ_ACQ_FMC_H_
#define SRC_UI_ACQ_ACQ_FMC_H_

#include <ACQ_common.h>
#include "vch.h"

#define FMC_LINE_RECV_TIMEOUT	30000.0
void ACQ_FMC_trig_in_interrupt_callback_function(void);
void ACQ_FMC_interrupt_startup(unsigned int images,unsigned int n_images_burst,unsigned int focal_laws_per_block,unsigned int focal_laws,unsigned int emi_prom,char acquisition_mode,u32 addr_ini,u32 addr_end,u32 acq_size,u32 n_pros_delay_sync, char pause_after_burst);
void ACQ_FMC_continuous_interrupt_callback_function(void);
void ACQ_FMC_n_images_interrupt_callback_function(void);
void ACQ_FMC_stop_trigger(void);
void ACQ_FMC_pause_trigger(void);
void ACQ_FMC_start_trigger(void);
int ACQ_FMC_set_up(TVCH *vch);
void ACQ_FMC_errorPRO(void);
void ACQ_FMC_errorPRO_link(u32 rd_addr);

void ACQ_FMC_stop_continous(void);
int ACQ_FMC_pause_continous(TMSG_Pause *msg);
int ACQ_FMC_continue_continous();
void ACQ_FMC_hard_break(TVCH *vch);

int FMC_transfer(TVCH *vch);
int FMC_transfer_focal_law_ETH(TVCH *vch, u32 focal_law);
int FMC_transfer_focal_law_GTX(TVCH *vch, u32 idx_fl);
int FMC_transfer_write_header(TVCH *vch, u32 idx_fl,u32 n_buffer_data);
int FMC_transfer_GTX_header_old(TVCH *vch,u8 minibase_gtx_header,u32 image_counter,u8 last_behaviour);
int FMC_transfer_GTX_header(TVCH *vch,u8 minibase_gtx_header,u32 gtx_block_count,u32 image_index,u16 focal_law_index,u16 valid_focal_laws_in_last_block,u8 last_block,u8 last_behaviour,u8 timestamp_present, u32 oob_data);
int FMC_transfer_block_GTX_link_pro(TVCH *vch,u8 last_block);
int FMC_transfer_image_GTX_link_pro(TVCH *vch);

#endif /* SRC_UI_ACQ_ACQ_FMC_H_ */
