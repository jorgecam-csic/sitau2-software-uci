/*
 * datamover_remote.h
 *
 *  Created on: 2 abr. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_DATAMOVER_DATAMOVER_REMOTE_C_
#define SRC_SITAU2_DATAMOVER_DATAMOVER_REMOTE_C_

#include "datamover_structs.h"
#include "bcc_bussar.h"
#include "datamover_driver.h"
#include "afe_bussar.h"
#include "bussar_addr.h"
#include "datamover_remote.h"


u32 get_datamover2mem_next_addr(u8 minibase)
{
	u32 registro;
	if(BCC_read_reg(minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,&registro))
	{
		xil_printf("Error leyendo ADDR_REG del AFE2MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
u32 get_mem2lvds_next_addr(u8 minibase)
{
	u32 registro;
	if(BCC_read_reg(minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,&registro))
	{
		xil_printf("Error leyendo ADDR_REG del AFE2MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
status_t get_beamformer2mem_datamover_status(u8 minibase)
{
	status_t registro;
	if(BCC_read_reg(minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_STATUS,&registro.status_32))
	{
		xil_printf("Error leyendo STATUS del BFM2MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
status_t get_afe2mem_datamover_status(u8 minibase)
{
	status_t registro;
	if(BCC_read_reg(minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STATUS,&registro.status_32))
	{
		xil_printf("Error leyendo STATUS del AFE2MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
u32 get_beamformer2mem_next_addr(u8 minibase)
{
	u32 registro;
	if(BCC_read_reg(minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,&registro))
	{
		xil_printf("Error leyendo ADDR_REG del BFM2MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
u32 get_mem2bf_prom_next_addr(u8 minibase)
{
	u32 registro;
	if(BCC_read_reg(minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,&registro))
	{
		xil_printf("Error leyendo ADDR_REG del BFM2MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
u32 get_afe2mem_datamover_next_addr(u8 minibase)
{
	u32 registro;
	if(BCC_read_reg(minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,&registro))
	{
		xil_printf("Error leyendo ADDR_REG del AFE2MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
u32 get_mem2beamformer_datamover_next_addr(u8 minibase)
{
	u32 registro;
	if(BCC_read_reg(minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,&registro))
	{
		xil_printf("Error leyendo ADDR_REG del MEM2BFM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
status_t get_mem2beamformer_datamover_status(u8 minibase)
{
	status_t registro;
	while(BCC_read_reg(minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STATUS,&registro.status_32))
	{
		xil_printf("Error leyendo STATUS del MEM2BFM\n\r");
		MCBCC_print_regs();
	}


	return registro;
}

status_t get_mem2emi_prom_beamformer_datamover_status(u8 minibase)
{
	status_t registro;
	if(BCC_read_reg(minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_STATUS,&registro.status_32))
	{
		xil_printf("Error leyendo STATUS del MEM2BF0\n\r");
		MCBCC_print_regs();
	}

	return registro;
}

status_t get_minibase2uci_buffer_datamover_status(u8 minibase)
{
	status_t registro;
	if(BCC_read_reg(minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_STATUS,&registro.status_32))
	{
		xil_printf("Error leyendo STATUS del MEM2BF0\n\r");
		MCBCC_print_regs();
	}

	return registro;
}
status_t get_uci_buffer2minibase_datamover_status(u8 minibase)
{
	status_t registro;
	if(BCC_read_reg(minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_STATUS,&registro.status_32))
	{
		xil_printf("Error leyendo STATUS del BF02MEM\n\r");
		MCBCC_print_regs();
	}

	return registro;
}

int set_mem2beamformer_256bit_datamover_next_addr(u32 start_addr,u8 minibase)
{
	u32 registro;

	BCC_write_reg_inmediate(minibase, DATAMOVER256_BUSSAR_SUBMOD_ADDR, MM2S_STARTADDR, start_addr);

	return registro;
}

u32 get_mem2beamformer_256bit_datamover_next_addr(u8 minibase)
{
	u32 start_addr;

	BCC_read_reg(minibase, DATAMOVER256_BUSSAR_SUBMOD_ADDR, MM2S_STARTADDR, &start_addr);

	return start_addr;
}

int set_afe2mem_256bit_datamover_next_addr(u32 start_addr,u8 minibase)
{
	u32 registro;

	BCC_write_reg_inmediate(minibase, DATAMOVER256_BUSSAR_SUBMOD_ADDR, S2MM_STARTADDR, start_addr);

	return registro;
}

int config_mem2beamformer(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t mm2s_startaddr_l;
	//command_t s2mm_command_l;
	command_t mm2s_command_l;
	auto_t mm2s_auto_l;
	status_t mm2s_status_l;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	mm2s_startaddr_l = start_addr;
	mm2s_auto_l = auto_inc;
	mm2s_command_l.command_32 = 0x00000000;
	mm2s_command_l.BIT.auto_start = 0;
	mm2s_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.auto_increment = (auto_inc?1:0);
	mm2s_command_l.BIT.ext_trigger = external_trigger;
	mm2s_command_l.BIT.last = 1;
	mm2s_command_l.BIT.start = trig_now;
	mm2s_status_l.status_32 =0x0;
	mm2s_status_l.BIT.tag_send = 5;

	//s2mm_command_l.command_32 = 0x00000000;

	//num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_status_l.status_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STATUS,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_startaddr_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_auto_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);

	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;

}
int config_mem2beamformer_sec_trig(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 sec_trigger,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t mm2s_startaddr_l;
	//command_t s2mm_command_l;
	command_t mm2s_command_l;
	auto_t mm2s_auto_l;
	status_t mm2s_status_l;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	mm2s_startaddr_l = start_addr;
	mm2s_auto_l = auto_inc;
	mm2s_command_l.command_32 = 0x00000000;
	mm2s_command_l.BIT.auto_start = 0;
	mm2s_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.auto_increment = (auto_inc?1:0);
	mm2s_command_l.BIT.ext_trigger = external_trigger;
	mm2s_command_l.BIT.last = 1;
	mm2s_command_l.BIT.start = trig_now;
	mm2s_command_l.BIT.sec_trigger = sec_trigger;
	mm2s_status_l.status_32 =0x0;
	mm2s_status_l.BIT.tag_send = 5;

	//s2mm_command_l.command_32 = 0x00000000;

	//num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_status_l.status_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STATUS,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_startaddr_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_auto_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);

	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;

}

int mem2beamformer_inmediate_command(u32 btt,u32 autostart,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 sec_trigger,u8 circular_mem,u8 minibase)
{
	command_t mm2s_command_l;

	mm2s_command_l.command_32 = 0x00000000;
	mm2s_command_l.BIT.auto_start = autostart;
	mm2s_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.auto_increment = (auto_inc?1:0);
	mm2s_command_l.BIT.ext_trigger = external_trigger;
	mm2s_command_l.BIT.last = 1;
	mm2s_command_l.BIT.start = trig_now;
	mm2s_command_l.BIT.sec_trigger = sec_trigger;
	mm2s_command_l.BIT.cir_ena = circular_mem;

	BCC_write_reg_inmediate(minibase, DATAMOVER256_BUSSAR_SUBMOD_ADDR, MM2S_COMMAND, mm2s_command_l.command_32);

	return 0;
}



int config_mem2beamformer_sec_trig_circular(u32 start_addr,u32 end_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 sec_trigger,u8 circular_mem,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t mm2s_startaddr_l;
	//command_t s2mm_command_l;
	command_t mm2s_command_l;
	auto_t mm2s_auto_l;
	status_t mm2s_status_l;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	mm2s_startaddr_l = start_addr;
	mm2s_auto_l = auto_inc;
	mm2s_command_l.command_32 = 0x00000000;
	mm2s_command_l.BIT.auto_start = 0;
	mm2s_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.auto_increment = (auto_inc?1:0);
	mm2s_command_l.BIT.ext_trigger = external_trigger;
	mm2s_command_l.BIT.last = 1;
	mm2s_command_l.BIT.start = trig_now;
	mm2s_command_l.BIT.sec_trigger = sec_trigger;
	mm2s_command_l.BIT.cir_ena = circular_mem;
	mm2s_status_l.status_32 =0x0;
	mm2s_status_l.BIT.tag_send = 5;


	//s2mm_command_l.command_32 = 0x00000000;

	//num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_status_l.status_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STATUS,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_startaddr_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_auto_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);
	if(circular_mem)
		num_words+=push_bussar_write_reg(end_addr-btt,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,CIR_BUS_ENDADDR,commnd_buf);

	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;

}
int config_afe2mem(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t s2mm_startaddr_l;
	command_t s2mm_command_l;
	//command_t mm2s_command_l;
	auto_t s2mm_auto_l;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	s2mm_startaddr_l = start_addr;
	s2mm_auto_l = auto_inc;
	s2mm_command_l.command_32 = 0x00000000;
	s2mm_command_l.BIT.auto_start = 0;
	s2mm_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	s2mm_command_l.BIT.auto_increment = (auto_inc?1:0);
	s2mm_command_l.BIT.ext_trigger = external_trigger;
	s2mm_command_l.BIT.last = 1;
	s2mm_command_l.BIT.start = trig_now;

	//mm2s_command_l.command_32 = 0x00000000;

	//num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_startaddr_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_auto_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);
	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;
}

int config_remote_256bit_datamover_stream2mem(u32 start_addr,u32 end_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 circular_mem,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t s2mm_startaddr_l;
	command_t s2mm_command_l;
	//command_t mm2s_command_l;
	auto_t s2mm_auto_l;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	s2mm_startaddr_l = start_addr;
	s2mm_auto_l = auto_inc;
	s2mm_command_l.command_32 = 0x00000000;
	s2mm_command_l.BIT.auto_start = 0;
	s2mm_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	s2mm_command_l.BIT.auto_increment = (auto_inc?1:0);
	s2mm_command_l.BIT.ext_trigger = external_trigger;
	s2mm_command_l.BIT.last = 1;
	s2mm_command_l.BIT.start = trig_now;
	s2mm_command_l.BIT.cir_ena = circular_mem;

	//mm2s_command_l.command_32 = 0x00000000;

	//num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_startaddr_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_auto_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_auto_l,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,S2MM_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(start_addr,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,CIR_BUS_STARTADDR,commnd_buf);
	if(circular_mem)
		num_words+=push_bussar_write_reg(end_addr-btt,minibase,DATAMOVER256_BUSSAR_SUBMOD_ADDR,CIR_BUS_ENDADDR,commnd_buf);
	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;
}


int config_remote_datamover32_disable_read(u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;

	command_t mm2s_command_l;

	mm2s_command_l.command_32 = 0x00000000;

	num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);

	return num_words;
}

int config_remote_datamover32_disable_write(u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;

	command_t s2mm_command_l;

	s2mm_command_l.command_32 = 0x00000000;

	num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);

	return num_words;
}

int config_mem2emi_prom_beamformer2mem(u32 res_addr,u32 res_btt,u32 res_autoinc,u32 scratch_addr,u32 scratch_btt,u32 scratch_autoinc,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t s2mm_startaddr_l;
	startaddr_t mm2s_startaddr_l;
	auto_t s2mm_auto_l;
	auto_t mm2s_auto_l;
	command_t s2mm_command_l;
	command_t mm2s_command_l;

	mm2s_command_l.command_32 = 0x00000000;

	mm2s_startaddr_l = scratch_addr;
	mm2s_auto_l = scratch_autoinc;
	mm2s_command_l.BIT.auto_start = 0;
	mm2s_command_l.BIT.btt = scratch_btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.auto_increment = (scratch_autoinc?1:0);
	mm2s_command_l.BIT.ext_trigger = 1;
	mm2s_command_l.BIT.last = 1;
	mm2s_command_l.BIT.start = 0;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	s2mm_startaddr_l = res_addr;
	s2mm_auto_l = res_autoinc;
	s2mm_command_l.command_32 = 0x00000000;
	s2mm_command_l.BIT.auto_start = 0;
	s2mm_command_l.BIT.btt = res_btt&DATAMOVER_BTT_MASK;
	s2mm_command_l.BIT.auto_increment = (res_autoinc?1:0);
	s2mm_command_l.BIT.ext_trigger = 1;
	s2mm_command_l.BIT.last = 1;
	s2mm_command_l.BIT.start = 0;


	num_words+=push_bussar_write_reg(mm2s_startaddr_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_auto_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);

	num_words+=push_bussar_write_reg(s2mm_startaddr_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_auto_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);

	if(res_btt>DATAMOVER_BTT_MASK||scratch_btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;

}


int config_beamformer2mem_circular(u32 start_addr,u32 end_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 circular_mem,u8 minibase,commnd_buffer_t *commnd_buf,u8 writeonly0_readwrite1)
{
	int num_words=0;
	startaddr_t s2mm_startaddr_l;
	startaddr_t mm2s_startaddr_l;
	auto_t s2mm_auto_l;
	auto_t mm2s_auto_l;
	command_t s2mm_command_l;
	command_t mm2s_command_l;

	mm2s_command_l.command_32 = 0x00000000;
	mm2s_startaddr_l = start_addr;
	mm2s_auto_l = auto_inc;
	mm2s_command_l.BIT.auto_start = 0;
	mm2s_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.auto_increment = (auto_inc?1:0);
	mm2s_command_l.BIT.ext_trigger = (external_trigger&&writeonly0_readwrite1?1:0);
	mm2s_command_l.BIT.last = 1;// = 0;
	mm2s_command_l.BIT.start = trig_now;
	mm2s_command_l.BIT.cir_ena = circular_mem;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	s2mm_startaddr_l = start_addr;
	s2mm_auto_l = auto_inc;
	s2mm_command_l.command_32 = 0x00000000;
	s2mm_command_l.BIT.auto_start = 0;
	s2mm_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	s2mm_command_l.BIT.auto_increment = (auto_inc?1:0);
	s2mm_command_l.BIT.ext_trigger = (external_trigger?1:0);
	s2mm_command_l.BIT.last = 1;// = 0;
	s2mm_command_l.BIT.start = trig_now;
	s2mm_command_l.BIT.cir_ena = circular_mem;

	if(writeonly0_readwrite1)
	{
		num_words+=push_bussar_write_reg(mm2s_startaddr_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,commnd_buf);
		num_words+=push_bussar_write_reg(mm2s_auto_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_AUTO,commnd_buf);
		num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);
	}
	else
		num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);


	num_words+=push_bussar_write_reg(s2mm_startaddr_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_auto_l,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(start_addr,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,CIR_BUS_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(end_addr-btt,minibase,DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,CIR_BUS_ENDADDR,commnd_buf);

	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;
}

int config_beamformer2mem(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 minibase,commnd_buffer_t *commnd_buf,u8 writeonly0_readwrite1)
{
	int num_words=0;
	startaddr_t s2mm_startaddr_l;
	startaddr_t mm2s_startaddr_l;
	auto_t s2mm_auto_l;
	auto_t mm2s_auto_l;
	command_t s2mm_command_l;
	command_t mm2s_command_l;

	mm2s_command_l.command_32 = 0x00000000;
	mm2s_startaddr_l = start_addr;
	mm2s_auto_l = auto_inc;
	mm2s_command_l.BIT.auto_start = 0;
	mm2s_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.auto_increment = (auto_inc?1:0);
	mm2s_command_l.BIT.ext_trigger = (external_trigger&&writeonly0_readwrite1?1:0);
	mm2s_command_l.BIT.last = 1;// = 0;
	mm2s_command_l.BIT.start = trig_now;

	//COMPROBAR BTT, ADDR Y ADDR+BTT
	s2mm_startaddr_l = start_addr;
	s2mm_auto_l = auto_inc;
	s2mm_command_l.command_32 = 0x00000000;
	s2mm_command_l.BIT.auto_start = 0;
	s2mm_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	s2mm_command_l.BIT.auto_increment = (auto_inc?1:0);
	s2mm_command_l.BIT.ext_trigger = (external_trigger?1:0);
	s2mm_command_l.BIT.last = 1;// = 0;
	s2mm_command_l.BIT.start = trig_now;

	if(writeonly0_readwrite1)
	{
		num_words+=push_bussar_write_reg(mm2s_startaddr_l,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,commnd_buf);
		num_words+=push_bussar_write_reg(mm2s_auto_l,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_AUTO,commnd_buf);
	}

	num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_startaddr_l,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_auto_l,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);

	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;

}

int config_minibase2uci_buffer(u32 start_addr,u32 btt,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t mm2s_startaddr_l;
	command_t mm2s_command_l;
	auto_t mm2s_auto_l;

	mm2s_startaddr_l = start_addr;
	mm2s_command_l.command_32 = 0x00000000;
	mm2s_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	mm2s_command_l.BIT.last = 0;
	mm2s_command_l.BIT.start = 1;

	num_words+=push_bussar_write_reg(mm2s_startaddr_l,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(0x000000,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_AUTO,commnd_buf);
	num_words+=push_bussar_write_reg(mm2s_command_l.command_32,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,MM2S_COMMAND,commnd_buf);

	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;
}
int config_uci_buffer2minibase(u32 start_addr,u32 btt,u8 minibase,commnd_buffer_t *commnd_buf)
{
	int num_words=0;
	startaddr_t s2mm_startaddr_l;
	command_t s2mm_command_l;
	auto_t s2mm_auto_l;

	s2mm_startaddr_l = start_addr;
	s2mm_command_l.command_32 = 0x00000000;
	s2mm_command_l.BIT.btt = btt&DATAMOVER_BTT_MASK;
	s2mm_command_l.BIT.last = 1;
	s2mm_command_l.BIT.start = 1;

	num_words+=push_bussar_write_reg(s2mm_startaddr_l,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_STARTADDR,commnd_buf);
	num_words+=push_bussar_write_reg(s2mm_command_l.command_32,minibase,DATAMOVER032_BUSSAR_SUBMOD_ADDR,S2MM_COMMAND,commnd_buf);

	if(btt>DATAMOVER_BTT_MASK) return -1;
	else return num_words;
}


void BF0_datamover_remote_write_inmediate_MM2S_addr(u32 MM2S_remote_addr,u8 minibase)
{
	BCC_write_reg_inmediate(minibase, DATAMOVBEAMF_BUSSAR_SUBMOD_ADDR,MM2S_STARTADDR,MM2S_remote_addr);
}

#endif /* SRC_SITAU2_DATAMOVER_DATAMOVER_REMOTE_C_ */
