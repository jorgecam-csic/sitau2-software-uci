/*
 * pulser_bussar.h
 *
 *  Created on: 21 mar. 2019
 *      Author: Cruza
 */
#include "xil_types.h"
#include "bcc_bussar.h"

#ifndef SRC_SITAU2_PULSER_PULSER_BUSSAR_H_
#define SRC_SITAU2_PULSER_PULSER_BUSSAR_H_

#define PULSER_TICS_PER_US	(1000/4)

typedef union delay_mem_ini_end_u
{
	u32 delay_mem_ini_end_u32;
	struct
	{
		u16 delay_mem_ini;
		u16 delay_mem_end;
	}BITS;
}delay_mem_ini_end_t;

typedef struct pul_general_ctrl_s
{
	union
	{
		u32 pul_general_ctrl_u32;
		struct
		{
			u32 pul_enable : 1;
			u32 auto_prom : 1;
			u32 gnd0_hiz1 : 1;
			u32 pulser_load : 1;
			u32 pulser_auto_load : 1;
			u32 reserved2 : 5;
			u32 pulser_current : 2;
			u32 pulser_status : 4;
			u32 n_pulses : 8;
			u32 reserved3 : 4;
			u32 stop : 1;
			u32 external_trigger : 1;
			u32 not_busy : 1;
			u32 software_trigger : 1;
		}BITS;
	};
}pul_general_ctrl_t;
// typedef struct pulser_hw_struct_s
// {
	// pul_general_ctrl_t pul_general_ctrl;
	// u32 pulser_main_delay;
	// u32 pulse_width;
	// u32 reserved1;
	// delay_mem_ini_end_t delay_mem_ini_end;
	// u32 delay_mem_val;
	// u32 delay_mem_ptr;
	// u32 reserved2;
// }PULSER_HW_struct_t;

typedef struct pulser_ut_struct_s
{
	u32 pulse_width;
	u32 pulser_main_delay;
	delay_mem_ini_end_t delay_mem_ini_end;
	u32 delay_mem_ptr;
	pul_general_ctrl_t pul_general_ctrl;
}PULSER_UT_struct_t;

#define PULSER_GEN_R		0
#define PULSER_MAIN_DELAY_R 1
#define PUL_SHAPE_1_R 		2
#define PUL_SHAPE_2_R 		3
#define DELAY_MEM_PROG_R	4
#define DELAY_MEM_DATA_R	5
#define DELAY_MEM_ADDR_R	6
#define CODE_CONF_R			7

#define PULSER_CODE			0xC51CC0F1

int verifica_codigo_pulser(void);
void print_retardos(u16* retardos, int n_leyes_focales);
//void ordena_canales(u16* retardos_dst,u16* retardos_org,int n_leyes_focales);
int prog_pulser_UT_regs(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p);
int prog_pulser_focal_laws(u16* retardos,int num_retardos,u16 dir_ini,u8 minibase,commnd_buffer_t *commnd_buf_p);
int retardos_emision_FMC32(u16* puntero);
int retardos_emision_PLANO(u16* puntero);
int retardos_emision_PWIC(u16* puntero,float *angulos,float d,float c, int n_angulos);
int prog_ini_promediado(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p);
int prog_actualiza_promediado(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p);
int prog_actualiza_promediado_abs(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p,int n_fl);
int prog_ini_promediado_abs(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p);
#endif /* SRC_SITAU2_PULSER_PULSER_BUSSAR_H_ */
