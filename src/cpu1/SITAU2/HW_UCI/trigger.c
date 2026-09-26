/*
 * trigger.c
 *
 *  Created on: 6 de jul. de 2017
 *      Author: csic
 */
#include "trigger.h"
#include "uci_set.h"

volatile u32 *TRIG_HW_base=(u32*)XPAR_TRIG_HW_AXILITE_0_BASEADDR;
volatile u32 Trigger_sources_mask=0x00000000;
volatile u32 Trigger_events_waited=0x00000000;
volatile u32 Trigger_events_before_wait=0x00000000;
volatile u8 Waiting_trigger=0;

void TRIG_general_enable(u8 enable)
{
	TRIG_HW_base[TRIG_INT_OFFSET]=(enable?1:0);
}

void TRIG_init()
{
	gic_utils_register_interrupt(XPAR_FABRIC_TRIG_HW_AXILITE_0_TRIG_INT_INTR,TRIG_int_function);
	TRIG_HW_base[TRIG_INT_OFFSET]=0x1;
}

void TRIG_int_function()
{
	Trigger_events_waited=0x0;
	Trigger_events_before_wait=0x0;
	if(Waiting_trigger)
		Trigger_events_waited=TRIG_HW_base[TRIG_FLAGS_OFFSET]&TRIG_HW_base[TRIG_ENA_OFFSET];
	else
		Trigger_events_before_wait=TRIG_HW_base[TRIG_FLAGS_OFFSET]&TRIG_HW_base[TRIG_ENA_OFFSET];

	//xil_printf("Trigger_events_before_wait=0x%08X\n\r",Trigger_events_before_wait);
	//xil_printf("Trigger_events_waited     =0x%08X\n\r",Trigger_events_waited);

}

//void TRIG_set_interrupt_source(u8 source)
//{
//	TRIG_HW_base[TRIG_ENA_OFFSET]|=1<<source;
//}
void TRIG_set_interrupt_source(u32 source)
{
	volatile u32 temp;
	if (source != 0) Waiting_trigger = 1;


	TRIG_HW_base[TRIG_ENA_OFFSET]=0;
	temp=TRIG_HW_base[TRIG_FLAGS_OFFSET]; //Leemos los flags por si hubiera uno por ahí activado que no debe.

	temp=TRIG_HW_base[TRIG_ENA_OFFSET];
	temp|=source;
	TRIG_HW_base[TRIG_ENA_OFFSET]=temp;

}

//void TRIG_remove_interrupt_source(u8 source)
//{
//	TRIG_HW_base[TRIG_ENA_OFFSET]&=~(1<<source);
//}
void TRIG_remove_interrupt_source(u32 source)
{
	if (source != 0) Waiting_trigger = 1;
	TRIG_HW_base[TRIG_ENA_OFFSET]&=~source;
}

void TRIG_reset_all_interrupt_sources()
{
	TRIG_HW_base[TRIG_ENA_OFFSET]=0x00000000;
}

u32 TRIG_get_waited_triggers()
{
	//if (Trigger_events_waited != 0) xil_printf("\r\n%x",Trigger_events_waited);
   return Trigger_events_waited;
}

void TRIG_reset_waited_triggers()
{
	Trigger_events_waited = 0;
}

u32 TRIG_get_unwaited_triggers()
{
	return Trigger_events_before_wait;
}

int TRIG_wait_trigger(float us_timeout)
{
int result;

	if (us_timeout > 0)
	{
		if ((result = TIMER_set_one_count(us_timeout,TIMER_SLEEP)) < 0) return RLOG(result);
	}

	if (us_timeout > 0)
	{
		if ((result = TIMER_Start(TIMER_SLEEP)) < 0) return RLOG(result);
	}

	Waiting_trigger=1;
	while(!Trigger_events_waited)
	{
		if(us_timeout==0)
			break;
		WFI_trigger();
		if(us_timeout>0&&TIMER_Timeout(TIMER_SLEEP))
		{
			Waiting_trigger=0;
			return ELOG(charEUCI_TRIG_Trigger_timeout, EUCI_TRIG_Trigger_timeout);
		}
	}
	Trigger_events_waited=0x0;
	Waiting_trigger=0;
	if(us_timeout==0 && Trigger_events_waited==0)
		return ELOG(charEUCI_TRIG_Trigger_timeout, EUCI_TRIG_Trigger_timeout);

	if((us_timeout>0) && (result=TIMER_Stop(TIMER_SLEEP)))
			return RLOG(result);

	return 0;
}


