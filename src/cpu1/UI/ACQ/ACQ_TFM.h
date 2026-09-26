/*
 * ACQ_FMC.h
 *
 *  Created on: 13 sept. 2022
 *      Author: Cruza
 */

#ifndef SRC_UI_ACQ_TFM_H_
#define SRC_UI_ACQ_TFM_H_

#include "vch.h"
#include "ACQ_common.h"

#define TFM_LINE_RECV_TIMEOUT	10000.0

/*
void ACQ_FMC_continuous_interrupt_callback_function(void);
void ACQ_FMC_n_images_interrupt_callback_function(void);
void ACQ_FMC_interrupt_startup(unsigned int images,unsigned int images_per_block,unsigned int focal_laws,unsigned int emi_prom,unsigned short acquisition_mode,u32 addr_ini,u32 addr_end,u32 acq_size);
void ACQ_FMC_stop_trigger(void);
void ACQ_FMC_start_trigger(void);

void ACQ_FMC_errorPRO(void);

void ACQ_FMC_stop_continous();

int FMC_transfer(TVCH *vch);
int FMC_transfer_focal_law_ETH(TVCH *vch, u32 focal_law);
int FMC_transfer_focal_law_GTX(TVCH *vch, u32 idx_fl);
int FMC_transfer_write_header(TVCH *vch, u32 idx_fl,u32 n_buffer_data);
int FMC_transfer_GTX_header(TVCH *vch,u8 minibase_gtx_header,u32 image_counter);
int FMC_transfer_image_GTX_link_pro(TVCH *vch);
*/

void TFM_conf_mode_ACQ();
void TFM_process_burst();
void TFM_FSM(void);
void TFM_pause_trigger(void);
int TFM_set_up(TVCH *vch);
void TFM_errorPRO(void);
void TFM_continuous_interrupt_callback_function(void);
void TFM_n_images_interrupt_callback_function(void);
void TFM_stop_trigger(void);
void TFM_start_trigger(void);
void TFM_interrupt_startup(TVCH *vch,unsigned int images,unsigned int n_emission_focal_laws,unsigned int n_lines_image,unsigned int n_parallel_bf,unsigned int acquisition_addr,unsigned int scratch_addr,unsigned int tfm_image_addr,MBF_image_type_t MBF_image_type, char pause_after_burst);
int  TFM_transfer(TVCH *vch);
int TFM_transfer_fsm(TVCH *vch);
int TFM_send_image(TVCH *vch);
void TFM_DEBUG_void_forward_focal_law(int focal_law,TVCH *vch);

#endif /* SRC_UI_ACQ_TFM_H_ */
