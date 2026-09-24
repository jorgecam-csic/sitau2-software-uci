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

#include "vch.h"
#include "log.h"
#include "vch_tad.h"
#include "uci_reg.h"
#include "uci_data.h"
#include "uci_error_code.h"
#include "trigger.h"
#include "calc.h"
#include "vch_prg.h"
#include "ACQ_FMC.h"

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

TVCH gb_fp_virtual_channel[MAX_VIRTUAL_CHANNELS];
THW_Config sitau_hw;
int gb_firmware_type = FIRMWARE_FULL_PARALLEL;
//int gb_firmware_type = FIRMWARE_PHASED_ARRAY;
static unsigned long gb_fp_no_hardware_acq_offset = 0;

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int SITAU_LoadConfig(THW_Config *src)
{
	if (src == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	sitau_hw.n_bases = src->n_bases;
//	if (reg->n_bases < MAX_N_BASES) sitau_hw.n_bases = reg->n_bases;
	//else sitau_hw.n_bases = MAX_N_BASES;
   //BASE_ResetRegisters();
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int SITAU_PRG(void)
{
/*	for (b=0; b<sitau_hw.n_bases; b++)
	{
		sitau_hw.base[b].enabled = 1;
	}*/
	gb_log_prg_reg = 0;
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_Process_NoHardware2 (int offset, unsigned short n_samples)
{
short i, data, min_offset;
int result;
u32 data_u32;
unsigned short *ptr_data_ushort = (unsigned short *)&data_u32;

   min_offset = gb_fp_no_hardware_acq_offset + offset - 32768;
	for (data=min_offset, i=0; i<n_samples; i+=2, data+=100)
	{
		if (data > 32767)  data = min_offset;
		ptr_data_ushort[0] = data;
		data += 100;
		ptr_data_ushort[1] = data;
		if (data > 32767)  data = min_offset;
		if ((result = UCI_MemoryWrite_uint32(data_u32)) < 0) return RLOG(result);
	}
	if (gb_log_acquiring_frame == 1) LOG_ACQUIRING_FRAME("(%d samples)... ", n_samples);
	return 0;
}

int VCH_Process_NoHardware_sitau2 (unsigned short n_samples, signed short offset)
{
	short i, data, min_offset;
	int result;
	u32 data_u32;
	int sample,canal;

	unsigned short *ptr_data_ushort = (unsigned short *)&data_u32;

	for(sample=0;sample<n_samples;sample++)
	{
		for (canal=0;canal<32;)
		{
			data=canal++*100+sample+offset;
			ptr_data_ushort[0] = data;
			data=canal++*100+sample+offset;
			ptr_data_ushort[1] = data;
			if ((result = UCI_MemoryWrite_uint32(data_u32)) < 0) return RLOG(result);
		}
	}
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_PrintConfig (TVCH *vch)
{
int l, c, i;
TVCH_BASE *base = NULL;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	if (vch->enabled == 0) return EUCI_NONE;
  
	xil_printf("\r\n\r\n   AFE.BUSSAR.-");
	xil_printf("\r\n      external_trigger: %d", vch->afe.AFE_BUSSAR_UT.external_trigger);
	xil_printf("\r\n      dec_ratio:        %d", vch->afe.AFE_BUSSAR_UT.dec_ratio);
	xil_printf("\r\n      num_samples:      %d", vch->afe.AFE_BUSSAR_UT.num_samples);
	xil_printf("\r\n      water_delay:      %d", vch->afe.AFE_BUSSAR_UT.water_delay);
	xil_printf("\r\n      log2_promediados: %d", vch->afe.AFE_BUSSAR_UT.log2_promediados);

	xil_printf("\r\n\r\n   remote_addr_ini: 0x%08X", vch->remote_addr_ini);
	xil_printf("\r\n   remote_addr_cur: 0x%08X", vch->remote_addr_cur);
	xil_printf("\r\n   remote_addr_end: 0x%08X", vch->remote_addr_end);

	xil_printf("\r\n\r\n   PULSER-");
	xil_printf("\r\n      pulse_width:          %d", vch->pulser.pulse_width);
	xil_printf("\r\n      pulser_main_delay:    %d", vch->pulser.pulser_main_delay);
	xil_printf("\r\n      delay_mem_ptr:        %d", vch->pulser.delay_mem_ptr);
	xil_printf("\r\n      delay_mem_ini_end.-");
	xil_printf("\r\n         delay_mem_ini:     %d", vch->pulser.delay_mem_ini_end.BITS.delay_mem_ini);
	xil_printf("\r\n         delay_mem_end:     %d", vch->pulser.delay_mem_ini_end.BITS.delay_mem_end);
	xil_printf("\r\n      pul_general_ctrl.-");
	xil_printf("\r\n         pul_enable:        %d", vch->pulser.pul_general_ctrl.BITS.pul_enable);
	xil_printf("\r\n         auto_prom:         %d", vch->pulser.pul_general_ctrl.BITS.auto_prom);
	xil_printf("\r\n         gnd0_hiz1:         %d", vch->pulser.pul_general_ctrl.BITS.gnd0_hiz1);
	xil_printf("\r\n         pulser_load:       %d", vch->pulser.pul_general_ctrl.BITS.pulser_load);
	xil_printf("\r\n         pulser_auto_load:  %d", vch->pulser.pul_general_ctrl.BITS.pulser_auto_load);
	xil_printf("\r\n         pulser_current:    %d", vch->pulser.pul_general_ctrl.BITS.pulser_current);
	xil_printf("\r\n         pulser_status:     %d", vch->pulser.pul_general_ctrl.BITS.pulser_status);
	xil_printf("\r\n         n_pulses:          %d", vch->pulser.pul_general_ctrl.BITS.n_pulses);
	xil_printf("\r\n         stop:              %d", vch->pulser.pul_general_ctrl.BITS.stop);
	xil_printf("\r\n         external_trigger:  %d", vch->pulser.pul_general_ctrl.BITS.external_trigger);
	xil_printf("\r\n         not_busy:          %d", vch->pulser.pul_general_ctrl.BITS.not_busy);
	xil_printf("\r\n         software_trigger:  %d", vch->pulser.pul_general_ctrl.BITS.software_trigger);

	for (i=0; i<vch->n_bases; i++)
	{
		base = &vch->base[i];
		xil_printf("\r\nBASE %d", base->bcc_addr);

		xil_printf("\r\n\r\n   EMISSION FOCAL LAW.- (%d focal laws)", vch->n_efl);
		for (l=i=0; l<vch->n_efl && l<MOD_PUL_MAX_FOCAL_LAW; l++)
		{
			xil_printf("\r\n(%3d) ", l);
			for (c=0; c<MOD_MAX_CH; c++, i++) xil_printf("%5d ", base->efl[i]);
		}
	}

   
   xil_printf("\r\n\r\n   AFE.SPI.reg51-");
   xil_printf("\r\n      LPF_PROGRAMMABILITY:    %d Bit[1:3]: 000: 15 MHz,010: 20 MHz,011: 30 MHz,100: 10 MHz", vch->afe.AFE_SPI_UT.reg51.BIT.LPF_PROGRAMMABILITY);
   xil_printf("\r\n      PGA_INTEGRATOR_DISABLE: %d Bit[4]: 1: Disables offset integrator for PGA.", vch->afe.AFE_SPI_UT.reg51.BIT.PGA_INTEGRATOR_DISABLE);
   xil_printf("\r\n      PGA_CLAMP_LEVEL:        %d Bit[5:7]: In normal operation, clamp function can be set as 000 in the low noise mode.", vch->afe.AFE_SPI_UT.reg51.BIT.PGA_CLAMP_LEVEL);
   xil_printf("\r\n      PGA_GAIN_CONTROL:       %d Bit[13]: 0:24 dB; 1:30 dB", vch->afe.AFE_SPI_UT.reg51.BIT.PGA_GAIN_CONTROL);
   
   xil_printf("\r\n\r\n   AFE.SPI.reg52-");
   xil_printf("\r\n      ACTIVE_TERMINATION:         %d Bit[4:0]: Control individual de la red de resistecias de terminaciÃƒÂ³n", vch->afe.AFE_SPI_UT.reg52.BIT.ACTIVE_TERMINATION);
   xil_printf("\r\n      ACT_TER_ENA_CTRL:           %d Bit[5]: 1: Enable internal active termination individual resistor control", vch->afe.AFE_SPI_UT.reg52.BIT.ACT_TER_ENA_CTRL);
   xil_printf("\r\n      PRESET_ACTIVE_TERMINATIONS: %d Bit[7:6]: 00: 50 Ohm; 01: 100 Ohm; 10: 200 Ohm; 11: 400 Ohm.", vch->afe.AFE_SPI_UT.reg52.BIT.PRESET_ACTIVE_TERMINATIONS);
   xil_printf("\r\n      ACTIVE_TERMINATION_ENABLE:  %d Bit[8]: Enable active termination", vch->afe.AFE_SPI_UT.reg52.BIT.ACTIVE_TERMINATION_ENABLE);
   xil_printf("\r\n      LNA_INPUT_CLAMP_SETTING:    %d Bit[10:9]: 00: Auto setting; 01: 1.5 Vpp; 10: 1.15 Vpp; 11: 0.6 Vpp", vch->afe.AFE_SPI_UT.reg52.BIT.LNA_INPUT_CLAMP_SETTING);
   xil_printf("\r\n      LNA_INTEGRATOR_DISABLE:     %d Bit[12]: Disable offset integrator for LNA. Mirar datasheet", vch->afe.AFE_SPI_UT.reg52.BIT.LNA_INTEGRATOR_DISABLE);
   xil_printf("\r\n      LNA_GAIN:                   %d Bit[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved", vch->afe.AFE_SPI_UT.reg52.BIT.LNA_GAIN);
   xil_printf("\r\n      LNA_INDIVIDUAL_CH_CNTL:     %d Bit[15]: 1: Activa control individual de la ganancia LNA con el registro 57.", vch->afe.AFE_SPI_UT.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL);
   
   xil_printf("\r\n\r\n   AFE.SPI.reg59-");
   xil_printf("\r\n      HPF_LNA:              %d Bit[3:2]: High Pass Filter: 00: 100 kHz, 01: 50 kHz, 10: 200 kHz, 11: 150 kHz with 0.015 uF", vch->afe.AFE_SPI_UT.reg59.BIT.HPF_LNA);
   xil_printf("\r\n      DIG_TGC_ATT_GAIN:     %d Bit[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N 6 dB attenuation when 59[7] = 1", vch->afe.AFE_SPI_UT.reg59.BIT.DIG_TGC_ATT_GAIN);
   xil_printf("\r\n      DIG_TGC_ATT:          %d Bit[7]: 0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator", vch->afe.AFE_SPI_UT.reg59.BIT.DIG_TGC_ATT);
   xil_printf("\r\n      CW_SUM_AMP_PDN:       %d Bit[8]: 0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode.", vch->afe.AFE_SPI_UT.reg59.BIT.CW_SUM_AMP_PDN);
   xil_printf("\r\n      PGA_TEST_MODE:        %d Bit[9]: 0: Normal CW operation, 1: PGA outputs appear at CW outputs.", vch->afe.AFE_SPI_UT.reg59.BIT.PGA_TEST_MODE);
   
   xil_printf("\r\n\r\n   AFE.SPI.reg61-");
   xil_printf("\r\n      V2I_CLAMP:            %d Bit[13]: 0: Clamp disabled, 1: Clamp enabled at the V2I input.", vch->afe.AFE_SPI_UT.reg61.BIT.V2I_CLAMP);
   xil_printf("\r\n      LPF_5MHz:             %d Bit[14]: 0: 5-MHz LPF disabled, 1: 5-MHz LPF enabled", vch->afe.AFE_SPI_UT.reg61.BIT.LPF_5MHz);
   xil_printf("\r\n      PGA_CLAMP_minus6dBFS: %d Bit[15]: 0: Disable the 6-dBFS clamp. PGA_CLAMP is set by Reg51[7:5]. 1: Enable the 6-dBFS clamp.", vch->afe.AFE_SPI_UT.reg61.BIT.LPF_5MHz);
   
   xil_printf("\r\n\r\n   tgc_config-");
   xil_printf("\r\n      tgc_ini_delay:        %d", vch->tgc.cfg.tgc_ini_delay);
   xil_printf("\r\n      delay_mem_ini_end.-");
   xil_printf("\r\n         tgc_mem_ini  :     %d", vch->tgc.cfg.tgc_mem_ini_end.BITS.tgc_mem_ini);
   xil_printf("\r\n         tgc_mem_end  :     %d", vch->tgc.cfg.tgc_mem_ini_end.BITS.tgc_mem_end);
   xil_printf("\r\n      tgc_general_ctrl.-");
   xil_printf("\r\n         tgc_enable:        %d", vch->tgc.cfg.tgc_general_ctrl.BITS.tgc_enable);
   xil_printf("\r\n         stop:              %d", vch->tgc.cfg.tgc_general_ctrl.BITS.stop);
   xil_printf("\r\n         external_trigger:  %d", vch->tgc.cfg.tgc_general_ctrl.BITS.external_trigger);
   xil_printf("\r\n         not_busy:          %d", vch->tgc.cfg.tgc_general_ctrl.BITS.not_busy);
   xil_printf("\r\n         software_trigger:  %d", vch->tgc.cfg.tgc_general_ctrl.BITS.software_trigger);
   
   xil_printf("\r\n");
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Reset (void)
{
int id_vch, i, b, result;

   for (id_vch=0; id_vch<MAX_VIRTUAL_CHANNELS; id_vch++)
   {
      gb_fp_virtual_channel[id_vch].acquiring = 0;
      gb_fp_virtual_channel[id_vch].id = id_vch;

      if (id_vch == 0) gb_fp_virtual_channel[id_vch].enabled = 1;
      else gb_fp_virtual_channel[id_vch].enabled = 0;

      gb_fp_virtual_channel[id_vch].prf_time_line = 0;
      gb_fp_virtual_channel[id_vch].start_element = 0;
      gb_fp_virtual_channel[id_vch].n_efl = 0;
      gb_fp_virtual_channel[id_vch].pa_image.n_lines = 0;

      gb_fp_virtual_channel[id_vch].gain_da = 0;
      gb_fp_virtual_channel[id_vch].gain_hi = 0;

      gb_fp_virtual_channel[id_vch].fir.enabled = 0;
      gb_fp_virtual_channel[id_vch].fir.n_coefficients = FIR_N_COEFFICIENTS;
      for (i=0; i<FIR_N_COEFFICIENTS; i++) gb_fp_virtual_channel[id_vch].fir.coefficients[i] = 0;
      gb_fp_virtual_channel[id_vch].fir.coefficients[FIR_N_COEFFICIENTS/2] = 0x7FFF;

      gb_fp_virtual_channel[id_vch].processing_type = 0;
      gb_fp_virtual_channel[id_vch].wait_prf_signals_processing = 0;
      gb_fp_virtual_channel[id_vch].n_signals_processing_avr = 8;
      gb_fp_virtual_channel[id_vch].n_signals_processing_emi  =3;

      gb_fp_virtual_channel[id_vch].enabled_timestamp = 0;
      gb_fp_virtual_channel[id_vch].enabled_buffer_dma = 0;
      gb_fp_virtual_channel[id_vch].enabled_temperature = 0;
      gb_fp_virtual_channel[id_vch].enabled_encoder_1 = 0;
      gb_fp_virtual_channel[id_vch].enabled_encoder_2 = 0;
      gb_fp_virtual_channel[id_vch].enabled_encoder_3 = 0;
      gb_fp_virtual_channel[id_vch].enabled_encoder_4 = 0;

      gb_fp_virtual_channel[id_vch].n_bases = 1;
      for (b=0; b<gb_fp_virtual_channel[id_vch].n_bases && b<MAX_N_BASES; b++)
      {
         gb_fp_virtual_channel[id_vch].base[b].bcc_addr = gb_fp_virtual_channel[id_vch].n_bases-b;
      }
      if ((result = VCH_UpdateAcquisitionSize(&gb_fp_virtual_channel[id_vch])) < 0) return RLOG(result);
   }
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_FP_UpdateAcquisitionSize (TVCH *vch)
{
u32 n_samples;

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	n_samples = vch->afe.AFE_BUSSAR_UT.num_samples;

	vch->size_header_img_fp = 0;

	// Start Start Control Word (0xCEBO)
	vch->size_header_img_fp++;

	// Id. Virtual Channel
	vch->size_header_img_fp++;

	// Acquisition Size
	vch->size_header_img_fp++;

	// Id. Focal Law
	vch->size_header_img_fp++;

	// N. AScan Signals
	vch->size_header_img_fp++;

	// N. AScan Samples
	vch->size_header_img_fp++;

	// Acquisition Counter
	vch->size_header_img_fp++;

	// Time Stamp
	if (vch->enabled_timestamp) vch->size_header_img_fp++;

	// Encoder 1
	if (vch->enabled_encoder_1) vch->size_header_img_fp += 6;

	// Encoder 2
	if (vch->enabled_encoder_2) vch->size_header_img_fp += 6;

	// Encoder 3
	if (vch->enabled_encoder_3) vch->size_header_img_fp += 6;

	// Encoder 4
	if (vch->enabled_encoder_4) vch->size_header_img_fp += 6;

	// Offset Encoder Trigger
	vch->size_header_img_fp++;

	// Temperature
	if (vch->enabled_temperature)
	{
	}

	// Buffer DMA
	if (vch->enabled_buffer_dma) vch->size_header_img_fp++;

	// Start End Control Word (0xC0C0)
	vch->size_header_img_fp++;

	vch->size_header_line_fp = 0;

	vch->acquisition_size_fp = 0;

	vch->size32_fp_image = (MOD_MAX_CH * (n_samples / 2));
	vch->size32_fp_frame = (unsigned long)vch->size_header_line_fp +
		+ (vch->n_bases * vch->size32_fp_image) +
		+ vch->size_header_img_fp;

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_PA_UpdateAcquisitionSize (TVCH *vch)
{
int i, j, b, n_parallel_lines, n_lines_last_block, i_block;
u32 data_u32, n_lines, n_blocks, n_blocks_32b;
u8 n_lines_block[MAX_BLOCKS];

	if (vch == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

	if (vch->bf_regs.parallel_lines.BITS.parallel_lines == 0) n_parallel_lines = 1;
	else n_parallel_lines = vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer + 1;

	n_blocks = RoundSup((float)vch->pa_image.n_lines / (float)n_parallel_lines);
	n_blocks_32b = RoundSup((float)n_blocks / 4.0);

	n_lines_last_block = vch->pa_image.n_lines % n_parallel_lines;
	n_lines_last_block = n_lines_last_block != 0 ? n_lines_last_block : n_parallel_lines;
	for (i_block=0; i_block<n_blocks-1 && i_block<MAX_BLOCKS; i_block++) n_lines_block[i_block] = n_parallel_lines;
	n_lines_block[i_block] = n_lines_last_block;

	vch->pa_image.n_blocks = 0;
	vch->pa_image.n_blocks_32b = 0;

	for (i=i_block=0; i<n_blocks_32b && i_block<MAX_BLOCKS; i++)
	{
		data_u32 = 0;
		for (j=0; j<4; j++,i_block++)
		{
			if (i_block < n_blocks) n_lines = n_lines_block[i_block];
			else n_lines = 0;
			data_u32 = data_u32 | (n_lines << j*8);
		}
		vch->pa_image.block_32b[i] = data_u32;
	}

	vch->pa_image.n_blocks_32b = n_blocks_32b;
	vch->pa_image.n_blocks = n_blocks;
	vch->pa_image.n_blocks_frame = n_blocks<<16 | n_blocks_32b;

	vch->size_header_img_pa = 0;

	// HEADER
	vch->size_header_img_pa++;

	// ID DEL CANAL VIRTUAL
	vch->size_header_img_pa++;

	// NUMERO DE DATOS DE LA ACQUISICION
	vch->size_header_img_pa++;

	// ESCRIBE EL NUMERO DE LINEAS CONFORMADAS EN PARALELO POR CADA CONFORMADOR
	vch->size_header_img_pa++;

	// NUMERO DE SEÃ‘ALES A-SCAN (Lineas de la imagen PA)
	vch->size_header_img_pa++;

	// NUMERO DE DATOS DE CADA A-SCAN (LÃ­nea)
	vch->size_header_img_pa++;

	// NUMERO DE BLOQUES QUE FORMAN LA IMAGEN [N_BLOQUES]
	vch->size_header_img_pa++;

	// NUMERO DE SEÃ‘ALES A-SCAN (LINEAS) DE CADA BLOQUE
	vch->size_header_img_pa += vch->pa_image.n_blocks_32b;

	// CONTADOR DE IMAGENES PA
	vch->size_header_img_pa++;

	// TIME STAMP
	if (vch->enabled_timestamp) vch->size_header_img_pa++;

	// ENCODER 0
	if (vch->enabled_encoder_1) vch->size_header_img_pa += 6;

	// ENCODER 1
	if (vch->enabled_encoder_2) vch->size_header_img_pa += 6;

	// ENCODER 2
	if (vch->enabled_encoder_3) vch->size_header_img_pa += 6;

	// ENCODER 3
	if (vch->enabled_encoder_4) vch->size_header_img_pa += 6;

	// OFFSET TRIGGER ENCODER
	vch->size_header_img_pa++;

	// VALORES DE TEMPERATURA
	if (vch->enabled_temperature)
	{
	}

	// NUMERO DE DATOS EN EL BUFFER DMA
	if (vch->enabled_buffer_dma) vch->size_header_img_pa++;

	// FOOTER
	vch->size_header_img_pa++;

	vch->size_header_line_pa = 0;

	vch->pa_image.image_size_32b = 0;
	vch->pa_image.frame_size_32b = 0;

	for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)
		vch->pa_image.image_size_32b = vch->pa_image.n_beamformed_samples *vch->pa_image.n_lines / 2;

	vch->pa_image.frame_size_32b = (unsigned long)vch->size_header_line_pa +
            + vch->pa_image.image_size_32b +
            + vch->size_header_img_pa;

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_UpdateAcquisitionSize (TVCH *vch)
{
	int result;

	if ((result = VCH_FP_UpdateAcquisitionSize(vch)) < 0) return RLOG(result);
	if ((result = VCH_PA_UpdateAcquisitionSize(vch)) < 0) return RLOG(result);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Set_Config_v0 (TMSG_Config_v0 *msg)
{
int b, result, id_vch;
TVCH *vch = NULL;
TVCH_BASE *base = NULL;
u8 ceillog2_n_bases;

   if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
   id_vch = msg->id_vch;
   if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);

   vch = &gb_fp_virtual_channel[id_vch];
   vch->acquiring = 0;
   vch->hardware_set_up = 0;
   vch->id = id_vch;
   vch->enabled = msg->enabled;
   vch->prf_time_line = msg->prf_time_line;

   vch->pa_image.n_beamformed_samples = (u32)msg->n_beamformed_samples;
   if (vch->pa_image.n_beamformed_samples % 2 != 0)
      vch->pa_image.n_beamformed_samples++;

   vch->gain_da = msg->gain_da;
	vch->gain_hi = msg->gain_hi;

	vch->enabled_timestamp = msg->enabled_timestamp;
	vch->enabled_buffer_dma = msg->enabled_buffer_dma;
	vch->enabled_temperature = msg->enabled_temperature;
	vch->enabled_encoder_1 = msg->enabled_encoder_1;
	vch->enabled_encoder_2 = msg->enabled_encoder_2;
	vch->enabled_encoder_3 = msg->enabled_encoder_3;
	vch->enabled_encoder_4 = msg->enabled_encoder_4;

	vch->afe.AFE_BUSSAR_UT.num_samples = (u32)msg->n_acq_samples;
	if (vch->afe.AFE_BUSSAR_UT.num_samples % 2 != 0) vch->afe.AFE_BUSSAR_UT.num_samples++;
    vch->afe.AFE_BUSSAR_UT.external_trigger = msg->external_trigger;
    vch->afe.AFE_BUSSAR_UT.dec_ratio = msg->decimation_factor;
    vch->afe.AFE_BUSSAR_UT.water_delay = msg->water_delay;


    vch->afe.AFE_SPI_UT.reg21.REG = msg->afe_spi.reg21.REG;
    vch->afe.AFE_SPI_UT.reg33.REG = msg->afe_spi.reg33.REG;
    vch->afe.AFE_SPI_UT.reg51.REG = msg->afe_spi.reg51.REG;
    vch->afe.AFE_SPI_UT.reg52.REG = msg->afe_spi.reg52.REG;
    vch->afe.AFE_SPI_UT.reg59.REG = msg->afe_spi.reg59.REG;
    vch->afe.AFE_SPI_UT.reg61.REG = msg->afe_spi.reg61.REG;

    {
    	vch->afe.AFE_SPI_UT.reg21.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg33.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED1 = 0;
    	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED2 = 0;
    	vch->afe.AFE_SPI_UT.reg52.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg59.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg59.BIT.RESERVED1 = 0;
    	vch->afe.AFE_SPI_UT.reg61.BIT.RESERVED0 = 0;
    }

    //vch->remote_addr_ini = FP_REMOTE_START_ADDR;
    vch->remote_addr_ini = FP_REMOTE_START_ADDR;
    vch->remote_addr_cur = FP_REMOTE_START_ADDR;
    vch->remote_addr_end = FP_REMOTE_LAST_ADDR;
    //
//    xil_printf("\n\rESTO NO ES ASIIIII, solo es prueba!!!!\n\r");
//    vch->remote_addr_ini = 0xB0000000;
//    vch->remote_addr_cur = 0xB0000000;
//    vch->remote_addr_end = FP_REMOTE_LAST_ADDR;

    vch->pulser.pulser_main_delay = msg->pulser.delay;
    vch->pulser.pulse_width = msg->pulser.pulse_width;
    vch->pulser.pul_general_ctrl.BITS.pul_enable = msg->pulser.enabled;
    vch->pulser.pul_general_ctrl.BITS.gnd0_hiz1 = msg->pulser.high_z;
    vch->pulser.pul_general_ctrl.BITS.pulser_current = msg->pulser.mode;
    vch->pulser.pul_general_ctrl.BITS.n_pulses = msg->pulser.n_pulses;
    vch->pulser.delay_mem_ini_end.BITS.delay_mem_ini = 0;

    vch->pulser.delay_mem_ini_end.BITS.delay_mem_end = (vch->n_efl * MOD_MAX_CH) - 1;
    vch->pulser.pul_general_ctrl.BITS.auto_prom=1;
    vch->pulser.pul_general_ctrl.BITS.pulser_load=1;
    vch->pulser.pul_general_ctrl.BITS.pulser_auto_load=1;
    vch->pulser.pul_general_ctrl.BITS.stop=0;
    vch->pulser.pul_general_ctrl.BITS.external_trigger=1;
    vch->pulser.pul_general_ctrl.BITS.software_trigger=0;

	vch->processing_type = msg->processing_type;
    vch->wait_prf_signals_processing = msg->wait_prf_signals_processing;
    vch->n_signals_processing_emi = msg->n_signals_processing_emi;
    vch->n_signals_processing_avr = 1<<msg->n_signals_processing_avr;
    ceillog2_n_bases = ceil_Log2(msg->n_bases);

	if(vch->processing_type==PROCESSING_TYPE_NONE)
	{
	    vch->afe.AFE_BUSSAR_UT.log2_promediados = 0;
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.BITS.dsr = ceillog2_n_bases;
	}
	else if (vch->processing_type==PROCESSING_TYPE_PROM_B)
	{
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.enable = 1;
		vch->emi_prom_config_acquisition.BITS.auto_mode = 1;
		vch->emi_prom_config_acquisition.BITS.prom = 1;
		vch->emi_prom_config_acquisition.BITS.num_rpt = vch->n_signals_processing_avr - 1;

		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.prom = 1;
		vch->emi_prom_config_beamforming.BITS.dsr  = (msg->n_signals_processing_avr+ceillog2_n_bases) & 0xF;
		vch->emi_prom_config_beamforming.BITS.num_rpt = vch->n_signals_processing_avr - 1;
		vch->emi_prom_config_beamforming.BITS.ce_bits=0xFF;
		vch->emi_prom_config_beamforming.BITS.mem2prom32 = 1;
		vch->emi_prom_config_beamforming.BITS.prom2mem16 = 1;
		vch->emi_prom_config_beamforming.BITS.auto_mode = 1;
	}
	else if (vch->processing_type==PROCESSING_TYPE_EMI_B)
	{
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.enable = 1;
		vch->emi_prom_config_acquisition.BITS.emi = 1;
		vch->emi_prom_config_acquisition.BITS.auto_mode = 1;
		vch->emi_prom_config_acquisition.BITS.num_rpt = (vch->n_signals_processing_emi-1) & 0xFF;

		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.emi = 1;
		vch->emi_prom_config_beamforming.BITS.num_rpt = (vch->n_signals_processing_emi-1)&0xFF;
		vch->emi_prom_config_beamforming.BITS.ce_bits=0xFF;
		vch->emi_prom_config_beamforming.BITS.mem2prom32 = 0;
		vch->emi_prom_config_beamforming.BITS.prom2mem16 = 0;
		vch->emi_prom_config_beamforming.BITS.auto_mode = 1;
		vch->emi_prom_config_beamforming.BITS.dsr = ceillog2_n_bases;

	}
	else if (vch->processing_type==PROCESSING_TYPE_PROM_A)
	{
	    vch->afe.AFE_BUSSAR_UT.log2_promediados = msg->log2_promediados;
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.last_rpt = 1;
	}


    vch->n_bases = msg->n_bases;
    for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)//
    {
	   base = &vch->base[b];
	   //base->bcc_addr = b+1;
	   base->bcc_addr = vch->n_bases-b;
	}

   // CONFIGURACION DESDE LA UCI FIN -------------------------------------------------------------

   if ((result = VCH_UpdateAcquisitionSize(&gb_fp_virtual_channel[id_vch])) < 0) return RLOG(result);

	//if ((result = FP_UCI_SetVirtualChannel_ConfigHardware(&gb_fp_virtual_channel[id_vch], 1)) < 0)
    //     return RLOG(result);




   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Set_Config_v1 (TMSG_Config_v1 *msg)
{
int b, result, id_vch;
TVCH *vch = NULL;
TVCH_BASE *base = NULL;
u8 ceillog2_n_bases;

   if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
   id_vch = msg->id_vch;
   if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);

   vch = &gb_fp_virtual_channel[id_vch];
   vch->acquiring = 0;
   vch->hardware_set_up = 0;
   vch->acquisition_type = msg->acquisition_type;
   vch->signal_mode = 0;
   vch->gtx_n_focal_law = msg->gtx_n_focal_law;
   vch->id = id_vch;
   vch->enabled = msg->enabled;
   vch->prf_time_line = msg->prf_time_line;

   vch->pa_image.n_beamformed_samples = (u32)msg->n_beamformed_samples;
   if (vch->pa_image.n_beamformed_samples % 2 != 0)
      vch->pa_image.n_beamformed_samples++;

   vch->gain_da = msg->gain_da;
	vch->gain_hi = msg->gain_hi;

	vch->enabled_timestamp = msg->enabled_timestamp;
	vch->enabled_buffer_dma = msg->enabled_buffer_dma;
	vch->enabled_temperature = msg->enabled_temperature;
	vch->enabled_encoder_1 = msg->enabled_encoder_1;
	vch->enabled_encoder_2 = msg->enabled_encoder_2;
	vch->enabled_encoder_3 = msg->enabled_encoder_3;
	vch->enabled_encoder_4 = msg->enabled_encoder_4;

	vch->afe.AFE_BUSSAR_UT.num_samples = (u32)msg->n_acq_samples;
	if (vch->afe.AFE_BUSSAR_UT.num_samples % 2 != 0) vch->afe.AFE_BUSSAR_UT.num_samples++;
    vch->afe.AFE_BUSSAR_UT.external_trigger = msg->external_trigger;
    vch->afe.AFE_BUSSAR_UT.dec_ratio = msg->decimation_factor;
    vch->afe.AFE_BUSSAR_UT.water_delay = msg->water_delay;


    vch->afe.AFE_SPI_UT.reg21.REG = msg->afe_spi.reg21.REG;
    vch->afe.AFE_SPI_UT.reg33.REG = msg->afe_spi.reg33.REG;
    vch->afe.AFE_SPI_UT.reg51.REG = msg->afe_spi.reg51.REG;
    vch->afe.AFE_SPI_UT.reg52.REG = msg->afe_spi.reg52.REG;
    vch->afe.AFE_SPI_UT.reg59.REG = msg->afe_spi.reg59.REG;
    vch->afe.AFE_SPI_UT.reg61.REG = msg->afe_spi.reg61.REG;

    {
    	vch->afe.AFE_SPI_UT.reg21.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg33.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED1 = 0;
    	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED2 = 0;
    	vch->afe.AFE_SPI_UT.reg52.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg59.BIT.RESERVED0 = 0;
    	vch->afe.AFE_SPI_UT.reg59.BIT.RESERVED1 = 0;
    	vch->afe.AFE_SPI_UT.reg61.BIT.RESERVED0 = 0;
    }

    //vch->remote_addr_ini = FP_REMOTE_START_ADDR;
    vch->remote_addr_ini = FP_REMOTE_START_ADDR;
    vch->remote_addr_cur = FP_REMOTE_START_ADDR;
    vch->remote_addr_end = FP_REMOTE_LAST_ADDR;
    //
//    xil_printf("\n\rESTO NO ES ASIIIII, solo es prueba!!!!\n\r");
//    vch->remote_addr_ini = 0xB0000000;
//    vch->remote_addr_cur = 0xB0000000;
//    vch->remote_addr_end = FP_REMOTE_LAST_ADDR;

    vch->pulser.pulser_main_delay = msg->pulser.delay;
    vch->pulser.pulse_width = msg->pulser.pulse_width;
    vch->pulser.pul_general_ctrl.BITS.pul_enable = msg->pulser.enabled;
    vch->pulser.pul_general_ctrl.BITS.gnd0_hiz1 = msg->pulser.high_z;
    vch->pulser.pul_general_ctrl.BITS.pulser_current = msg->pulser.mode;
    vch->pulser.pul_general_ctrl.BITS.n_pulses = msg->pulser.n_pulses;
    vch->pulser.delay_mem_ini_end.BITS.delay_mem_ini = 0;

    vch->pulser.delay_mem_ini_end.BITS.delay_mem_end = (vch->n_efl * MOD_MAX_CH) - 1;
    vch->pulser.pul_general_ctrl.BITS.auto_prom=1;
    vch->pulser.pul_general_ctrl.BITS.pulser_load=1;
    vch->pulser.pul_general_ctrl.BITS.pulser_auto_load=1;
    vch->pulser.pul_general_ctrl.BITS.stop=0;
    vch->pulser.pul_general_ctrl.BITS.external_trigger=1;
    vch->pulser.pul_general_ctrl.BITS.software_trigger=0;

	vch->processing_type = msg->processing_type;
    vch->wait_prf_signals_processing = msg->wait_prf_signals_processing;
    vch->n_signals_processing_emi = msg->n_signals_processing_emi;
    vch->n_signals_processing_avr = 1<<msg->n_signals_processing_avr;
    ceillog2_n_bases = ceil_Log2(msg->n_bases);

	if(vch->processing_type==PROCESSING_TYPE_NONE)
	{
	    vch->afe.AFE_BUSSAR_UT.log2_promediados = 0;
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.BITS.dsr = ceillog2_n_bases;
	}
	else if (vch->processing_type==PROCESSING_TYPE_PROM_B)
	{
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.enable = 1;
		vch->emi_prom_config_acquisition.BITS.auto_mode = 1;
		vch->emi_prom_config_acquisition.BITS.prom = 1;
		vch->emi_prom_config_acquisition.BITS.num_rpt = vch->n_signals_processing_avr - 1;

		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.prom = 1;
		vch->emi_prom_config_beamforming.BITS.dsr  = (msg->n_signals_processing_avr+ceillog2_n_bases) & 0xF;
		vch->emi_prom_config_beamforming.BITS.num_rpt = vch->n_signals_processing_avr - 1;
		vch->emi_prom_config_beamforming.BITS.ce_bits=0xFF;
		vch->emi_prom_config_beamforming.BITS.mem2prom32 = 1;
		vch->emi_prom_config_beamforming.BITS.prom2mem16 = 1;
		vch->emi_prom_config_beamforming.BITS.auto_mode = 1;
	}
	else if (vch->processing_type==PROCESSING_TYPE_EMI_B)
	{
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.enable = 1;
		vch->emi_prom_config_acquisition.BITS.emi = 1;
		vch->emi_prom_config_acquisition.BITS.auto_mode = 1;
		vch->emi_prom_config_acquisition.BITS.num_rpt = (vch->n_signals_processing_emi-1) & 0xFF;

		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.emi = 1;
		vch->emi_prom_config_beamforming.BITS.num_rpt = (vch->n_signals_processing_emi-1)&0xFF;
		vch->emi_prom_config_beamforming.BITS.ce_bits=0xFF;
		vch->emi_prom_config_beamforming.BITS.mem2prom32 = 0;
		vch->emi_prom_config_beamforming.BITS.prom2mem16 = 0;
		vch->emi_prom_config_beamforming.BITS.auto_mode = 1;
		vch->emi_prom_config_beamforming.BITS.dsr = ceillog2_n_bases;

	}
	else if (vch->processing_type==PROCESSING_TYPE_PROM_A)
	{
	    vch->afe.AFE_BUSSAR_UT.log2_promediados = msg->log2_promediados;
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.last_rpt = 1;
	}


    vch->n_bases = msg->n_bases;
    for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)
    {
	   base = &vch->base[b];
	   //base->bcc_addr = b+1;
	   base->bcc_addr = vch->n_bases-b;
	}

   // CONFIGURACION DESDE LA UCI FIN -------------------------------------------------------------

   if ((result = VCH_UpdateAcquisitionSize(&gb_fp_virtual_channel[id_vch])) < 0) return RLOG(result);

	//if ((result = FP_UCI_SetVirtualChannel_ConfigHardware(&gb_fp_virtual_channel[id_vch], 1)) < 0)
    //     return RLOG(result);




   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Set_Config (TMSG_Config *msg)
{
int b, result, id_vch;
TVCH *vch = NULL;
TVCH_BASE *base = NULL;
u8 ceillog2_n_bases;

	if (gb_sitau_status != ST_None) return ELOG(charEUCI_IsRunning, EUCI_IsRunning);
	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	id_vch = msg->id_vch;
	if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);

	vch = &gb_fp_virtual_channel[id_vch];
	vch->acquiring = 0;
	vch->hardware_set_up = 0;
	vch->acquisition_type = msg->acquisition_type;
	vch->signal_mode = msg->signal_mode;
	vch->gtx_n_focal_law = msg->gtx_n_focal_law;
	vch->id = id_vch;
	vch->enabled = msg->enabled;
	vch->prf_time_line = msg->prf_time_line;
	vch->prf_time_burst = msg->prf_time_burst;
	printf("\r\nPRF Burst: %f", vch->prf_time_burst);

	vch->pa_image.n_beamformed_samples = (u32)msg->n_beamformed_samples;
	if (vch->pa_image.n_beamformed_samples % 2 != 0) vch->pa_image.n_beamformed_samples++;

	vch->gain_da = msg->gain_da;
	vch->gain_hi = msg->gain_hi;

	vch->enabled_timestamp = msg->enabled_timestamp;
	vch->enabled_buffer_dma = msg->enabled_buffer_dma;
	vch->enabled_temperature = msg->enabled_temperature;
	vch->enabled_encoder_1 = msg->enabled_encoder_1;
	vch->enabled_encoder_2 = msg->enabled_encoder_2;
	vch->enabled_encoder_3 = msg->enabled_encoder_3;
	vch->enabled_encoder_4 = msg->enabled_encoder_4;

	vch->afe.AFE_BUSSAR_UT.num_samples = (u32)msg->n_acq_samples;
	if (vch->afe.AFE_BUSSAR_UT.num_samples % 2 != 0) vch->afe.AFE_BUSSAR_UT.num_samples++;
    vch->afe.AFE_BUSSAR_UT.external_trigger = msg->external_trigger;
    vch->afe.AFE_BUSSAR_UT.dec_ratio = msg->decimation_factor;
    vch->afe.AFE_BUSSAR_UT.water_delay = msg->water_delay;


    vch->afe.AFE_SPI_UT.reg21.REG = msg->afe_spi.reg21.REG;
    vch->afe.AFE_SPI_UT.reg33.REG = msg->afe_spi.reg33.REG;
    vch->afe.AFE_SPI_UT.reg51.REG = msg->afe_spi.reg51.REG;
    vch->afe.AFE_SPI_UT.reg52.REG = msg->afe_spi.reg52.REG;
    vch->afe.AFE_SPI_UT.reg59.REG = msg->afe_spi.reg59.REG;
    vch->afe.AFE_SPI_UT.reg61.REG = msg->afe_spi.reg61.REG;

	vch->afe.AFE_SPI_UT.reg21.BIT.RESERVED0 = 0;
	vch->afe.AFE_SPI_UT.reg33.BIT.RESERVED0 = 0;
	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED0 = 0;
	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED1 = 0;
	vch->afe.AFE_SPI_UT.reg51.BIT.RESERVED2 = 0;
	vch->afe.AFE_SPI_UT.reg52.BIT.RESERVED0 = 0;
	vch->afe.AFE_SPI_UT.reg59.BIT.RESERVED0 = 0;
	vch->afe.AFE_SPI_UT.reg59.BIT.RESERVED1 = 0;
	vch->afe.AFE_SPI_UT.reg61.BIT.RESERVED0 = 0;

    vch->remote_addr_ini = FP_REMOTE_START_ADDR;
    vch->remote_addr_cur = FP_REMOTE_START_ADDR;
    vch->remote_addr_end = FP_REMOTE_LAST_ADDR;

    vch->pulser.pulser_main_delay = msg->pulser.delay;
    vch->pulser.pulse_width = msg->pulser.pulse_width;
    vch->pulser.pul_general_ctrl.BITS.pul_enable = msg->pulser.enabled;
    vch->pulser.pul_general_ctrl.BITS.gnd0_hiz1 = msg->pulser.high_z;
    vch->pulser.pul_general_ctrl.BITS.pulser_current = msg->pulser.mode;
    vch->pulser.pul_general_ctrl.BITS.n_pulses = msg->pulser.n_pulses;
    vch->pulser.delay_mem_ini_end.BITS.delay_mem_ini = 0;

    vch->pulser.delay_mem_ini_end.BITS.delay_mem_end = (vch->n_efl * MOD_MAX_CH) - 1;
    vch->pulser.pul_general_ctrl.BITS.auto_prom=1;
    vch->pulser.pul_general_ctrl.BITS.pulser_load=1;
    vch->pulser.pul_general_ctrl.BITS.pulser_auto_load=1;
    vch->pulser.pul_general_ctrl.BITS.stop=0;
    vch->pulser.pul_general_ctrl.BITS.external_trigger=1;
    vch->pulser.pul_general_ctrl.BITS.software_trigger=0;

	vch->processing_type = msg->processing_type;
    vch->wait_prf_signals_processing = msg->wait_prf_signals_processing;
    vch->n_signals_processing_emi = msg->n_signals_processing_emi;
    vch->n_signals_processing_avr = 1<<msg->n_signals_processing_avr;
    ceillog2_n_bases = ceil_Log2(msg->n_bases);

	if (vch->processing_type == PROCESSING_TYPE_NONE)
	{
		vch->afe.AFE_BUSSAR_UT.log2_promediados = 0;
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.BITS.dsr = ceillog2_n_bases;
	}
	else if (vch->processing_type==PROCESSING_TYPE_PROM_B)
	{
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.enable = 1;
		vch->emi_prom_config_acquisition.BITS.auto_mode = 1;
		vch->emi_prom_config_acquisition.BITS.prom = 1;
		vch->emi_prom_config_acquisition.BITS.num_rpt = vch->n_signals_processing_avr - 1;

		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.prom = 1;
		vch->emi_prom_config_beamforming.BITS.dsr  = (msg->n_signals_processing_avr) & 0xF;
		vch->emi_prom_config_beamforming.BITS.num_rpt = vch->n_signals_processing_avr - 1;
		vch->emi_prom_config_beamforming.BITS.ce_bits=0xFF;
		vch->emi_prom_config_beamforming.BITS.mem2prom32 = 1;
		vch->emi_prom_config_beamforming.BITS.prom2mem16 = 1;
		vch->emi_prom_config_beamforming.BITS.auto_mode = 1;
	}
	else if (vch->processing_type==PROCESSING_TYPE_EMI_B)
	{
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.enable = 1;
		vch->emi_prom_config_acquisition.BITS.emi = 1;
		vch->emi_prom_config_acquisition.BITS.auto_mode = 1;
		vch->emi_prom_config_acquisition.BITS.num_rpt = (vch->n_signals_processing_emi-1) & 0xFF;

		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.emi = 1;
		vch->emi_prom_config_beamforming.BITS.num_rpt = (vch->n_signals_processing_emi-1)&0xFF;
		vch->emi_prom_config_beamforming.BITS.ce_bits=0xFF;
		vch->emi_prom_config_beamforming.BITS.mem2prom32 = 0;
		vch->emi_prom_config_beamforming.BITS.prom2mem16 = 0;
		vch->emi_prom_config_beamforming.BITS.auto_mode = 1;
		vch->emi_prom_config_beamforming.BITS.dsr = ceillog2_n_bases;

	}
	else if (vch->processing_type==PROCESSING_TYPE_PROM_A)
	{
	    vch->afe.AFE_BUSSAR_UT.log2_promediados = msg->log2_promediados;
		vch->emi_prom_config_acquisition.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_acquisition.BITS.last_rpt = 1;
		vch->emi_prom_config_beamforming.emi_prom_config_u32 = 0x0;
		vch->emi_prom_config_beamforming.BITS.enable = 1;
		vch->emi_prom_config_beamforming.BITS.last_rpt = 1;
	}

    vch->n_bases = msg->n_bases;
    for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)
    {
    	base = &vch->base[b];
    	base->bcc_addr = vch->n_bases-b;
	}
    if ((result = VCH_UpdateAcquisitionSize(&gb_fp_virtual_channel[id_vch])) < 0) return RLOG(result);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Set_FIR (TMSG_FIR *msg)
{
int id_vch;

	if (gb_sitau_status != ST_None) return ELOG(charEUCI_IsRunning, EUCI_IsRunning);
	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
   id_vch = msg->id_vch;
   if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);

   gb_fp_virtual_channel[id_vch].fir.enabled = msg->enabled;
   gb_fp_virtual_channel[id_vch].fir.n_coefficients = msg->n_coefficients;
   memcpy(gb_fp_virtual_channel[id_vch].fir.coefficients, msg->coefficients, msg->n_coefficients * sizeof(short int));

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Set_EmissionFocalLaws (TMSG_EmissionFocalLaws *msg)
{
int b, result, id_vch;
u32 i, n_data;
TVCH *vch = NULL;

	if (gb_sitau_status != ST_None) return ELOG(charEUCI_IsRunning, EUCI_IsRunning);
	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	id_vch = msg->id_vch;
	if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);

	vch = &gb_fp_virtual_channel[id_vch];
	vch->pulser.delay_mem_ini_end.BITS.delay_mem_ini = 0;
	vch->pulser.delay_mem_ini_end.BITS.delay_mem_end = (msg->n_fl * MOD_MAX_CH) - 1;

	vch->n_efl = msg->n_fl;
	for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)
	{
		n_data = msg->n_fl * MOD_MAX_CH;
		for (i=0; i<n_data; i++)
			vch->base[b].efl[i] = msg->base[b].efl[i];
    }
	if ((result = VCH_UpdateAcquisitionSize(&gb_fp_virtual_channel[id_vch])) < 0) return RLOG(result);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Set_EmissionFocalLawsExtended (TMSG_EmissionFocalLawsExtended *msg)
{
int b, result, id_vch;
u32 i, n_data;
TVCH *vch = NULL;

	if (gb_sitau_status != ST_None) return ELOG(charEUCI_IsRunning, EUCI_IsRunning);
	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	id_vch = msg->id_vch;
	if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);

	vch = &gb_fp_virtual_channel[id_vch];
	if (msg->id_first_base + MAX_N_BASES_MSG > vch->n_bases)
		return ELOG(charEUCI_IdBASE, EUCI_IdBASE);
	if (vch->n_efl != msg->n_fl)
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	//vch->pulser.delay_mem_ini_end.BITS.delay_mem_ini = 0;
	//vch->pulser.delay_mem_ini_end.BITS.delay_mem_end = (msg->n_fl * MOD_MAX_CH) - 1;

	//vch->n_efl = msg->n_fl;
	for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)
	{
		n_data = msg->n_fl * MOD_MAX_CH;
		for (i=0; i<n_data; i++)
			vch->base[msg->id_first_base+b].efl[i] = msg->base[b].efl[i];
    }
	if ((result = VCH_UpdateAcquisitionSize(&gb_fp_virtual_channel[id_vch])) < 0) return RLOG(result);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_Set_Beamformer (TMSG_Beamformer *msg)
{
int b, result, id_vch;
unsigned short n_fl;
u32 i;
TVCH *vch;
TVCH_BASE *base = NULL;
u32 datamover_inc;
//	xil_printf("\n\rCONFORMACI�N INACTIVA, SALIENDO DE LA PROGRAMACI�N DE PAR�METROS DEL CONFORMADOR\n\r");
//	return 0;
	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	id_vch = msg->id_vch;
	if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);

	vch = &gb_fp_virtual_channel[id_vch];

	vch->pa_image.n_lines = 0;
	vch->afe.AFE_BUSSAR_UT.num_samples = msg->n_acq_samples;
	vch->pa_image.n_beamformed_samples = msg->n_beamformed_samples;
	n_fl = msg->n_fl;
	vch->pa_image.n_lines = n_fl;
	vch->bf_regs.image_lines.BITS.image_lines = vch->pa_image.n_lines - 1;

	if(msg->beamforming_type == BEAMFORMING_PA)
	{
		vch->bf_regs.bits_ida.BITS.tfm=0;
		vch->bf_regs.bits_ida.BITS.pwi=0;
		vch->bf_regs.bits_ida.BITS.ipa=0;
		vch->bf_regs.parallel_lines.BITS.parallel_lines = msg->n_fl_parallel-1;//msg->base[b].n_fl_parallel-1;
		if (msg->n_fl_parallel == 1)
			vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer = 4-1; //Si estamos en modo conformación única, dec_beamformer es 3
		else
			vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer = ((msg->n_fl_parallel)/NUM_BEAMFORMER_COLUMNS)-1;// msg->base[b].n_fl_parallel / 2;

		if(msg->n_fl_parallel == 1)
			vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog = 1; //Si estamos en modo conformación única, los parámetros de conformación van en modo serie: sin alternar entre columnas de conformadores
		else
			vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog = 0; //Esto es modo PA con lineas en paralelo
	}
	else if(msg->beamforming_type == BEAMFORMING_TFM)
	{
		vch->bf_regs.bits_ida.BITS.tfm=1;
		vch->bf_regs.bits_ida.BITS.pwi=0;
		vch->bf_regs.bits_ida.BITS.ipa=0;
		vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer = msg->n_fl_parallel-1;
		vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog=0;
		vch->bf_regs.parallel_lines.BITS.parallel_lines = n_fl-1;
	}
	else if(msg->beamforming_type == BEAMFORMING_PWI)
	{
		vch->bf_regs.bits_ida.BITS.tfm=0;
		vch->bf_regs.bits_ida.BITS.pwi=1;
		vch->bf_regs.bits_ida.BITS.ipa=0;
		vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer = msg->n_fl_parallel-1;
		vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog=0;
		vch->bf_regs.parallel_lines.BITS.parallel_lines = n_fl-1;
	}
	else if(msg->beamforming_type == BEAMFORMING_IPA)
	{
		vch->bf_regs.bits_ida.BITS.tfm=0;
		vch->bf_regs.bits_ida.BITS.pwi=0;
		vch->bf_regs.bits_ida.BITS.ipa=1;
		vch->bf_regs.bf_gen_ctrl.BITS.dec_beamformer = msg->n_fl_parallel-1;
		vch->bf_regs.bf_gen_ctrl.BITS.serial_param_prog=0;
		vch->bf_regs.parallel_lines.BITS.parallel_lines = n_fl-1;
	}

	// Reception Focal Law


	// Registros Beamformer



	vch->bf_regs.prog_addr_ini_end.BITS.prog_addr_ini = 0;
//	vch->bf_regs.T0_BF.T0[0] = msg->t0_max;//100;//(msg->n_acq_samples - msg->n_beamformed_samples)/2;//APAÑO!!!! PC NO MANDA BIEN T0!!!!!!!!
//	vch->bf_regs.T0_BF.T0[0] = (msg->n_acq_samples - msg->n_beamformed_samples)/2;//APAÑO!!!! PC NO MANDA BIEN T0!!!!!!!!
	if(msg->t0_max>2000)
		vch->bf_regs.T0_BF.T0[0] = 2000;
	else
		vch->bf_regs.T0_BF.T0[0] =  msg->t0_max+10;
	xil_printf("\n\rmsg->t0_max = %d\n\r",msg->t0_max);
	//xil_printf("MAAAL, msg->t0_max maaal!!");


	vch->bf_regs.n_foci = msg->n_beamformed_samples;
	vch->bf_regs.shft_bits = msg->shift_bits;
	vch->size32_bf_memory = msg->n_lword;

	vch->remote_addr_bf_ini	= FP_REMOTE_HALF_ADDR;
	vch->remote_addr_bf_cur	= FP_REMOTE_HALF_ADDR;
	vch->remote_addr_bf_end	= FP_REMOTE_SCRATCH_ADDR-1;
	vch->emi_prom_scratch_addr = FP_REMOTE_SCRATCH_ADDR;

	for (b=0; b<vch->n_bases && b<MAX_N_BASES; b++)
	{
		if ((base = &vch->base[b]) == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);

		// Memoria de parámetros de tiempos de vuelta
		for (i=0; i<vch->size32_bf_memory && i<MOD_BEAMFORMER_MEMORY_SIZE; i++)
			base->bf_memory[i] = msg->base[b].bf_memory[i];

//		datamover_inc = DDR_MINIBASE_SIZE / 2 / NUM_BEAMFORMER_DATAMOVERS;
//		datamover_inc -= datamover_inc % DDR_MINIBASE_ALIGN_SIZE;//Alineamos el tamaño
   }

   if ((result = VCH_UpdateAcquisitionSize(vch)) < 0) return RLOG(result);

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_Set_TFM_FMC_Forward (TMSG_TFM_FMC_Forward *msg)
{
int id_vch;
TVCH *vch;

	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	if (msg->size32_memory > 0)
	{
		id_vch = msg->id_vch;
		if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);
		if (msg->size32_memory > TFM_FMC_FORWARD_MEMORY_SIZE) return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
		vch = &gb_fp_virtual_channel[id_vch];

		vch->tfm_fmc_forward.size32_memory = msg->size32_memory;
		vch->tfm_fmc_forward.size32_forward_focal_law = msg->size32_forward_focal_law;
		vch->tfm_fmc_forward.n_forward_focal_law = msg->n_forward_focal_law;
		memcpy(vch->tfm_fmc_forward.memory, msg->memory, msg->size32_memory * sizeof(u32));
	}
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int VCH_Set_TFM_PWI_Forward (TMSG_TFM_PWI_Forward *msg)
{
int id_vch;
TVCH *vch;

	if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);
	if (msg->size32_memory > 0)
	{
		id_vch = msg->id_vch;
		if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);
		if (msg->size32_memory > TFM_FMC_FORWARD_MEMORY_SIZE) return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
		vch = &gb_fp_virtual_channel[id_vch];

		vch->tfm_pwi_forward.size32_memory = msg->size32_memory;
		vch->tfm_pwi_forward.size32_forward_focal_law = msg->size32_forward_focal_law;
		vch->tfm_pwi_forward.n_forward_focal_law = msg->n_forward_focal_law;
		memcpy(vch->tfm_pwi_forward.forwardMemory, msg->forwardMemory, msg->size32_memory * sizeof(u32));
		memcpy(vch->tfm_pwi_forward.angleMemory, msg->anglesMemory, msg->n_forward_focal_law * sizeof(u16));
	}
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un barrido phased array.

@return Ver cÃƒÂ¯Ã‚Â¿Ã‚Â½digos de error.
*/
// -----------------------------------------------------------------------------
int VCH_Set_TGC (TMSG_TGC *msg)
{
int i, id_vch;
unsigned short n_points;
TVCH *vch = NULL;

	if (msg == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	id_vch = msg->id_vch;
	if (id_vch >= MAX_VIRTUAL_CHANNELS) return ELOG(charEUCI_IdVirtualChannel, EUCI_IdVirtualChannel);
	if (msg->n_points < 0 || msg->n_points > TGC_MAX_SIZE)
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);

	if ((vch = &gb_fp_virtual_channel[id_vch]) == NULL) return ELOG(charEUCI_PtrAccess_NULL, EUCI_PtrAccess_NULL);
	n_points = vch->tgc.n_points = msg->n_points;
	for (i=0; i<n_points && i<MOD_PUL_MAX_FOCAL_LAW; i++) vch->tgc.curve[i] = msg->curve[i];

	vch->tgc.cfg.tgc_ini_delay=0;   //HAY QUE CONFIGURARLO DESDE EL PC
	vch->tgc.cfg.tgc_mem_ini_end.BITS.tgc_mem_ini=0;
	vch->tgc.cfg.tgc_mem_ini_end.BITS.tgc_mem_end=(n_points?n_points-1:0);
	vch->tgc.cfg.tgc_general_ctrl.tgc_general_ctrl_u32=0;
	vch->tgc.cfg.tgc_general_ctrl.BITS.external_trigger = (vch->tgc.n_points ? 1 : 0);
	vch->tgc.cfg.tgc_general_ctrl.BITS.tgc_enable = (vch->tgc.n_points ? 1 : 0);
	return EUCI_NONE;
}

// -----------------------------------------------------------------------------
/**
Configura el evento de disparo.

@return Ver cÃ¯Â¿Â½digos de error.
*/
// -----------------------------------------------------------------------------
/*int UCI_Set_TriggerSource_v0 (TMSG_TriggerSource_v0 *msg)
{
T_Trigger trg;
int result;

   if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	TRIG_reset_all_interrupt_sources();

   trg.REG = 0;
   gb_uci.ind_acquisitions = 0;
   gb_uci.n_acquisitions = msg->n_acquisitions;
   gb_uci.delay_sync_n_acq = msg->delay_sync_n_acq;
   gb_uci.n_acquisitions_interval = 1;
   gb_uci.acquisition_mode = 0;
   if (msg->trigger_source.BIT.EXT == 1)
   {
      trg.BIT.EXT = 1;
      gb_uci.trigger_source = TRG_SCAN_EXT;
   }
   else if (msg->trigger_source.BIT.PRF_1 == 1)
   {
      trg.BIT.PRF_1 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_1);
      timer_start(TIMER_SCAN_PRF_1);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.PRF_2 == 1)
   {
      trg.BIT.PRF_2 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_2);
      timer_start(TIMER_SCAN_PRF_2);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.ENC_1 == 1)
   {
      if ((result = ENC_config(0,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_1 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 0;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_2 == 1)
   {
      if ((result = ENC_config(1,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_2 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 1;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_3 == 1)
   {
      if ((result = ENC_config(2,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_3 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 2;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_4 == 1)
   {
      if ((result = ENC_config(3,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_4 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 3;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else
   {
      gb_uci.trigger_source = TRG_SW;
      gb_sitau_status = ST_None;
      return EUCI_NONE;
   }
	gb_encoder_trigger_value_offset = 0;
   TRIG_set_interrupt_source(trg.REG);

	//if ((result = VCH_PRG(&gb_fp_virtual_channel[0])) < 0) return RLOG(result);
	gb_sitau_status = ST_WaitTrigger;
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
int UCI_Set_TriggerSource (TMSG_TriggerSource *msg)
{
T_Trigger trg;
int result;

   if (msg == NULL) return ELOG(charEUCI_Parameter, EUCI_Parameter);

	TRIG_reset_all_interrupt_sources();

   trg.REG = 0;
   gb_uci.ind_acquisitions = 0;
   xil_printf("\n\r--------------CUIDADO, NUMERO DE ADQUICIONES MODIFICADO PARA PRUEBAS-------------------\n\r");
   gb_uci.n_acquisitions = msg->n_acquisitions*128;
   gb_uci.delay_sync_n_acq = msg->delay_sync_n_acq;
   gb_uci.n_acquisitions_interval = msg->n_acquisitions_interval;
   gb_uci.acquisition_mode = msg->acquisition_mode;

   if (msg->trigger_source.BIT.EXT == 1)
   {
      trg.BIT.EXT = 1;
      gb_uci.trigger_source = TRG_SCAN_EXT;
   }
   else if (msg->trigger_source.BIT.PRF_1 == 1)
   {
      trg.BIT.PRF_1 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_1);
      timer_start(TIMER_SCAN_PRF_1);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.PRF_2 == 1)
   {
      trg.BIT.PRF_2 = 1;
      timer_set_periodic_count_no_int(msg->prf_time,TIMER_SCAN_PRF_2);
      timer_start(TIMER_SCAN_PRF_2);
      gb_uci.trigger_source = TRG_SCAN_PRF;
   }
   else if (msg->trigger_source.BIT.ENC_1 == 1)
   {
      if ((result = ENC_config(0,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_1 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 0;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_2 == 1)
   {
      if ((result = ENC_config(1,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_2 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 1;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_3 == 1)
   {
      if ((result = ENC_config(2,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_3 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 2;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else if (msg->trigger_source.BIT.ENC_4 == 1)
   {
      if ((result = ENC_config(3,1,1,1,msg->encoder_steps,0)) < 0) return RLOG(result);
      trg.BIT.ENC_4 = 1;
      gb_uci.trigger_source = TRG_SCAN_ENC;
		gb_uci.encoder_trigger = 3;
		gb_uci.encoder_trigger_steps = msg->encoder_steps;
   }
   else
   {
      gb_uci.trigger_source = TRG_SW;
      gb_sitau_status = ST_None;
      return EUCI_NONE;
   }
	gb_encoder_trigger_value_offset = 0;

	TRIG_set_interrupt_source(trg.REG);

	if (gb_fp_virtual_channel[gb_uci.active_virtual_channel].enabled == 1)
	{
		if (gb_log_acquiring == 1)
			LOG_ACQUIRING("\r\nSET_UP FP_VCH %d (%3d Lines) (%d Samples)... ",
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].id,
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_efl,
				gb_fp_virtual_channel[gb_uci.active_virtual_channel].afe.AFE_BUSSAR_UT.num_samples);

		ACQ_FMC_set_up(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]); // Est� ser� m�s complicado m�s tarde

	}


   //TRIG_set_interrupt_source(trg.REG);
   //if ((result = VCH_PRG(&gb_fp_virtual_channel[0])) < 0) return RLOG(result);
   gb_sitau_status = ST_WaitTrigger;
   return EUCI_NONE;
}

*/
