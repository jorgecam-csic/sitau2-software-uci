/*
 * trigger.h
 *
 *  Created on: 6 de jul. de 2017
 *      Author: csic
 */

#ifndef SRC_TRIGGER_H_
#define SRC_TRIGGER_H_

#include "xparameters.h"
#include "xil_types.h"
#include "ttimer.h"
#include "gic_utils.h"
#include "log.h"
#include "uci_error_code.h"
#define TRIGGER_SW2HW		0
#define TRIGGER_EXT			1
#define TRIGGER_PRFTIMER1	2
#define TRIGGER_PRFTIMER2	3
#define TRIGGER_TIMESTAMP	4
#define TRIGGER_ENCODER1	5
#define TRIGGER_ENCODER2	6
#define TRIGGER_ENCODER3	7
#define TRIGGER_ENCODER4	8

#define	TRIGGER_SW2HW_MASK		(1<<TRIGGER_SW2HW)
#define	TRIGGER_EXT_MASK		(1<<TRIGGER_EXT)
#define	TRIGGER_PRFTIMER1_MASK	(1<<TRIGGER_PRFTIMER1)
#define	TRIGGER_PRFTIMER2_MASK	(1<<TRIGGER_PRFTIMER2)
#define	TRIGGER_TIMESTAMP_MASK	(1<<TRIGGER_TIMESTAMP)
#define	TRIGGER_ENCODER1_MASK	(1<<TRIGGER_ENCODER1)
#define	TRIGGER_ENCODER2_MASK	(1<<TRIGGER_ENCODER2)
#define	TRIGGER_ENCODER3_MASK	(1<<TRIGGER_ENCODER3)
#define	TRIGGER_ENCODER4_MASK	(1<<TRIGGER_ENCODER4)

#define NUM_TRIGGER_SOURCES	9

#define TRIG_ENA_OFFSET		0
#define TRIG_FLAGS_OFFSET	1
#define TRIG_VAL_OFFSET 	2
#define TRIG_INT_OFFSET		3 // bit0: 1 habilita interrupciones

typedef union T_Trigger {
	u32 REG;		// unsigned short Access
	struct {
		u32 SW2HW	   :1;  // Bit[0]: Software UCI
		u32 EXT        :1;  // Bit[1]: External Signal
		u32 PRF_1		:1;  // Bit[2]: PRF Timer 1
		u32 PRF_2		:1;  // Bit[3]: PRF Timer 2
		u32 TIME_STAMP :1;  // Bit[4]: Time Stamp
		u32 ENC_1		:1;  // Bit[5]: Encoder 1
		u32 ENC_2		:1;  // Bit[6]: Encoder 2
		u32 ENC_3		:1;  // Bit[7]: Encoder 3
		u32 ENC_4		:1;  // Bit[8]: Encoder 4
      u32 NOP        :23; // Bit[9:31]: Reserved
	} BIT;
} T_Trigger;

void TRIG_general_enable(u8 enable);
void TRIG_init();
void TRIG_int_function();
//void TRIG_set_interrupt_source(u8 source);
void TRIG_set_interrupt_source(u32 source);
//void TRIG_remove_interrupt_source(u8 source);
void TRIG_remove_interrupt_source(u32 source);
void TRIG_reset_all_interrupt_sources();
u32 TRIG_get_waited_triggers();
void TRIG_reset_waited_triggers();
u32 TRIG_get_unwaited_triggers();
int TRIG_wait_trigger(float us_timeout);

#endif /* SRC_TRIGGER_H_ */
