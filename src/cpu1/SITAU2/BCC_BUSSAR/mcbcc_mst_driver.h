/*
 * mcbcc_mst_driver.h
 *
 *  Created on: 11 de oct. de 2016
 *      Author: csic
 */

#ifndef _MCBCC_MST_DRIVER_H_
#define _MCBCC_MST_DRIVER_H_

#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "global.h"
#include "xil_types.h"

extern const u32 MCBCC_Baseaddress;

#define MCBCC_INT_NUMBER	XPAR_FABRIC_UCI_AXI_MASTER_INTERRUPT_INTR

extern volatile u16 MCBCC_int_flags_copy;

#define MCBCC_TIMEOUT		10

#ifdef ETHERNET_SITAU
#ifdef SITAU2
#error "Definido ETHERNET_SITAU y SITAU2 a la vez"
#endif
#define NUM_WORDS_MASK	0x3FFFFF
#endif

#ifdef SITAU2
#ifdef ETHERNET_SITAU
#error "Definido ETHERNET_SITAU y SITAU2 a la vez"
#endif
#define NUM_WORDS_MASK	0x1FFFFF
#endif

#define	WR_WORD_HEAD		0
#define	WR_WORD_DATA		1
#define	WR_STATUS			2
#define	WR_WORD_COUNT		3
#define	RD_WORD_HEAD		4
#define	RD_WORD_DATA		5
#define	RD_STATUS			6
#define	RD_WORD_COUNT		7
#define	WR_RD_DIFF			8
#define	RD_STREAM_COND		9
#define	RD_STREAM_WORD		10
#define	REG_TO_STREAM_DATA	11
#define	BUSSAR_AXIL_TIMOUT	12
#define	BUSSAR_AXIL_CONF	13
#define	PRO_REG				14
#define	INTERRUPTS			15
#define	BUSSAR_AXIL_REG0	0X100
#define	BUSSAR_AXIL_REGL	0X1FF

#define MODULE_FIFO_DELAY_BITS 7

#define	BUSSAR_AXIL_BASE	0X400
#define	BUSSAR_AXIL_LAST	0X7FF

#define	POR_NBIT	8			//	PRO Rise flank bit number
#define	POF_NBIT	7			//	PRO Fall flank bit number
#define	ERE_NBIT	6			//	Emulator Read End bit number
#define	ERT_NBIT	5			//	Emulator Read Timeout bit number
#define	DRE_NBIT	4			//	Data Read End bit number
#define	WSE_NBIT	3			//	Write Stream End bit number
#define	RSE_NBIT	2			//	Read Stream End bit number
#define	TRG_NBIT	1			//	TRiGger bit number
#define	TRE_NBIT	0			//  TRigger Error bit number

// INTERRUPTS
#define	PORF_MASK	(1<<8)			//	PRO Rise flank interrupt Flag
#define	POFF_MASK	(1<<7)			//	PRO Fall flank interrupt Flag
#define	EREF_MASK	(1<<6)			//	Emulator Read End interrupt Flag
#define	ERTF_MASK	(1<<5)			//	Emulator Read Timeout interrupt Flag
#define	DREF_MASK	(1<<4)			//	Data Read End interrupt Flag
#define	WSEF_MASK	(1<<3)			//	Write Stream End interrupt Flag
#define	RSEF_MASK	(1<<2)			//	Read Stream End interrupt Flag
#define	TRGF_MASK	(1<<1)			//	TRiGger Interrupt Flag
#define	TREF_MASK	(1<<0)			//	TRigger Error Interrupt Flag

#define	PORE_MASK	((1<<8)<<16)	//	PRO Rise flank interrupt Enable
#define	POFE_MASK	((1<<7)<<16)	//	PRO Fall flank interrupt Enable
#define	EREE_MASK	((1<<6)<<16)	//	Emulator Read End interrupt Enable
#define	ERTE_MASK	((1<<5)<<16)	//	Emulator Read Timeout interrupt Enable
#define	DREE_MASK	((1<<4)<<16)	//	Data Read End interrupt Enable
#define	WSEE_MASK	((1<<3)<<16)	//	Write Stream End interrupt Enable
#define	RSEE_MASK	((1<<2)<<16)	//	Read Stream End interrupt Enable
#define	TRGE_MASK	((1<<1)<<16)	//	TRiGger Interrupt Enable
#define	TREE_MASK	((1<<0)<<16)	//	TRigger Error Interrupt Enable

#define	EINT_MASK	(1<<31)			//	Enable Interrupts

// Configuración de comportamiento del LST en envíos en ráfaga: registros WR_STATUS y RD_STATUS
#define LAST_FORCE_ZERO		0		//  Fuerza el LAST de la última palabra de la transacción (ultimo VLD) a cero.
#define LAST_ERROR			1		//  No poner esta configuración ya que depende de la implementación HW (si se hace primero el AND o el OR, en un caso es fijo a uno y el otro fijo a 0)
#define LAST_DONT_TOUCH		2		//  El último LAST se deja como está
#define LAST_FORCE_ONE		3		//  Fuerza el LAST de la última palabra de la transacción (ultimo VLD) a uno.

// RD_STATUS
#define RVLD_MASK	(1<<31)
#define RRDY_MASK	(1<<30)
#define RONE_MASK	(1<<29)
#define RVHD_MASK	(1<<28)
#define RVDT_MASK	(1<<27)
#define RLHD_MASK	(1<<26)
#define RLDT_MASK	(1<<26)
#define SRA_MASK	(1<<24)
#define RSAL_MASK	(1<<23)
#define RSOL_MASK	(1<<22)

// RD_STREAM_COND
#define WTSC_MASK	(1<<31)
#define CTSC_MASK	(1<<30)
#define RTS_MASK	(1<<29)
#define RTSL_MASK	(1<<28)
#define RTSA_MASK	(1<<27)
#define RTSN_MASK	(1<<26)
//#define SRA_MASK	(1<<24) // Ya definido


#define TRGI	(1<<4)			//	TRiGger In				Valor actual del trigger de entrada
#define POAR	(1<<3)			//	Pro Out Automatic Rise	Automatiza el funcionamiento del encendido del PRO_out: PRO_out se pone a UNO  cuando hay un flanco de subida TRGI
#define POAF	(1<<2)			//	Pro Out Automatic Fall	Automatiza el funcionamiento del apagado   del PRO_out: PRO_out se pone a CERO cuando hay un flanco de subida de PRO_in
#define PROI	(1<<1)			//	PRO In					Valor de la entrada PRO_in
#define PROO	(1<<0)			//	PRO Out					Valor de la salida PRO_out


#define MCBCC_MST_ERR_READ_OP_BUSY				-1
#define MCBCC_MST_ERR_READ_OP_STREAM_NRDY		-2

int MCBCC_print_int_flags(u16 flags);
int MCBCC_get_read_stream_end(void);
void error_timeout_function(const char *timeout_access, u32 n_data, int data_count);
void MCBCC_clear_counters(void);
u32 MCBCC_get_rd_HEAD(void);
void MCBCC_POAF(u8 enable);
void MCBCC_TRIG2PRO(u8 enable);
int MCBCC_send_HEAD(u32 head);
int MCBCC_send_DATA(u32 data);
void MCBCC_setup_PRO_reg_auto(void);
void MCBCC_int_function();
void MCBCC_lvds_2_stream_after_n_words_int_no_block(u32 num_words_to_stream,u32 num_words_before_stream, u8 last_behavior);
void MCBCC_print_regs(void);
int MCBCC_autonum(void);
int MCBCC_delay_fifo_ini(u8 delay_per_module,u8 num_bases);
u32 MCBCC_init(void);
void MCBCC_send_double_word(u32 head_word,u32 data_word);
u32* MCBCC_activate_emu(u8 mod,u8 submod);
void MCBCC_deactivate_emu(void);
void MCBCC_lvds_2_stream_after_n_words(u32 num_words_to_stream,u32 words_to_wait);
void MCBCC_lvds_2_stream_word_cond(u32 num_words_to_stream,u32 word_cond);
void MCBCC_stream_2_lvds(u32 num_words);
void MCBCC_zeros_2_lvds(u32 num_words);
u16 MCBCC_get_flags(u16 mask);
u16 MCBCC_get_reg_flags(u16 mask);
void MCBCC_amplia_2_stream_after_0_words_no_int(u32 num_words_to_stream);
u32 MCBCC_check_rd_stream_flag(void);
u32 MCBCC_get_PRO(void);
u32 MCBCC_get_PROI(void);
void MCBCC_set_PRO(u8 proval);
void MCBCC_int_disable(u32 flags);
void MCBCC_int_enable(u32 flags);
int MCBCC_write_word32_to_stream(u32 word,u8 last);
int MCBCC_get_rd_vld_in_flag(void);
u32 MCBCC_get_rd_stream_words_left(void);
int MCBCC_asign_callback(void (*callback)(void),unsigned char n_bit_flag);
int MCBCC_remove_callback(unsigned char n_bit_flag);


#endif /* _MCBCC_MST_DRIVER_H_ */
