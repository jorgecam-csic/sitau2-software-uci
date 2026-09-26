/*
 * xcugic_mod_hw.c
 *
 *  Created on: 15 de feb. de 2017
 *      Author: csic
 */

#ifndef SRC_XCUGIC_MOD_HW_C_
#define SRC_XCUGIC_MOD_HW_C_

void Set_Up_Gic_Interrupt(XScuGic *InstancePtr, u32 CpuID, u16 Int_Id);
s32 XScuGic_DeviceInitialize_mod(u32 DeviceId);

#endif /* SRC_XCUGIC_MOD_HW_C_ */
