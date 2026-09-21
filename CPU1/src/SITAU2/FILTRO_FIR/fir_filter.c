/*
 * fir_filter.c
 *
 *  Created on: 1 sept. 2020
 *      Author: Cruza
 */


#include "xil_types.h"
#include "switch_driver.h"
#include "mcbcc_mst_driver.h"
#include "axil2stream_fifo_driver.h"
#include "fir_filter.h"
#include "bussar_addr.h"

s16 hilbert_coef_old[FIR_NUM_COEF_SIMETRICOS] = {
		0xFFF7,
		0x0000,
		0xFFEC,
		0x0000,
		0xFFD8,
		0x0000,
		0xFFB7,
		0x0000,
		0xFF84,
		0x0000,
		0xFF3A,
		0x0000,
		0xFED2,
		0x0000,
		0xFE42,
		0x0000,
		0xFD80,
		0x0000,
		0xFC79,
		0x0000,
		0xFB12,
		0x0000,
		0xF917,
		0x0000,
		0xF61A,
		0x0000,
		0xF0FE,
		0x0000,
		0xE5A1,
		0x0000,
		0xAEC8,
		0x0000};	// 0

s16 hilbert_coef[FIR_NUM_COEF_SIMETRICOS] = {-28,0,-18,0,-51,0,-104,0,-179,0,-280,0,-409,0,-575,0,-786,0,-1058,0,-1417,0,-1914,0,-2659,0,-3939,0,-6812,0,-20813,0};

s16 filtro_unidad_casi[FIR_NUM_COEF_SIMETRICOS] =
{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0x7FFF
};
s16 filtro_mitad[FIR_NUM_COEF_SIMETRICOS] =
{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0x4000
};

s16 filtro_barbaro[FIR_NUM_COEF_SIMETRICOS] =
{
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF,
		0x7FFF
};

int calc_interleaving(int dec_beamformer)
{
	switch(dec_beamformer)
	{
		case 4:
			return 0;
		case 8:
			return 1;
		case 16:
			return 2;
		case 32:
			return 3;
		default:
			return -1;
	}
}

int config_interleaving(u8 interleaving,u8 video)
{
	u8* switch_baseaddr = stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX];
	u32 interleaving32=(u32)(interleaving<<24|interleaving<<16|interleaving<<8|interleaving);

	switch_set_master_2_slave_enable_connect(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_FIR_CONFIG_IDX,REGFIFO_SWITCH_REGFIFO_SLAVE_FROM_AXILFIFO_IDX,0);
	axil2stream_fifo_send(&interleaving32,1);
	switch_disable_master(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_FIR_CONFIG_IDX);
	if(video)
	{
		switch_set_master_2_slave_enable_connect(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_HIL_CONFIG_IDX,REGFIFO_SWITCH_REGFIFO_SLAVE_FROM_AXILFIFO_IDX,0);
		axil2stream_fifo_send(&interleaving32,1);
		switch_disable_master(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_HIL_CONFIG_IDX);
	}

	//Ajuste retardo filtro FIR y Hilbert
	{
		u32 *ptr=(u32*)XPAR_AXI_GPIO_DSR_BASEADDR;
		u16 delay;
		u8 bit_shift_DSR;

		bit_shift_DSR=15;

		delay = 62*(1<<interleaving);

		ptr[2]=0x2;
		ptr[2]=0x0;
		ptr[0]=(delay<<4)|(bit_shift_DSR&0xF);
		ptr[2]=0x8;
		ptr[2]=0x1;
	}

	return 0;
}

int config_FIR_coeficients(s16* coefficients)
{
	int i;
	u8* switch_baseaddr = stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX];
	s32 copia_filtros_s32[32];//Copiamos a un buffer de 32 bits. Aunque no se hace uso de los 32, hay que enviarlos así.
	for(i=0;i<32;i++)
		copia_filtros_s32[i]=(s32)coefficients[i];

	switch_set_master_2_slave_enable_connect(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_FIR_RELOAD_IDX,REGFIFO_SWITCH_REGFIFO_SLAVE_FROM_AXILFIFO_IDX,0);
	axil2stream_fifo_send((u32*)copia_filtros_s32,32);
	switch_disable_master(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_FIR_RELOAD_IDX);

	return 0;
}

int config_HIL_coeficients(s16* coefficients)
{
	int i;
	u8* switch_baseaddr = stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX];
	s32 copia_filtros_s32[32];//Copiamos a un buffer de 32 bits. Aunque no se hace uso de los 32, hay que enviarlos así.
	for(i=0;i<32;i++)
		copia_filtros_s32[i]=(s32)coefficients[i];

	switch_set_master_2_slave_enable_connect(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_HIL_RELOAD_IDX,REGFIFO_SWITCH_REGFIFO_SLAVE_FROM_AXILFIFO_IDX,0);
	axil2stream_fifo_send((u32*)copia_filtros_s32,32);
	switch_disable_master(switch_baseaddr,REGFIFO_SWITCH_MASTER_TO_HIL_RELOAD_IDX);

	return 0;
}

int config_FIR_coeficients_remote_get_commands(s16* coefficients,commnd_buffer_t *commnd_buf)
{
	int i;
	int ret;
	int num_words=0;
	u8 addr_broadcast=0;
	u8* switch_baseaddr = stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX];
	bcc_header_t command_write_broadcast;

	ret=switch_remote_032_select_slave_get_command(addr_broadcast,SSWITCH032_MASTER_TO_COE_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0,commnd_buf);
	if(ret<0)
		return ret;
	num_words+=ret;

	ret=switch_remote_032_update_get_command(0, commnd_buf);
	if(ret<0)
		return ret;

	num_words+=ret;
	command_write_broadcast.HEADER=0x0;
	command_write_broadcast.STREAM_CMD.CTRL.BITS.ROS = 1;
	command_write_broadcast.STREAM_CMD.CTRL.BITS.WR  = 1;
	command_write_broadcast.STREAM_CMD.LENGHT=32;
	command_write_broadcast.STREAM_CMD.MODULE=addr_broadcast;

	num_words+=push_bussar_u32(command_write_broadcast.HEADER, commnd_buf);

	for(i=0;i<32;i++)
		num_words+=push_bussar_u32((u32)coefficients[i],commnd_buf);

	ret=switch_remote_032_select_slave_get_command(addr_broadcast,SSWITCH032_MASTER_TO_COE_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,1,commnd_buf);
	if(ret<0)
		return ret;
	num_words+=ret;
	ret=switch_remote_032_update_get_command(addr_broadcast, commnd_buf);
	if(ret<0)
		return ret;

	return num_words;
}

int config_FIR_interleaving_remote_get_commands(commnd_buffer_t *commnd_buf)
{
	int i;
	int ret;
	int num_words=0;
	u8 addr_broadcast=0;
	u8* switch_baseaddr = stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX];
	bcc_header_t command_write_broadcast;
	u8 interleaving=0;

	s32 copia_filtros_s32[32];//Copiamos a un buffer de 32 bits. Aunque no se hace uso de los 32, hay que enviarlos así.

	ret=switch_remote_032_select_slave_get_command(addr_broadcast,SSWITCH032_MASTER_TO_CFG_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0,commnd_buf);
	if(ret<0)
		return ret;
	num_words+=ret;

	ret=switch_remote_032_update_get_command(addr_broadcast, commnd_buf);
	if(ret<0)
		return ret;

	num_words+=ret;
	command_write_broadcast.HEADER=0x0;
	command_write_broadcast.STREAM_CMD.CTRL.BITS.ROS = 1;
	command_write_broadcast.STREAM_CMD.CTRL.BITS.WR  = 1;
	command_write_broadcast.STREAM_CMD.LENGHT=1;
	command_write_broadcast.STREAM_CMD.MODULE=addr_broadcast;

	num_words+=push_bussar_u32(command_write_broadcast.HEADER, commnd_buf);

	num_words+=push_bussar_u32((u32)interleaving,commnd_buf);

	ret=switch_remote_032_select_slave_get_command(addr_broadcast,SSWITCH032_MASTER_TO_CFG_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,1,commnd_buf);
	if(ret<0)
		return ret;

	num_words+=ret;


	return num_words;
}
int config_FIR_bit_shift_remote_get_commands(u8 bit_shift,commnd_buffer_t *commnd_buf)
{
	u8 addr_broadcast=0;
	return push_bussar_write_reg((1<<MISC_FIL_SHFT_BIT_CE)|(bit_shift&MISC_FIL_SHFT_MASK), addr_broadcast, MISC_BUSSAR_SUBMOD_ADDR, MISC_FIL_SHFT_REG, commnd_buf);
}
