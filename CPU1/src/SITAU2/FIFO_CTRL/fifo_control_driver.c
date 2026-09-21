/*
 * fifo_control_driver.c
 *
 *  Created on: 2 abr. 2020
 *      Author: Jorge
 */
#include "fifo_control_driver.h"
#include "bussar_addr.h"

const char Fifo_sensor_char[NUM_FIFO_SENSORS][50] = {"AFE output FIFO","Beamformer input FIFO","Beamformer output0 FIFO","Beamformer output1 FIFO"};

void fifo_control_clear_all(u8 module)
{
	BCC_write_reg_inmediate(module,FIFO_CONTROL_SUBMOD_ADDR,OR_ALL_FLAGS,0x0);
}

u32 fifo_control_get_or_flag(u8 module)
{
	int ret;
	u32 flags;
	ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,OR_ALL_FLAGS,&(flags));
	if(ret)
		xil_printf("\n\rError leyendo registro en la funcion fifo_control_get_or_flag()\n\r");

	return flags;
}

u32 fifo_control_get_all_flags(u8 module,fifo_control_regs_t *fifo_ctrl_regs)
{
	int ret;

	if((ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,OR_ALL_FLAGS,&(fifo_ctrl_regs->or_all_flags)))<0) goto error_fifo_control_get_all_flags;

	if((ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,OVR_FLW_FLAG,&(fifo_ctrl_regs->overflow)))<0) goto error_fifo_control_get_all_flags;

	if((ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,NOT_END_FLAG,&(fifo_ctrl_regs->not_ends)))<0) goto error_fifo_control_get_all_flags;

	if((ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,PRE_TRG_FLAG,&(fifo_ctrl_regs->samples_pre_trigger)))<0) goto error_fifo_control_get_all_flags;

	if((ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,NOT_TRG_FLAG,&(fifo_ctrl_regs->not_triggered_frames)))<0) goto error_fifo_control_get_all_flags;

	if((ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,TRIGGER_CNT,&(fifo_ctrl_regs->trigger_count)))<0) goto error_fifo_control_get_all_flags;

	return 0;

error_fifo_control_get_all_flags:

	xil_printf("\n\rError leyendo registro en la funcion fifo_control_get_all_flags()\n\r");

	return ret;
}

fifo_control_counters_t fifo_control_get_counters(u8 module,int fifo_sensor_number)
{
	fifo_control_counters_t fifo_cnt;
	int ret;

	fifo_cnt.fifo_counter_word = 0;
	if((ret=BCC_read_reg(module,FIFO_CONTROL_SUBMOD_ADDR,COUNTERS+fifo_sensor_number,&fifo_cnt.fifo_counter_word))<0)
		xil_printf("\n\rError leyendo registro en la funcion fifo_control_get_counters()\n\r");

	return fifo_cnt;
}

void fifo_control_print_all(u8 module)
{
	int fifo_idx;
	fifo_control_counters_t fifo_ctrl_cnt;
	fifo_control_regs_t fifo_ctrl_regs;
	xil_printf("\n\r");
	xil_printf("idx: Numero de fifo\n\r");
	xil_printf("NTR: Error de trigger: Tramas terminadas sin que haya habido un trigger entre medias.\n\r");
	xil_printf("NED: Ha habido un trigger sin que se haya recibido el end de la anterior y habiéndose recibido alguna muestra.\n\r");
	xil_printf("PRE: Ha habido muestras entre el final de una trama y el trigger de la siguiente.\n\r");
	xil_printf("OVF: Habían muestras válidas sin que el RDY estuviese a 1.\n\r");

	xil_printf("\n\ridx NTR NED PRE OVF\n\r");

	for(fifo_idx=0;fifo_idx<NUM_FIFO_SENSORS;fifo_idx++)
	{
		fifo_ctrl_cnt = fifo_control_get_counters(module,fifo_idx);
		xil_printf("%2d: ",fifo_idx);
		xil_printf("%3d %3d %3d %3d - ",fifo_ctrl_cnt.BITS.not_triggered_frames,fifo_ctrl_cnt.BITS.samples_pre_trigger,fifo_ctrl_cnt.BITS.not_ends,fifo_ctrl_cnt.BITS.overflows);
		xil_printf("%s\n\r",Fifo_sensor_char[fifo_idx]);
	}
	fifo_control_get_all_flags(module,&fifo_ctrl_regs);
	xil_printf("\n\rFLAGS:\n\r");
	xil_printf("OR_ALL_FLAGS: 0x%08X\n\r",fifo_ctrl_regs.or_all_flags);
	xil_printf("OVR_FLW_FLAG: 0x%08X\n\r",fifo_ctrl_regs.overflow);
	xil_printf("NOT_END_FLAG: 0x%08X\n\r",fifo_ctrl_regs.not_ends);
	xil_printf("PRE_TRG_FLAG: 0x%08X\n\r",fifo_ctrl_regs.samples_pre_trigger);
	xil_printf("NOT_TRG_FLAG: 0x%08X\n\r",fifo_ctrl_regs.not_triggered_frames);

	xil_printf("\n\rNumero de disparos: %d\n\r",fifo_ctrl_regs.trigger_count);

}
