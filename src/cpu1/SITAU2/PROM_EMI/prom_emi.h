/*
 * prom_emi.h
 *
 *  Created on: 22 dic. 2020
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_PROM_EMI_PROM_EMI_H_
#define SRC_SITAU2_PROM_EMI_PROM_EMI_H_

#include "xil_types.h"
#include "bcc_bussar.h"

#define EMI_PROM_CONTROL_REG	0
#define EMI_PROM_ESTADO_REG		1

#define EMI_PROM_MAN_CS_EMIPROMBF0	0
#define EMI_PROM_MAN_CS_BFDATAMOV0	1
#define EMI_PROM_MAN_CS_LVDSDATAMOV	2
#define EMI_PROM_MAN_CS_BEAMFORMER	3

typedef struct emi_prom_config_s
{
	union
	{
		u32 emi_prom_config_u32;
		struct
		{
			u32 num_rpt		: 8;
			u32 enable		: 1;//NO SE USA Y EL MODO AUTO TAMPOCO LO USA, PIN VACÍO
			u32 prom		: 1;
			u32 emi			: 1;
			u32 mem2prom32	: 1;//NO SE USA Y EL MODO AUTO TAMPOCO LO USA, PIN VACÍO
			u32 prom2mem16	: 1;//NO SE USA Y EL MODO AUTO TAMPOCO LO USA, PIN VACÍO
			u32 first_rpt	: 1;
			u32 last_rpt	: 1;
			u32 auto_mode	: 1;
			u32 dsr 		: 4;
			u32 reserved0	: 4;
			u32 ce_bits		: 8;
		}BITS;
	};
}emi_prom_config_t;


typedef struct emi_prom_estado_s
{
	union
	{
		u32 emi_prom_estado_u32;
		struct
		{
			u32 num_rpt_cnt	: 8;
			u32 reserved0	: 16;
			u32 last_error	: 1;
			u32 overflow	: 1;
			u32 reserved1	: 6;
		}BITS;
	};
}emi_prom_estado_t;

int emi_prom_prog(emi_prom_config_t *emi_prom_config_p,u8 minibase,commnd_buffer_t *commnd_buf_p);
emi_prom_estado_t emi_prom_prog_get_errors(u8 minibase);
void emi_prom_prog_inmediat(emi_prom_config_t *emi_prom_config_p,u8 minibase);


#endif /* SRC_SITAU2_PROM_EMI_PROM_EMI_H_ */
