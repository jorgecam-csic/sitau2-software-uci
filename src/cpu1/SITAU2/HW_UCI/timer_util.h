/*
 * timer_util.h
 *
 *  Created on: 14 de feb. de 2017
 *      Author: csic
 */

#ifndef SRC_TIMER_UTIL_H_
#define SRC_TIMER_UTIL_H_

#include "xparameters.h"
#include "xtmrctr.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "global.h"

#define TCSR0	0	//Timer 0 Control and Status Register
#define TLR0	1	//Timer 0 Load Register
#define TCR0	2	//Timer 0 Counter Register
#define TCSR1	4	//Timer 1 Control and Status Register
#define TLR1	5	//Timer 1 Load Register
#define TCR1	6	//Timer 1 Counter Register

#define AXI_TIMER_FREQ			125000000
#define NUM_TIMERS_PERIPH		XPAR_XTMRCTR_NUM_INSTANCES
#define NUM_TIMERS_PER_PERIPH	2
#define TOTAL_TIMERS			(NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH)

// Timers
#define ID_TIMER_MUX       0 //!< Timer asociado al retardo de los multiplexores 
#define ID_TIMER_PRF_LINE  2 //!< Timer asociado a la PRF
//#define ID_TIMER_USER_1    2
//#define ID_TIMER_USER_2    3
//#define ID_TIMER_WATCHDOG  5

//#define TIMER_SCAN_PRF_LINE   0
//#define TIMER_SCAN_MUX        1
#define TIMER_SLEEP           4
#define TIMEOUT_TIMER		  5
#define ID_TIMER_TIMESTAMP 6 //  64 bits
#define TIMER_SCAN_PRF_1      8
#define TIMER_SCAN_PRF_2      9


int timer_stop(u8 timer_id);
int timer_init_all_default(void);
int timer_int_ini(XScuGic* IntcInstancePtr,
		XTmrCtr* TmrCtrInstancePtr,
		u16 DeviceId,
		u16 IntrId,
		Xil_ExceptionHandler Interrupt_handler);
int timer_set_one_count(float seconds,u8 timer_id);
int timer_set_periodic_count(float seconds,u8 timer_id);
int timer_sleep_us_block(u32 us,u8 timer_id);

void timer_default_int_handler(void *InstancePtr,u8 timer_number_inside_periph);
int timer_start(u8 timer_id);
int timer_set_periodic_count_no_int(float seconds,u8 timer_id);

extern volatile u8 Timer_flag[NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH];
extern XTmrCtr	Timer[];
int timer_set_one_count_no_int(float seconds,u8 timer_id);
int timer_set_periodic_count_no_int(float seconds,u8 timer_id);

#endif /* SRC_TIMER_UTIL_H_ */
