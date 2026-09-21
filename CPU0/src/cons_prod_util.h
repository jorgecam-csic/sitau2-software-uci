/*
 * cons_prod_util.h
 *
 *  Created on: 17/11/2016
 *      Author: csic
 */

#ifndef CONS_PROD_UTIL_H_
#define CONS_PROD_UTIL_H_

#include "xil_types.h"

typedef struct cons_prod_s
{
	u8 *buffer_start_address;
	u32 buffer_size;
	u32 read_position; //indica la próxima posición a leer.
	u32 write_position;//indica la próxima posición a escribir.
	u32 int_threshold;//threshold==0 significa que solo interrumpa cuando no había datos y ahora sí
	u8  cons_wake_up_int_ena;//habilita la interrupción del productor hacia el consumidor.
	u8  mutex_ID;
	u8  signal_ID;
	u8  cons_cpu_ID;
	u8  prod_cpu_ID;
}cons_prod_t;


#include "global.h"
#include "xil_exception.h"
#include "shared_mem_def.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "mutex_utils.h"
#include "gic_utils.h"
#include "xil_cache.h"
//Si read_position == write_position, no hay nada el el buffer.
//El buffer nunca se puede llenar entero, se aprovechan todas las posiciones salvo una.



extern cons_prod_t* cons_prod_array_p[];

#define SIZE_PER_STRUCT_ALIGN	(CEILING((sizeof(struct cons_prod_s)),CACHELINE_BYTE_SIZE)*CACHELINE_BYTE_SIZE)

#define HOST2UCI_BUF_ID	0
#define UCI2HOST_BUF_ID	1
#define IMAGE_BUF_ID	2

#define MAX_CONS_PROD_STRUCTS	3

#define MUTEX_START_NUMBER		8
#define SIC_START_NUMBER		8

u32 prod_num_data_to_write(cons_prod_t* cons_prod_p);
u32 cons_num_data_to_read(cons_prod_t* cons_prod_p);

void prod_refresh_write(cons_prod_t* cons_prod_p,u32 n_bytes);
void cons_refresh_read(cons_prod_t* cons_prod_p,u32 n_bytes);

cons_prod_t* prod_init(u8 cons_prod_id);
cons_prod_t* cons_init(u8 cons_prod_id,Xil_InterruptHandler cons_wake_up_interrupt);

int cons_prod_init_structs(void);

void prod_memcpy(cons_prod_t* cons_prod_p,u32 n_bytes,u8* source_address);
void cons_memcpy(cons_prod_t* cons_prod_p,u32 n_bytes,u8* destination_address);

u32 prod_memref(cons_prod_t* cons_prod_p,u32 n_bytes,u8** ref_address_p);
u32 cons_memref(cons_prod_t* cons_prod_p,u32 n_bytes,u8** ref_address_p);

void cons_set_int_threshold(cons_prod_t* cons_prod_p,u32 threshold);

#endif /* CONS_PROD_UTIL_H_ */
