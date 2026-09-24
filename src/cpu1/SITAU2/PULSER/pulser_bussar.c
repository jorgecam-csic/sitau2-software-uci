/*
 * pulser_bussar.c
 *
 *  Created on: 26 abr. 2019
 *      Author: Cruza
 */
#include "bcc_bussar.h"
#include "xil_types.h"
#include "pulser_bussar.h"
#include "bussar_addr.h"
#include "math.h"
#include "global.h"
#include "log.h"

//u8 canales_ITT_canon[32]={1,3,2,4,5,7,6,8,9,11,10,12,13,15,14,16,17,19,18,20,21,23,22,24,26,28,25,27,30,32,29,31};
static PULSER_UT_struct_t ut_regs_prom;

int prog_ini_promediado(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	ut_regs_prom = *ut_regs;
	ut_regs_prom.delay_mem_ini_end.BITS.delay_mem_end = ut_regs->delay_mem_ini_end.BITS.delay_mem_ini + 31;

	return prog_pulser_UT_regs(&ut_regs_prom,minibase,commnd_buf_p);
}
int prog_actualiza_promediado(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	if(ut_regs_prom.delay_mem_ini_end.BITS.delay_mem_ini+32+31 > ut_regs->delay_mem_ini_end.BITS.delay_mem_end)
		ut_regs_prom.delay_mem_ini_end.BITS.delay_mem_ini = ut_regs->delay_mem_ini_end.BITS.delay_mem_ini;
	else
		ut_regs_prom.delay_mem_ini_end.BITS.delay_mem_ini += 32;


	ut_regs_prom.delay_mem_ini_end.BITS.delay_mem_end = ut_regs_prom.delay_mem_ini_end.BITS.delay_mem_ini+31;
	ut_regs_prom.delay_mem_ptr = (u32)ut_regs_prom.delay_mem_ini_end.BITS.delay_mem_ini;

	return prog_pulser_UT_regs(&ut_regs_prom,minibase,commnd_buf_p);
}
int prog_ini_promediado_abs(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	PULSER_UT_struct_t ut_regs_prom_abs;
	ut_regs_prom_abs = *ut_regs;
	ut_regs_prom_abs.delay_mem_ini_end.BITS.delay_mem_ini = 0;
	ut_regs_prom_abs.delay_mem_ini_end.BITS.delay_mem_end = 0xFFFF;
	ut_regs_prom_abs.delay_mem_ptr = 0;
	ut_regs_prom_abs.pul_general_ctrl.BITS.pulser_auto_load = 0;
	return prog_pulser_UT_regs(&ut_regs_prom_abs,minibase,commnd_buf_p);
}
int prog_actualiza_promediado_abs(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p,int n_fl)
{
	PULSER_UT_struct_t ut_regs_prom_abs;
	ut_regs_prom_abs = *ut_regs;
	ut_regs_prom_abs.delay_mem_ini_end.BITS.delay_mem_ini = n_fl*32;
	ut_regs_prom_abs.delay_mem_ini_end.BITS.delay_mem_end =	0xFFFF;
	ut_regs_prom_abs.delay_mem_ptr = ut_regs_prom_abs.delay_mem_ini_end.BITS.delay_mem_ini;
	ut_regs_prom_abs.pul_general_ctrl.BITS.pulser_auto_load = 0;
	return prog_pulser_UT_regs(&ut_regs_prom_abs,minibase,commnd_buf_p);
}
int verifica_codigo_pulser(void)
{
	u32 codigo_leido;
	int ret;
	if(ret=BCC_read_reg(1,PULSER_BUSSAR_SUBMOD_ADDR,CODE_CONF_R,&codigo_leido))
	{
		xil_printf("NO SE HA PODIDO VERIFICAR EL CÓDIGO DEL PÚLSER. LECTURA EN BUSSAR ERRONEA\n\r");
		return -1;
	}
	if(codigo_leido!=PULSER_CODE)
	{
		xil_printf("LECTURA INCORRECTA DEL CÓDIGO DEL PÚLSER.\n\r");
		xil_printf("Valor leido		: %08X\n\r",codigo_leido);
		xil_printf("Valor esperado	: %08X\n\r",(u32)PULSER_CODE);
		return -1;
	}
	return 0;
}
int retardos_emision_PWIC(u16* puntero,float *angulos,float d,float c, int n_angulos)
{
	int ang,elem;
	u16* p_tmp=puntero;
	for(ang=0;ang<n_angulos;ang++)
		for(elem=0;elem<32;elem++)
		{
			if(angulos[ang]< 0) *p_tmp++=(u16)((sinf(angulos[ang]*PI/180)*d/c*(elem+1-32))*PULSER_TICS_PER_US);
			if(angulos[ang]>=0) *p_tmp++=(u16)((sinf(angulos[ang]*PI/180)*d/c*(elem+1- 1))*PULSER_TICS_PER_US);
		}

	return (int)(p_tmp-puntero);
}

//void ordena_canales(u16* retardos_dst,u16* retardos_org,int n_leyes_focales)
//{
//	int foc,elem;
//
//	for(foc=0;foc<n_leyes_focales;foc++)
//		for(elem=0;elem<32;elem++)
//			retardos_dst[foc*32+canales_ITT_canon[elem]-1]=retardos_org[foc*32+elem];
//
//}

void print_retardos(u16* retardos, int n_leyes_focales)
{
	int foc,elem;

	xil_printf("\r\n\r\nRETARDOS DE EMISIÓN EN TICKS 5ns:\r\n");
	for(foc=0;foc<n_leyes_focales;foc++)
	{
		for(elem=0;elem<32;elem++)
			xil_printf("%4d",*retardos++);
		xil_printf("\r\n");
	}
	xil_printf("\r\n");
}

int retardos_emision_FMC32(u16* puntero)
{
	int i,j;
	for(i=0;i<32;i++)
		for(j=0;j<32;j++)
			*puntero++=(i==j?0:0xFFFF);
	return 1024;
}
int retardos_emision_PLANO(u16* puntero)
{
	int i,j;
	for(i=0;i<32;i++)
		for(j=0;j<32;j++)
			*puntero++=1;
	return 1024;
}

int prog_pulser_focal_laws(u16* retardos, int num_retardos, u16 dir_ini, u8 minibase, commnd_buffer_t *commnd_buf_p)
{
int i;
int num_words=0;
int result;
	//ESTO RESUELVE UN ERROR EN EL HARDWARE QUE AL ESCRIBIR LA MEMORIA VOLVÍA A 0 AL LLEGAR A LA POSICIÓN delay_mem_ptr_end
	if ((result = push_bussar_write_reg((u32)0xFFFF0000, minibase, PULSER_BUSSAR_SUBMOD_ADDR, DELAY_MEM_PROG_R, commnd_buf_p)) < 0)
	  return RLOG(result);

	if ((result = push_bussar_write_reg((u32)dir_ini, minibase, PULSER_BUSSAR_SUBMOD_ADDR, DELAY_MEM_ADDR_R, commnd_buf_p)) < 0)
      return RLOG(result);

	num_words += result;
	for (i=0; i<num_retardos; i++)
	{
		if ((result = push_bussar_write_reg((u32)retardos[i], minibase, PULSER_BUSSAR_SUBMOD_ADDR, DELAY_MEM_DATA_R, commnd_buf_p)) < 0)
         return RLOG(result);
		num_words += result;
	}
	return num_words;
}

int prog_pulser_focal_laws_promediado(u16* retardos, int num_leyes, u16 dir_ini, u8 minibase, commnd_buffer_t *commnd_buf_p,int promediado)
{
int i,ley,proms,elem;
int num_words=0;
int result;
	//ESTO RESUELVE UN ERROR EN EL HARDWARE QUE AL ESCRIBIR LA MEMORIA VOLVÍA A 0 AL LLEGAR A LA POSICIÓN delay_mem_ptr_end
	if ((result = push_bussar_write_reg((u32)0xFFFF0000, minibase, PULSER_BUSSAR_SUBMOD_ADDR, DELAY_MEM_PROG_R, commnd_buf_p)) < 0)
	  return RLOG(result);

	if ((result = push_bussar_write_reg((u32)dir_ini, minibase, PULSER_BUSSAR_SUBMOD_ADDR, DELAY_MEM_ADDR_R, commnd_buf_p)) < 0)
      return RLOG(result);

	num_words += result;
	if(promediado>1)
	{
		for(ley=0; ley<num_leyes; ley++)
		{
			for(proms=0;proms<promediado;proms++)
			{
				for(elem=0; elem<32; elem++)
				{
					if ((result = push_bussar_write_reg((u32)retardos[elem+proms*32+ley*promediado*32], minibase, PULSER_BUSSAR_SUBMOD_ADDR, DELAY_MEM_DATA_R, commnd_buf_p)) < 0)
						return RLOG(result);
					num_words += result;
				}
			}
		}
	}
	return num_words;
}
int prog_pulser_UT_regs(PULSER_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	int num_words=0;
	int ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->pulse_width,minibase,PULSER_BUSSAR_SUBMOD_ADDR,PUL_SHAPE_1_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->pulser_main_delay,minibase,PULSER_BUSSAR_SUBMOD_ADDR,PULSER_MAIN_DELAY_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->delay_mem_ini_end.delay_mem_ini_end_u32,minibase,PULSER_BUSSAR_SUBMOD_ADDR,DELAY_MEM_PROG_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->delay_mem_ptr,minibase,PULSER_BUSSAR_SUBMOD_ADDR,DELAY_MEM_ADDR_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->pul_general_ctrl.pul_general_ctrl_u32,minibase,PULSER_BUSSAR_SUBMOD_ADDR,PULSER_GEN_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	return num_words;
}
