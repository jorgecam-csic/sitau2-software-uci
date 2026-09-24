/*
 * ACQ_FMC.h
 *
 *  Created on: 13 sept. 2022
 *      Author: Cruza
 */

#ifndef SRC_UI_ACQ_PA_H_
#define SRC_UI_ACQ_PA_H_

#include "vch.h"

#define PA_LINE_RECV_TIMEOUT	1000.0
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
int PA_set_up(TVCH *vch);
void PA_errorPRO(void);
void PA_continuous_interrupt_callback_function(void);
void PA_n_images_interrupt_callback_function(void);
void PA_stop_trigger(void);
void PA_start_trigger(void);
void PA_interrupt_startup(unsigned int images,unsigned int n_images_burst,unsigned int n_lines_image,unsigned int emi_prom,unsigned short acquisition_mode,u32 addr_ini,u32 addr_end,u32 acq_size, char pause_after_burst);
int  PA_transfer(TVCH *vch);
int PA_send_image(TVCH *vch);
#endif /* SRC_UI_ACQ_PA_H_ */
