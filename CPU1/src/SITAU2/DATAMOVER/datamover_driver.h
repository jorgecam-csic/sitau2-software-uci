/*
 * datamover_driver.h
 *
 *  Created on: 17 de oct. de 2016
 *      Author: csic
 */

#ifndef _DATAMOVER_DRIVER_H_
#define _DATAMOVER_DRIVER_H_

#include <stdio.h>
#include "xparameters.h"
#include "xbasic_types.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "gic_utils.h"
#include "mcbcc_mst_driver.h"
#include "datamover_structs.h"

#define S2MM_STARTADDR		0
#define S2MM_COMMAND		1
#define S2MM_STATUS			2
#define S2MM_AUTO			3
#define MM2S_STARTADDR		4
#define MM2S_COMMAND		5
#define MM2S_STATUS			6
#define MM2S_AUTO			7
#define CIR_BUS_STARTADDR	8
#define CIR_BUS_ENDADDR		9
//STATUS reg masks
#define DATAMOVER_END_MASK		0X80000000	//Datamover ha terminado una transacción
#define DATAMOVER_IEN_MASK		0X40000000	//Interrupción activada
#define DATAMOVER_ERR_MASK		0X20000000	//El último comando enviado no se transmitió
#define DATAMOVER_TGO_MASK		0X000F0000	//Tag para el proximo comando (TaG Out)
#define DATAMOVER_TGI_MASK		0X0000000F	//Tag de la ultima transacción
#define DATAMOVER_NOE_MASK		0X00000080	//La ultima transacción ha sido sin errores
#define DATAMOVER_AXE_MASK		0X00000040	//Ha habido un error del axi en la ultima transacción
#define DATAMOVER_ADE_MASK		0X00000020	//Error de direccionamieto
#define DATAMOVER_LTE_MASK		0X00000010	//Error en el flag LAST: en S2MM el flag LAST ha sido antes de tiempo o no ha llegado o ha llegado y no debería haberlo hecho
//COMMAND reg masks
#define DATAMOVER_STR_MASK		0X80000000	//Comienza la transacción
#define DATAMOVER_LST_MASK		0X40000000	//Para MM2S: Indica al datamover que envíe la señal LAST en el ultimo pulso; para S2MM indica que va a recibir el bit LST, si no se corresponde puede dar el error DATAMOVER_LTE_MASK
#define DATAMOVER_QUE_MASK		0X20000000	//Hay un comando en cola: no se debe tratar de enviar uno hasta que no esté a 0
#define DATAMOVER_AUT_MASK		0X10000000	//Activa el auto start: al terminar una transsacción reenvía el comando programado automáticamente (especialmente util con autoincrement)
#define DATAMOVER_INC_MASK		0X08000000	//Activa el auto increment: al enviar una transacción, incrementa STARTADDR con el valor de AUTO_INC
#define DATAMOVER_TRG_MASK		0X04000000	//Activa el disparo externo (pata equivalente al bit DATAMOVER_STR_MASK)
#define DATAMOVER_BTT_MASK		0X007FFFFC	//Bytes a transferir (23 bits, 8388607 bytes máximo). En teoría, hay que alinearlos al ancho del STREAM, pues normalmente el STREAM no se usan las patas para transefencias desalineadas.

#define DATAMOVER_INT_ID	(XPAR_FABRIC_DMA_LVDS_DATAMOVER_CTRL_AXI_LITE_0_INTERRUPT_INTR)

#define NUM_DATAMOVERS		2

#define LOCAL_DATAMOVER_ID			0
#define ANY_REMOTE_DATAMOVER_ID		1

typedef union
{
	u8 DATAMOV_all_flags;
	struct datamov_int_flags_s
	{
		u8 S2MM_end : 1;
		u8 MM2S_end : 1;
	} DATAMOV_bit_flags;
}datamov_flags_t;

typedef struct datamov_instance_s
{
	u32 *baseaddress;
	u32 S2MM_status_reg_int;
	u32 MM2S_status_reg_int;
	datamov_flags_t* datamov_flags;
	u16 datamov_id;
	u16 int_id;
	s8	S2MM_pending;
	s8	MM2S_pending;
	u8	S2MM_tag_filter_ena;
	u8	MM2S_tag_filter_ena;
	u8	S2MM_tag_filter;
	u8	MM2S_tag_filter;
}datamov_instance_t;



extern volatile datamov_flags_t Datamov_flags[NUM_DATAMOVERS];
extern datamov_instance_t Datamover_instances[NUM_DATAMOVERS];

u32 get_afe2mem_datamover_next_addr(u8 minibase);

void DATAMOV_get_mem_int(datamov_instance_t *datamover_instance_p,void *src_addr,u32 btt,u8 tag,u8 last);
void DATAMOV_put_mem_int(datamov_instance_t *datamover_instance_p,void *dst_addr,u32 btt,u8 tag,u8 last);

void DATAMOV_get_mem(volatile u32* datamover_base_addr,void *src_addr,u32 btt);
void DATAMOV_put_mem(volatile u32* datamover_base_addr,void *dst_addr,u32 btt);

void DATAMOV_get_mem_no_int(datamov_instance_t *datamover_instance_p,void *src_addr,u32 btt);
void DATAMOV_put_mem_no_int(datamov_instance_t *datamover_instance_p,void *dst_addr,u32 btt);


void DATAMOV_int_setup(datamov_instance_t *datamover_instance_p);
void DATAMOV_int_function(void *handler);
void DATAMOV_init_structs(void);

u8 DATAMOV_S2MM_end_status(datamov_instance_t *datamover_instance_p);
u8 DATAMOV_MM2S_end_status(datamov_instance_t *datamover_instance_p);

u8 DATAMOV_poll_S2MM_end(datamov_instance_t *datamover_instance_p);
u8 DATAMOV_poll_MM2S_end(datamov_instance_t *datamover_instance_p);
#endif /* _DATAMOVER_DRIVER_H_ */
