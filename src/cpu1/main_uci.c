/*
 * Copyright (c) 2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include "xparameters.h"
#include "xil_types.h"
#include "xstatus.h"
//
//#include "xbasic_types.h"
#include "xil_cache.h"

#include "platform.h"
#include "uci_set.h"
#include "uci_reg.h"

#include "mcbcc_mst_driver.h"
#include "xscutimer.h"
#include "global.h"
#include "cons_prod_util.h"
#include "xscugic.h"
#include "ge_sitau_uci.h"

#include "xil_cache.h"
#include "mmu_config.h"
#include "timer_util.h"
#include "timestamp.h"

#include "trigger.h"
#include "version_utils.h"
#include "uci_phased_array.h"
#include "uci_acquire.h"
#include "uci_dma.h"
#include "afe_bussar.h"
#include "pulser_bussar.h"
#include "bcc_bussar.h"
#include "timestamp.h"
#include "TLV5626.h"
#include "switch_driver.h"
#include "axil2stream_fifo_driver.h"
#include "tgc_bussar.h"
#include "afe5808a.h"
#include "tgc_bussar.h"
#include "afe_bussar.h"
#include "vch.h"
#include "axi2gtx.h"
#include "ACQ_FMC.h"
#include "uci_fsm.h"
#include "gtx_control.h"

/*
 * memory_test.c: Test memory ranges present in the Hardware Design.
 *
 * This application runs with D-Caches disabled. As a result cacheline requests
 * will not be generated.
 *
 * For MicroBlaze/PowerPC, the BSP doesn't enable caches and this application
 * enables only I-Caches. For ARM, the BSP enables caches by default, so this
 * application disables D-Caches before running memory tests.
 */
#define INTC_DIST_BASE_ADDR	XPAR_SCUGIC_DIST_BASEADDR
#define TAM_BUFFER_TEMP	64
void putnum(unsigned int num);
volatile int new_host_data=0;

extern u32 MMUTable;

#define CALIB_AFES_DYN	1



void tgc_test()
{
	volatile u32 data_read;
	int num_ptos=0,i,j,valor=0;
	u8 tgc_values[500];
	TGC_UT_struct_t tgc_regs;
	//u32 *puntero_remoto=(u32*)(0x43C3041C);

	//BCC_write_reg_inmediate(0,4,0,0x00000001);
	//i=BCC_read_reg(1,4,0,&data_read);

	BCC_write_reg_inmediate(1,4,2,0x0000FFFF);
	i=BCC_read_reg(1,4,2,&data_read);

	BCC_write_reg_inmediate(1,4,2,0x00000010);
	i=BCC_read_reg(1,4,2,&data_read);

	BCC_write_reg_inmediate(1,4,2,0x0000FFFF);
	i=BCC_read_reg(1,4,2,&data_read);

	BCC_write_reg_inmediate(1,4,2,0x00000020);
	i=BCC_read_reg(1,4,2,&data_read);

	BCC_write_reg_inmediate(1,4,2,0x000FFFFF);
	i=BCC_read_reg(1,4,2,&data_read);

	for(i=0;num_ptos<128;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor++;

	for(i=0;num_ptos<256;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor--;
	tgc_values[0]=0x40;

	for(i=0;num_ptos<384;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor++;

	for(i=0;num_ptos<500;num_ptos++,i++)
		tgc_values[num_ptos]=2*valor--;
	/*
	for(i=0;num_ptos<768;num_ptos++,i++)
			tgc_values[num_ptos]=255-i;

	for(i=0;num_ptos<1024;num_ptos++,i++)
			tgc_values[num_ptos]=255-i;
*/
	i=BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);

	prog_tgc_curve(tgc_values, num_ptos, 0, 1, &Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);

	for(i=0;i<40;i++)
	{
		BCC_write_reg_inmediate(1,4,TGC_MEM_PNTR_R,i);
		for(j=0;j<50;j++);
		BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
		xil_printf("\n\rTGC_MEM_PNTR_R: 0x%08X\n\r",data_read);
		BCC_read_reg(1,4,TGC_MEM_DATA_R,&data_read);
		xil_printf("TGC_MEM_DATA_R: 0x%08X\n\r",data_read);
	}

	i=BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);

	Global_commnd_buf.total_len = 0;

	tgc_regs.tgc_ini_delay = 10;
	tgc_regs.tgc_mem_ini_end.BITS.tgc_mem_ini = 0;
	tgc_regs.tgc_mem_ini_end.BITS.tgc_mem_end = num_ptos-1;
	tgc_regs.tgc_general_ctrl.tgc_general_ctrl_u32 = 0;
	tgc_regs.tgc_general_ctrl.BITS.tgc_enable = 0;
	tgc_regs.tgc_general_ctrl.BITS.stop = 1;
	tgc_regs.tgc_general_ctrl.BITS.software_trigger = 0;//En cuanto mandemos esta configuracion se disparara
	tgc_regs.tgc_prescaler = 0;

	prog_tgc_UT_regs(&tgc_regs,1,&Global_commnd_buf);
	BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len = 0;

	i=BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);

	//for(i=0;i<4;i++)
		BCC_write_reg_inmediate(1,4,0,0x000a0001);   //   TGC EN, el a es innecesario
		BCC_write_reg_inmediate(1,4,0,0x880a0001);	 //	  SW Trigger y TGC EN, el segundo 8 y el a es innecesario
	for(i=0;i<40;i++)
	{
		BCC_read_reg(1,4,TGC_GEN_R,&data_read);
		xil_printf("\n\rTGC_GEN_R:      0x%08X\n\r",data_read);
		BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
		xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);
	}

	xil_printf("Quitamos bit de test...\n\r");

	BCC_write_reg_inmediate(1,4,TGC_GEN_R,0xF0000001); //Se escribe en todo: SW TRIG, EXT TRIG MASK, END IGNORE Y STOP,
														// viendo el HW debería tener prioridad el stop

	for(i=0;i<40;i++)
		{
			BCC_read_reg(1,4,TGC_GEN_R,&data_read);
			xil_printf("\n\rTGC_GEN_R:      0x%08X\n\r",data_read);
			BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
			xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);
		}
	i=BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
	xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);
}

prueba_AFES_TGC()
{
	AFE_UT_t afe_regs;
	u32 value, i;
	u8 minibase_addr=1;
	hw_uint32_ptr afe5808a_0;
	float tiempo_acq_us;
	int n_muestras;

	ctrl_afe_t afe_ctrl_reg;


	n_muestras = 900;


	{
		afe5808a_0=MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);

		config_afe(afe5808a_0); //Aplicamos la configuracion por defecto de afe5808a.c



		afe_ctrl_reg.REG = 0;
		afe_ctrl_reg.BIT.external_trigger = 1;

		afe5808a_0[AFE_WATER_DELAY_REG_OFFSET] = 0;
		afe5808a_0[AFE_ACQ_REG_OFFSET]         = n_muestras;
		afe5808a_0[AFE_CTRL_REG_OFFSET]        = afe_ctrl_reg.REG;

		MCBCC_deactivate_emu();
	}
	{
		TGC_UT_struct_t tgc_regs;
		volatile u32 data_read;
		int num_ptos=0,i,j,valor=0;
		u16 tgc_values[128];
		int num_puntos_tgc=128;
		int prescaler;
		int valor_max_TGC = 511;

		prescaler = ((n_muestras*2)/num_puntos_tgc)-1;
		if(prescaler<0) prescaler=0; //No va a suceder.

		for(i=0;i<=num_puntos_tgc/2;i++)
		{
			tgc_values[i]=i*511/(num_puntos_tgc/2);
			tgc_values[num_puntos_tgc-i]=i*511/(num_puntos_tgc/2);
		}
//		for(i=0;i<=num_puntos_tgc/2;i++)
//		{
//			if(i<16) tgc_values[i]=0;
//			else if (i<200) tgc_values[i]=500;
//			else tgc_values[i]=200;
//		}

		i=BCC_read_reg(1,4,TGC_MEM_PNTR_R,&data_read);
		xil_printf("TGC_MEM_PNTR_R: 0x%08X\n\r",data_read);

		prog_tgc_curve(tgc_values, num_puntos_tgc, 0, 1, &Global_commnd_buf);
		BCC_SendBuffer(&Global_commnd_buf);


		tgc_regs.tgc_ini_delay = 0;
		tgc_regs.tgc_mem_ini_end.BITS.tgc_mem_ini = 0;
		tgc_regs.tgc_mem_ini_end.BITS.tgc_mem_end = num_ptos-1;
		tgc_regs.tgc_general_ctrl.tgc_general_ctrl_u32 = 0;
		tgc_regs.tgc_general_ctrl.BITS.tgc_enable = 1;
		tgc_regs.tgc_general_ctrl.BITS.external_trigger = 1;
		tgc_regs.tgc_general_ctrl.BITS.stop = 1;
		tgc_regs.tgc_general_ctrl.BITS.software_trigger = 0;//En cuanto mandemos esta configuracion se disparara
		tgc_regs.tgc_prescaler = prescaler;

		prog_tgc_UT_regs(&tgc_regs,1,&Global_commnd_buf);
		BCC_SendBuffer(&Global_commnd_buf);
		Global_commnd_buf.total_len = 0;
	}

	MCBCC_set_PRO(1);
	MCBCC_set_PRO(0);

}

void desactivar_la_cosa_rara(void)
{

	/* Write to ACTLR */
	__asm__("mrc	p15, 0, r0, c1, c0, 1		/* Read ACTLR*/ \r\n\
			 bic	r0, r0, #(0x01 << 6)		/* unset SMP bit */\r\n\
			 mcr	p15, 0, r0, c1, c0, 1		/* Write ACTLR*/\r\n");
}
void activar_la_cosa_rara(void)
{

	/* Write to ACTLR */
	__asm__("mrc	p15, 0, r0, c1, c0, 1		/* Read ACTLR*/ \r\n\
			 orr	r0, r0, #(0x01 << 6)		/* unset SMP bit */\r\n\
			 mcr	p15, 0, r0, c1, c0, 1		/* Write ACTLR*/\r\n");
}
void desactivar_la_cosa_rara2(void)
{

	/* Write to ACTLR */
	__asm__("mrc	p15, 0, r0, c1, c0, 1		/* Read ACTLR*/ \r\n\
			 bic	r0, r0, #(0x01 )		/* Cache/TLB maintenance broadcast deactivated*/\r\n\
			 mcr	p15, 0, r0, c1, c0, 1		/* Write ACTLR*/\r\n");
}
void interrupcion_datos_en_el_buffer(void)
{
	new_host_data=1;
	dbg_printf("INT CPU0 to CPU1\r\n");
}
init_pulser_simple()
{
	volatile int ret;
	u16 retardos_calc[32*1024];
	u16 retardos_cano[32*1024];

	float angulos[256];
	float* ang_p;
	int n_ang=31,i;
	float d=0.3;
	float c=1.5;
	ang_p=angulos;

	for(i=0;i<32;i++)
		*ang_p++= i* 1.0 - 15;

	n_ang=(u16)(ang_p-angulos);
	//ret=retardos_emision_FMC32(retardos_calc);
	//ret=retardos_emision_PLANO(retardos_calc);
	ret=retardos_emision_PWIC(retardos_calc,angulos,d,c,n_ang);
	//ordena_canales(retardos_cano,retardos_calc,n_ang);

	xil_printf("\r\n");
	xil_printf("d = %d\r\n",d);
	xil_printf("c = %d\r\n",c);
	xil_printf("ANGULOS:\r\n",c);
	for(i=0;i<n_ang;i++)
		printf("%f ",angulos[i]);
	xil_printf("\r\nRETARDOS NORMAL:");
	print_retardos(retardos_calc, n_ang);
	xil_printf("\r\nRETARDOS CANON:");
	print_retardos(retardos_cano, n_ang);

	ret=prog_pulser_focal_laws(retardos_cano,ret,0,1,&Global_commnd_buf);

	ret=BCC_SendBuffer(&Global_commnd_buf);
	Global_commnd_buf.total_len = 0;
}

void init_system()
{
	volatile u32* ptr;
	u32 delay;
	version_control_init();
	if(gic_utils_init()) dbg_printf("Problema en el inicio del GIC\r\n");
	Xil_ExceptionDisable();
	if(timer_init_all_default()<0) dbg_printf("Problema en el inicio del los Timers\r\n");
	if(mutex_utils_init()) dbg_printf("Problema en el inicio del Mutex\r\n");
	cons_prod_init_structs();
	MCBCC_init();
	DATAMOV_init_structs();
	DMA_init(&Datamover_instances[LOCAL_DATAMOVER_ID]);
	timer_init_all_default();
	timestamp_init();
	TRIG_init();
	axil2stream_fifo_init();
	axi2gtx_fifo_init();

	enable_caches();
	HOST2UCI_cons_p=cons_init(HOST2UCI_BUF_ID,interrupcion_datos_en_el_buffer);
	UCI2HOST_prod_p=prod_init(UCI2HOST_BUF_ID);
	IMAGE_prod_p=prod_init(IMAGE_BUF_ID);

	//Por defecto, los datos no pasan por el filtro
  	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[DSP_DMA_SWITCH_TABLE_IDX],DSP_DMA_SWITCH_MASTER_TO_DDRZYN_IDX,DSP_DMA_SWITCH_SLAVE_FROM_BCCMST_IDX,0);

  	ptr=(u32*)XPAR_AXI_GPIO_DSR_BASEADDR;
#if 1
  	delay = 62; //Esto hay que cambiarlo según
  	ptr[0]=15;
  	ptr[2]=0x2;
  	ptr[2]=0x0;
  	ptr[0]=delay<<4|14;
  	ptr[2]=0x8;
  	ptr[2]=0x1;
#else
  	ptr[0]=15;
  	ptr[2]=0x2;
  	ptr[2]=0x0;
#endif
	Xil_ExceptionEnable();
}


int main(void)
{
//int i;//, result,num_bases;
//u32 waited_trigs;
int result;
u32 num_datos_to_read=0;
u32* ptr;

	version_control_init();
	print_version();
	UCI_Alarm2(0);

	SHRD_CPU1_Loaded(1);
	SHRD_CPU1_StatusEnabled(gb_uci.status_enabled);
	UCI_Alarm2(1);

	xil_printf("---Starting UCI---\r\n");
	xil_printf("Iniciando MMMU...\r\n");

	cpu1_uci_mmu_config();

	xil_printf("MMMU iniciada.\r\n");
	enable_caches();

	init_system();
	MCBCC_TRIG2PRO(0);
	UCI_set_Sync_is_PRO();
	enable_caches();

	enable_all_UCI_gtx(0); // por si acaso desactivamos GTX UCI

	TIMER_set_periodic_count(1000.0,TIMER_SCAN_PRF_1);

	TIMER_Start(TIMER_SCAN_PRF_1);

	ptr = (u32*)XPAR_PWR_GPIO_BASEADDR;
	*ptr = 0x3;

	timer_set_one_count(0.2,4);
	timer_start(4);

	xil_printf("Descargando...\r\n");
	while(!Timer_flag[4]) WFI_timer();

	ptr = (u32*)XPAR_PWR_GPIO_BASEADDR;
	*ptr = 0x0;

    timer_set_one_count(0.2,4);
    timer_start(4);

    xil_printf("Encendiendo fuente de alta a (0 V)...\r\n");
    while(!Timer_flag[4]) WFI_timer();

    TLV5626_ini((u32*)XPAR_TLV5626_AXI_LITE_TOP_0_BASEADDR);
    timer_set_one_count(0.2,4);
    timer_start(4);

    xil_printf("...");
    while(!Timer_flag[4]) WFI_timer();
	TLV5626_value((u32*)XPAR_TLV5626_AXI_LITE_TOP_0_BASEADDR,0);
	xil_printf("Fuente encendida\r\n");
	TIMER_Stop(TIMER_SCAN_PRF_1);
	SHRD_CPU1_HighVoltage(1);

    if ((result = VCH_Reset()) < 0) RLOG(result);
    UCI_SetupDefault();
    UCI_ResetConfig();
	
	SHRD_CPU1_FSM_Started(1);
	UCI_Alarm2(0);
    while(1)
	{
		if (cons_num_data_to_read(HOST2UCI_cons_p) > 0)
		{
			new_host_data=0;
			if (UCI_Receive() < 0)
			{
				if (UCI_Receive() < 0)
				{
					num_datos_to_read = cons_num_data_to_read(HOST2UCI_cons_p);
					if(num_datos_to_read)
					{
						cons_refresh_read(HOST2UCI_cons_p, num_datos_to_read);
						xil_printf("\r\nFree Unknown Data: %d", num_datos_to_read);
					}
				}
			}
		}
		else
		{
			if (gb_uci.status_enabled == 1) SHRD_UpdateStatus();
			if (TRIG_get_waited_triggers() == 0 && new_host_data == 0)
				WFI_main();
		}
		UCIFSM_main();
	}

    xil_printf("---Exiting UCI---\r\n");
    for(;;);

    return 0;
}

