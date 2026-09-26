/*
 * switch_driver.c
 *
 *  Created on: 5 nov. 2019
 *      Author: csic
 */
#include "xil_types.h"
#include "xaxis_switch.h"
#include "switch_driver.h"
#include "mcbcc_mst_driver.h"
#include "bussar_addr.h"

u8* stream_switch_base_addrs[NUM_SWITCHES] =
{
		XPAR_AXIS_SWITCH_0_BASEADDR,
		XPAR_AXIS_SWITCH_1_BASEADDR,
		XPAR_AXIS_SWITCH_2_BASEADDR,
		(XPAR_UCI_AXI_MASTER_BASEADDR+BUSSAR_AXIL_BASE)
};

int switch_set_master_2_slave_enable_connect(u8 *switch_baseaddress, u8 master_out, u8 slave_in,u8 disable)
{
	u32 *ptr=(u32*) (switch_baseaddress+XAXIS_SCR_MI_MUX_START_OFFSET+master_out*4);
	u32 temp=0;
	temp|=disable?XAXIS_SCR_MI_X_DISABLE_MASK:0;
	temp|=XAXIS_SCR_MI_X_MUX_MASK&slave_in;

	*ptr = temp;
	switch_reg_update(switch_baseaddress);
	return 0;
}

int switch_disable_master(u8 *switch_baseaddress, u8 master_out)
{
	u32 *ptr=(u32*) (switch_baseaddress+XAXIS_SCR_MI_MUX_START_OFFSET+master_out*4);

	*ptr = XAXIS_SCR_MI_X_DISABLE_MASK;
	switch_reg_update(switch_baseaddress);
	return 0;
}

int switch_reg_update(u8 *switch_baseaddress)
{
	u32 *ptr=(u32*) (switch_baseaddress+XAXIS_SCR_CTRL_OFFSET);

	*ptr=XAXIS_SCR_CTRL_REG_UPDATE_MASK;

	return 0;
}

int switch_remote_032_select_slave_get_command(u8 minibase_addr,u8 master_out, u8 slave_in,u8 disable,commnd_buffer_t *commnd_buf)
{
	u8 reg_num=(XAXIS_SCR_MI_MUX_START_OFFSET+master_out*4)/4;
	u32 regval=(XAXIS_SCR_MI_X_MUX_MASK&slave_in)|(disable?XAXIS_SCR_MI_X_DISABLE_MASK:0);
	int num_words=0;

	num_words+=push_bussar_write_reg(regval,minibase_addr,SSWITCH032_BUSSAR_SUBMOD_ADDR,reg_num,commnd_buf);

	return num_words;
}

int switch_remote_032_update_get_command(u8 minibase_addr,commnd_buffer_t *commnd_buf)
{
	u8 reg_num = XAXIS_SCR_CTRL_OFFSET/4;
	u32 regval = XAXIS_SCR_CTRL_REG_UPDATE_MASK;
	int num_words=0;

	num_words+=push_bussar_write_reg(regval,minibase_addr,SSWITCH032_BUSSAR_SUBMOD_ADDR,reg_num,commnd_buf);

	return num_words;
}

int switch_remote_256_select_slave_get_command(u8 minibase_addr,u8 master_out, u8 slave_in,u8 disable,commnd_buffer_t *commnd_buf)
{
	u8 reg_num=(XAXIS_SCR_MI_MUX_START_OFFSET+master_out*4)/4;
	u32 regval=(XAXIS_SCR_MI_X_MUX_MASK&slave_in)|(disable?XAXIS_SCR_MI_X_DISABLE_MASK:0);
	int num_words=0;

	num_words+=push_bussar_write_reg(regval,minibase_addr,SSWITCH256_BUSSAR_SUBMOD_ADDR,reg_num,commnd_buf);

	return num_words;
}

int switch_remote_256_update_get_command(u8 minibase_addr,commnd_buffer_t *commnd_buf)
{
	u8 reg_num = XAXIS_SCR_CTRL_OFFSET/4;
	u32 regval = XAXIS_SCR_CTRL_REG_UPDATE_MASK;
	int num_words=0;

	num_words+=push_bussar_write_reg(regval,minibase_addr,SSWITCH256_BUSSAR_SUBMOD_ADDR,reg_num,commnd_buf);

	return num_words;
}


int switch_remote_512_select_slave_get_command(u8 minibase_addr,u8 master_out, u8 slave_in,u8 disable,commnd_buffer_t *commnd_buf)
{
	u8 reg_num=(XAXIS_SCR_MI_MUX_START_OFFSET+master_out*4)/4;
	u32 regval=(XAXIS_SCR_MI_X_MUX_MASK&slave_in)|(disable?XAXIS_SCR_MI_X_DISABLE_MASK:0);
	int num_words=0;

	num_words+=push_bussar_write_reg(regval,minibase_addr,SSWITCH512_BUSSAR_SUBMOD_ADDR,reg_num,commnd_buf);

	return num_words;
}

int switch_remote_512_update_get_command(u8 minibase_addr,commnd_buffer_t *commnd_buf)
{
	u8 reg_num = XAXIS_SCR_CTRL_OFFSET/4;
	u32 regval = XAXIS_SCR_CTRL_REG_UPDATE_MASK;
	int num_words=0;

	num_words+=push_bussar_write_reg(regval,minibase_addr,SSWITCH512_BUSSAR_SUBMOD_ADDR,reg_num,commnd_buf);

	return num_words;
}

