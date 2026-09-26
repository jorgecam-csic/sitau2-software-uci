// -----------------------------------------------------------------------------
/**
@file uci_set.c

@brief Este archivo contiene la implementaciÃ¯Â¿Â½n de las funciones de la clase \c FL_Aperture.<br>

Esta clase tienen la funcionalidad necesaria para definir las aperturas que componen
un barrido.

@author (rg) Ricardo GonzÃ¯Â¿Â½lez

<pre>
MODIFICATION HISTORY:

Ver   Who  Date       Changes
----- ---- ---------- -----------------------------------------------------------
1.00a (rg) 16/01/2017 First release
</pre>
*/

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================
#include "gtx_control.h"
#include "vch_prg.h"
#include "log.h"
#include "vch_tad.h"
#include "uci_error_code.h"
#include "switch_driver.h"
#include "uci_reg.h"
#include "vch.h"
#include "datamover_remote.h"
#include "tgc_bussar.h"
#include "afe5808a.h"
#include "fir_filter.h"

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_PRG_EmissionFocalLaws(TVCH *vch)
{
int b,result;
TVCH_BASE *base = NULL;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	Global_commnd_buf.total_len = 0;
	for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)
	{
		if ((base = &vch->base[b]) == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
		if ((result = prog_pulser_focal_laws(base->efl, vch->n_efl * MOD_MAX_CH, 0, base->bcc_addr, &Global_commnd_buf)) < 0)
			return RLOG(result);
	}
	if (gb_hw_sitau_enabled == 1)
	{
		if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
			return RLOG(result);
	}
	Global_commnd_buf.total_len = 0;
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------

int VCH_PRG_Registers_FMC(TVCH *vch)
{
int result = EUCI_NONE;
u32 btt;
u32 addr;
u32 align_remainder;
bf_general_ctrl_t bf_gen_ctrl_tmp;
emi_prom_config_t emi_prom_config_rst;
const u8 bcc_addr_broadcast = 0;
u32 enabled_modules=0;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	Global_commnd_buf.total_len = 0;


	enabled_modules=(1<<MISC_PROENA_BIT_AFES)|(1<<MISC_PROENA_BIT_PULSER)|(1<<MISC_PROENA_BIT_TGC);// Datamover 256 lo activan los AFEs
	bcc_set_pro_and_busy_mask_buffer(bcc_addr_broadcast,enabled_modules,~enabled_modules,&Global_commnd_buf);

//	if ((result = config_mem2beamformer(0,0,0,0,0,bcc_addr_broadcast, &Global_commnd_buf)) < 0)
//		xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

	config_remote_datamover32_disable_read(bcc_addr_broadcast,&Global_commnd_buf);
	config_remote_datamover32_disable_write(bcc_addr_broadcast,&Global_commnd_buf);


	bf_gen_ctrl_tmp.bf_general_ctrl_u32 = 0;
	bf_gen_ctrl_tmp.BITS.auto_sequence = 1;
	bf_gen_ctrl_tmp.BITS.external_trigger = 0;

	if ((result = push_bussar_write_reg((u32)bf_gen_ctrl_tmp.bf_general_ctrl_u32,bcc_addr_broadcast,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,&Global_commnd_buf)) < 0)
		xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

	vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

	//vch->pulser.pul_general_ctrl.BITS.pulser_current = 3;
	vch->pulser.pul_general_ctrl.BITS.external_trigger = 1;

	//			vch->tgc.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

	vch->afe.AFE_BUSSAR_UT.external_trigger = 1;


  if ((result = prog_afe_SPI_regs(vch->afe.AFE_SPI_UT, bcc_addr_broadcast)) < 0)
	 xil_printf("ERROR_PROG_SPI_AFE\n\r");


	if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT, &Global_commnd_buf, bcc_addr_broadcast)) < 0)
		xil_printf("ERROR_PROG_AFE_REGS\n\r");


	if ((result = prog_pulser_UT_regs(&vch->pulser, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
		xil_printf("ERROR_PROG_PULSER_REGS\n\r");

	//Esto igual no se debe hacer aqui
	//if ((result = prog_tgc_curve(base->tgc.curve,base->tgc.n_points, base->tgc_config.tgc_mem_ini_end.BITS.tgc_mem_ini, bcc_addr_broadcast,&Global_commnd_buf)) < 0)
		//xil_printf("ERROR_PROG_TGC_CURVE\n\r");

	if ((result = prog_tgc_UT_regs(&vch->tgc.cfg, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
		xil_printf("ERROR_PROG_TGC_REGS\n\r");

	emi_prom_config_rst.emi_prom_config_u32 = 0x0;
	emi_prom_config_rst.BITS.ce_bits = 0xFF;

	if ((result = emi_prom_prog(&emi_prom_config_rst, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
		xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");
	if ((result = emi_prom_prog(&vch->emi_prom_config_acquisition, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
		xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

	btt = vch->afe.AFE_BUSSAR_UT.num_samples*32*2; //cambiar 32 por numero de scans a adquirir, de momento el maximo, 32
	addr = vch->remote_addr_ini;
	align_remainder = addr%FP_REMOTE_ALIGN_SIZE;

	if (align_remainder)
		addr = addr - align_remainder + FP_REMOTE_ALIGN_SIZE;

	//DESCONFIGURACIÓN DE MEM2BEAMFORMER!!! CUDIDADO, PRUEBA!
	if ((result = config_mem2beamformer(0,0,0,0,0,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
		xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");


	//if(gb_uci.acquisition_mode==AQUISITION_MODE_CIRCULAR_BUFFER)
	if(1)//Ahora ya solo hay modo circular buffer
	{
		if ((result = config_remote_256bit_datamover_stream2mem((u32)vch->remote_addr_ini,vch->remote_addr_end,btt,btt,1,0,1,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");
	}
	else
	{
		if ((result = config_afe2mem((u32)addr,btt,btt,1,0,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");
	}


	//Configuramos SWITCH (AFE-->MEM)
	if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_DMA_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
		xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");

	if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_DMA_IDX,1,&Global_commnd_buf)) < 0)
		xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");

	if ((result = switch_remote_512_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
		xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");


	if(gb_shared->mk32_version & MK32_FILT_FMC_PCIE_MSK) // Bitstream sin comformador y con filtros FMC con PCIe
	{
		static u8 shift=15;
		config_FIR_coeficients_remote_get_commands(vch->fir.coefficients,&Global_commnd_buf);
		config_FIR_interleaving_remote_get_commands(&Global_commnd_buf);
		config_FIR_bit_shift_remote_get_commands(shift,&Global_commnd_buf);
	}

	if (gb_hw_sitau_enabled == 1)
	{
	  if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		 xil_printf("ERROR_BCC_SEND\n\r");
	}


   // TGC *****************************************************

   // ...

   // TGC *****************************************************

   Global_commnd_buf.total_len = 0;

   return EUCI_NONE;
}


// -----------------------------------------------------------------------------
int VCH_PRG_Registers_no_broadcast(TVCH *vch)
{
int b, result = EUCI_NONE;
u32 btt;
u32 addr;
u32 align_remainder;
bf_general_ctrl_t bf_gen_ctrl_tmp;
emi_prom_config_t emi_prom_config_rst;
TVCH_BASE *base = NULL;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	Global_commnd_buf.total_len = 0;
	for (b=0; b<vch->n_bases; b++)
	{
		if ((base = &vch->base[b]) == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
		if ((result = config_mem2beamformer(0,0,0,0,0,base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		if ((result = config_mem2emi_prom_beamformer2mem((u32)0,0,0,0,0,base->bcc_addr,&Global_commnd_buf,0)) < 0)
			xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");

		bf_gen_ctrl_tmp.bf_general_ctrl_u32 = 0;
		bf_gen_ctrl_tmp.BITS.auto_sequence = 1;
		bf_gen_ctrl_tmp.BITS.external_trigger = 0;

		if ((result = push_bussar_write_reg((u32)bf_gen_ctrl_tmp.bf_general_ctrl_u32,base->bcc_addr,BEAMFORMER_BUSSAR_SUBMOD_ADDR,BEAMFORMER_CTRL_BITS_REG,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		//vch->pulser.pul_general_ctrl.BITS.pulser_mode = 3;
		vch->pulser.pul_general_ctrl.BITS.external_trigger = 1;

		//			vch->tgc.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		vch->afe.AFE_BUSSAR_UT.external_trigger = 1;

		if (gb_hw_sitau_enabled == 1)
		{
		  if ((result = prog_afe_SPI_regs(vch->afe.AFE_SPI_UT, base->bcc_addr)) < 0)
			 xil_printf("ERROR_PROG_SPI_AFE\n\r");
		}

		if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT, &Global_commnd_buf, base->bcc_addr)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");

		{
			ctrl_afe_t reg0_afe;
			reg0_afe.REG=0x00000000;
			reg0_afe.BIT.dec_ratio = vch->afe.AFE_BUSSAR_UT.dec_ratio;
			reg0_afe.BIT.external_trigger = vch->afe.AFE_BUSSAR_UT.external_trigger;
			reg0_afe.BIT.pwdn_glb = 1;

			push_bussar_write_reg(reg0_afe.REG,1,AFE_BUSSAR_SUBMOD_ADDR,AFE_CTRL_REG_OFFSET,&Global_commnd_buf);
		}

		if ((result = prog_pulser_UT_regs(&vch->pulser, base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_PULSER_REGS\n\r");

		//Esto igual no se debe hacer aqui
		//if ((result = prog_tgc_curve(base->tgc.curve,base->tgc.n_points, base->tgc_config.tgc_mem_ini_end.BITS.tgc_mem_ini, base->bcc_addr,&Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_TGC_CURVE\n\r");

		if ((result = prog_tgc_UT_regs(&vch->tgc.cfg, base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_TGC_REGS\n\r");

		emi_prom_config_rst.emi_prom_config_u32 = 0x0;
		emi_prom_config_rst.BITS.ce_bits = 0xFF;

		if ((result = emi_prom_prog(&emi_prom_config_rst, base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");
		if ((result = emi_prom_prog(&vch->emi_prom_config_acquisition, base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

		btt = vch->afe.AFE_BUSSAR_UT.num_samples*32*2; //cambiar 32 por numero de scans a adquirir, de momento el maximo, 32
		addr = vch->remote_addr_ini;
		align_remainder = addr%FP_REMOTE_ALIGN_SIZE;

		if (align_remainder)
			addr = addr - align_remainder + FP_REMOTE_ALIGN_SIZE;

		if ((result = config_afe2mem((u32)addr,btt,btt,1,0,base->bcc_addr,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");

		//Configuramos SWITCH (AFE-->MEM)
		if ((result = switch_remote_512_select_slave_get_command(base->bcc_addr,SSWITCH512_MASTER_TO_DMA_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");

		if ((result = switch_remote_512_select_slave_get_command(base->bcc_addr,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_DMA_IDX,1,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");

		if ((result = switch_remote_512_update_get_command(base->bcc_addr,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");

		//if ((id_base = base->bcc_addr) >= MAX_N_BASES) return ELOG(charEUCI_IdBASE, EUCI_IdBASE);
	}
	//xil_printf("\n\n*************SOBREESCRIBIENDO VALOR DE DIG_TGC_ATT*************\r\n");
	//base->afe.AFE_SPI_UT.reg59.BIT.DIG_TGC_ATT=1;
  
	if (gb_hw_sitau_enabled == 1)
	{
	  if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		 xil_printf("ERROR_BCC_SEND\n\r");
	}
   // TGC *****************************************************

   // ...
   
   // TGC *****************************************************
   
   Global_commnd_buf.total_len = 0;

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_PRG_BeamformerRegisters(TVCH *vch)
{
	int b, result = EUCI_NONE;
	u32 btt_mem2beamformer;
	u32 btt_beamformer2mem;
	u32 inc_beamformer2mem;
	u32 addr_mem2beamformer;
	u8 ce_man_emi_prom;
	int one_line;
	u8 emi_prom_enable,emi0_prom1;
	TVCH_BASE *base = NULL;
	bf_general_ctrl_t bf_general_ctrl_temp;

	bf_general_ctrl_temp = vch->bf_regs.bf_gen_ctrl;
	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	Global_commnd_buf.total_len = 0;
	for (b=0; b<vch->n_bases; b++)
	{
		if ((base = &vch->base[b]) == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
		emi_prom_enable = vch->emi_prom_config_beamforming.BITS.enable && (vch->emi_prom_config_beamforming.BITS.prom || vch->emi_prom_config_beamforming.BITS.emi);
		if (emi_prom_enable) emi0_prom1 = vch->emi_prom_config_beamforming.BITS.prom;

		//Los registros TFM a 0:
		vch->bf_regs.prog_addr_ini_end_ida.BITS.prog_addr_ini_ida = 0;
//		vch->bf_regs.bits_ida.bits_ida_u32 = 0;
//		vch->bf_regs.bits_ida.BITS.tfm = 0;
//		vch->bf_regs.bits_ida.BITS.pwi = 0;
		//base->beamformer.regs.shft_bits = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_start_addr = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_ida_offset = 0;

		//Poniendo el conformador con 0(+1) lÃ­neas en paralelo y "serial_param_prog" desactivamos el parallel beamforming
		one_line = (vch->bf_regs.parallel_lines.BITS.parallel_lines == 0) && (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 1);

		//vch->bf_regs.bf_gen_ctrl.bf_general_ctrl_u32 = 0;
		vch->bf_regs.bf_gen_ctrl.BITS.auto_sequence = 1;
		vch->bf_regs.bf_gen_ctrl.BITS.external_trigger = 1;

		if ((result = beamformer_prog_registros(&vch->bf_regs,base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		btt_mem2beamformer = vch->afe.AFE_BUSSAR_UT.num_samples*32*2;

		addr_mem2beamformer = vch->remote_addr_ini;

		btt_beamformer2mem = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;

		if (one_line) inc_beamformer2mem = btt_beamformer2mem;
		else inc_beamformer2mem = btt_beamformer2mem * NUM_BEAMFORMER_DATAMOVERS;


		vch->afe.AFE_BUSSAR_UT.external_trigger = 0;
		if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT,&Global_commnd_buf,base->bcc_addr)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");

		vch->pulser.pul_general_ctrl.BITS.external_trigger = 0;
		if ((result = prog_pulser_UT_regs(&vch->pulser,base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");

		if ((result = prog_tgc_curve(&vch->tgc.curve[0],vch->tgc.n_points, vch->tgc.cfg.tgc_mem_ini_end.BITS.tgc_mem_ini, base->bcc_addr, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");

		if ((result = config_afe2mem((u32)0,0,0,0,0,base->bcc_addr,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");

		if ((result = config_mem2beamformer(addr_mem2beamformer,btt_mem2beamformer,btt_mem2beamformer,1,0,base->bcc_addr,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		if(emi_prom_enable==0)
		{
			if ((result = config_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, 1, 0, base->bcc_addr, &Global_commnd_buf,0)) < 0)
				xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		}
		else if(emi0_prom1==0)
		{
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr, btt_beamformer2mem, 0, base->bcc_addr, &Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");
		}
		else if(emi0_prom1==1)
		{
			u32 btt_beamformer2mem_scratch_prom=btt_beamformer2mem*2;//En promediado, los datos intermedios son a 32 bits en vez de 16
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr,btt_beamformer2mem_scratch_prom,0,base->bcc_addr,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");
		}

		ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_EMIPROMBF0)|(1<<EMI_PROM_MAN_CS_BFDATAMOV0);
		vch->emi_prom_config_beamforming.BITS.ce_bits = ce_man_emi_prom;

		if ((result = emi_prom_prog(&(vch ->emi_prom_config_beamforming),base->bcc_addr,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

		//Configuramos SWITCH256 (MEM-->512)
		if ((result = switch_remote_256_select_slave_get_command(base->bcc_addr,SSWITCH256_MASTER_TO_512_IDX,SSWITCH256_SLAVE_FROM_MEM_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_256\n\r");
		if ((result = switch_remote_256_select_slave_get_command(base->bcc_addr,SSWITCH256_MASTER_TO_GTX_IDX,SSWITCH256_SLAVE_FROM_MEM_IDX,1,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_256\n\r");
		if ((result = switch_remote_256_update_get_command(base->bcc_addr,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_256\n\r");

		//Configuramos SWITCH512 (512-->BFM)
		if ((result = switch_remote_512_select_slave_get_command(base->bcc_addr,SSWITCH512_MASTER_TO_DMA_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,1,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_512\n\r");
		if ((result = switch_remote_512_select_slave_get_command(base->bcc_addr,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_512\n\r");
		if ((result = switch_remote_512_update_get_command(base->bcc_addr,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_512\n\r");

	}
	if (gb_hw_sitau_enabled == 1)
	{
		if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		return RLOG(result);
	}

	return EUCI_NONE;
}

int VCH_PRG_Registers_PA_DMA(TVCH *vch)
{
	const u8 bcc_addr_broadcast = 0;
	int result;
	u32 enabled_modules;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	Global_commnd_buf.total_len = 0;

	enabled_modules=(1<<MISC_PROENA_BIT_BFMR)|(1<<MISC_PROENA_BIT_DMVR_BFMR)|(1<<MISC_PROENA_BIT_TGC)|(1<<MISC_PROENA_BIT_AFES)|(1<<MISC_PROENA_BIT_PULSER);// Datamover 256 lo activan los AFEs
	bcc_set_pro_and_busy_mask_buffer(bcc_addr_broadcast,enabled_modules,~enabled_modules,&Global_commnd_buf);

	{
		u32 btt;
		u32 addr;
		u32 align_remainder;
		bf_general_ctrl_t bf_gen_ctrl_tmp;
		emi_prom_config_t emi_prom_config_rst;


		if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);


		vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		//vch->pulser.pul_general_ctrl.BITS.pulser_current = 3;
		vch->pulser.pul_general_ctrl.BITS.external_trigger = 1;

		//vch->tgc.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		vch->afe.AFE_BUSSAR_UT.external_trigger = 1;

		if (gb_hw_sitau_enabled == 1)
		{
		  if ((result = prog_afe_SPI_regs(vch->afe.AFE_SPI_UT, bcc_addr_broadcast)) < 0)
			 xil_printf("ERROR_PROG_SPI_AFE\n\r");
		}

		if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT, &Global_commnd_buf, bcc_addr_broadcast)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");


		if ((result = prog_pulser_UT_regs(&vch->pulser, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_PULSER_REGS\n\r");

		//Esto igual no se debe hacer aqui
		//if ((result = prog_tgc_curve(base->tgc.curve,base->tgc.n_points, base->tgc_config.tgc_mem_ini_end.BITS.tgc_mem_ini, bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_TGC_CURVE\n\r");

		if ((result = prog_tgc_UT_regs(&vch->tgc.cfg, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_TGC_REGS\n\r");

		emi_prom_config_rst.emi_prom_config_u32 = 0x0;
		emi_prom_config_rst.BITS.ce_bits = 0xFF;

		//if ((result = emi_prom_prog(&emi_prom_config_rst, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

//		if ((result = emi_prom_prog(&vch->emi_prom_config_acquisition, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
//			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");
/*
		btt = vch->afe.AFE_BUSSAR_UT.num_samples*32*2;
		addr = vch->remote_addr_ini;
		align_remainder = addr%FP_REMOTE_ALIGN_SIZE;

		if (align_remainder)
			addr = addr - align_remainder + FP_REMOTE_ALIGN_SIZE;
*/

		//Configuramos SWITCH (AFE-->BFM)
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_DMA_IDX,SSWITCH512_SLAVE_FROM_DMA_IDX,1,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		if ((result = switch_remote_512_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		if ((result = config_afe2mem((u32)0,0,0,0,0,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");

	}


	{
		int b, result = EUCI_NONE;
		u32 btt_beamformer2mem;
		u32 inc_beamformer2mem;
		u8 ce_man_emi_prom;
		int one_line;
		u8 emi_prom_enable,emi0_prom1;
		TVCH_BASE *base = NULL;


		emi_prom_enable = vch->emi_prom_config_beamforming.BITS.enable && (vch->emi_prom_config_beamforming.BITS.prom || vch->emi_prom_config_beamforming.BITS.emi);

		if (emi_prom_enable) emi0_prom1 = vch->emi_prom_config_beamforming.BITS.prom;

		//Los registros TFM a 0:
		vch->bf_regs.prog_addr_ini_end_ida.BITS.prog_addr_ini_ida = 0;
//		vch->bf_regs.bits_ida.bits_ida_u32 = 0;
//		vch->bf_regs.bits_ida.BITS.tfm = 0;
//		vch->bf_regs.bits_ida.BITS.pwi = 0;
		//base->beamformer.regs.shft_bits = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_start_addr = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_ida_offset = 0;

		//Poniendo el conformador con 0(+1) lÃ­neas en paralelo y "serial_param_prog" desactivamos el parallel beamforming
		one_line = (vch->bf_regs.parallel_lines.BITS.parallel_lines == 0) && (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 1);


		vch->bf_regs.bf_gen_ctrl.BITS.auto_sequence = 1;
		vch->bf_regs.bf_gen_ctrl.BITS.external_trigger = 1;

		if ((result = beamformer_prog_registros(&vch->bf_regs,bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		btt_beamformer2mem = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;

		if (one_line) inc_beamformer2mem = btt_beamformer2mem;
		else inc_beamformer2mem = btt_beamformer2mem * NUM_BEAMFORMER_DATAMOVERS;

		if(emi_prom_enable==0)
		{
			if ((result = config_beamformer2mem_circular(vch->remote_addr_bf_ini,vch->remote_addr_bf_end, btt_beamformer2mem, inc_beamformer2mem, 1, 0,1,bcc_addr_broadcast, &Global_commnd_buf,0)) < 0)
				xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		}
		else if(emi0_prom1==0)
		{
//			              config_mem2emi_prom_beamformer2mem(u32 res_addr           ,u32 res_btt        , u32 res_autoinc   , u32 scratch_addr          , u32 scratch_btt   , u32 scratch_autoinc, u8 minibase       , commnd_buffer_t *commnd_buf)
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr, btt_beamformer2mem,                   0, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");

		}
		else if(emi0_prom1==1)
		{
			u32 btt_beamformer2mem_scratch_prom=btt_beamformer2mem*2;//En promediado, los datos intermedios son a 32 bits en vez de 16
//			              config_mem2emi_prom_beamformer2mem(u32 res_addr           ,u32 res_btt        , u32 res_autoinc   , u32 scratch_addr          , u32 scratch_btt               , u32 scratch_autoinc, u8 minibase       , commnd_buffer_t *commnd_buf)
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr,btt_beamformer2mem_scratch_prom,                   0, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");

		}
		if(1)
		{
			if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_SMT_IDX,SSWITCH032_SLAVE_FROM_DDR_IDX,0,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_REMOTE_SWITCH_032\n\r");
			if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_SMP_IDX,SSWITCH032_SLAVE_FROM_BCC_IDX,0,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_REMOTE_SWITCH_032\n\r");
			if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_BCC_IDX,SSWITCH032_SLAVE_FROM_SUM_IDX,0,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_REMOTE_SWITCH_032\n\r");
		}


		if ((result = switch_remote_032_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_032\n\r");

		ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_EMIPROMBF0)|(1<<EMI_PROM_MAN_CS_BFDATAMOV0)|(1<<EMI_PROM_MAN_CS_BEAMFORMER);
		vch->emi_prom_config_beamforming.BITS.ce_bits = ce_man_emi_prom;
		if ((result = emi_prom_prog(&(vch->emi_prom_config_beamforming),bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

	}

	if (gb_hw_sitau_enabled == 1)
	{
		if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		return RLOG(result);
	}

	return EUCI_NONE;
}

int VCH_PRG_Registers_TFM(TVCH *vch)
{
	const u8 bcc_addr_broadcast = 0;
	int result = EUCI_NONE;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	Global_commnd_buf.total_len = 0;


	//AFE
	{
		u32 btt;
		u32 addr;
		u32 align_remainder;
		emi_prom_config_t emi_prom_config_rst;
		const u8 bcc_addr_broadcast = 0;

		vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		//vch->pulser.pul_general_ctrl.BITS.pulser_current = 3;
		vch->pulser.pul_general_ctrl.BITS.external_trigger = 1;
		vch->pulser.pul_general_ctrl.BITS.pulser_auto_load = 0;
		vch->pulser.pul_general_ctrl.BITS.auto_prom        = 0;

		//			vch->tgc.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		vch->afe.AFE_BUSSAR_UT.external_trigger = 1;


		if ((result = prog_afe_SPI_regs(vch->afe.AFE_SPI_UT, bcc_addr_broadcast)) < 0)
			xil_printf("ERROR_PROG_SPI_AFE\n\r");


		if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT, &Global_commnd_buf, bcc_addr_broadcast)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");


		if ((result = prog_pulser_UT_regs(&vch->pulser, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_PULSER_REGS\n\r");

		//Esto igual no se debe hacer aqui
		//if ((result = prog_tgc_curve(base->tgc.curve,base->tgc.n_points, base->tgc_config.tgc_mem_ini_end.BITS.tgc_mem_ini, bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_TGC_CURVE\n\r");

		if ((result = prog_tgc_UT_regs(&vch->tgc.cfg, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_TGC_REGS\n\r");

		emi_prom_config_rst.emi_prom_config_u32 = 0x0;
		emi_prom_config_rst.BITS.ce_bits = 0xFF;

		if ((result = emi_prom_prog(&emi_prom_config_rst, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");
		if ((result = emi_prom_prog(&vch->emi_prom_config_acquisition, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

		btt = vch->afe.AFE_BUSSAR_UT.num_samples*32*2; //cambiar 32 por numero de scans a adquirir, de momento el maximo, 32
		addr = vch->remote_addr_ini;
		align_remainder = addr%FP_REMOTE_ALIGN_SIZE;

//		if (align_remainder)
//			addr = addr - align_remainder + FP_REMOTE_ALIGN_SIZE;

//		//El caso de TFM es muy simple. Siempre la misma dirección, en lectura y en escritura
//		if ((result = config_afe2mem((u32)addr,btt,0,1,0,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
//				xil_printf("ERROR_PROG_AFE2MEM\n\r");
		//El caso de TFM ERA muy simple. Ahora usamos circular
		            //config_remote_256bit_datamover_stream2mem(u32 start_addr           ,u32 end_addr        ,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 circular_mem,u8 minibase       ,commnd_buffer_t *commnd_buf)

		if ((result = config_remote_256bit_datamover_stream2mem((u32)vch->remote_addr_ini,vch->remote_addr_end,btt    ,btt         ,1                  ,0          ,1              ,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");
		//Pero para la lectura tenemos que usar el trigger secundario del datamover
		            //config_mem2beamformer_sec_trig_circular(u32 start_addr           ,u32 end_addr        ,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 sec_trigger,u8 circular_mem,u8 minibase       ,commnd_buffer_t *commnd_buf)
		if ((result = config_mem2beamformer_sec_trig_circular((u32)vch->remote_addr_ini,vch->remote_addr_end,btt    ,0           ,0                  ,0          ,1             ,0              ,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_MEM2BF\n\r");

	}



	{
		int b, result = EUCI_NONE;
		u32 btt_beamformer2mem;
		u32 inc_beamformer2mem;
		u8 ce_man_emi_prom;
		int one_line;
		u8 emi_prom_enable,emi0_prom1;
		TVCH_BASE *base = NULL;

//
//		emi_prom_enable = vch->emi_prom_config_beamforming.BITS.enable && (vch->emi_prom_config_beamforming.BITS.prom || vch->emi_prom_config_beamforming.BITS.emi);
//
//		if (emi_prom_enable) emi0_prom1 = vch->emi_prom_config_beamforming.BITS.prom;

		//Los registros TFM a 0:
		vch->bf_regs.prog_addr_ini_end_ida.BITS.prog_addr_ini_ida = 0;
//		vch->bf_regs.bits_ida.bits_ida_u32 = 0;
//		vch->bf_regs.bits_ida.BITS.tfm = 0;
//		vch->bf_regs.bits_ida.BITS.pwi = 0;
		//base->beamformer.regs.shft_bits = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_start_addr = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_ida_offset = 0;

		//Poniendo el conformador con 0(+1) lÃ­neas en paralelo y "serial_param_prog" desactivamos el parallel beamforming
		one_line = (vch->bf_regs.parallel_lines.BITS.parallel_lines == 0) && (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 1);


		vch->bf_regs.bf_gen_ctrl.BITS.auto_sequence = 1;
		vch->bf_regs.bf_gen_ctrl.BITS.external_trigger = 1;
		//vch->bf_regs.shft_bits = 5;

		if ((result = beamformer_prog_registros(&vch->bf_regs,bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		btt_beamformer2mem = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;

		if (one_line) inc_beamformer2mem = btt_beamformer2mem;
		else inc_beamformer2mem = btt_beamformer2mem * NUM_BEAMFORMER_DATAMOVERS;

		if(vch->n_efl==1)
		{
//			              config_beamformer2mem_circular(u32 start_addr         ,u32 end_addr           ,u32 btt           ,u32 auto_inc      ,u8 external_trigger,u8 trig_now,u8 circular_mem,u8 minibase       ,commnd_buffer_t *commnd_buf,u8 writeonly0_readwrite1)
			if ((result = config_beamformer2mem_circular(vch->remote_addr_bf_ini,vch->remote_addr_bf_end,btt_beamformer2mem,btt_beamformer2mem,1                  ,0          ,0              ,bcc_addr_broadcast, &Global_commnd_buf        ,0                       )) < 0)
				xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");
		}
		else
		{
			u32 btt_beamformer2mem_scratch_prom;
			u32 inc_beamformer2mem_scratch_prom;

			btt_beamformer2mem_scratch_prom=btt_beamformer2mem*2;//En promediado, los datos intermedios son a 32 bits en vez de 16
			inc_beamformer2mem_scratch_prom=btt_beamformer2mem_scratch_prom * NUM_BEAMFORMER_DATAMOVERS;
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr,btt_beamformer2mem_scratch_prom,inc_beamformer2mem_scratch_prom,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");

		}


		if ((result = switch_remote_032_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_032\n\r");

		ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_EMIPROMBF0)|(1<<EMI_PROM_MAN_CS_BFDATAMOV0); //IMPORTANTE, AQUÍ NO
		vch->emi_prom_config_beamforming.BITS.ce_bits = ce_man_emi_prom;
		if ((result = emi_prom_prog(&(vch->emi_prom_config_beamforming),bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

	}

	//SWITCHES
	{

		//Configuramos SWITCH 512 (AFE-->DDR)
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_DMA_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		//Configuramos SWITCH 512 (DDR-->BFM)
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_DMA_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		//UPDATE SWITCH 512
		if ((result = switch_remote_512_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");

		//Configuramos SWITCH 32 (PROM-->DDR)
		//UPDATE SWITCH 32
		if ((result = switch_remote_032_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_032\n\r");


	}
	if (gb_hw_sitau_enabled == 1)
	{
		if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		return RLOG(result);
	}
	sw256_remote_mem_to_512_switch(bcc_addr_broadcast,0);
	sw256_remote_mem_to_gtx_switch(bcc_addr_broadcast,1);

	return EUCI_NONE;
}

int VCH_PRG_Registers_TFM_no_int(TVCH *vch)
{
	const u8 bcc_addr_broadcast = 0;
	int result = EUCI_NONE;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	Global_commnd_buf.total_len = 0;


	//AFE
	{
		u32 btt;
		u32 addr;
		u32 align_remainder;
		emi_prom_config_t emi_prom_config_rst;
		const u8 bcc_addr_broadcast = 0;

		vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		//vch->pulser.pul_general_ctrl.BITS.pulser_current = 3;
		vch->pulser.pul_general_ctrl.BITS.external_trigger = 1;
		vch->pulser.pul_general_ctrl.BITS.pulser_auto_load = 0;
		vch->pulser.pul_general_ctrl.BITS.auto_prom        = 0;

		//			vch->tgc.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		vch->afe.AFE_BUSSAR_UT.external_trigger = 1;


		if ((result = prog_afe_SPI_regs(vch->afe.AFE_SPI_UT, bcc_addr_broadcast)) < 0)
			xil_printf("ERROR_PROG_SPI_AFE\n\r");


		if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT, &Global_commnd_buf, bcc_addr_broadcast)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");


		if ((result = prog_pulser_UT_regs(&vch->pulser, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_PULSER_REGS\n\r");

		//Esto igual no se debe hacer aqui
		//if ((result = prog_tgc_curve(base->tgc.curve,base->tgc.n_points, base->tgc_config.tgc_mem_ini_end.BITS.tgc_mem_ini, bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_TGC_CURVE\n\r");

		if ((result = prog_tgc_UT_regs(&vch->tgc.cfg, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_TGC_REGS\n\r");

		emi_prom_config_rst.emi_prom_config_u32 = 0x0;
		emi_prom_config_rst.BITS.ce_bits = 0xFF;

		if ((result = emi_prom_prog(&emi_prom_config_rst, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");
		if ((result = emi_prom_prog(&vch->emi_prom_config_acquisition, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

		btt = vch->afe.AFE_BUSSAR_UT.num_samples*32*2; //cambiar 32 por numero de scans a adquirir, de momento el maximo, 32
		addr = vch->remote_addr_ini;
		align_remainder = addr%FP_REMOTE_ALIGN_SIZE;

//		if (align_remainder)
//			addr = addr - align_remainder + FP_REMOTE_ALIGN_SIZE;

		//El caso de TFM es muy simple. Siempre la misma dirección, en lectura y en escritura
		if ((result = config_afe2mem((u32)addr,btt,0,1,0,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_AFE2MEM\n\r");
		//Pero para la lectura tenemos que usar el trigger secundario del datamover
		if ((result = config_mem2beamformer_sec_trig(addr,btt,0,0,0,1,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_MEM2BF\n\r");

	}



	{
		int b, result = EUCI_NONE;
		u32 btt_beamformer2mem;
		u32 inc_beamformer2mem;
		u8 ce_man_emi_prom;
		int one_line;
		u8 emi_prom_enable,emi0_prom1;
		TVCH_BASE *base = NULL;

//
//		emi_prom_enable = vch->emi_prom_config_beamforming.BITS.enable && (vch->emi_prom_config_beamforming.BITS.prom || vch->emi_prom_config_beamforming.BITS.emi);
//
//		if (emi_prom_enable) emi0_prom1 = vch->emi_prom_config_beamforming.BITS.prom;

		//Los registros TFM a 0:
		vch->bf_regs.prog_addr_ini_end_ida.BITS.prog_addr_ini_ida = 0;
//		vch->bf_regs.bits_ida.bits_ida_u32 = 0;
//		vch->bf_regs.bits_ida.BITS.tfm = 0;
//		vch->bf_regs.bits_ida.BITS.pwi = 0;
		//base->beamformer.regs.shft_bits = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_start_addr = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_ida_offset = 0;

		//Poniendo el conformador con 0(+1) lÃ­neas en paralelo y "serial_param_prog" desactivamos el parallel beamforming
		one_line = (vch->bf_regs.parallel_lines.BITS.parallel_lines == 0) && (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 1);


		vch->bf_regs.bf_gen_ctrl.BITS.auto_sequence = 1;
		vch->bf_regs.bf_gen_ctrl.BITS.external_trigger = 1;
		//vch->bf_regs.shft_bits = 5;

		if ((result = beamformer_prog_registros(&vch->bf_regs,bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		btt_beamformer2mem = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;

		if (one_line) inc_beamformer2mem = btt_beamformer2mem;
		else inc_beamformer2mem = btt_beamformer2mem * NUM_BEAMFORMER_DATAMOVERS;

		if(vch->n_efl==1)
		{
//			              config_beamformer2mem_circular(u32 start_addr         ,u32 end_addr           ,u32 btt           ,u32 auto_inc      ,u8 external_trigger,u8 trig_now,u8 circular_mem,u8 minibase       ,commnd_buffer_t *commnd_buf,u8 writeonly0_readwrite1)
			if ((result = config_beamformer2mem_circular(vch->remote_addr_bf_ini,vch->remote_addr_bf_end,btt_beamformer2mem,btt_beamformer2mem,1                  ,0          ,0              ,bcc_addr_broadcast, &Global_commnd_buf        ,0                       )) < 0)
				xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");
		}
		else
		{
			u32 btt_beamformer2mem_scratch_prom;
			u32 inc_beamformer2mem_scratch_prom;

			btt_beamformer2mem_scratch_prom=btt_beamformer2mem*2;//En promediado, los datos intermedios son a 32 bits en vez de 16
			inc_beamformer2mem_scratch_prom=btt_beamformer2mem_scratch_prom * NUM_BEAMFORMER_DATAMOVERS;
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr,btt_beamformer2mem_scratch_prom,inc_beamformer2mem_scratch_prom,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");

		}


		if ((result = switch_remote_032_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_032\n\r");

		ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_EMIPROMBF0)|(1<<EMI_PROM_MAN_CS_BFDATAMOV0); //IMPORTANTE, AQUÍ NO
		vch->emi_prom_config_beamforming.BITS.ce_bits = ce_man_emi_prom;
		if ((result = emi_prom_prog(&(vch->emi_prom_config_beamforming),bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

	}

	//SWITCHES
	{

		//Configuramos SWITCH 512 (AFE-->DDR)
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_DMA_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		//Configuramos SWITCH 512 (DDR-->BFM)
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_DMA_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		//UPDATE SWITCH 512
		if ((result = switch_remote_512_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");

		//Configuramos SWITCH 32 (PROM-->DDR)
		//UPDATE SWITCH 32
		if ((result = switch_remote_032_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_032\n\r");


	}
	if (gb_hw_sitau_enabled == 1)
	{
		if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		return RLOG(result);
	}
	sw256_remote_mem_to_512_switch(bcc_addr_broadcast,0);
	sw256_remote_mem_to_gtx_switch(bcc_addr_broadcast,1);

	return EUCI_NONE;
}

int VCH_PRG_BeamformerRegisters_LVDS_DMA_no_prom_allowed(TVCH *vch)
{
	const u8 bcc_addr_broadcast = 0;
	int result;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);


	{
		u32 btt;
		u32 addr;
		u32 align_remainder;
		bf_general_ctrl_t bf_gen_ctrl_tmp;
		emi_prom_config_t emi_prom_config_rst;


		if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

		Global_commnd_buf.total_len = 0;


		vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		//vch->pulser.pul_general_ctrl.BITS.pulser_current = 3;
		vch->pulser.pul_general_ctrl.BITS.external_trigger = 1;

		//vch->tgc.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		vch->afe.AFE_BUSSAR_UT.external_trigger = 1;

		if (gb_hw_sitau_enabled == 1)
		{
		  if ((result = prog_afe_SPI_regs(vch->afe.AFE_SPI_UT, bcc_addr_broadcast)) < 0)
			 xil_printf("ERROR_PROG_SPI_AFE\n\r");
		}

		if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT, &Global_commnd_buf, bcc_addr_broadcast)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");


		if ((result = prog_pulser_UT_regs(&vch->pulser, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_PULSER_REGS\n\r");

		//Esto igual no se debe hacer aqui
		//if ((result = prog_tgc_curve(base->tgc.curve,base->tgc.n_points, base->tgc_config.tgc_mem_ini_end.BITS.tgc_mem_ini, bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_TGC_CURVE\n\r");

		if ((result = prog_tgc_UT_regs(&vch->tgc.cfg, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_TGC_REGS\n\r");

		emi_prom_config_rst.emi_prom_config_u32 = 0x0;
		emi_prom_config_rst.BITS.ce_bits = 0xFF;

		if ((result = emi_prom_prog(&emi_prom_config_rst, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

		if ((result = emi_prom_prog(&vch->emi_prom_config_acquisition, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");
/*
		btt = vch->afe.AFE_BUSSAR_UT.num_samples*32*2;
		addr = vch->remote_addr_ini;
		align_remainder = addr%FP_REMOTE_ALIGN_SIZE;

		if (align_remainder)
			addr = addr - align_remainder + FP_REMOTE_ALIGN_SIZE;
*/

		//Configuramos SWITCH (AFE-->BFM)
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_DMA_IDX,SSWITCH512_SLAVE_FROM_DMA_IDX,1,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		if ((result = switch_remote_512_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_DATAACQ\n\r");
		if ((result = config_afe2mem((u32)0,0,0,0,0,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_AFE_DATAMOVER\n\r");
	}

	{
		int b, result = EUCI_NONE;
		u32 btt_beamformer2mem;
		u32 inc_beamformer2mem;
		u8 ce_man_emi_prom;
		int one_line;
		u8 emi_prom_enable,emi0_prom1;
		TVCH_BASE *base = NULL;



		//Los registros TFM a 0:
		vch->bf_regs.prog_addr_ini_end_ida.BITS.prog_addr_ini_ida = 0;
//		vch->bf_regs.bits_ida.bits_ida_u32 = 0;
//		vch->bf_regs.bits_ida.BITS.tfm = 0;
//		vch->bf_regs.bits_ida.BITS.pwi = 0;
		//base->beamformer.regs.shft_bits = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_start_addr = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_ida_offset = 0;

		//Poniendo el conformador con 0(+1) lÃ­neas en paralelo y "serial_param_prog" desactivamos el parallel beamforming
		one_line = (vch->bf_regs.parallel_lines.BITS.parallel_lines == 0) && (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 1);


		vch->bf_regs.bf_gen_ctrl.BITS.auto_sequence = 1;
		vch->bf_regs.bf_gen_ctrl.BITS.external_trigger = 1;

		if ((result = beamformer_prog_registros(&vch->bf_regs,bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		btt_beamformer2mem = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;

		if (one_line) inc_beamformer2mem = btt_beamformer2mem;
		else inc_beamformer2mem = btt_beamformer2mem * NUM_BEAMFORMER_DATAMOVERS;

		emi_prom_enable = vch->emi_prom_config_beamforming.BITS.enable && (vch->emi_prom_config_beamforming.BITS.prom || vch->emi_prom_config_beamforming.BITS.emi);

		//if (emi_prom_enable) emi0_prom1 = vch->emi_prom_config_beamforming.BITS.prom;

		//ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_DATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROM0);
		if(emi_prom_enable==0)
		{
			if ((result = config_beamformer2mem_circular(vch->remote_addr_bf_ini,vch->remote_addr_bf_end, btt_beamformer2mem, inc_beamformer2mem, 1, 0,1,bcc_addr_broadcast, &Global_commnd_buf,0)) < 0)
				xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

			xil_printf("\n\r----------------------VERIFICAR SI EL HW ES COMPATIBLE CON ESTE MODO-----------------\n\r");

			//if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_DDR_IDX,SSWITCH032_SLAVE_FROM_PRM_IDX,0,&Global_commnd_buf)) < 0)
				//xil_printf("ERROR_REMOTE_SWITCH_032\n\r");
			//if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_PRM_IDX,SSWITCH032_SLAVE_FROM_DDR_IDX,1,&Global_commnd_buf)) < 0)// Si no hay promediado desactivamos esta parte
				//xil_printf("ERROR_REMOTE_SWITCH_032\n\r");

			{
				if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_SMT_IDX,SSWITCH032_SLAVE_FROM_DDR_IDX,0,&Global_commnd_buf)) < 0)
					xil_printf("ERROR_REMOTE_SWITCH_032\n\r");
				if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_SMP_IDX,SSWITCH032_SLAVE_FROM_BCC_IDX,0,&Global_commnd_buf)) < 0)
					xil_printf("ERROR_REMOTE_SWITCH_032\n\r");
				if ((result = switch_remote_032_select_slave_get_command(bcc_addr_broadcast,SSWITCH032_MASTER_TO_BCC_IDX,SSWITCH032_SLAVE_FROM_SUM_IDX,0,&Global_commnd_buf)) < 0)
					xil_printf("ERROR_REMOTE_SWITCH_032\n\r");
			}


			if ((result = switch_remote_032_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_REMOTE_SWITCH_032\n\r");

		}
		else
		{
			xil_printf("Modo EMI ni PROM están habilitados todavía en PA\n\r");
			return -1;
		}
		//vch->emi_prom_config_beamforming.BITS.ce_bits = ce_man_emi_prom;

		//if ((result = emi_prom_prog(&(vch->emi_prom_config_beamforming),bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

	}

	if (gb_hw_sitau_enabled == 1)
	{
		if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		return RLOG(result);
	}

	return EUCI_NONE;
}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_PRG_PA_no_DDR_Registers(TVCH *vch)
{
	const u8 bcc_addr_broadcast = 0;
	int result = EUCI_NONE;

	xil_printf("Este modo no esta implementado");
	return -1;
	//Parte de adquisición
	{

		u32 btt;
		u32 addr;
		u32 align_remainder;
		bf_general_ctrl_t bf_gen_ctrl_tmp;
		emi_prom_config_t emi_prom_config_rst;


		if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

		Global_commnd_buf.total_len = 0;


		vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		//vch->pulser.pul_general_ctrl.BITS.pulser_current = 3;
		vch->pulser.pul_general_ctrl.BITS.external_trigger = 1;

		//			vch->tgc.tgc_general_ctrl.tgc_general_ctrl_u32 = 0x00000000;

		vch->afe.AFE_BUSSAR_UT.external_trigger = 1;

		if (gb_hw_sitau_enabled == 1)
		{
		  if ((result = prog_afe_SPI_regs(vch->afe.AFE_SPI_UT, bcc_addr_broadcast)) < 0)
			 xil_printf("ERROR_PROG_SPI_AFE\n\r");
		}

		if ((result = get_prog_afe_commands(vch->afe.AFE_BUSSAR_UT, &Global_commnd_buf, bcc_addr_broadcast)) < 0)
			xil_printf("ERROR_PROG_AFE_REGS\n\r");


		if ((result = prog_pulser_UT_regs(&vch->pulser, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_PULSER_REGS\n\r");

		//Esto igual no se debe hacer aqui
		//if ((result = prog_tgc_curve(base->tgc.curve,base->tgc.n_points, base->tgc_config.tgc_mem_ini_end.BITS.tgc_mem_ini, bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			//xil_printf("ERROR_PROG_TGC_CURVE\n\r");

		if ((result = prog_tgc_UT_regs(&vch->tgc.cfg, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_TGC_REGS\n\r");

		emi_prom_config_rst.emi_prom_config_u32 = 0x0;
		emi_prom_config_rst.BITS.ce_bits = 0xFF;

		if ((result = emi_prom_prog(&emi_prom_config_rst, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

		if ((result = emi_prom_prog(&vch->emi_prom_config_acquisition, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");

		btt = vch->afe.AFE_BUSSAR_UT.num_samples*32*2; //cambiar 32 por numero de scans a adquirir, de momento el maximo, 32
		addr = vch->remote_addr_ini;
		align_remainder = addr%FP_REMOTE_ALIGN_SIZE;

		if (align_remainder)
			addr = addr - align_remainder + FP_REMOTE_ALIGN_SIZE;


		//Configuramos SWITCH (AFE-->BFM)
		if ((result = switch_remote_512_select_slave_get_command(bcc_addr_broadcast,SSWITCH512_MASTER_TO_BFM_IDX,SSWITCH512_SLAVE_FROM_AFE_IDX,0,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_512\n\r");

		if ((result = switch_remote_512_update_get_command(bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_REMOTE_SWITCH_512\n\r");

	}
	// Conformación
	{
		int b, result = EUCI_NONE;
		u32 btt_mem2beamformer;
		u32 btt_beamformer2mem;
		u32 inc_beamformer2mem;
		u32 addr_mem2beamformer;
		u8 ce_man_emi_prom;
		int one_line;
		u8 emi_prom_enable,emi0_prom1;
		//TVCH_BASE *base = NULL;

		//if ((base = &vch->base[b]) == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

		//Los registros TFM a 0:
		vch->bf_regs.prog_addr_ini_end_ida.BITS.prog_addr_ini_ida = 0;
//		vch->bf_regs.bits_ida.bits_ida_u32 = 0;
//		vch->bf_regs.bits_ida.BITS.tfm = 0;
//		vch->bf_regs.bits_ida.BITS.pwi = 0;
		//base->beamformer.regs.shft_bits = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_start_addr = 0;
		vch->bf_regs.ln_planner_reg.BITS.ln_plan_ida_offset = 0;

		//Poniendo el conformador con 0(+1) lineas en paralelo y "serial_param_prog" desactivamos el parallel beamforming
		one_line = (vch->bf_regs.parallel_lines.BITS.parallel_lines == 0) && (vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog == 1);

		Global_commnd_buf.total_len = 0;
		vch->bf_regs.bf_gen_ctrl.BITS.auto_sequence = 1;
		vch->bf_regs.bf_gen_ctrl.BITS.external_trigger = 1;

		if ((result = beamformer_prog_registros(&vch->bf_regs,bcc_addr_broadcast, &Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");

		if (one_line)
			inc_beamformer2mem = btt_beamformer2mem;
		else
			inc_beamformer2mem = btt_beamformer2mem * NUM_BEAMFORMER_DATAMOVERS; // Esto es para que cada datamover vaya colocando los datos a continuación unos de otros

		if(emi_prom_enable)
		{
			xil_printf("\n\rERROR: EMI y promediado aún no implementado en PA!!\n\r");
		}
		else
		{
			xil_printf("\n\r----------------------VERIFICAR SI ESTE MODO FUNCIONA, PROBABLEMENTE NO-----------------\n\r");
			//if((result = switch_remote_512_select_slave_get_command(0,SSWITCH032_MASTER_TO_PRM_IDX,SSWITCH032_SLAVE_FROM_BCC_IDX,0,&Global_commnd_buf))<0)
				//xil_printf("ERROR_SWITCH32_REGS\n\r");
			//if((result = switch_remote_512_select_slave_get_command(0,SSWITCH032_MASTER_TO_BCC_IDX,SSWITCH032_SLAVE_FROM_PRM_IDX,0,&Global_commnd_buf))<0)
				//xil_printf("ERROR_SWITCH32_REGS\n\r");
			if((result = switch_remote_512_update_get_command(0,&Global_commnd_buf))<0)
				xil_printf("ERROR_SWITCH32_REGS\n\r");
		}

		for(u8 base_addr=1;base_addr<=vch->n_bases;base_addr++)
		{
			emi_prom_enable = vch->emi_prom_config_beamforming.BITS.enable && (vch->emi_prom_config_beamforming.BITS.prom || vch->emi_prom_config_beamforming.BITS.emi);
			if (emi_prom_enable) emi0_prom1 = vch->emi_prom_config_beamforming.BITS.prom;

			if(base_addr==1)
			{
				emi_prom_config_t config_promemi;

				config_promemi.emi_prom_config_u32=0;
				config_promemi.BITS.ce_bits = EMI_PROM_MAN_CS_EMIPROMBF0;
				config_promemi.BITS.auto_mode = 0;
				config_promemi.BITS.emi = 0;
				config_promemi.BITS.last_rpt = 0;
				config_promemi.BITS.prom = 1;		 //	Aquí convierte la entrada a 32 bits, con promediado, pero como si fuera la primera
				config_promemi.BITS.first_rpt = 1;   // Pero como si fuera el primer promeidado, es decir, no suma nada
				config_promemi.BITS.dsr = vch->emi_prom_config_beamforming.BITS.dsr; // Esto se calculó en el momento de la programación
				emi_prom_prog(&config_promemi,base_addr,&Global_commnd_buf);
			}
			else if (base_addr==vch->n_bases)
			{
				emi_prom_config_t config_promemi;

				config_promemi.emi_prom_config_u32=0;
				config_promemi.BITS.ce_bits = EMI_PROM_MAN_CS_EMIPROMBF0;
				config_promemi.BITS.auto_mode = 0;
				config_promemi.BITS.emi = 0;
				config_promemi.BITS.prom = 1;		 //	Aquí convierte la entrada a 32 bits, con promediado, pero como si fuera la primera
				config_promemi.BITS.first_rpt = 0;   // Pero como si fuera el primer promeidado, es decir, no suma nada
				config_promemi.BITS.dsr = vch->emi_prom_config_beamforming.BITS.dsr; // Esto se calculó en el momento de la programación
				emi_prom_prog(&config_promemi,base_addr,&Global_commnd_buf);
			}
			else
			{
				emi_prom_config_t config_promemi;

				config_promemi.emi_prom_config_u32=0;
				config_promemi.BITS.ce_bits = EMI_PROM_MAN_CS_EMIPROMBF0;
				config_promemi.BITS.auto_mode = 0;
				config_promemi.BITS.emi = 0;
				config_promemi.BITS.prom = 1;		 //	Aquí convierte la entrada a 32 bits, con promediado, pero como si fuera la primera
				config_promemi.BITS.first_rpt = 0;   // Pero como si fuera el primer promeidado, es decir, no suma nada
				config_promemi.BITS.dsr = vch->emi_prom_config_beamforming.BITS.dsr; // Esto se calculó en el momento de la programación
				emi_prom_prog(&config_promemi,base_addr,&Global_commnd_buf);
			}

		}




		/* DE MOMENTO EL PROMEDIADO NO VA
		btt_mem2beamformer = vch->afe.AFE_BUSSAR_UT.num_samples*32*2;

		addr_mem2beamformer = vch->remote_addr_ini;

		btt_beamformer2mem = vch->bf_regs.n_foci * (vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer+1) * 2;

		if (one_line) inc_beamformer2mem = btt_beamformer2mem;
		else inc_beamformer2mem = btt_beamformer2mem * 2;

		ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_DATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROM0);


		if(emi_prom_enable==0)
		{
			if ((result = config_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, 1, 0, bcc_addr_broadcast, &Global_commnd_buf,0)) < 0)
				xil_printf("ERROR_PROG_BEAMFORMER_REG\n\r");
			ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_DATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROM0);

		}
		else if(emi0_prom1==0)
		{
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr, btt_beamformer2mem, 0, bcc_addr_broadcast, &Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");
			ce_man_emi_prom = (1<<EMI_PROM_MAN_CS_DATAMOV0)|(1<<EMI_PROM_MAN_CS_EMIPROM0);

		}
		else if(emi0_prom1==1)
		{
			u32 btt_beamformer2mem_scratch_prom=btt_beamformer2mem*2;//En promediado, los datos intermedios son a 32 bits en vez de 16
			if ((result = config_mem2emi_prom_beamformer2mem(vch->remote_addr_bf_ini, btt_beamformer2mem, inc_beamformer2mem, vch->emi_prom_scratch_addr,btt_beamformer2mem_scratch_prom,0,bcc_addr_broadcast,&Global_commnd_buf)) < 0)
				xil_printf("ERROR_PROG_DATAMOVER_REG\n\r");

		}
		vch->emi_prom_config_beamforming.BITS.ce_bits = ce_man_emi_prom;

		if ((result = emi_prom_prog(&(vch ->emi_prom_config_beamforming),bcc_addr_broadcast,&Global_commnd_buf)) < 0)
			xil_printf("ERROR_PROG_EMI_PROM_MANAGER\n\r");
		*/
	}

	if (gb_hw_sitau_enabled == 1)
	{
	  if ((result = BCC_SendBuffer(&Global_commnd_buf)) < 0)
		 xil_printf("ERROR_BCC_SEND\n\r");
	}

	return EUCI_NONE;
}

// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_PRG_ForwardFocalLaws(TVCH *vch)
{
	int result;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);


	if (vch->bf_regs.bits_ida.BITS.tfm)
	{
		if ((beamformer_prog_one_fwrd_focal_law(vch->tfm_fmc_forward.memory,vch->pa_image.n_lines,0,&vch->bf_regs,&Global_commnd_buf)) < 0)
			return RLOG(result);
		beamformer_set_ln_plan_reg(0x0,0);
	}
	else if (vch->bf_regs.bits_ida.BITS.pwi)
	{
		if ((beamformer_prog_one_fwrd_focal_law(vch->tfm_pwi_forward.forwardMemory,vch->pa_image.n_lines,0,&vch->bf_regs,&Global_commnd_buf)) < 0)
			return RLOG(result);
		beamformer_set_ln_plan_reg(0x0,0);
	}

/*
	if ((beamformer_prog_one_fwrd_focal_law(vch->tfm_forward.memory,vch->pa_image.n_lines,0,&vch->bf_regs,&Global_commnd_buf)) < 0)
		return RLOG(result);
	if ((beamformer_prog_one_fwrd_focal_law(vch->tfm_forward.memory,vch->pa_image.n_lines,128,&vch->bf_regs,&Global_commnd_buf)) < 0)
		return RLOG(result);
	if ((beamformer_prog_one_fwrd_focal_law(vch->tfm_forward.memory,vch->pa_image.n_lines,256,&vch->bf_regs,&Global_commnd_buf)) < 0)
		return RLOG(result);
	if ((beamformer_prog_one_fwrd_focal_law(vch->tfm_forward.memory,vch->pa_image.n_lines,384,&vch->bf_regs,&Global_commnd_buf)) < 0)
		return RLOG(result);
*/
/*
	for(u8 addr=1;addr<=4;addr++)
	{
		if ((result = beamformer_fwrd_params(vch->tfm_forward.memory, vch->tfm_forward.size32_memory, &vch->bf_regs, addr, &Global_commnd_buf)) < 0)
			return RLOG(result);
	}
	*/
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_PRG_BeamformerFocalLaws(TVCH *vch)
{
	int result, b = 0;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	for(b=0;b<vch->n_bases;b++)
	{
		if ((result = beamformer_prog_params(vch->base[b].bf_memory, vch->size32_bf_memory, &vch->bf_regs, vch->base[b].bcc_addr, &Global_commnd_buf)) < 0)
			return RLOG(result);
	}

	return EUCI_NONE;
}

