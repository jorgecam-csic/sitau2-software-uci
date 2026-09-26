/*
 * gic_utils.h
 *
 *  Created on: 18/11/2016
 *      Author: csic
 */

#ifndef GIC_UTILS_H_
#define GIC_UTILS_H_

#include "xscugic.h"
#include "global.h"

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

extern XScuGic Gic_instance;

int gic_utils_init(void);

void gic_utils_enable_interrupt(u32 interrupt_id);
int gic_utils_register_interrupt(u32 interrupt_id,Xil_ExceptionHandler interrupt_function);
int gic_utils_software_interrupt(u32 interrupt_id,u8 cpu_id);
void Set_Up_Gic_Interrupt(XScuGic *InstancePtr, u32 CpuID, u16 Int_Id);

#endif /* GIC_UTILS_H_ */
