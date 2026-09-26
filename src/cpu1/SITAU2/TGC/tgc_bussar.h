
#ifndef SRC_SITAU2_TGC_BUSSAR_H_
#define SRC_SITAU2_TGC_BUSSAR_H_

#define TGC_INIT_CONF_WORD	0x8000D001

typedef union tgc_mem_ini_end_u
{
	u32 tgc_mem_ini_end_u32;
	struct
	{
		u16 tgc_mem_ini;
		u16 tgc_mem_end;
	}BITS;
}tgc_mem_ini_end_t;

typedef struct tgc_general_ctrl_s
{
	union
	{
		u32 tgc_general_ctrl_u32;
		struct
		{
			u32 tgc_enable : 1; 		//Habilita la función de TGC. En general si hay TGC poner a 1
			u32 stream_end_correct : 1; //(LECTURA) Esto indica que el número de valores recibidos coinciden con los parámetros tgc_mem_ini y tgc_mem_end
			u32 last_received : 1;		//(LECTURA) Esto sirve para indicar que el último valor que se ha escrito estaba con el LAST a 1. Para asegurar que el stream mandado a la memoria es correcto
			u32 dac_sleep : 1;			//Modo bajo consumo del DAC
			u32 reserved   : 23;		//Nada
			u32 acq_end_ignore : 1;		//Si está a 1, aunque se haya terminado la adquisición, la TCG continúa. Normalmente a 0
			u32 stop : 1;				//Para sacar de estado raro. En general a 0
			u32 external_trigger: 1; 	//Habilita que la TGC se active con el PRO. En general si hay TGC poner a 1
			u32 not_busy : 1;			//Indica (a 0) si el subsistema está ocupado.
			u32 software_trigger : 1;	//Para hacer una prueba de funcionamiento y disparar por SW. Poner siempre a 0
		}BITS;
	};
}tgc_general_ctrl_t;

typedef struct tgc_ut_struct_s
{
	u32 tgc_ini_delay;						// Tiempo de ganancia inicial en pasos del presacler
	u32 tgc_prescaler;						// Prescaler de la TGC en intervalos de 10ns
	tgc_mem_ini_end_t tgc_mem_ini_end;		// Puntos de la memoria de la TGC iniciales y finales
	tgc_general_ctrl_t tgc_general_ctrl;	// Configuración general
}TGC_UT_struct_t;

#define TGC_GEN_R			0
#define TGC_INI_DELAY_R 	1
#define PRESENT_ATTN_R	 	2 // Ganancia fija
#define TGC_PRESCALER_R 	3
#define TGC_MEM_DATA_R		4
#define TGC_MEM_PROG_R		5
#define TGC_MEM_PNTR_R		6
#define MAIN_DELAY_ATTN_R	7

int prog_tgc_curve(u16* puntos_tgc, int num_puntos, u16 dir_ini, u8 minibase, commnd_buffer_t *commnd_buf_p);
int prog_tgc_UT_regs(TGC_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p);
//int init_remote_TLV(u8 minibase);

#endif /* SRC_SITAU2_TGC_BUSSAR_H_ */
