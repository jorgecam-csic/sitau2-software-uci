/*
 * ACQ_common.c
 *
 *  Created on: 24 oct. 2023
 *      Author: kokor
 */

#include "ACQ_common.h"

volatile ACQ_hndlr_t *Current_ACQ_hndlr_ptr=NULL;

int DEBUG_BEAMFORMER = 0;


#define DEBUG_ADDR_WRITE_FMC_INI	0
int Debug_addr_write=DEBUG_ADDR_WRITE_FMC_INI;

void ACQ_debug_beamformer(void)
{
	u32 acq2mem_addr,acq2bfm_addr,bfm2mem_addr,mem2bfm_addr;
	beamformer_ut_struct_t bf_ut_struct_print;
	beamformer_get_regs(&bf_ut_struct_print,1);
	beamformer_print_regs(&bf_ut_struct_print);

	acq2mem_addr=get_afe2mem_datamover_next_addr(1);
	acq2bfm_addr=get_mem2beamformer_datamover_next_addr(1);
	bfm2mem_addr=get_beamformer2mem_next_addr(1);
	mem2bfm_addr=get_mem2bf_prom_next_addr(1);

	xil_printf("\n\r");
	xil_printf("acq2mem_addr=0x%08X\n\r",acq2mem_addr);
	xil_printf("acq2bfm_addr=0x%08X\n\r",acq2bfm_addr);
	xil_printf("bfm2mem_addr=0x%08X\n\r",bfm2mem_addr);
	xil_printf("mem2bfm_addr=0x%08X\n\r",mem2bfm_addr);
}
