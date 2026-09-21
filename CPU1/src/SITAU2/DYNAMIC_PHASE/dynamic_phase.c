/*
 * dynamic_phase.c
 *
 *  Created on: 12 may. 2022
 *      Author: csic
 */
#include "xil_types.h"
#include "dynamic_phase.h"
#include "mcbcc_mst_driver.h"
#include "bussar_addr.h"
#include "afe5808a.h"



void set_dynamic_phase(u8 minibase,int phase_delay_ticks)
{
	volatile u32* remote_addr;
	int aux;
	remote_addr=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);
	remote_addr[PS_TICK_TARGET_REG]=phase_delay_ticks;
	aux=remote_addr[PS_TICK_TARGET_REG];
	// Empieza el proceso
	remote_addr[PS_CONTROL_REG]=1<<PHASE_SHIFT_TRIG;
	TIMER_Sleep(100.0);
	/*while(aux & (1<<PHASE_SHIFT_TRIG))
	{
		aux=remote_addr[PS_CONTROL_REG];
	}*/

	MCBCC_deactivate_emu();

}

void set_dynamic_phase_ps(u8 minibase,int phase_delay_picoseconds)
{
	int phase_ticks;
	phase_ticks=(int)(((float)phase_delay_picoseconds)*56*VCO_FREQ_MHZ/1.0e6); //Mirar 7 series Cocking resources user guide
	set_dynamic_phase(minibase,phase_ticks);
}

/*
void set_dynamic_phase(u8 minibase,int phase_delay_ticks)
{
	volatile u32* remote_addr;
	int current_ticks;
	remote_addr=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);
	current_ticks=remote_addr[PS_TICK_COUNT_REG];
	while(current_ticks!=phase_delay_ticks)
	{
		if(current_ticks<phase_delay_ticks)
			remote_addr[PS_CONTROL_REG]=1<<PHASE_UP_BIT;
		if(current_ticks>phase_delay_ticks)
			remote_addr[PS_CONTROL_REG]=1<<PHASE_DOWN_BIT;

		wait_for(1000);
		current_ticks=remote_addr[PS_TICK_COUNT_REG];
		xil_printf("Current TICKS = %d/%d\n\r",current_ticks,phase_delay_ticks);
	}

	MCBCC_deactivate_emu();

}*/

