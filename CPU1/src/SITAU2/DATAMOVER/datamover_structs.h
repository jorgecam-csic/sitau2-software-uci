/*
 * datamover_structs.h
 *
 *  Created on: 2 abr. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_DATAMOVER_DATAMOVER_STRUCTS_H_
#define SRC_SITAU2_DATAMOVER_DATAMOVER_STRUCTS_H_

#include "xil_types.h"

typedef u32 startaddr_t;
typedef u32 auto_t;
typedef u32 cir_start_addr_t;
typedef u32 cir_end_addr_t;

typedef union
{
	u8 status_u8;
	struct
	{
		u8 tag_rcv : 4;
		u8 laer : 1;
		u8 adde : 1;
		u8 axie : 1;
		u8 okey : 1;
	}BIT;
}status_response_t;

typedef union
{
	u32 status_32;
	struct
	{
		status_response_t status_response;
		u32 res_1 : 8;
		u32 tag_send : 4;
		u32 res_2 : 9;
		u32 error : 1;
		u32 int_ena : 1;
		u32 transfer_flag : 1;
	}BIT;

}status_t;


typedef union
{
	u32 command_32;
	struct
	{
		u32 btt : 23;
		u32 res : 1;
		u32 sec_trigger : 1;
		u32 cir_ena : 1;
		u32 ext_trigger : 1;
		u32 auto_increment : 1;
		u32 auto_start : 1;
		u32 queue : 1;
		u32 last : 1;
		u32 start : 1;
	}BIT;
}command_t;

typedef struct datamover_struct_s
{
	startaddr_t s2mm_startaddr;
	command_t s2mm_command;
	status_t s2mm_status;
	auto_t s2mm_auto;
	startaddr_t mm2s_startaddr;
	command_t mm2s_command;
	status_t mm2s_status;
	auto_t mm2s_auto;
	cir_start_addr_t cir_start_addr;
	cir_end_addr_t cir_end_addr;

}datamover_struct_t;

#endif /* SRC_SITAU2_DATAMOVER_DATAMOVER_STRUCTS_H_ */
