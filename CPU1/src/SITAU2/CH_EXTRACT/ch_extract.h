/*
 * ch_extract.h
 *
 *  Created on: 30 jul. 2020
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_CH_EXTRACT_CH_EXTRACT_H_
#define SRC_SITAU2_CH_EXTRACT_CH_EXTRACT_H_

#include "xparameters.h"
#include "xil_types.h"


typedef struct ch_extract_ctrl_s
{
	union
	{
		u32 ch_extract_ctrl_u32;
		struct
		{
			u32 samples_per_channel : 18;
			u32 reserved0 : 2;
			u32 num_interleaved_channels : 8;
			u32 extra_data_error : 1;
			u32 go_to_idle : 1;
			u32 bypass : 1;
			u32 start_enable : 1;
		}BITS;
	};
}ch_extract_ctrl_t;

int  channel_extract_configure_basic(int valid_channels,int total_interleaved,int num_samples_per_channel);
int  channel_extract_configure_basic_no_test(int valid_channels,int total_interleaved,int num_samples_per_channel);
int  channel_extract_check_status(ch_extract_ctrl_t *ctrl_reg);
void channel_extract_print_status(ch_extract_ctrl_t *ctrl_reg);

#define CH_EXTRACT_BASEADDRESS	(XPAR_CHANNEL_EXTRACT_AXIL_0_BASEADDR)

#define CH_EXTRACT_CONTROL_OFFSET		0
#define CH_EXTRACT_VALID_MAP_OFFSET		4

#endif /* SRC_SITAU2_CH_EXTRACT_CH_EXTRACT_H_ */
