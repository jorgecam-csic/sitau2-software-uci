/*
 * ch_extract.c
 *
 *  Created on: 30 jul. 2020
 *      Author: Cruza
 */
#include "ch_extract.h"

int channel_extract_configure_basic(int valid_channels,int num_interleaved_channels,int num_samples_per_channel)
{
	u32 *Channel_extract_regs = (u32*)CH_EXTRACT_BASEADDRESS;
	u32 valid_bits;
	ch_extract_ctrl_t ctrl_reg;

	ctrl_reg.ch_extract_ctrl_u32=Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET];


	do {
		ctrl_reg.ch_extract_ctrl_u32=Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET];
	}while(ctrl_reg.BITS.start_enable==1);



	valid_bits = (1<<valid_channels)-1;
	ctrl_reg.ch_extract_ctrl_u32 = 0;
	ctrl_reg.BITS.samples_per_channel = num_samples_per_channel;
	ctrl_reg.BITS.num_interleaved_channels = (num_interleaved_channels>>1)-1;
	ctrl_reg.BITS.start_enable = 1;

	Channel_extract_regs[CH_EXTRACT_VALID_MAP_OFFSET] = valid_bits;
	Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET]   = ctrl_reg.ch_extract_ctrl_u32;

	ctrl_reg.ch_extract_ctrl_u32=Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET];

	//channel_extract_print_status(&ctrl_reg);

	if (ctrl_reg.BITS.start_enable==0)
	{
		xil_printf("ERROR en extractor de canales: No se ha acabado de programar \n\r");
		return -1; //Se ha intentado programar un número impar de canales
	}

	if(num_interleaved_channels%2)
	{
		xil_printf("ERROR en extractor de canales: Se está tratando de programar un número impar de canales entrelazados\n\r");
		return -1; //Se ha intentado programar un número impar de canales
	}

	return 0;
}
int channel_extract_configure_basic_no_test(int valid_channels,int num_interleaved_channels,int num_samples_per_channel)
{
	u32 *Channel_extract_regs = (u32*)CH_EXTRACT_BASEADDRESS;
	u32 valid_bits;
	ch_extract_ctrl_t ctrl_reg;

	ctrl_reg.ch_extract_ctrl_u32=Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET];


//	do {
//		ctrl_reg.ch_extract_ctrl_u32=Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET];
//	}while(ctrl_reg.BITS.start_enable==1);



	valid_bits = (1<<valid_channels)-1;
	ctrl_reg.ch_extract_ctrl_u32 = 0;
	ctrl_reg.BITS.samples_per_channel = num_samples_per_channel;
	ctrl_reg.BITS.num_interleaved_channels = (num_interleaved_channels>>1)-1;
	ctrl_reg.BITS.start_enable = 1;

	Channel_extract_regs[CH_EXTRACT_VALID_MAP_OFFSET] = valid_bits;
	Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET]   = ctrl_reg.ch_extract_ctrl_u32;

	ctrl_reg.ch_extract_ctrl_u32=Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET];

	//channel_extract_print_status(&ctrl_reg);

	if (ctrl_reg.BITS.start_enable==0)
	{
		xil_printf("ERROR en extractor de canales: No se ha acabado de programar \n\r");
		return -1; //Se ha intentado programar un número impar de canales
	}

	if(num_interleaved_channels%2)
	{
		xil_printf("ERROR en extractor de canales: Se está tratando de programar un número impar de canales entrelazados\n\r");
		return -1; //Se ha intentado programar un número impar de canales
	}

	return 0;
}


int  channel_extract_get_status(ch_extract_ctrl_t *ctrl_reg)
{
	u32 *Channel_extract_regs = (u32*)CH_EXTRACT_BASEADDRESS;
	ctrl_reg->ch_extract_ctrl_u32 = Channel_extract_regs[CH_EXTRACT_CONTROL_OFFSET];
	return ctrl_reg->BITS.start_enable;
}

void channel_extract_print_status(ch_extract_ctrl_t *ctrl_reg)
{
	if(ctrl_reg->BITS.start_enable)
	{
		xil_printf("\n\r");
		xil_printf("Extractor de canales ACTIVO\n\r");
		xil_printf("Quedan %d muestras por procesar, la siguiente muestra de entrada debería corresponder a los canales %d y %d.\n\r",ctrl_reg->BITS.samples_per_channel,(ctrl_reg->BITS.num_interleaved_channels)<<1,(ctrl_reg->BITS.num_interleaved_channels)<<1);
		xil_printf("El bit extra_data_error está a %d\n\r",ctrl_reg->BITS.extra_data_error);
		xil_printf("\n\r");
	}
	else
	{
		xil_printf("\n\r");
		xil_printf("Extractor de 0 NO ACTIVO\n\r");
		xil_printf("Programadas %d muestras para procesar de %d canales.\n\r",ctrl_reg->BITS.samples_per_channel,(ctrl_reg->BITS.num_interleaved_channels+1)<<1);
		xil_printf("El bit extra_data_error está a %d\n\r",ctrl_reg->BITS.extra_data_error);
		xil_printf("\n\r");
	}
}


