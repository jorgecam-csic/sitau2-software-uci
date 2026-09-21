/*
 * beamform.c
 *
 *  Created on: 4 nov. 2019
 *      Author: csic
 */

#include "beamformer_bussar.h"
#include "bcc_bussar.h"
#include "xil_types.h"
#include "pulser_bussar.h"
#include "bussar_addr.h"
#include "math.h"
#include "global.h"
#include "switch_driver.h"
#include "mcbcc_mst_driver.h"


void beamformer_prog_cos_pwi_reg(beamformer_ut_struct_t *bf_ut_struct,u16 *cos_pwi_mem, u32 focal_law,u8 minibase)
{
	bf_ut_struct->cos_pwi = (u32)(cos_pwi_mem[focal_law]);
	BCC_write_reg_inmediate(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_COS_PWI_REG,(u32)bf_ut_struct->cos_pwi);
}

int beamformer_print_ctrl_reg(bf_general_ctrl_t *ctrl_reg)
{
	xil_printf("\n\rctrl_reg:\n\r");

	xil_printf("[ 0]ctrl_reg->BITS.unarmed:            %01X\n\r",ctrl_reg->BITS.unarmed);
	xil_printf("[ 1]ctrl_reg->BITS.init_start:         %01X\n\r",ctrl_reg->BITS.init_start);
	xil_printf("[ 2]ctrl_reg->BITS.external_trigger:   %01X\n\r",ctrl_reg->BITS.external_trigger);
	xil_printf("[ 3]ctrl_reg->BITS.param_rdy:          %01X\n\r",ctrl_reg->BITS.param_rdy);
	xil_printf("\n\r");
	xil_printf("[ 4]ctrl_reg->BITS.end_beamform:       %01X\n\r",ctrl_reg->BITS.end_beamform);
	xil_printf("[ 5]ctrl_reg->BITS.acquiring:          %01X\n\r",ctrl_reg->BITS.acquiring);
	xil_printf("[ 6]ctrl_reg->BITS.trigger_software:   %01X\n\r",ctrl_reg->BITS.trigger_software);
	xil_printf("[ 7]ctrl_reg->BITS.param_stream_err:   %01X\n\r",ctrl_reg->BITS.param_stream_err);
	xil_printf("\n\r");
	xil_printf("[ 8]ctrl_reg->BITS.start_param_deser:  %01X\n\r",ctrl_reg->BITS.start_param_deser);
	xil_printf("[ 9]ctrl_reg->BITS.busy_param_deser:   %01X\n\r",ctrl_reg->BITS.busy_param_deser);
	xil_printf("[10]ctrl_reg->BITS.end_param_deser:    %01X\n\r",ctrl_reg->BITS.end_param_deser);
	xil_printf("[11]ctrl_reg->BITS.serial_param_prog:  %01X\n\r",ctrl_reg->BITS.serial_param_prog);
	xil_printf("\n\r");
	xil_printf("[15-12]ctrl_reg->BITS.serial_bf_start: %01X\n\r",ctrl_reg->BITS.serial_bf_start);
	xil_printf("\n\r");
	xil_printf("[16]ctrl_reg->BITS.break_line_planner: %01X\n\r",ctrl_reg->BITS.break_line_planner);
	xil_printf("[17]ctrl_reg->BITS.enable_line_planner:%01X\n\r",ctrl_reg->BITS.enable_line_planner);
	xil_printf("[18]ctrl_reg->BITS.end_beamform_cycle: %01X\n\r",ctrl_reg->BITS.end_beamformer_cycle);
	xil_printf("[19]ctrl_reg->BITS.end_shot_cycle:     %01X\n\r",ctrl_reg->BITS.end_shot_cycle);
	xil_printf("\n\r");
	xil_printf("[20]ctrl_reg->BITS.end_image_cycle:    %01X\n\r",ctrl_reg->BITS.end_image_cycle);
	xil_printf("[21]ctrl_reg->BITS.reserved0:          %01X\n\r",ctrl_reg->BITS.reserved0);
	xil_printf("[22]ctrl_reg->BITS.auto_sequence:      %01X\n\r",ctrl_reg->BITS.auto_sequence);
	xil_printf("[23]ctrl_reg->BITS.go_idle:            %01X\n\r",ctrl_reg->BITS.go_idle);
	xil_printf("\n\r");
	xil_printf("[28-24]ctrl_reg->BITS.dec_beamformer: %02X\n\r",ctrl_reg->BITS.dec_beamformer);
	xil_printf("[30-29]ctrl_reg->BITS.reserved1:       %01X\n\r",ctrl_reg->BITS.reserved1);
	xil_printf("[31]ctrl_reg->BITS.not_busy:           %01X\n\r",ctrl_reg->BITS.not_busy);
	xil_printf("\n\r");

	return 0;
}

int beamformer_print_ida_reg(bits_ida_t *ida_reg)
{
	xil_printf("\n\rctrl_reg:\n\r");

	xil_printf("[ 0]ida_reg->BITS.tfm:                   %01X\n\r",ida_reg->BITS.tfm);
	xil_printf("[ 1]ida_reg->BITS.pwi:                   %01X\n\r",ida_reg->BITS.pwi);
	xil_printf("[ 1]ida_reg->BITS.ipa:                   %01X\n\r",ida_reg->BITS.ipa);
	xil_printf("[6-3]ida_reg->BITS.reserved:             %01X\n\r",ida_reg->BITS.reserved);
	xil_printf("[ 7]ida_reg->BITS.param_stream_err_ida:  %01X\n\r",ida_reg->BITS.param_stream_err_ida);
	xil_printf("[ 8]ida_reg->BITS.start_param_deser_ida: %01X\n\r",ida_reg->BITS.start_param_deser_ida);
	xil_printf("[ 9]ida_reg->BITS.busy_param_deser_ida:  %01X\n\r",ida_reg->BITS.busy_param_deser_ida);
	xil_printf("[10]ida_reg->BITS.end_param_deser_ida:   %01X\n\r",ida_reg->BITS.end_param_deser_ida);
	xil_printf("[31-11]ida_reg->BITS.reserved1:          %01X\n\r",ida_reg->BITS.reserved1);
	xil_printf("\n\r");

	return 0;
}

int beamformer_get_regs(beamformer_ut_struct_t *bf_ut_struct,u8 minibase)
{
	int error=0;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,&bf_ut_struct->bf_gen_ctrl.bf_general_ctrl_u32						)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_SAMPLES_REG  ,&bf_ut_struct->n_foci        										)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PAR_LINES_REG,&bf_ut_struct->parallel_lines.parallel_lines_u32					)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_IMG_LINES_REG,&bf_ut_struct->image_lines.image_lines_u32   						)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PROG_ADDR_REG,&bf_ut_struct->prog_addr_ini_end.prog_addr_ini_end_u32				)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_BITS_REG_IDA_REG,&bf_ut_struct->bits_ida.bits_ida_u32								)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PROG_ADDR_IDA_REG,&bf_ut_struct->prog_addr_ini_end_ida.prog_addr_ini_end_ida_u32	)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_LINE_PLAN_ADDR_REG,&bf_ut_struct->ln_planner_reg.ln_planner_reg_u32				)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_SHIFT_BITS_REG,&bf_ut_struct->shft_bits											)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_IMAGES_FL_LOADED_REG,&bf_ut_struct->images_fl_loaded								)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_FWD_FL_LOADED_REG,&bf_ut_struct->fw_fl_loaded		 								)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_COS_PWI_REG,&bf_ut_struct->cos_pwi			    								)) error++;
	if(BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_T0_BF_OFFSET_REG,&bf_ut_struct->T0_BF.T0[0]										)) error++;

	return error;
}

int beamformer_print_regs(beamformer_ut_struct_t *bf_ut_struct)
{

	//beamformer_print_ctrl_reg(&bf_ut_struct->bf_gen_ctrl);
	//beamformer_print_ida_reg(&bf_ut_struct->bits_ida);

	xil_printf("n_foci:                 %d\n\r",bf_ut_struct->n_foci);
	xil_printf("total_parallel_lines:   %d\n\r",bf_ut_struct->parallel_lines.BITS.parallel_lines);
	xil_printf("current_parallel_lines: %d\n\r",bf_ut_struct->parallel_lines.BITS.current_parallel_lines);
	xil_printf("image_lines:            %d\n\r",bf_ut_struct->image_lines.BITS.image_lines);
	xil_printf("current_image_lines:    %d\n\r",bf_ut_struct->image_lines.BITS.current_image_lines);
	xil_printf("actual_prgaddr_ini:     %d\n\r",bf_ut_struct->prog_addr_ini_end.BITS.prog_addr_ini);
	xil_printf("prog_addr_end:          %d\n\r",bf_ut_struct->prog_addr_ini_end.BITS.prog_addr_end);
	xil_printf("actual_prgaddr_ini_ida: %d\n\r",bf_ut_struct->prog_addr_ini_end_ida.BITS.prog_addr_ini_ida);
	xil_printf("prog_addr_end_ida:      %d\n\r",bf_ut_struct->prog_addr_ini_end_ida.BITS.prog_addr_end_ida);
	xil_printf("ln_plan_start_addr:     %d\n\r",bf_ut_struct->ln_planner_reg.BITS.ln_plan_start_addr);
	xil_printf("ln_plan_ida_offset:     %d\n\r",bf_ut_struct->ln_planner_reg.BITS.ln_plan_ida_offset);
	xil_printf("shft_bits:              %d\n\r",bf_ut_struct->shft_bits);
	xil_printf("images_fl_loaded:       %d\n\r",bf_ut_struct->images_fl_loaded);
	xil_printf("fw_fl_loaded:           %d\n\r",bf_ut_struct->fw_fl_loaded);
	xil_printf("cos_pwi:                %d\n\r",bf_ut_struct->cos_pwi);
	xil_printf("T0_BEAMFORMMING:        %d\n\r",bf_ut_struct->T0_BF.T0[0]);

	return 0;
}
int beamformer_SW_start(beamformer_ut_struct_t *bf_ut_struct,u8 minibase)
{
	bf_general_ctrl_t reg_tmp;
	reg_tmp.bf_general_ctrl_u32=0x0;
	reg_tmp.BITS.dec_beamformer=bf_ut_struct->bf_gen_ctrl.BITS.dec_beamformer;
	reg_tmp.BITS.trigger_software=1;
	BCC_write_reg_inmediate(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,reg_tmp.bf_general_ctrl_u32);

	return 0;

}
int beamformer_Init_start(beamformer_ut_struct_t *bf_ut_struct,u8 minibase)
{
	bf_general_ctrl_t reg_tmp;
	int dummy_wait;
	reg_tmp.bf_general_ctrl_u32=0x0;
	reg_tmp.BITS.dec_beamformer=bf_ut_struct->bf_gen_ctrl.BITS.dec_beamformer;
	reg_tmp.BITS.init_start=1;
	reg_tmp.BITS.external_trigger=1;
	reg_tmp.BITS.auto_sequence=1;

	BCC_write_reg_inmediate(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,reg_tmp.bf_general_ctrl_u32);
	for(dummy_wait=0;dummy_wait<100;dummy_wait++);
}

int beamformer_Stop_auto_sequence(u8 minibase)
{
	bf_general_ctrl_t reg_tmp;

	reg_tmp.bf_general_ctrl_u32=0x0;
	reg_tmp.BITS.break_line_planner=1;

	BCC_write_reg_inmediate(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,reg_tmp.bf_general_ctrl_u32);

	return 0;

}

int beamformer_set_ln_plan_reg(u32 ln_planner_reg_u32,u8 minibase)
{

	BCC_write_reg_inmediate(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_LINE_PLAN_ADDR_REG,ln_planner_reg_u32);

	return 0;
}



int beamformer_prog_registros(beamformer_ut_struct_t *bf_ut_struct,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	int num_words=0;
	int ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->n_foci,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_SAMPLES_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->parallel_lines.parallel_lines_u32,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PAR_LINES_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->image_lines.image_lines_u32,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_IMG_LINES_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->prog_addr_ini_end.prog_addr_ini_end_u32,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PROG_ADDR_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->prog_addr_ini_end_ida.prog_addr_ini_end_ida_u32,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PROG_ADDR_IDA_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->ln_planner_reg.ln_planner_reg_u32,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_LINE_PLAN_ADDR_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((s32)bf_ut_struct->shft_bits,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_SHIFT_BITS_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->T0_BF.T0[0],minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_T0_BF_OFFSET_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((s32)bf_ut_struct->bits_ida.bits_ida_u32,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_BITS_REG_IDA_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	ret_val=push_bussar_write_reg((u32)bf_ut_struct->bf_gen_ctrl.bf_general_ctrl_u32,minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,commnd_buf_p);
	if(ret_val<0) return ret_val;
	num_words+=ret_val;

	return num_words;

}

int beamformer_prog_params(u32 *focal_laws_p,int focal_laws_words32,beamformer_ut_struct_t *bf_ut_struct,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	u32* sswitch;
	//static int num_execs=0;
	//num_execs++;

	//Configuramos Registros beamformer
	{
		commnd_buf_p->total_len = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.auto_sequence = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.break_line_planner = 1;
		bf_ut_struct->bf_gen_ctrl.BITS.go_idle = 1;
		beamformer_prog_registros(bf_ut_struct,minibase,commnd_buf_p);
		BCC_SendBuffer(commnd_buf_p);
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
	}
	{
		commnd_buf_p->total_len = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.auto_sequence = 1;
		bf_ut_struct->bf_gen_ctrl.BITS.break_line_planner = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.go_idle = 0;
		beamformer_prog_registros(bf_ut_struct,minibase,commnd_buf_p);
		BCC_SendBuffer(commnd_buf_p);
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
	}

	//Configuramos SWITCH
	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BFM_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}

	//Configuramos BCC en modo stream escritura
	{
		bcc_header_t comando_escritura_stream;
		bcc_header_t comando_extesion_longitud;
		u16 LSW,MSW;
		u32 n_data;

		n_data = focal_laws_words32;

		LSW=(u16)(n_data&0xFFFF);
		MSW=(u16)(n_data>>16);

		comando_escritura_stream.HEADER = 0x00000000;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
		comando_escritura_stream.STREAM_CMD.MODULE  = minibase;
		comando_escritura_stream.STREAM_CMD.LENGHT = LSW;

		if(MSW)
		{
			comando_extesion_longitud.HEADER = 0x00000000;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
			comando_extesion_longitud.SPECIAL_CMD.MODULE = minibase;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		}
		if(MSW)
			MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
		MCBCC_send_HEAD(comando_escritura_stream.HEADER);
	}

	//Enviamos los parámetros
	{
		commnd_buffer_t param_buf;
		param_buf.data = focal_laws_p;
		param_buf.total_len = focal_laws_words32;
		param_buf.max_len = focal_laws_words32;

		BCC_SendBuffer(&param_buf);
	}

	//Comprobamos OK
	{
		int ret;
		bf_general_ctrl_t bf_ctrl;
		image_lines_t image_lines;

		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_IMG_LINES_REG,&image_lines.image_lines_u32);
		if(ret<0)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: no se recibió la respuesta de lectura posterior al envío del stream.\n\r");
			return -1;
		}
		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,&bf_ctrl.bf_general_ctrl_u32);

		if(ret<0)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: no se recibió la respuesta de lectura posterior al envío del stream.\n\r");
			return -1;
		}
		if(bf_ctrl.BITS.param_stream_err)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: El conformador indicó error al recibir los parámetros: Número de datos no múltiplo de una línea.\n\r");
			beamformer_print_ctrl_reg(&bf_ctrl);
			return -1;
		}
		if(!bf_ctrl.BITS.end_param_deser)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: el envío finalizó, pero el conformador no indicó que terminó de recibir los parámetros.\n\r");
			beamformer_print_ctrl_reg(&bf_ctrl);
			return -1;
		}
	}

	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BFM_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,1);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}

	if(0)
	{
		commnd_buf_p->total_len = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.init_start = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.enable_line_planner = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.break_line_planner = 1;
		beamformer_prog_registros(bf_ut_struct,minibase,commnd_buf_p);
		BCC_SendBuffer(commnd_buf_p);
	}
	if(0)
	{
		commnd_buf_p->total_len = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.init_start = 1;
		bf_ut_struct->bf_gen_ctrl.BITS.enable_line_planner = 1;
		bf_ut_struct->bf_gen_ctrl.BITS.break_line_planner = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.go_idle = 0;
		beamformer_prog_registros(bf_ut_struct,minibase,commnd_buf_p);
		BCC_SendBuffer(commnd_buf_p);
	}

	return 0;
}

int beamformer_prog_one_fwrd_focal_law(u32 *focal_laws_p,int n_lines,u32 offset,beamformer_ut_struct_t *bf_ut_struct,commnd_buffer_t *commnd_buf_p)
{
	u8 minibase=0;

	u32* sswitch;
	bits_ida_t registro_ida;
	u32 focal_laws_words32=n_lines*BEAMFORMER_SIZE_FMC_FWD_WORD32;

	if (bf_ut_struct->bits_ida.BITS.tfm)
		focal_laws_words32=n_lines*BEAMFORMER_SIZE_FMC_FWD_WORD32;
	else if (bf_ut_struct->bits_ida.BITS.pwi)
		focal_laws_words32=n_lines*BEAMFORMER_SIZE_PWI_FWD_WORD32;
	else if (bf_ut_struct->bits_ida.BITS.ipa)
		focal_laws_words32 = 0;
	else
		focal_laws_words32 = 0;


	if(focal_laws_words32 == 0) //Para IPA, no deberíamos grabar ninguna FL ida
		return 0;


	//Configuramos Registros beamformer
	{
		commnd_buf_p->total_len = 0;
		bf_ut_struct->prog_addr_ini_end_ida.BITS.prog_addr_ini_ida=(u16)offset;
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.break_line_planner = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.go_idle = 0;
		bf_ut_struct->bits_ida.BITS.start_param_deser_ida = 1;
		beamformer_prog_registros(bf_ut_struct,minibase,commnd_buf_p);
		BCC_SendBuffer(commnd_buf_p);
		bf_ut_struct->bits_ida.BITS.start_param_deser_ida = 0;
	}



	//Configuramos SWITCH
	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BFI_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}

	//Configuramos BCC en modo stream escritura
	{
		bcc_header_t comando_escritura_stream;
		bcc_header_t comando_extesion_longitud;
		u16 LSW,MSW;
		u32 n_data;

		n_data = focal_laws_words32;

		LSW=(u16)(n_data&0xFFFF);
		MSW=(u16)(n_data>>16);

		comando_escritura_stream.HEADER = 0x00000000;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
		comando_escritura_stream.STREAM_CMD.MODULE  = minibase;
		comando_escritura_stream.STREAM_CMD.LENGHT = LSW;

		if(MSW)
		{
			comando_extesion_longitud.HEADER = 0x00000000;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
			comando_extesion_longitud.SPECIAL_CMD.MODULE = minibase;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		}
		if(MSW)
			MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
		MCBCC_send_HEAD(comando_escritura_stream.HEADER);
	}

	//Enviamos los parámetros
	{
		commnd_buffer_t param_buf;
		param_buf.data = focal_laws_p;
		param_buf.total_len = focal_laws_words32;
		param_buf.max_len = focal_laws_words32;

		BCC_SendBuffer(&param_buf);
	}

	//Comprobamos OK
	{
		int ret;
		bits_ida_t bits_ida_ctrl;
		prog_addr_ini_end_ida_t prog_addr_ida;
		bf_general_ctrl_t bf_ctrl_bits;

		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PROG_ADDR_IDA_REG,&prog_addr_ida.prog_addr_ini_end_ida_u32);

		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_BITS_REG_IDA_REG,&bits_ida_ctrl.bits_ida_u32);

		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,&bf_ctrl_bits.bf_general_ctrl_u32);

		if(ret<0)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: no se recibió la respuesta de lectura posterior al envío del stream.\n\r");
			return -1;
		}
		if(bits_ida_ctrl.BITS.param_stream_err_ida)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: El conformador indicó error al recibir los parámetros: Número de datos no múltiplo de una línea.\n\r");
			beamformer_print_ida_reg(&bits_ida_ctrl);
			beamformer_print_ctrl_reg(&bf_ctrl_bits);
			//return -1;
		}
		if(!bits_ida_ctrl.BITS.end_param_deser_ida)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: el envío finalizó, pero el conformador no indicó que terminó de recibir los parámetros.\n\r");
			beamformer_print_ida_reg(&bits_ida_ctrl);
			beamformer_print_ctrl_reg(&bf_ctrl_bits);
			//return -1;
		}
	}

	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BFI_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,1);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}


	return 0;
}


int beamformer_fwrd_params(u32 *focal_laws_p,int focal_laws_words32,beamformer_ut_struct_t *bf_ut_struct,u8 minibase,commnd_buffer_t *commnd_buf_p)
{
	u32* sswitch;
	//static int num_execs=0;
	//num_execs++;
	bits_ida_t registro_ida;
	//Configuramos Registros beamformer
	if(0){
		commnd_buf_p->total_len = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.start_param_deser = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.break_line_planner = 0;
		bf_ut_struct->bf_gen_ctrl.BITS.go_idle = 0;
		bf_ut_struct->bits_ida.BITS.start_param_deser_ida = 1;
		beamformer_prog_registros(bf_ut_struct,minibase,commnd_buf_p);
		BCC_SendBuffer(commnd_buf_p);
		bf_ut_struct->bits_ida.BITS.start_param_deser_ida = 0;
	}
	{
		BCC_write_reg_inmediate(minibase, BEAMFORMER_BUSSAR_SUBMOD_ADDR, BEAMFORMER_PROG_ADDR_IDA_REG, bf_ut_struct->prog_addr_ini_end_ida.prog_addr_ini_end_ida_u32);
		bf_ut_struct->bits_ida.BITS.start_param_deser_ida = 1;
		BCC_write_reg_inmediate(minibase, BEAMFORMER_BUSSAR_SUBMOD_ADDR, BEAMFORMER_BITS_REG_IDA_REG, bf_ut_struct->bits_ida.bits_ida_u32);
		bf_ut_struct->bits_ida.BITS.start_param_deser_ida = 0;
	}



	//Configuramos SWITCH
	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BFI_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,0);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}

	//Configuramos BCC en modo stream escritura
	{
		bcc_header_t comando_escritura_stream;
		bcc_header_t comando_extesion_longitud;
		u16 LSW,MSW;
		u32 n_data;

		n_data = focal_laws_words32;

		LSW=(u16)(n_data&0xFFFF);
		MSW=(u16)(n_data>>16);

		comando_escritura_stream.HEADER = 0x00000000;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.ROS = 1;
		comando_escritura_stream.STREAM_CMD.CTRL.BITS.WR  = 1;
		comando_escritura_stream.STREAM_CMD.MODULE  = minibase;
		comando_escritura_stream.STREAM_CMD.LENGHT = LSW;

		if(MSW)
		{
			comando_extesion_longitud.HEADER = 0x00000000;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.SPC = 1;
			comando_extesion_longitud.SPECIAL_CMD.CTRL.BITS.WR = 1;
			comando_extesion_longitud.SPECIAL_CMD.MODULE = minibase;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_DIR = SPCREG_WORD_COUNT_EXT_REG;
			comando_extesion_longitud.SPECIAL_CMD.SPC_WORD.BITS.SPC_VAL = MSW;
		}
		if(MSW)
			MCBCC_send_HEAD(comando_extesion_longitud.HEADER);
		MCBCC_send_HEAD(comando_escritura_stream.HEADER);
	}

	//Enviamos los parámetros
	{
		commnd_buffer_t param_buf;
		param_buf.data = focal_laws_p;
		param_buf.total_len = focal_laws_words32;
		param_buf.max_len = focal_laws_words32;

		BCC_SendBuffer(&param_buf);
	}

	//Comprobamos OK
	{
		int ret;
		bits_ida_t bits_ida_ctrl;
		prog_addr_ini_end_ida_t prog_addr_ida;
		bf_general_ctrl_t bf_ctrl_bits;


		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_PROG_ADDR_IDA_REG,&prog_addr_ida.prog_addr_ini_end_ida_u32);

		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_BITS_REG_IDA_REG,&bits_ida_ctrl.bits_ida_u32);

		ret = BCC_read_reg(minibase,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,&bf_ctrl_bits.bf_general_ctrl_u32);

		if(ret<0)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: no se recibió la respuesta de lectura posterior al envío del stream.\n\r");
			return -1;
		}
		if(bits_ida_ctrl.BITS.param_stream_err_ida)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: El conformador indicó error al recibir los parámetros: Número de datos no múltiplo de una línea.\n\r");
			beamformer_print_ida_reg(&bits_ida_ctrl);
			beamformer_print_ctrl_reg(&bf_ctrl_bits);
			//return -1;
		}
		if(!bits_ida_ctrl.BITS.end_param_deser_ida)
		{
			xil_printf("Error al finalizar el envío de parámetros al conformador: el envío finalizó, pero el conformador no indicó que terminó de recibir los parámetros.\n\r");
			beamformer_print_ida_reg(&bits_ida_ctrl);
			beamformer_print_ctrl_reg(&bf_ctrl_bits);
			//return -1;
		}
	}

	{
		sswitch=MCBCC_activate_emu(minibase,SSWITCH032_BUSSAR_SUBMOD_ADDR);

		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX],SSWITCH032_MASTER_TO_BFI_IDX, SSWITCH032_SLAVE_FROM_BCC_IDX,1);

		switch_reg_update(stream_switch_base_addrs[REMOTE_SWITCH_TABLE_IDX]);

		MCBCC_deactivate_emu();
	}


	return 0;
}

