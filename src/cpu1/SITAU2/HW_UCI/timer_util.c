/*
 * timer_util.c
 *
 *  Created on: 13 de feb. de 2017
 *      Author: csic
 */

#include "timer_util.h"

XTmrCtr	Timer[NUM_TIMERS_PERIPH];
volatile u8 Timer_flag[NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH];
extern XScuGic Gic_instance;

const u16 Timer_interrupt_id_table[NUM_TIMERS_PERIPH] =
{
		XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR,
		XPAR_FABRIC_AXI_TIMER_1_INTERRUPT_INTR,
		XPAR_FABRIC_AXI_TIMER_2_INTERRUPT_INTR,
		XPAR_FABRIC_TIMESTAMP_TIMER_INTERRUPT_INTR,
		XPAR_FABRIC_TRIGGER_TIMER_INTERRUPT_INTR
};

int timer_sleep_us_block(u32 us,u8 timer_id)
{
	u16 DeviceId=timer_id>>1;
	u8 TmrCtrNumber=timer_id%NUM_TIMERS_PER_PERIPH;
	u32 count=(u32)(us*(AXI_TIMER_FREQ/1000000));

	XTmrCtr_Stop(&Timer[DeviceId], TmrCtrNumber);
	XTmrCtr_SetResetValue(&Timer[DeviceId], TmrCtrNumber,count);
	XTmrCtr_SetOptions(&Timer[DeviceId], TmrCtrNumber,	XTC_INT_MODE_OPTION | XTC_DOWN_COUNT_OPTION | XTC_EXT_COMPARE_OPTION);
	Timer_flag[timer_id]=0;

	XTmrCtr_Start(&Timer[DeviceId], TmrCtrNumber);

	while(!Timer_flag[timer_id]);
		WFI_timer();

	Timer_flag[timer_id]=0;

	return 0;
}
int timer_start(u8 timer_id)
{
	u16 DeviceId=timer_id>>1;
	u8 TmrCtrNumber=timer_id%NUM_TIMERS_PER_PERIPH;

	XTmrCtr_Start(&Timer[DeviceId], TmrCtrNumber);
	return 0;
}
int timer_stop(u8 timer_id)
{
	u16 DeviceId=timer_id>>1;
	u8 TmrCtrNumber=timer_id%NUM_TIMERS_PER_PERIPH;

	XTmrCtr_Stop(&Timer[DeviceId], TmrCtrNumber);
	return 0;
}
int timer_set_one_count(float seconds,u8 timer_id)
{
	u16 DeviceId=timer_id>>1;
	u8 TmrCtrNumber=timer_id%NUM_TIMERS_PER_PERIPH;
	//u32 count=(u32)(seconds*Timer[DeviceId].Config.SysClockFreqHz);
	u32 count=(u32)(seconds*Timer[DeviceId].Config.SysClockFreqHz);

	if(count==0) count=1; //se pone una cuenta mínima.

	XTmrCtr_SetResetValue(&Timer[DeviceId], TmrCtrNumber,count);
	Timer_flag[timer_id]=0;
	XTmrCtr_SetOptions(&Timer[DeviceId], TmrCtrNumber,	XTC_INT_MODE_OPTION | XTC_DOWN_COUNT_OPTION | XTC_EXT_COMPARE_OPTION);

	return 0;
}
int timer_set_periodic_count(float seconds,u8 timer_id)
{
	u16 DeviceId=timer_id>>1;
	u8 TmrCtrNumber=timer_id%NUM_TIMERS_PER_PERIPH;
	u32 count=(u32)(seconds*AXI_TIMER_FREQ);

	if(count==0) count=1; //se pone una cuenta mínima.

	XTmrCtr_SetResetValue(&Timer[DeviceId], TmrCtrNumber,count);
	Timer_flag[timer_id]=0;
	XTmrCtr_SetOptions(&Timer[DeviceId], TmrCtrNumber,	XTC_INT_MODE_OPTION | XTC_DOWN_COUNT_OPTION | XTC_EXT_COMPARE_OPTION | XTC_AUTO_RELOAD_OPTION);

	return 0;
}
void timer_default_int_handler(void *InstancePtr,u8 timer_number_inside_periph)
{
	XTmrCtr *TmrCtrPtr = InstancePtr;
	u16 DeviceId;
	DeviceId=TmrCtrPtr->Config.DeviceId;
	Timer_flag[DeviceId*NUM_TIMERS_PER_PERIPH+timer_number_inside_periph]=1;
	DMB();
	DSB();
	//xil_printf("TIMER INT: %d\n\r",DeviceId*NUM_TIMERS_PER_PERIPH+timer_number_inside_periph);
}

int timer_int_ini(XScuGic* IntcInstancePtr,
		XTmrCtr* TmrCtrInstancePtr,
		u16 DeviceId,
		u16 IntrId,
		Xil_ExceptionHandler Interrupt_handler)
{
	int Status;

	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly, use the 1st timer in the device (0)
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, 0);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;


	if(IntrId)
	{
		if(Interrupt_handler==NULL)
		{
			Status = XScuGic_Connect(IntcInstancePtr, IntrId,
						 (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
						 TmrCtrInstancePtr);
			if (Status != XST_SUCCESS)
				return Status;

			XTmrCtr_SetHandler(TmrCtrInstancePtr, timer_default_int_handler,TmrCtrInstancePtr);
		}
		else
		{
			Status = XScuGic_Connect(IntcInstancePtr, IntrId,
						 Interrupt_handler,
						 TmrCtrInstancePtr);
		}
		XScuGic_InterruptMaptoCpu(IntcInstancePtr, THIS_CPU_ID, IntrId);
		XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId,	0xA0, 0x3);
		XScuGic_Enable(IntcInstancePtr, IntrId);
	}

	return XST_SUCCESS;
}

int timer_init_all_default(void)
{
	u16 i,j;
	int status;
	for(i=0;i<NUM_TIMERS_PERIPH;i++)
	{
		status=timer_int_ini(&Gic_instance,&Timer[i],i,Timer_interrupt_id_table[i],NULL);
		if(status!=XST_SUCCESS)
			return -i;
		for(j=0;j<NUM_TIMERS_PER_PERIPH;j++)
			Timer_flag[i*NUM_TIMERS_PER_PERIPH+j]=0;
	}
	return i;
}

int timer_set_one_count_no_int(float seconds,u8 timer_id)
{
	u16 DeviceId=timer_id>>1;
	u8 TmrCtrNumber=timer_id%NUM_TIMERS_PER_PERIPH;
	u32 count=(u32)(seconds*AXI_TIMER_FREQ);

	if(count==0) count=1; //se pone una cuenta mínima.

	XTmrCtr_SetResetValue(&Timer[DeviceId], TmrCtrNumber,count);
	Timer_flag[timer_id]=0;
	XTmrCtr_SetOptions(&Timer[DeviceId], TmrCtrNumber, XTC_DOWN_COUNT_OPTION | XTC_EXT_COMPARE_OPTION);

	return 0;
}
int timer_set_periodic_count_no_int(float seconds,u8 timer_id)
{
	u16 DeviceId=timer_id>>1;
	u8 TmrCtrNumber=timer_id%NUM_TIMERS_PER_PERIPH;
	u32 count=(u32)(seconds*AXI_TIMER_FREQ);

	if(count==0) count=1; //se pone una cuenta mínima.

	XTmrCtr_SetResetValue(&Timer[DeviceId], TmrCtrNumber,count);
	Timer_flag[timer_id]=0;
	XTmrCtr_SetOptions(&Timer[DeviceId], TmrCtrNumber, XTC_DOWN_COUNT_OPTION | XTC_EXT_COMPARE_OPTION | XTC_AUTO_RELOAD_OPTION);

	return 0;
}
