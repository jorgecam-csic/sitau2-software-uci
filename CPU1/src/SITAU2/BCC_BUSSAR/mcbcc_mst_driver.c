/*
 * mcbcc_mst_driver.c
 *
 *  Created on: 11 de oct. de 2016
 *      Author: csic
 */
#include "mcbcc_mst_driver.h"
#include "log.h"
#include "uci_reg.h"
#include "xil_types.h"

void (*MCBCC_Callback_array[15])(void);

const u32 MCBCC_Baseaddress = XPAR_UCI_AXI_MASTER_BASEADDR;
volatile u16 MCBCC_int_flags_copy = 0x0000;

const char MCBCC_reg_names[16][32] = {
		"WR_WORD_HEAD\t\t",
		"WR_WORD_DATA\t\t",
		"WR_STATUS\t\t",
		"WR_WORD_COUNT\t",
		"RD_WORD_HEAD\t\t",
		"RD_WORD_DATA\t\t",
		"RD_STATUS\t\t",
		"RD_WORD_COUNT\t",
		"WR_RD_DIFF\t\t",
		"RD_STREAM_COND\t",
		"RD_STREAM_WORD\t",
		"REG_TO_STREAM_DATA\t",
		"BUSSAR_AXIL_TIMOUT\t",
		"BUSSAR_AXIL_CONF\t",
		"PRO_REG\t\t",
		"INTERRUPTS\t\t"};

int MCBCC_print_int_flags(u16 flags)
{
	if(flags&(1<<TRE_NBIT)) xil_printf("TRE FLAG: TRigger Error bit:          ON\n\r");
	if(flags&(1<<TRG_NBIT)) xil_printf("TRG FLAG: TRiGger:                    ON\n\r");
	if(flags&(1<<RSE_NBIT)) xil_printf("RSE FLAG: Read Stream End Flag:       ON\n\r");
	if(flags&(1<<WSE_NBIT)) xil_printf("WSE FLAG: Write Stream End Flag:      ON\n\r");
	if(flags&(1<<DRE_NBIT)) xil_printf("DRE FLAG: Data Read End Flag:         ON\n\r");
	if(flags&(1<<ERT_NBIT)) xil_printf("ERT FLAG: Emulator Read Timeout Flag: ON\n\r");
	if(flags&(1<<ERE_NBIT)) xil_printf("ERE FLAG: Emulator Read End Flag:     ON\n\r");
	if(flags&(1<<POF_NBIT)) xil_printf("POF FLAG: PRO Fall flank Flag:        ON\n\r");
	if(flags&(1<<POR_NBIT)) xil_printf("POR FLAG: PRO Rise flank Flag:        ON\n\r");
}

int MCBCC_get_read_stream_end(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	return (base[RD_STATUS]&SRA_MASK?0:1);
}

void MCBCC_clear_counters(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[WR_WORD_COUNT]=0;
	base[RD_WORD_COUNT]=0;
	base[WR_RD_DIFF]=0;
}
u32 MCBCC_get_rd_HEAD(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	return base[RD_WORD_HEAD];

}

int MCBCC_send_HEAD(u32 head)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[WR_WORD_HEAD]=head;
	return 0;
}
int MCBCC_send_DATA(u32 data)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[WR_WORD_DATA]=data;
	return 0;
}
void error_timeout_function(const char *timeout_access, u32 n_data, int data_count)
{
	xil_printf("\r\nUNEXPECTED TIMEOUT ON MCBCC: %s - %d data - %d read data",timeout_access, n_data, data_count);
}

void MCBCC_TRIG2PRO(u8 enable)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	if(enable)	base[PRO_REG]|= POAR;
	else 		base[PRO_REG]&=~POAR;
}
void MCBCC_POAF(u8 enable)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	if(enable)	base[PRO_REG]|= POAF;
	else 		base[PRO_REG]&=~POAF;
}
void MCBCC_setup_PRO_reg_auto(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[PRO_REG] = POAR|POAF;
}
u32 MCBCC_init(void)
{
	volatile u32 rd_word;
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	for(int bit_interrupt=0;bit_interrupt<16;bit_interrupt++)
		MCBCC_Callback_array[bit_interrupt]=NULL;
	base[WR_RD_DIFF]=0;
	base[RD_STATUS]=0x40000000;
	base[INTERRUPTS]=0;
	gic_utils_register_interrupt(MCBCC_INT_NUMBER,MCBCC_int_function);
	MCBCC_int_enable(RSEE_MASK|POFE_MASK|PORE_MASK);
	base[INTERRUPTS]|=EINT_MASK;
	rd_word=base[INTERRUPTS];


	return rd_word;
}

int MCBCC_asign_callback(void (*callback)(void),unsigned char n_bit_flag)
{
	if(n_bit_flag>=16) return -1;
	MCBCC_Callback_array[n_bit_flag]=callback;
	return 0;
}
int MCBCC_remove_callback(unsigned char n_bit_flag)
{
	if(n_bit_flag>=16) return -1;
	MCBCC_Callback_array[n_bit_flag]=NULL;
	return 0;
}

void MCBCC_int_function()
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 reg_temp;

	reg_temp=base[INTERRUPTS]&0xFFFF;
	MCBCC_int_flags_copy|=(u16)(reg_temp);

	//Llamamos a las callbacks
	for(int bit_interrupt=0;bit_interrupt<15;bit_interrupt++)
	{
		if((reg_temp&(1<<bit_interrupt))&&(MCBCC_Callback_array[bit_interrupt]!=NULL))
		{
			MCBCC_int_flags_copy&=(0xFFFFFFFF^(1<<bit_interrupt));
			MCBCC_Callback_array[bit_interrupt]();
		}
	}



	//Xil_DCacheFlushLine(&MCBCC_int_flags_copy);
	//dmb();
	//xil_printf("i");
//	if(reg_temp&PORF_MASK) xil_printf("R");
//	if(MCBCC_int_flags_copy&PORF_MASK) xil_printf("r");
	//if(reg_temp&POFF_MASK) xil_printf("F");
	//if(reg_temp&RSEF_MASK) xil_printf("B");
	//xil_printf("\n\rQUITAR!! MCBCC_int_function base[INTERRUPTS]=0x%08X!!!\n\r",reg_temp);
	//xil_printf("\n\rQUITAR!! MCBCC_int_flags_copy=0x%08X!!!\n\r",MCBCC_int_flags_copy);
}

void MCBCC_int_enable(u32 flags)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[INTERRUPTS]|=((flags)&0xFFFF0000);
}

void MCBCC_int_disable(u32 flags)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[INTERRUPTS]&=(~(flags&0xFFFF0000));
}

void MCBCC_print_regs(void)
{
	int i;
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	u32  copia_registros[16];

	for(i=0;i<16;i++)
		copia_registros[i]=base[i];

	xil_printf("\n");
	for(i=0;i<16;i++)
		xil_printf("\r\n%02d-%s: 0x%08X",i,MCBCC_reg_names[i],copia_registros[i]);
	xil_printf("\r\n");
}

void MCBCC_lvds_2_stream_after_n_words_int_no_block(u32 num_words_to_stream,u32 num_words_before_stream, u8 last_behavior)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 tmp;
	u32 rd_status_backup;
	rd_status_backup=base[RD_STATUS];
	base[INTERRUPTS]|=EINT_MASK|RSEE_MASK;						// Activo solo la interrupción de fin de lectura
	tmp=base[INTERRUPTS];										// Borro todas las interrupciones haciendo una lectura
	tmp=MCBCC_get_flags(RSEF_MASK);								//y eL flag  copia
	num_words_to_stream&=NUM_WORDS_MASK;						// Máscara (2^23)-1 bytes max
	//base[RD_STATUS]=RRDY_MASK|(last_behavior?RSOL_MASK:0)|num_words_to_stream;	// LVDS RDY, last comportamiento antiguo y el número de muestras hacia DMA
	base[RD_STATUS]=RRDY_MASK|(last_behavior<<22)|num_words_to_stream;				// LVDS RDY, last bien hecho y el y el número de muestras hacia DMA
	num_words_before_stream&=NUM_WORDS_MASK;					// Máscara (2^23)-1 bytes max
	base[RD_STREAM_COND]=CTSC_MASK|num_words_before_stream;	// Lectura desde el módulo AMPLIA o LVDS después de 'num_words_before_stream' palabras.
	//base[RD_STATUS]=rd_status_backup;
}

void MCBCC_set_PRO(u8 proval)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	if(proval)	base[PRO_REG]|= PROO;
	else 		base[PRO_REG]&=~PROO;

}
u32 MCBCC_get_PRO(void)
{
	volatile u32* base=(u32*)XPAR_UCI_AXI_MASTER_BASEADDR;
	return (base[PRO_REG]&PROO);
}


u32 MCBCC_get_PROI(void)
{
	volatile u32* base=(u32*)XPAR_UCI_AXI_MASTER_BASEADDR;
	return (base[PRO_REG]&PROI);
}
int MCBCC_autonum(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	u32 start_num=1;
	u32 timeout_corto=10000;
	volatile u32 read_val;
	int num_bases32;
	u32 cabecera_leida;

	base[WR_WORD_HEAD]=0x00112233; // Enviar algo para por si acaso
	base[WR_WORD_HEAD]=0x00445566; // Enviar algo para por si acaso

	read_val=base[RD_STATUS];
	base[WR_WORD_HEAD]=0x0900E000|start_num; // Autonum
	num_bases32 = -1;
	while(timeout_corto--)
	{
		read_val=base[RD_STATUS]&RVHD_MASK;
		if(read_val)
		{
			cabecera_leida=base[RD_WORD_HEAD];
			if ((cabecera_leida&0xFFFFFF00)!=0x0900E000)
				num_bases32 = -1;
			else
				num_bases32 = (cabecera_leida&0x000000FF) - start_num;

			break;

		}
	}

	return num_bases32;

}

int MCBCC_delay_fifo_ini(u8 delay_per_module,u8 num_bases)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	u32 start_num=1;
	u32 timeout_corto=100;
	volatile u32 read_val;
	int num_bases32;
	u32 cabecera_leida;
	u16 retardo_maximo;
	read_val=base[RD_STATUS];
	base[WR_WORD_HEAD]=0x09008000|delay_per_module; // retardo por módulo.

	timeout_corto=5000;
	while(timeout_corto--)
	{
		read_val=base[RD_STATUS]&RVHD_MASK;
		if(read_val)
		{
			cabecera_leida=base[RD_WORD_HEAD]; // por debug
			break;
		}
	}


	//retardo_maximo = 4 + delay_per_module*num_bases;
	//prueba
	retardo_maximo = 0 + delay_per_module*num_bases;

	if(retardo_maximo>(1<<MODULE_FIFO_DELAY_BITS)-1)
	{
		xil_printf("\n\rERROR en MCBCC_delay_fifo_ini, el retardo del primer módulo supera el maximo permitido por el hardware:\n\r Profundidad de la fifo = %d\n\r retardo_maximo = %d\n\r",(1<<(MODULE_FIFO_DELAY_BITS))-1, retardo_maximo);
		return -1;
	}

	timeout_corto=500;
	base[WR_WORD_HEAD]=0x0900C000|retardo_maximo; // retardo del primer módulo.
	while(timeout_corto--)
	{
		read_val=base[RD_STATUS]&RVHD_MASK;
		if(read_val)
		{
			cabecera_leida=base[RD_WORD_HEAD]; // por debug
			break;
		}
	}

	{
		char retardo_minibase_2,retardo_minibase_1;
		/*
		base[WR_WORD_HEAD]=0x11024000;
		timeout_corto=500;
		while(timeout_corto--)
		{
			read_val=base[RD_STATUS]&RVHD_MASK;
			if(read_val)
			{
				cabecera_leida=base[RD_WORD_HEAD]; // por debug
				break;
			}
		}

		retardo_minibase_2=cabecera_leida & 0xFFF;
		base[WR_WORD_HEAD]=0x09024000 | (retardo_minibase_2+3); */
		/*
		base[WR_WORD_HEAD]=0x11014000;
		timeout_corto=500;
		while(timeout_corto--)
		{
			read_val=base[RD_STATUS]&RVHD_MASK;
			if(read_val)
			{
				cabecera_leida=base[RD_WORD_HEAD]; // por debug
				break;
			}
		}

		retardo_minibase_1=cabecera_leida & 0xFFF;
		base[WR_WORD_HEAD]=0x09014000 | (retardo_minibase_1-4);
	*/
	}

	return (cabecera_leida&0xFFF);

}

void MCBCC_send_double_word(u32 head_word,u32 data_word)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[WR_WORD_HEAD]=head_word;
	base[WR_WORD_DATA]=data_word;
}

u32* MCBCC_activate_emu(u8 mod,u8 submod)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[BUSSAR_AXIL_CONF]=(mod<<16)|(submod<<8);
	base[BUSSAR_AXIL_TIMOUT]=(0x80000000)|((base[BUSSAR_AXIL_TIMOUT]&0xFFFFFF));
	base[WR_RD_DIFF]=0;
	return &base[BUSSAR_AXIL_REG0];
}
void MCBCC_deactivate_emu(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[BUSSAR_AXIL_TIMOUT]&=(~0x80000000);
}

void MCBCC_lvds_2_stream_after_n_words(u32 num_words_to_stream,u32 words_to_wait)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 tmp;
	//base[INTERRUPTS]=0;
	tmp=base[INTERRUPTS];
	base[RD_STATUS]=0x40000000|num_words_to_stream;
	base[RD_STREAM_COND]=0x40000000|words_to_wait;
}

void MCBCC_amplia_2_stream_after_0_words_no_int(u32 num_words_to_stream)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 tmp;
	//base[INTERRUPTS]=0;
	tmp=base[INTERRUPTS];							// Aunque no activo las interrupciones, borro los flag para luego asegurar que ha terminado.
	base[RD_STATUS]=0x40C00000|num_words_to_stream;	// LVDS RDY, ultima palabra con LST a 1 y el número de muestras hacia DMA
	base[RD_STREAM_COND]=0x40000000;				// Lectura desde el módulo AMPLIA después de 0 palabras.
}

u32 MCBCC_check_rd_stream_flag(void)				// Esta funcion borra los flags anteriores
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	u32 ret;
	ret=base[INTERRUPTS];
	return ret&RSEF_MASK;
}

void MCBCC_lvds_2_stream_word_cond(u32 num_words_to_stream,u32 word_cond)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[RD_STREAM_WORD]=word_cond;
	base[RD_STATUS]=0x00800000|num_words_to_stream; //Por defecto se deja el last como esté
	base[RD_STREAM_COND]=0x80000000;
}

void MCBCC_stream_2_lvds(u32 num_words)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	base[WR_STATUS]=0x10000000|num_words;
}
void MCBCC_zeros_2_lvds(u32 num_words)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	u32 comando=(1<<26)|num_words;
	base[WR_STATUS]=comando;
}

u16 MCBCC_get_flags(u16 mask)
{
	u16 ret_val;
	u16 act_mask;
	if(mask==0) act_mask=0xFFFF;
	else act_mask=mask;
	//Xil_ExceptionDisable();
	ret_val=MCBCC_int_flags_copy&act_mask;
	if(mask!=0 && ret_val != 0)	MCBCC_int_flags_copy&=(~act_mask);
	//Xil_ExceptionEnable();
	return (ret_val);
}
u16 MCBCC_get_reg_flags(u16 mask)
{
	u16 ret_val;
	u16 act_mask;
	volatile u32* base=(u32*)MCBCC_Baseaddress;

	if(mask==0) act_mask=0xFFFF;
	else act_mask=mask;

	ret_val=base[INTERRUPTS]&act_mask;

	return (ret_val);
}
int MCBCC_write_word32_to_stream(u32 word,u8 last)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	volatile u32 reg;

	reg=base[RD_STATUS];

	if(reg&SRA_MASK)
		return MCBCC_MST_ERR_READ_OP_BUSY;

	base[RD_STREAM_COND]=RTS_MASK|(last?RTSL_MASK:0);

	reg=base[RD_STREAM_COND]; // Borramos los flags antes de escribir
	base[REG_TO_STREAM_DATA]=word;

	reg=base[RD_STREAM_COND];
	while(reg&(RTSA_MASK|RTSN_MASK))
	{
		reg=base[RD_STREAM_COND];
		usleep_timer(2);
	}
	if(reg&RTSN_MASK) return MCBCC_MST_ERR_READ_OP_STREAM_NRDY;

	return 0;
}

int MCBCC_get_rd_vld_in_flag(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	return base[RD_STATUS]&RVLD_MASK;
}


u32 MCBCC_get_rd_stream_words_left(void)
{
	volatile u32* base=(u32*)MCBCC_Baseaddress;
	return base[RD_STATUS]&NUM_WORDS_MASK;
}
