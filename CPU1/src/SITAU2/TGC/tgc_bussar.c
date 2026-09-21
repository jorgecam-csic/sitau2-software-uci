
#include "bcc_bussar.h"
#include "xil_types.h"
#include "tgc_bussar.h"
#include "bussar_addr.h"
#include "math.h"
#include "global.h"
#include "log.h"

//int init_remote_TLV(u8 minibase)
//{
//	BCC_write_reg_inmediate(minibase,TGC_BUSSAR_SUBMOD_ADDR,DA_RAW_R,TGC_INIT_CONF_WORD);
//}
int prog_tgc_curve(u16* puntos_tgc, int num_puntos, u16 dir_ini, u8 minibase, commnd_buffer_t *commnd_buf_p)
{
int i;
int num_words=0;
int result;

	if ((result = push_bussar_write_reg((u32)dir_ini, minibase, TGC_BUSSAR_SUBMOD_ADDR, TGC_MEM_PNTR_R, commnd_buf_p)) < 0)
      return RLOG(result);

	num_words += result;
	for (i=0; i<num_puntos; i++)
	{
		if ((result = push_bussar_write_reg((u32)puntos_tgc[i], minibase, TGC_BUSSAR_SUBMOD_ADDR, TGC_MEM_DATA_R, commnd_buf_p)) < 0)
         return RLOG(result);
		num_words += result;
	}
	return num_words;
}


int prog_tgc_UT_regs(TGC_UT_struct_t *ut_regs,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	int num_words=0;
	int ret_val;
	tgc_general_ctrl_t tgc_stop;

	tgc_stop.tgc_general_ctrl_u32 = 0;
	tgc_stop.BITS.stop = 1;

	/*
	ret_val=push_bussar_write_reg((u32)tgc_stop.tgc_general_ctrl_u32,minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_GEN_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;
	 */
	ret_val=push_bussar_write_reg((u32)ut_regs->tgc_ini_delay,minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_INI_DELAY_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->tgc_mem_ini_end.tgc_mem_ini_end_u32,minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PROG_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->tgc_mem_ini_end.BITS.tgc_mem_ini,minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_MEM_PNTR_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->tgc_prescaler,minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_PRESCALER_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)ut_regs->tgc_general_ctrl.tgc_general_ctrl_u32,minibase,TGC_BUSSAR_SUBMOD_ADDR,TGC_GEN_R,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	return num_words;
}
