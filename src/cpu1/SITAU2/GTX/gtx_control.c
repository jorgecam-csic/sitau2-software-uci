/*
 * gtx_control.c
 *
 *  Created on: 24 ago. 2022
 *      Author: csic
 */
#include "gtx_control.h"
#include "switch_driver.h"
#include "bussar_addr.h"
#include "xparameters.h"
#include "mcbcc_mst_driver.h"

#define REG_GPIO_LINK_PRO_IN	2
#define REG_GPIO_LINK_PRO_OUT	0

#define LINK_PRO_OUT_MASK	1
#define LINK_PRO_IN_MASK	2

void mem2gtx_set_speed(u8 enable,u8 active_cycles,u8 total_cycles,u8 minibase)
{
	volatile u32* remote_addr;
	mem2gtx_speed_ctrl_reg_t reg_value;

	reg_value.mem2gtx_speed_ctrl_reg_32 = 0;
	reg_value.BITS.speed_control_enable = (enable?1:0);
	reg_value.BITS.active_cycles = active_cycles;
	reg_value.BITS.total_cycles = total_cycles;


	remote_addr=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);

	remote_addr[MISC_MEM2GTX_SPEED_REG] = reg_value.mem2gtx_speed_ctrl_reg_32;


	MCBCC_deactivate_emu();
}

void gtx_set_adv_regs(u8 minibase,u8 tx_diff,u8 lpm,u8 pma_tx_rst,u8 precursor,u8 postcursor,u8 dn_0_up_1)
{
	gtx_misc_adv_ctrl_reg_t gtx_adv_reg;

	dn_0_up_1=dn_0_up_1?1:0;

	gtx_adv_reg.gtx_misc_adv_ctrl_reg_32=0;
	gtx_adv_reg.BITS.tx_diff=tx_diff;
	gtx_adv_reg.BITS.lpm=lpm;
	gtx_adv_reg.BITS.pma_tx_rst=pma_tx_rst;
	gtx_adv_reg.BITS.precursor=precursor;
	gtx_adv_reg.BITS.postcursor=postcursor;

	BCC_write_reg_inmediate(minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_GTX_ADV_CTRL_DN+dn_0_up_1,gtx_adv_reg.gtx_misc_adv_ctrl_reg_32);
}

gtx_misc_fsm_ctrl_reg_t gtx_get_ctrl_reg(u8 minibase)
{
	gtx_misc_fsm_ctrl_reg_t register_ret;

	BCC_read_reg(minibase, MISC_BUSSAR_SUBMOD_ADDR, MISC_GTX_FSM_CTRL_REG, &register_ret);

	return register_ret;

}

gtx_status_counters_reg_t gtx_get_link_errors_dn(u8 minibase)
{
	volatile u32* remote_addr;
	gtx_status_counters_reg_t register_ret;
	gtx_status_counters_reg_t reset_counters_cmnd;

	remote_addr=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);

	register_ret.gtx_status_counters_reg_32 = remote_addr[MISC_DW_GTX_STS_REG];

	reset_counters_cmnd.gtx_status_counters_reg_32 = 0;
	//reset_counters_cmnd.BITS.reset_counters = 1;

	remote_addr[MISC_DW_GTX_STS_REG] = reset_counters_cmnd.gtx_status_counters_reg_32;
	reset_counters_cmnd.gtx_status_counters_reg_32 = 0;
	remote_addr[MISC_DW_GTX_STS_REG] = reset_counters_cmnd.gtx_status_counters_reg_32;

	MCBCC_deactivate_emu();

	return register_ret;
}

gtx_status_counters_reg_t gtx_get_link_errors_up(u8 minibase)
{
	volatile u32* remote_addr;
	gtx_status_counters_reg_t register_ret;
	gtx_status_counters_reg_t reset_counters_cmnd;

	remote_addr=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);

	register_ret.gtx_status_counters_reg_32 = remote_addr[MISC_UP_GTX_STS_REG];

	reset_counters_cmnd.gtx_status_counters_reg_32 = 0;
	//reset_counters_cmnd.BITS.reset_counters = 1;

	remote_addr[MISC_UP_GTX_STS_REG] = reset_counters_cmnd.gtx_status_counters_reg_32;
	reset_counters_cmnd.gtx_status_counters_reg_32 = 0;
	remote_addr[MISC_UP_GTX_STS_REG] = reset_counters_cmnd.gtx_status_counters_reg_32;

	MCBCC_deactivate_emu();

	return register_ret;
}

void gtx_print_all_errors(u8 num_bases)
{
	for(u8 base=1;base<=num_bases;base++)
	{
		gtx_status_counters_reg_t errors;
		errors = gtx_get_link_errors_up(base);
		xil_printf("\n\rBASE %2d",base);
		xil_printf("\n\rUPLINK:");
		gtx_print_errors(errors);
		errors = gtx_get_link_errors_dn(base);
		xil_printf("\n\rDOWNLINK:");
		gtx_print_errors(errors);
	}
}
void gtx_print_errors(gtx_status_counters_reg_t errors)
{
	xil_printf("\r\n");
	xil_printf("CHAN ERRORS: %3d\r\n",errors.BITS.channel_not_ready);
	xil_printf("SOFT ERRORS: %3d\r\n",errors.BITS.soft_errors);
	xil_printf("HARD ERRORS: %3d\r\n",errors.BITS.hard_errors);
}

void gtx_set_pro_link_to_gtx_control()
{
	BCC_write_reg_inmediate(0,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_LINK_REG,MISC_PRO_LINK_GTX_MASK);
}
void gtx_set_pro_link_to_lvds_control()
{
	BCC_write_reg_inmediate(0,MISC_BUSSAR_SUBMOD_ADDR,MISC_PRO_LINK_REG,MISC_PRO_LINK_LVDS_MASK|MISC_PRO_LINK_DISABLE_TRIG_SEC_MASK);
}

u32 gtx_get_link_pro_in()
{
	u32 *gpio_link_pro=(u32*)XPAR_AXI_GPIO_LINK_PRO_BASEADDR;
	return gpio_link_pro[REG_GPIO_LINK_PRO_IN]&LINK_PRO_IN_MASK;
}

void gtx_set_link_pro_out(u8 valor)
{
	u32 *gpio_link_pro=(u32*)XPAR_AXI_GPIO_LINK_PRO_BASEADDR;

	if(valor)
		gpio_link_pro[REG_GPIO_LINK_PRO_OUT]|=LINK_PRO_OUT_MASK;
	else
		gpio_link_pro[REG_GPIO_LINK_PRO_OUT]&=(~LINK_PRO_OUT_MASK);

}
void enable_all_UCI_gtx(int ena)
{
	volatile u32* GPIO_basereg;
	u32 valor_leido;

	GPIO_basereg=(volatile u32*) XPAR_MGT_CLK_GPIO_BASEADDR;

	valor_leido = GPIO_basereg[0];

	if(ena)
		GPIO_basereg[0] = valor_leido & 0xFFFFFF9F; //Activados GTX y hacemos reset a la máquina de estados
	else
		GPIO_basereg[0] = valor_leido | 0x00000060; //Desactivados GTX y hacemos reset a la máquina de estados

}

void enable_all_remote_gtx(int ena)
{
	volatile u32* remote_addr;

	u8 minibase;

	minibase=0;//TODOS los módulos
	remote_addr=MCBCC_activate_emu(minibase,MISC_BUSSAR_SUBMOD_ADDR);

	if(ena)
		remote_addr[MISC_GTX_FSM_CTRL_REG]=0x1000; //Activados GTX y hacemos reset a la máquina de estados
	else
		remote_addr[MISC_GTX_FSM_CTRL_REG]=0x1003; //Desactivados GTX y hacemos reset a la máquina de estados

	MCBCC_deactivate_emu();
}
int config_gtx(gtx_misc_fsm_ctrl_reg_t gtx_reg,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	int num_words=0;
	int ret_val;

	ret_val=push_bussar_write_reg((u32)gtx_reg.gtx_misc_fsm_ctrl_reg_32,minibase,MISC_BUSSAR_SUBMOD_ADDR,MISC_GTX_FSM_CTRL_REG,commnd_buf_p);

	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	return num_words;
}

void gtx_send_remote_repeat_header(u8 minibase_header,u8 last_behaviour)
{
	volatile u32* remote_addr;

	remote_addr=MCBCC_activate_emu(minibase_header,MISC_BUSSAR_SUBMOD_ADDR);

	if((remote_addr[MISC_GTX_HEADER_CTRL]&MISC_GTX_HEADER_CTRL_RDY_MASK)==0)
	{
		xil_printf("\n\rgtx_send_remote_header: Remote Header Register to Stream is not ready\n\r");
		while((remote_addr[MISC_GTX_HEADER_CTRL]&MISC_GTX_HEADER_CTRL_RDY_MASK)==0);
		xil_printf("\n\rgtx_send_remote_header: Remote Header Register to Stream is now ready\n\r");
	}


	if(last_behaviour)
		remote_addr[MISC_GTX_HEADER_CTRL]=MISC_GTX_HEADER_CTRL_LST_MASK|MISC_GTX_HEADER_CTRL_VLD_MASK;
	else
		remote_addr[MISC_GTX_HEADER_CTRL]=MISC_GTX_HEADER_CTRL_VLD_MASK;

	//while(!(remote_addr[MISC_GTX_HEADER_CTRL]&MISC_GTX_HEADER_CTRL_ACK_MASK));

	MCBCC_deactivate_emu();

}

void gtx_send_remote_header(u32* header_p,u32 header_size,u8 minibase_header,u8 last_behaviour)
{
	int i;
	volatile u32* remote_addr;

	remote_addr=MCBCC_activate_emu(minibase_header,MISC_BUSSAR_SUBMOD_ADDR);
	for(int i=0;i<header_size;i++)
		remote_addr[MISC_GTX_HEADER_DATA+i]=header_p[i];

	if((remote_addr[MISC_GTX_HEADER_CTRL]&MISC_GTX_HEADER_CTRL_RDY_MASK)==0)
	{
		xil_printf("\n\rgtx_send_remote_header: Remote Header Register to Stream is not ready\n\r");
		while((remote_addr[MISC_GTX_HEADER_CTRL]&MISC_GTX_HEADER_CTRL_RDY_MASK)==0);
		xil_printf("\n\rgtx_send_remote_header: Remote Header Register to Stream is now ready\n\r");
	}


	if(last_behaviour)
		remote_addr[MISC_GTX_HEADER_CTRL]=MISC_GTX_HEADER_CTRL_LST_MASK|MISC_GTX_HEADER_CTRL_VLD_MASK;
	else
		remote_addr[MISC_GTX_HEADER_CTRL]=MISC_GTX_HEADER_CTRL_VLD_MASK;

	//while(!(remote_addr[MISC_GTX_HEADER_CTRL]&MISC_GTX_HEADER_CTRL_ACK_MASK));

	MCBCC_deactivate_emu();

}
void gtx_remote_from_minibase_hea_down_to_host(u8 minibase_origen,u8 n_minibases)
{
	for(u8 minibase_paso=1;minibase_paso<=n_minibases;minibase_paso++)
	{
		MCBCC_activate_emu(minibase_paso,SSWITCHGTX_BUSSAR_SUBMOD_ADDR);
		if(minibase_paso<minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_DWTX_IDX,SSWITCHGTX_SLAVE_FROM_UPRX_IDX,0);
		else if (minibase_paso==minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_DWTX_IDX,SSWITCHGTX_SLAVE_FROM_HEA_IDX,0);
		else if (minibase_paso>minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_DWTX_IDX,SSWITCHGTX_SLAVE_FROM_UPRX_IDX,1);
		MCBCC_deactivate_emu();
	}
	MCBCC_activate_emu(minibase_origen,SSWITCH256_BUSSAR_SUBMOD_ADDR);
	switch_disable_master(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_512_IDX);
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_GTX_IDX,SSWITCH256_SLAVE_FROM_MEM_IDX,0);
	MCBCC_deactivate_emu();
}
void gtx_remote_from_minibase_mem_down_to_host(u8 minibase_origen,u8 n_minibases)
{
	for(u8 minibase_paso=1;minibase_paso<=n_minibases;minibase_paso++)
	{
		MCBCC_activate_emu(minibase_paso,SSWITCHGTX_BUSSAR_SUBMOD_ADDR);
		if(minibase_paso<minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_DWTX_IDX,SSWITCHGTX_SLAVE_FROM_UPRX_IDX,0);
		else if (minibase_paso==minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_DWTX_IDX,SSWITCHGTX_SLAVE_FROM_MEM_IDX,0);
		else if (minibase_paso>minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_DWTX_IDX,SSWITCHGTX_SLAVE_FROM_UPRX_IDX,1);
		MCBCC_deactivate_emu();
	}
	MCBCC_activate_emu(minibase_origen,SSWITCH256_BUSSAR_SUBMOD_ADDR);
	switch_disable_master(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_512_IDX);
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_GTX_IDX,SSWITCH256_SLAVE_FROM_MEM_IDX,0);
	MCBCC_deactivate_emu();
}
void gtx_remote_from_minibase_mem_up_to_uci(u8 minibase_origen,u8 n_minibases)
{

	for(u8 minibase_paso=1;minibase_paso<=n_minibases;minibase_paso++)
	{
		MCBCC_activate_emu(minibase_paso,SSWITCHGTX_BUSSAR_SUBMOD_ADDR);
		if(minibase_paso<minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_UPTX_IDX,SSWITCHGTX_SLAVE_FROM_DWRX_IDX,1);
		else if (minibase_paso==minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_UPTX_IDX,SSWITCHGTX_SLAVE_FROM_MEM_IDX,0);
		else if (minibase_paso>minibase_origen)
			switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_UPTX_IDX,SSWITCHGTX_SLAVE_FROM_DWRX_IDX,0);

		MCBCC_deactivate_emu();
	}
	MCBCC_activate_emu(minibase_origen,SSWITCH256_BUSSAR_SUBMOD_ADDR);
	switch_disable_master(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_512_IDX);
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_GTX_IDX,SSWITCH256_SLAVE_FROM_MEM_IDX,0);
	MCBCC_deactivate_emu();
}

void swgtx_remote_up_pass_throw_to_uci(int disable)
{
	MCBCC_activate_emu(0,SSWITCHGTX_BUSSAR_SUBMOD_ADDR);
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_UPTX_IDX,SSWITCHGTX_SLAVE_FROM_DWRX_IDX,disable);
	MCBCC_deactivate_emu();
}

void swgtx_remote_down_pass_throw_to_host(int disable)
{
	MCBCC_activate_emu(0,SSWITCHGTX_BUSSAR_SUBMOD_ADDR);
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCHGTX_MASTER_TO_DWTX_IDX,SSWITCHGTX_SLAVE_FROM_UPRX_IDX,disable);
	MCBCC_deactivate_emu();
}

void sw256_remote_mem_to_gtx_switch(u8 minibase,int disable)
{
	MCBCC_activate_emu(minibase,SSWITCH256_BUSSAR_SUBMOD_ADDR);
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_GTX_IDX,SSWITCH256_SLAVE_FROM_MEM_IDX,disable);
	MCBCC_deactivate_emu();
}

void sw256_remote_mem_to_512_switch(u8 minibase,int disable)
{
	MCBCC_activate_emu(minibase,SSWITCH256_BUSSAR_SUBMOD_ADDR);
	switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH256_MASTER_TO_512_IDX,SSWITCH256_SLAVE_FROM_MEM_IDX,disable);
	MCBCC_deactivate_emu();
}


