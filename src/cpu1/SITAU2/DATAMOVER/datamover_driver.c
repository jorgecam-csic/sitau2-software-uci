#include "datamover_driver.h"

const u16 Datamovers_interrupt_id_table[NUM_DATAMOVERS] =
{
		XPAR_FABRIC_DATAMOVER_CTRL_AXI_LITE_0_INTERRUPT_INTR,0
};

const u32* Datamovers_base_addrs[NUM_DATAMOVERS] =
{
		XPAR_DATAMOVER_CTRL_AXI_LITE_0_BASEADDR,
		(XPAR_UCI_AXI_MASTER_BASEADDR+BUSSAR_AXIL_BASE)
};

volatile datamov_flags_t Datamov_flags[NUM_DATAMOVERS];
datamov_instance_t Datamover_instances[NUM_DATAMOVERS];

void DATAMOV_put_mem(volatile u32* datamover_base_addr,void *dst_addr,u32 btt)
{

	datamover_base_addr[S2MM_STARTADDR]=(u32)dst_addr;
	datamover_base_addr[S2MM_COMMAND]=0xC0000000|btt;
}

void DATAMOV_get_mem(volatile u32* datamover_base_addr,void *src_addr,u32 btt)
{
	datamover_base_addr[MM2S_STARTADDR]=(u32)src_addr;
	datamover_base_addr[MM2S_COMMAND]=0xC0000000|btt;
}

void DATAMOV_set_tag_filter_S2MM(datamov_instance_t *datamover_instance_p,u8 tag,u8 enable)
{
	datamover_instance_p->S2MM_tag_filter_ena=enable;
	datamover_instance_p->S2MM_tag_filter=tag;
}

void DATAMOV_set_tag_filter_MM2S(datamov_instance_t *datamover_instance_p,u8 tag,u8 enable)
{
	datamover_instance_p->MM2S_tag_filter_ena=enable;
	datamover_instance_p->MM2S_tag_filter=tag;
}

void DATAMOV_put_mem_int(datamov_instance_t *datamover_instance_p,void *dst_addr,u32 btt,u8 tag,u8 last)
{
	u32 cmd_reg;
	u32 sts_reg;
	volatile u32* datamover_base_addr=datamover_instance_p->baseaddress;

	sts_reg=((tag<<16)&DATAMOVER_TGO_MASK)|DATAMOVER_IEN_MASK;

	cmd_reg=DATAMOVER_STR_MASK|(btt&DATAMOVER_BTT_MASK);
	if(last)
	{
		cmd_reg|=DATAMOVER_LST_MASK;
		DATAMOV_set_tag_filter_S2MM(datamover_instance_p,tag,1);
	}
	datamover_base_addr[S2MM_STARTADDR]=(u32)dst_addr;
	datamover_base_addr[S2MM_STATUS]=(u32)sts_reg;
	while(datamover_base_addr[S2MM_COMMAND]&DATAMOVER_QUE_MASK);
	datamover_instance_p->datamov_flags->DATAMOV_bit_flags.S2MM_end=0;
	datamover_base_addr[S2MM_COMMAND]=cmd_reg;
}

void DATAMOV_get_mem_int(datamov_instance_t *datamover_instance_p,void *src_addr,u32 btt,u8 tag,u8 last)
{
	u32 cmd_reg;
	u32 sts_reg;
	volatile u32* datamover_base_addr=datamover_instance_p->baseaddress;

	sts_reg=(tag<<16)|DATAMOVER_IEN_MASK;

	cmd_reg=DATAMOVER_STR_MASK|(btt&DATAMOVER_BTT_MASK);
	if(last)
	{
		cmd_reg|=DATAMOVER_LST_MASK;
		DATAMOV_set_tag_filter_MM2S(datamover_instance_p,tag,1);
	}

	datamover_base_addr[MM2S_STARTADDR]=(u32)src_addr;
	datamover_base_addr[MM2S_STATUS]=(u32)sts_reg;
	while(datamover_base_addr[MM2S_COMMAND]&DATAMOVER_QUE_MASK);
	datamover_instance_p->datamov_flags->DATAMOV_bit_flags.S2MM_end=0;
	datamover_base_addr[MM2S_COMMAND]=cmd_reg;
}

void DATAMOV_put_mem_no_int(datamov_instance_t *datamover_instance_p,void *dst_addr,u32 btt)
{
	volatile u32* datamover_base_addr=datamover_instance_p->baseaddress;

	datamover_base_addr[S2MM_STARTADDR]=(u32)dst_addr;
	datamover_base_addr[S2MM_COMMAND]=0xC0000000|btt;
}

void DATAMOV_get_mem_no_int(datamov_instance_t *datamover_instance_p,void *src_addr,u32 btt)
{
	volatile u32* datamover_base_addr=datamover_instance_p->baseaddress;

	datamover_base_addr[MM2S_STARTADDR]=(u32)src_addr;
	datamover_base_addr[MM2S_COMMAND]=0xC0000000|btt;
}

void DATAMOV_int_function(void *handler)
{
	datamov_instance_t *datamover_instance_p=(datamov_instance_t *)handler;
	volatile u32* datamover_base_addr=datamover_instance_p->baseaddress;
	u32 reg;

	//dbg_printf("DATAMOV INT!!\n\r");
	reg=datamover_base_addr[S2MM_STATUS];
	datamover_instance_p->S2MM_status_reg_int=reg;
	reg=datamover_base_addr[MM2S_STATUS];
	datamover_instance_p->MM2S_status_reg_int=reg;

	if(datamover_instance_p->S2MM_status_reg_int&DATAMOVER_END_MASK)
	{
		datamover_instance_p->S2MM_pending--;
		if(datamover_instance_p->S2MM_tag_filter_ena)
		{
			if(datamover_instance_p->S2MM_tag_filter==(datamover_instance_p->S2MM_status_reg_int&DATAMOVER_TGI_MASK))
				datamover_instance_p->datamov_flags->DATAMOV_bit_flags.S2MM_end=1;
		}
		else
			datamover_instance_p->datamov_flags->DATAMOV_bit_flags.S2MM_end=1;
	}
	if(datamover_instance_p->MM2S_status_reg_int&DATAMOVER_END_MASK)
	{
		datamover_instance_p->MM2S_pending--;
		if(datamover_instance_p->MM2S_tag_filter_ena)
		{
			if(datamover_instance_p->MM2S_tag_filter==(datamover_instance_p->MM2S_status_reg_int&DATAMOVER_TGI_MASK))
				datamover_instance_p->datamov_flags->DATAMOV_bit_flags.MM2S_end=1;
		}
		else
			datamover_instance_p->datamov_flags->DATAMOV_bit_flags.MM2S_end=1;
	}
}

void DATAMOV_int_setup(datamov_instance_t *datamover_instance_p)
{
	volatile u32* datamover_base_addr=datamover_instance_p->baseaddress;

	XScuGic_Connect(&Gic_instance, datamover_instance_p->int_id,(Xil_ExceptionHandler)DATAMOV_int_function,(void*)datamover_instance_p);

	XScuGic_InterruptMaptoCpu(&Gic_instance, THIS_CPU_ID, datamover_instance_p->int_id);
	XScuGic_SetPriorityTriggerType(&Gic_instance, datamover_instance_p->int_id,	0xA0, 0x3);
	XScuGic_Enable(&Gic_instance, datamover_instance_p->int_id);

	//Set_Up_Gic_Interrupt(&Gic_instance, THIS_CPU_GIC_MASK, datamover_instance_p->int_id);

	datamover_base_addr[S2MM_STATUS]=DATAMOVER_IEN_MASK;
	datamover_base_addr[MM2S_STATUS]=DATAMOVER_IEN_MASK;
}

void DATAMOV_init_structs(void)
{
	int i;
	for(i=0;i<NUM_DATAMOVERS;i++)
	{
		Datamover_instances[i].S2MM_status_reg_int=0;
		Datamover_instances[i].MM2S_status_reg_int=0;
		Datamover_instances[i].S2MM_pending=0;
		Datamover_instances[i].MM2S_pending=0;
		Datamover_instances[i].S2MM_tag_filter_ena=0;
		Datamover_instances[i].MM2S_tag_filter_ena=0;
		Datamover_instances[i].S2MM_tag_filter=0;
		Datamover_instances[i].MM2S_tag_filter=0;
		Datamover_instances[i].baseaddress=Datamovers_base_addrs[i];
		Datamover_instances[i].datamov_flags=&Datamov_flags[i];
		Datamover_instances[i].int_id=Datamovers_interrupt_id_table[i];
		Datamover_instances[i].datamov_id=i;
		if(Datamover_instances[i].int_id)		//Si vale 0 no inicializo
			DATAMOV_int_setup(&Datamover_instances[i]);
	}
}

u8 DATAMOV_poll_MM2S_end(datamov_instance_t *datamover_instance_p)
{
	return datamover_instance_p->datamov_flags->DATAMOV_bit_flags.MM2S_end;
}
u8 DATAMOV_poll_S2MM_end(datamov_instance_t *datamover_instance_p)
{
	return datamover_instance_p->datamov_flags->DATAMOV_bit_flags.S2MM_end;
}
u8 DATAMOV_S2MM_end_status(datamov_instance_t *datamover_instance_p)
{
	return (u8)(0xFF&datamover_instance_p->S2MM_status_reg_int);
}
u8 DATAMOV_MM2S_end_status(datamov_instance_t *datamover_instance_p)
{
	return (u8)(0xFF&datamover_instance_p->MM2S_status_reg_int);
}

