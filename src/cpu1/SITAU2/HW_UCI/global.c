/*
 * global.c
 *
 *  Created on: 29/11/2016
 *      Author: csic
 */


#include "global.h"
#include "timer_util.h"

int gb_hw_sitau_enabled = 1;

cons_prod_t* UCI2HOST_prod_p;
cons_prod_t* HOST2UCI_cons_p;
cons_prod_t* IMAGE_prod_p;


void void_printf( const char *ctrl1, ...)
{
	/*static int i=0;

	xil_printf("dbg_printf: %08X\r\n",i);*/
	//timer_sleep_us_block(2,3);
}

void usleep_for(int numero_absurdo)
{
	volatile int cuenta;
	for(cuenta=0;cuenta<numero_absurdo;cuenta++);
}


void usleep_timer(u32 us)
{
	timer_sleep_us_block(us,TIMER_SLEEP);
}
