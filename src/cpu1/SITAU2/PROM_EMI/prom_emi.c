/*
 * prom_emi.c
 *
 *  Created on: 23 dic. 2020
 *      Author: Cruza
 */
#include "prom_emi.h"
#include "bcc_bussar.h"
#include "bussar_addr.h"

int emi_prom_prog(emi_prom_config_t *emi_prom_config_p,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	return push_bussar_write_reg(emi_prom_config_p->emi_prom_config_u32,minibase,EMI_PROM_SUBMOD_ADDR,EMI_PROM_CONTROL_REG,commnd_buf_p);
}


void emi_prom_prog_inmediat(emi_prom_config_t *emi_prom_config_p,u8 minibase)
{
	BCC_write_reg_inmediate(minibase,EMI_PROM_SUBMOD_ADDR,EMI_PROM_CONTROL_REG,emi_prom_config_p->emi_prom_config_u32);
}


emi_prom_estado_t emi_prom_prog_get_errors(u8 minibase)
{
	emi_prom_estado_t ret_estado;
	int ret;
	ret_estado.emi_prom_estado_u32=0;
	if((ret=BCC_read_reg(minibase,EMI_PROM_SUBMOD_ADDR,EMI_PROM_ESTADO_REG,&ret_estado.emi_prom_estado_u32))!=0)
	{
		xil_printf("No se ha podido leer el registro de errores de EMI_PROM\n\r");
	}

	return ret_estado;
}

