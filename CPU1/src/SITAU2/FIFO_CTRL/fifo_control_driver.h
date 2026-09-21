/*
 * fifo_control_driver.h
 *
 *  Created on: 2 abr. 2020
 *      Author: Jorge
 */

#ifndef SRC_SITAU2_FIFO_CTRL_FIFO_CONTROL_DRIVER_H_
#define SRC_SITAU2_FIFO_CTRL_FIFO_CONTROL_DRIVER_H_
#include "bcc_bussar.h"
#include "xil_types.h"
#include "mcbcc_mst_driver.h"

#define NUM_FIFO_SENSORS 4

#define OR_ALL_FLAGS	0
#define OVR_FLW_FLAG	1
#define NOT_END_FLAG	2
#define PRE_TRG_FLAG	3
#define NOT_TRG_FLAG	4
#define TRIGGER_CNT		5

#define COUNTERS		32


typedef struct fifo_control_counters_s
{
	union
	{
		u32 fifo_counter_word;
		struct
		{
			u8 overflows;
			u8 not_ends;
			u8 samples_pre_trigger;
			u8 not_triggered_frames;
		}BITS;
	};
}fifo_control_counters_t;

typedef struct fifo_control_regs_s
{
	u32 or_all_flags;
	u32 overflow;
	u32 not_ends;
	u32 samples_pre_trigger;
	u32 not_triggered_frames;
	u32 trigger_count;
}fifo_control_regs_t;

u32 fifo_control_get_or_flag(u8 module);
u32 fifo_control_get_all_flags(u8 module,fifo_control_regs_t *fifo_ctrl_regs);
fifo_control_counters_t fifo_control_get_counters(u8 module,int fifo_sensor_number);
void fifo_control_print_all(u8 module);
void fifo_control_clear_all(u8 module);

#endif /* SRC_SITAU2_FIFO_CTRL_FIFO_CONTROL_DRIVER_H_ */
