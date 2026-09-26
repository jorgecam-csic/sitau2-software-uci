/*
 * afe_bussar.c
 *
 *  Created on: 26 mar. 2019
 *      Author: Jorge
 */


#include "afe_bussar.h"
#include "afe_error_code.h"
#include "afe5808a.h"
#include "log.h"
#include "switch_driver.h"
#include "beamformer_bussar.h"
#include "beamformer_param_example.h"
#include "delays_afe.h"

u8 Print_spi_afe_regs=1;
u8 Prog_siempre_regs_afe=1;

AFE_SPI_UT_t Last_prog_AFE_SPI_UT[MAX_MINIBASES];

//#ifdef MAX_N_BASES
//diagrama_ojos_t diagrama_de_ojos_completo[MAX_N_BASES];
//u8 afe_dly_final[MAX_N_BASES][N_CHANNELS];
//#else
//diagrama_ojos_t diagrama_de_ojos_completo[4];
//u8 afe_dly_final[4][N_CHANNELS];
//#endif
void get_spi_afe_regs(AFE_SPI_UT_t *regs, u8 minibase_addr)
{
	u32 afe5808a_0;
    afe5808a_0=(u32)MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);
    afe5808a_select(afe5808a_0,0x1);

	afe5808a_spi_read(afe5808a_0,21,&regs->reg21.REG);
	afe5808a_spi_read(afe5808a_0,33,&regs->reg33.REG);
	afe5808a_spi_read(afe5808a_0,51,&regs->reg51.REG);
	afe5808a_spi_read(afe5808a_0,52,&regs->reg52.REG);
	afe5808a_spi_read(afe5808a_0,59,&regs->reg59.REG);
	afe5808a_spi_read(afe5808a_0,61,&regs->reg61.REG);

    MCBCC_deactivate_emu();
}

void print_spi_afe_regs(AFE_SPI_UT_t regs)
{
  xil_printf("REGISTROS SPI DE LOS AFES\r\n");

  xil_printf("Registro 21: %04X\r\n",regs.reg21.REG);
  xil_printf("DIGITAL_HPF_FILTER_ENABLE_1_4  , 1: %2d. Bit[0]: no hace nada\r\n",regs.reg21.BIT.DIGITAL_HPF_FILTER_ENABLE_1_4);
  xil_printf("DIGITAL_HPF_FILTER_K_CH_1_4    , 3: %2d. Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B).\r\n",regs.reg21.BIT.DIGITAL_HPF_FILTER_K_CH_1_4);
  xil_printf("RESERVED0                      ,11: %2d. Bit[15:5]: \r\n",regs.reg21.BIT.RESERVED0);
  xil_printf("----------------\r\n");

  xil_printf("Registro 33: %04X\r\n",regs.reg33.REG);
  xil_printf("DIGITAL_HPF_FILTER_ENABLE_5_8  , 1: %2d. Bit[0]: no hace nada\r\n",regs.reg33.BIT.DIGITAL_HPF_FILTER_ENABLE_5_8);
  xil_printf("DIGITAL_HPF_FILTER_K_CH_5_6    , 3: %2d. Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B).\r\n",regs.reg33.BIT.DIGITAL_HPF_FILTER_K_CH_5_8);
  xil_printf("RESERVED0                      ,11: %2d. Bit[15:5]:\r\n",regs.reg33.BIT.RESERVED0);
  xil_printf("----------------\r\n");

  xil_printf("Registro 51: %04X\r\n",regs.reg51.REG);
  xil_printf("RESERVED0              , 1: %2d. Bit[0]: no hace nada\r\n",regs.reg51.BIT.RESERVED0);
  xil_printf("LPF_PROGRAMMABILITY    , 3: %2d. Bit[3:1]: 000: 15 MHz,010: 20 MHz,011: 30 MHz,100: 10 MHz\r\n",regs.reg51.BIT.LPF_PROGRAMMABILITY);
  xil_printf("PGA_INTEGRATOR_DISABLE , 1: %2d. Bit[4]: 1: Disables offset integrator for PGA. Mirar datasheet.\r\n",regs.reg51.BIT.PGA_INTEGRATOR_DISABLE);
  xil_printf("PGA_CLAMP_LEVEL        , 3: %2d. Bit[7:5]: In normal operation, clamp function can be set as 000 in the low noise mode.\r\n",regs.reg51.BIT.PGA_CLAMP_LEVEL);
  xil_printf("RESERVED1              , 5: %2d. Bit[12:8]: no hace nada\r\n",regs.reg51.BIT.RESERVED1);
  xil_printf("PGA_GAIN_CONTROL       , 1: %2d. Bit[13]: 0:24 dB; 1:30 dB\r\n",regs.reg51.BIT.PGA_GAIN_CONTROL);
  xil_printf("RESERVED2              , 2: %2d. Bit[15:14]: no hace nada\r\n",regs.reg51.BIT.RESERVED2);
  xil_printf("----------------\r\n");

  xil_printf("Registro 52: %04X\r\n",regs.reg52.REG);
  xil_printf("ACTIVE_TERMINATION          , 5: %2d. Bit[4:0]: Control individual de la red de resistecias de terminación\r\n",regs.reg52.BIT.ACTIVE_TERMINATION);
  xil_printf("ACT_TER_ENA_CTRL            , 1: %2d. Bit[5]: 1: Enable internal active termination individual resistor control\r\n",regs.reg52.BIT.ACT_TER_ENA_CTRL);
  xil_printf("PRESET_ACTIVE_TERMINATIONS  , 2: %2d. Bit[7:6]: 00: 50 Ohm; 01: 100 Ohm; 10: 200 Ohm; 11: 400 Ohm\r\n",regs.reg52.BIT.PRESET_ACTIVE_TERMINATIONS);
  xil_printf("ACTIVE_TERMINATION_ENABLE   , 1: %2d. Bit[8]: Enable active termination\r\n",regs.reg52.BIT.ACTIVE_TERMINATION_ENABLE);
  xil_printf("LNA_INPUT_CLAMP_SETTING     , 2: %2d. Bit[10:9]: 00: Auto setting; 01: 1.5 Vpp; 10: 1.15 Vpp; 11: 0.6 Vpp\r\n",regs.reg52.BIT.LNA_INPUT_CLAMP_SETTING);
  xil_printf("RESERVED0                   , 1: %2d. Bit[11]: Set to zero.\r\n",regs.reg52.BIT.RESERVED0);
  xil_printf("LNA_INTEGRATOR_DISABLE      , 1: %2d. Bit[12]: Disable offset integrator for LNA. Mirar datasheet\r\n",regs.reg52.BIT.LNA_INTEGRATOR_DISABLE);
  xil_printf("LNA_GAIN                    , 2: %2d. Bit[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved\r\n",regs.reg52.BIT.LNA_GAIN);
  xil_printf("LNA_INDIVIDUAL_CH_CNTL      , 1: %2d. Bit[15]: 1: Activa control individual de la ganancia LNA con el registro 57\r\n",regs.reg52.BIT.LNA_INDIVIDUAL_CH_CNTL);
  xil_printf("----------------\r\n");

  xil_printf("Registro 59: %04X\r\n",regs.reg59.REG);
  xil_printf("RESERVED0         , 2: %2d. Bit[1:0]: no hace nada\r\n",regs.reg59.BIT.RESERVED0);
  xil_printf("HPF_LNA           , 2: %2d. Bit[3:2]: 00: 100 kHz 01: 50 kHz 10: 200 kHz 11: 150 kHz with 0.015 uF on INMx\r\n",regs.reg59.BIT.HPF_LNA);
  xil_printf("DIG_TGC_ATT_GAIN  , 3: %2d. Bit[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1\r\n",regs.reg59.BIT.DIG_TGC_ATT_GAIN);
  xil_printf("DIG_TGC_ATT       , 1: %2d. Bit[7]: 0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator\r\n",regs.reg59.BIT.DIG_TGC_ATT);
  xil_printf("CW_SUM_AMP_PDN    , 1: %2d. Bit[8]: 0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode\r\n",regs.reg59.BIT.CW_SUM_AMP_PDN);
  xil_printf("PGA_TEST_MODE     , 1: %2d. Bit[9]: 0: Normal CW operation, 1: PGA outputs appear at CW outputs\r\n",regs.reg59.BIT.PGA_TEST_MODE);
  xil_printf("RESERVED1         , 6: %2d. Bit[15:10]: no hace nada\r\n",regs.reg59.BIT.RESERVED1);
  xil_printf("----------------\r\n");


  xil_printf("Registro 61: %04X\r\n",regs.reg61.REG);
  xil_printf("RESERVED0               ,13: %2d. Bit[12:0]: no hace nada\r\n",regs.reg61.BIT.RESERVED0);
  xil_printf("V2I_CLAMP               , 1: %2d. Bit[13]: 0: Clamp disabled 1: Clamp enabled \r\n",regs.reg61.BIT.V2I_CLAMP);
  xil_printf("LPF_5MHz                , 1: %2d. Bit[14]: 0: 5-MHz LPF disabled 1: 5-MHz LPF enabled. \r\n",regs.reg61.BIT.LPF_5MHz);
  xil_printf("PGA_CLAMP_minus6dBFS    , 1: %2d. Bit[15]: 0: Disable the –6-dBFS clamp. PGA_CLAMP is set by Reg51[7:5]. 1: Enable the –6-dBFS clamp. PGA_CLAMP\r\n",regs.reg61.BIT.LPF_5MHz);
  xil_printf("----------------\r\n");
}

void print_spi_afe_regs_resumen(AFE_SPI_UT_t regs)
{
  xil_printf("\n\rREGISTROS CLAVE SPI DE LOS AFES\r\n");
  xil_printf("DIGITAL_HPF_FILTER_ENABLE_1_4: FALTA");


  xil_printf("LPF_PROGRAMMABILITY     , 3: %2d. 51[3:1]: 000: 15 MHz,010: 20 MHz,011: 30 MHz,100: 10 MHz\r\n",regs.reg51.BIT.LPF_PROGRAMMABILITY);
  xil_printf("PGA_INTEGRATOR_DISABLE  , 1: %2d. Bit[4]: 1: Disables offset integrator for PGA. Mirar datasheet.\r\n",regs.reg51.BIT.PGA_INTEGRATOR_DISABLE);
  xil_printf("PGA_GAIN_CONTROL        , 1: %2d. 51[13]: 0:24 dB; 1:30 dB\r\n",regs.reg51.BIT.PGA_GAIN_CONTROL);
  xil_printf("LNA_INTEGRATOR_DISABLE  , 1: %2d. 52[12]: Disable offset integrator for LNA. Mirar datasheet\r\n",regs.reg52.BIT.LNA_INTEGRATOR_DISABLE);
  xil_printf("LNA_GAIN                , 2: %2d. 52[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved\r\n",regs.reg52.BIT.LNA_GAIN);
  xil_printf("HPF_LNA                 , 2: %2d. 59[3:2]: 00: 100 kHz  01: 50 kHz 10: 200 kHz 11: 150 kHz with 0.015 uF on INMx\r\n",regs.reg59.BIT.HPF_LNA);
  xil_printf("DIG_TGC_ATT_GAIN        , 3: %2d. 59[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1\n\r",regs.reg59.BIT.DIG_TGC_ATT_GAIN);
  xil_printf("DIG_TGC_ATT             , 1: %2d. 59[7]: 0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator\r\n",regs.reg59.BIT.DIG_TGC_ATT);
  xil_printf("LPF_5MHz                , 1: %2d. 61[14]: 0: 5-MHz LPF disabled 1: 5-MHz LPF enabled. \r\n",regs.reg61.BIT.LPF_5MHz);
  xil_printf("----------------\r\n");
}


int prog_afe_SPI_regs(AFE_SPI_UT_t AFE_SPI_UT_nuevo,u8 minibase_addr)
{
	u16 value, i;
	u32 afe5808a_0;
	// if(Print_spi_afe_regs)
	// {
		// print_spi_afe_regs(AFE_SPI_UT_nuevo);
		// print_spi_afe_regs_resumen(AFE_SPI_UT_nuevo);

	// }
	//minibase_addr=1;
	if(0)//if(minibase_addr==0)
	{
		for(u8 minibase_addr=1;minibase_addr<=4;minibase_addr++)
		{
			afe5808a_0=(u32)MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);
			afe5808a_select(afe5808a_0,0xf);

			afe5808a_spi_write(afe5808a_0,21,AFE_SPI_UT_nuevo.reg21.REG);
			afe5808a_spi_write(afe5808a_0,33,AFE_SPI_UT_nuevo.reg33.REG);
			afe5808a_spi_write(afe5808a_0,51,AFE_SPI_UT_nuevo.reg51.REG);
			afe5808a_spi_write(afe5808a_0,52,AFE_SPI_UT_nuevo.reg52.REG);
			afe5808a_spi_write(afe5808a_0,59,AFE_SPI_UT_nuevo.reg59.REG);
			afe5808a_spi_write(afe5808a_0,61,AFE_SPI_UT_nuevo.reg61.REG);

			MCBCC_deactivate_emu();
		}
	}
	else
	{
		afe5808a_0=(u32)MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);
		afe5808a_select(afe5808a_0,0xf);

		afe5808a_spi_write(afe5808a_0,21,AFE_SPI_UT_nuevo.reg21.REG);
		afe5808a_spi_write(afe5808a_0,33,AFE_SPI_UT_nuevo.reg33.REG);
		afe5808a_spi_write(afe5808a_0,51,AFE_SPI_UT_nuevo.reg51.REG);
		afe5808a_spi_write(afe5808a_0,52,AFE_SPI_UT_nuevo.reg52.REG);
		afe5808a_spi_write(afe5808a_0,59,AFE_SPI_UT_nuevo.reg59.REG);
		afe5808a_spi_write(afe5808a_0,61,AFE_SPI_UT_nuevo.reg61.REG);

		MCBCC_deactivate_emu();
	}

	return;
    if(		Last_prog_AFE_SPI_UT[minibase_addr].reg21.REG==AFE_SPI_UT_nuevo.reg21.REG &&
    		Last_prog_AFE_SPI_UT[minibase_addr].reg33.REG==AFE_SPI_UT_nuevo.reg33.REG &&
    		Last_prog_AFE_SPI_UT[minibase_addr].reg51.REG==AFE_SPI_UT_nuevo.reg51.REG &&
    		Last_prog_AFE_SPI_UT[minibase_addr].reg52.REG==AFE_SPI_UT_nuevo.reg52.REG &&
			Last_prog_AFE_SPI_UT[minibase_addr].reg59.REG==AFE_SPI_UT_nuevo.reg59.REG &&
			Last_prog_AFE_SPI_UT[minibase_addr].reg61.REG==AFE_SPI_UT_nuevo.reg61.REG)
    	return 0;

    afe5808a_0=(u32)MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);

    afe5808a_select(afe5808a_0,0xF);

    if(Last_prog_AFE_SPI_UT[minibase_addr].reg21.REG!=AFE_SPI_UT_nuevo.reg21.REG)
        	afe5808a_spi_write(afe5808a_0,21,AFE_SPI_UT_nuevo.reg21.REG);

    if(Last_prog_AFE_SPI_UT[minibase_addr].reg33.REG!=AFE_SPI_UT_nuevo.reg33.REG)
        	afe5808a_spi_write(afe5808a_0,33,AFE_SPI_UT_nuevo.reg33.REG);

    if(Last_prog_AFE_SPI_UT[minibase_addr].reg51.REG!=AFE_SPI_UT_nuevo.reg51.REG)
        	afe5808a_spi_write(afe5808a_0,51,AFE_SPI_UT_nuevo.reg51.REG);

    if(Last_prog_AFE_SPI_UT[minibase_addr].reg52.REG!=AFE_SPI_UT_nuevo.reg52.REG)
        	afe5808a_spi_write(afe5808a_0,52,AFE_SPI_UT_nuevo.reg52.REG);

    if(Last_prog_AFE_SPI_UT[minibase_addr].reg59.REG!=AFE_SPI_UT_nuevo.reg59.REG)
        	afe5808a_spi_write(afe5808a_0,59,AFE_SPI_UT_nuevo.reg59.REG);

    if(Last_prog_AFE_SPI_UT[minibase_addr].reg61.REG!=AFE_SPI_UT_nuevo.reg61.REG)
        	afe5808a_spi_write(afe5808a_0,61,AFE_SPI_UT_nuevo.reg61.REG);

    Last_prog_AFE_SPI_UT[minibase_addr]=AFE_SPI_UT_nuevo;

    MCBCC_deactivate_emu();

    return 1;
}

int get_prog_afe_commands(AFE_BUSSAR_UT_t afe_ut,commnd_buffer_t *commnd_buf,u8 minibase_addr)
{
	int num_words=0;
	ctrl_afe_t reg0_afe;
	reg0_afe.REG=0x00000000;
	reg0_afe.BIT.dec_ratio = afe_ut.dec_ratio;
	reg0_afe.BIT.external_trigger = afe_ut.external_trigger;
	num_words+=push_bussar_write_reg(reg0_afe.REG,minibase_addr,AFE_BUSSAR_SUBMOD_ADDR,AFE_CTRL_REG_OFFSET,commnd_buf);
	num_words+=push_bussar_write_reg(afe_ut.num_samples,minibase_addr,AFE_BUSSAR_SUBMOD_ADDR,AFE_ACQ_REG_OFFSET,commnd_buf);
	num_words+=push_bussar_write_reg((u32)afe_ut.log2_promediados,minibase_addr,AFE_BUSSAR_SUBMOD_ADDR,PROEMI_REG_OFFSET,commnd_buf);
	num_words+=push_bussar_write_reg(afe_ut.water_delay,minibase_addr,AFE_BUSSAR_SUBMOD_ADDR,AFE_WATER_DELAY_REG_OFFSET,commnd_buf);
	if(afe_ut.num_samples>AFE_MAX_SAMPLES)
		return ELOG(charERROR_NUM_SAMPLES_AFE,ERROR_NUM_SAMPLES_AFE);
	return num_words;
}

#define CALC_DELAYS	1
int all_afe_remote_align(u8 num_minibases)
{

	int ret=0;
	u8* delays;
	u8* tam_ojos;
    u8 delays_completo[N_CHANNELS_PER_AFE*N_AFES*MAX_MINIBASES],tam_ojos_completo[N_CHANNELS_PER_AFE*N_AFES*MAX_MINIBASES];
    u8* delays_to_print;
	hw_uint32_ptr afe5808a_0;
	ctrl_afe_t afe_ctrl_reg;
	u32 i;

	u8 minibase_addr;
    afe5808a_0=MCBCC_activate_emu(0,AFE_BUSSAR_SUBMOD_ADDR);

    afe_ctrl_reg.REG = 0x0;


    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;
    for(i=0;i<1000;i++);

    afe_ctrl_reg.REG = 0x0;

    afe_ctrl_reg.BIT.pwdn_glb = 0;
    afe_ctrl_reg.BIT.samples_ce = 1;
    afe_ctrl_reg.BIT.reset_serdes = 1;

    afe_ctrl_reg.BIT.afe_rst = 1;
    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;

    for(i=0;i<1000;i++);


    afe_ctrl_reg.BIT.reset_serdes = 0;
    afe_ctrl_reg.BIT.afe_rst = 0;
    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;


//	afe5808a_select((u32)afe5808a_0,0x0f);  //selecciono 4 afes con cs
//	afe5808a_spi_write((u32)afe5808a_0,22,1); // Deshabilitar demodulador
    MCBCC_deactivate_emu();

    for(i=0;i<1000;i++);

	for(minibase_addr=1;minibase_addr<=num_minibases;minibase_addr++)
	{
		u16 value;

		diagrama_ojos_t diagrama_ojos;



	    afe5808a_0=MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);


	    afe5808a_basic_set_up(afe5808a_0);

	 	if(CALC_DELAYS)
	 	{
		    delays=&delays_completo[(minibase_addr-1)*N_CHANNELS_PER_AFE*N_AFES];
		    tam_ojos=&tam_ojos_completo[(minibase_addr-1)*N_CHANNELS_PER_AFE*N_AFES];
		    diagrama_de_ojos((u32)afe5808a_0,&diagrama_ojos);
			//print_diagrama_de_ojos(&diagrama_ojos);
			calc_delays(&diagrama_ojos,delays,tam_ojos);
			xil_printf("Iniciando AFEs...\n\r");

	 	}
	 	else
	 		delays=&delays_completo_cst[(minibase_addr-1)*N_CHANNELS_PER_AFE*N_AFES];

//	 	if(delays_test_7_ones((u32)afe5808a_0,delays))
//	 		ret =-1;
	 	if(afe5808a_complete_skew_align_delays((u32)afe5808a_0,delays))
	 		ret =-1;

//		for(i=1; i<=8 ; i=i<<1)
//		{
//			afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
//			xil_printf("CALIB_REG AFE %X: %04X\r\n",i,afe5808a_0[3]);
//		}


//		for(i=1; i<=8 ; i=i<<1)
//		{
//			afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
//			config_afe((u32)afe5808a_0);
//			xil_printf("\r\n");
//			//afe5808a_ramp((u32)afe5808a_0);
//			print_current_afe_regs((u32)afe5808a_0);
//		}

		for(int i=1; i<=8 ; i=i<<1)
		{
			u16 dato_leido;
			afe5808a_select((u32)afe5808a_0,i);

//			afe5808a_test_pattern(afe5808a_0,2);
//			afe5808a_spi_read(afe5808a_0,2,&dato_leido);
			afe5808a_test_pattern(afe5808a_0,0);
			afe5808a_spi_read(afe5808a_0,2,&dato_leido);

		}

		//afe_set_dig_gain((u32)afe5808a_0,0xff);
		MCBCC_deactivate_emu();

	}
	{
		u8 minibase_addr,afe,channel;
		xil_printf("\n\r         ");
		for(afe=0;afe<N_AFES;afe++)
		{
			for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
				xil_printf("A%dB%d ",afe,channel);
			xil_printf("  ");
		}

		if(CALC_DELAYS)
			delays_to_print = delays_completo;
		else
			delays_to_print = delays_completo_cst;

		for(minibase_addr=1;minibase_addr<=num_minibases;minibase_addr++)
		{
			xil_printf("\n\rBASE %2d: ",minibase_addr);
			for(afe=0;afe<N_AFES;afe++)
			{
				for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
					xil_printf("%4d,",delays_to_print[(minibase_addr-1)*N_AFES*N_CHANNELS_PER_AFE+afe*N_CHANNELS_PER_AFE+channel]);
				xil_printf("  ");
			}

		}
		xil_printf("\n\r");
	}

	if(CALC_DELAYS)
	{
		u8 minibase_addr,afe,channel;
		xil_printf("\n\r         ");
		for(afe=0;afe<N_AFES;afe++)
		{
			for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
				xil_printf("A%dB%d ",afe,channel);
			xil_printf("  ");
		}

		for(minibase_addr=1;minibase_addr<=num_minibases;minibase_addr++)
		{
			xil_printf("\n\rBASE %2d: ",minibase_addr);
			for(afe=0;afe<N_AFES;afe++)
			{
				for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
					xil_printf("%4d ",tam_ojos_completo[(minibase_addr-1)*N_AFES*N_CHANNELS_PER_AFE+afe*N_CHANNELS_PER_AFE+channel]);
				xil_printf("  ");
			}

		}
		xil_printf("\n\r");
	}

	xil_printf("\n\rAFEs iniciados.\r\n");

//	xil_printf("\n\r                                             ¡¡¡AFEs MODO RAMPA DEBUG!!!.\r\n");
//	afe5808a_0=MCBCC_activate_emu(0,AFE_BUSSAR_SUBMOD_ADDR);
//	afe5808a_select((u32)afe5808a_0,0xF);
//	afe5808a_test_pattern(afe5808a_0,6);
//	MCBCC_deactivate_emu();

	return ret;


}

int all_afe_remote_align_5809(u8 num_minibases)
{

	int ret=0;
    u8 delays_completo[N_CHANNELS_PER_AFE*N_AFES*MAX_MINIBASES],tam_ojos_completo[N_CHANNELS_PER_AFE*N_AFES*MAX_MINIBASES];


	u8 minibase_addr;
	//num_minibases=1;
	for(minibase_addr=2;minibase_addr<=num_minibases;minibase_addr++)
	{
		u16 value;
		u32 i;
		hw_uint32_ptr afe5808a_0;
		ctrl_afe_t afe_ctrl_reg;
		diagrama_ojos_t diagrama_ojos;
	    u8* delays;
	    u8* tam_ojos;


	    delays=&delays_completo[(minibase_addr-1)*N_CHANNELS_PER_AFE*N_AFES];
	    tam_ojos=&tam_ojos_completo[(minibase_addr-1)*N_CHANNELS_PER_AFE*N_AFES];

	    afe5808a_0=MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);

	    //afe_ctrl_reg.REG = afe5808a_0[SPI_CTRL_REG_OFFSET];
	    //xil_printf("\n\rAFE_CTRL_REG: 0x%08X\n\r",afe_ctrl_reg.REG);


	    afe_ctrl_reg.REG = 0x0;



	    afe_ctrl_reg.BIT.a0 = 1;
	    afe_ctrl_reg.BIT.a1 = 1;
	    afe_ctrl_reg.BIT.a2 = 1;
	    afe_ctrl_reg.BIT.a3 = 1;

	    afe_ctrl_reg.BIT.pwdn_glb = 0;
	    afe_ctrl_reg.BIT.samples_ce = 1;
	    afe_ctrl_reg.BIT.reset_serdes = 1;

	    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;
	    xil_printf("\n\rAFE_CTRL_REG: 0x%08X\n\r",afe5808a_0[SPI_CTRL_REG_OFFSET]);
	    afe_ctrl_reg.BIT.reset_serdes = 0;
	    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;

	    xil_printf("\n\rAFE_CTRL_REG: 0x%08X\n\r",afe5808a_0[SPI_CTRL_REG_OFFSET]);

	    afe5808a_basic_set_up(afe5808a_0);
	/*
	    afe_ctrl_reg.BIT.afe_rst = 0;

	    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;

	    xil_printf("\n\rAFE_CTRL_REG: 0x%08X\n\r",afe5808a_0[SPI_CTRL_REG_OFFSET]);

	    afe5808a_reset(afe5808a_0);
	    for(i=0;i<1000000;i++);
	*/

	    afe5808a_0[4]=0xF5FAF5AA;
	    afe5808a_select((u32)afe5808a_0,0x0f);  //selecciono 4 afes con cs
	    afe5808a_spi_write((u32)afe5808a_0,22,1); // Deshabilitar demodulador

	    afe5808a_select((u32)afe5808a_0,0x00);  //deselecciono 4 afes cs

		for(i=1; i<=8 ; i=i<<1)
		{
			afe5808a_select((u32)afe5808a_0,i);  //deselecciono 4 afes cs
			afe5808a_spi_read((u32)afe5808a_0,22,&value); // Deshabilitar demodulador
			xil_printf("Resultado AFE %X: %04X\r\n",i,(u32)value);
		}


		for(i=1; i<=8 ; i=i<<1)
		{
			afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
			xil_printf("CALIB_REG AFE %X: %04X\r\n",i,afe5808a_0[3]);
		}
	 	if(0)
	 	{
			diagrama_de_ojos((u32)afe5808a_0,&diagrama_ojos);
			print_diagrama_de_ojos(&diagrama_ojos);
			calc_delays(&diagrama_ojos,delays,tam_ojos);
			xil_printf("Iniciando AFEs...\n\r");
	 	}
	 	else
	 		delays=&delays_completo_cst[(minibase_addr-1)*N_CHANNELS_PER_AFE*N_AFES];

	 	if(afe5808a_complete_skew_align_delays((u32)afe5808a_0,delays))
	 		ret =-1;

//		for(i=1; i<=8 ; i=i<<1)
//		{
//			afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
//			xil_printf("CALIB_REG AFE %X: %04X\r\n",i,afe5808a_0[3]);
//		}
		xil_printf("AFEs iniciados.\r\n");


		for(i=1; i<=8 ; i=i<<1)
		{
			afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
			config_afe((u32)afe5808a_0);
			xil_printf("\r\n");
			//afe5808a_ramp((u32)afe5808a_0);
			print_current_afe_regs((u32)afe5808a_0);
		}

		for(int i=1; i<=8 ; i=i<<1){
		//if(0){
			u16 dato_leido;
			afe5808a_select((u32)afe5808a_0,i);

			afe5808a_test_pattern(afe5808a_0,2);
			afe5808a_spi_read(afe5808a_0,2,&dato_leido);
			afe5808a_test_pattern(afe5808a_0,0);
			afe5808a_spi_read(afe5808a_0,2,&dato_leido);
//			afe5808a_test_pattern(afe5808a_0,7);
//			afe5808a_spi_read(afe5808a_0,2,&dato_leido);
		}


		//afe_set_dig_gain((u32)afe5808a_0,0xff);
		MCBCC_deactivate_emu();

	}
	{
		u8 minibase_addr,afe,channel;
		xil_printf("\n\r         ");
		for(afe=0;afe<N_AFES;afe++)
		{
			for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
				xil_printf("A%dB%d ",afe,channel);
			xil_printf("  ");
		}

		for(minibase_addr=1;minibase_addr<=num_minibases;minibase_addr++)
		{
			xil_printf("\n\rBASE %2d: ",minibase_addr);
			for(afe=0;afe<N_AFES;afe++)
			{
				for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
					xil_printf("%4d ",delays_completo[(minibase_addr-1)*N_AFES*N_CHANNELS_PER_AFE+afe*N_CHANNELS_PER_AFE+channel]);
				xil_printf("  ");
			}

		}
		xil_printf("\n\r");
	}

	{
		u8 minibase_addr,afe,channel;
		xil_printf("\n\r         ");
		for(afe=0;afe<N_AFES;afe++)
		{
			for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
				xil_printf("A%dB%d ",afe,channel);
			xil_printf(" ");
		}

		for(minibase_addr=1;minibase_addr<=num_minibases;minibase_addr++)
		{
			xil_printf("\n\rBASE %2d: ",minibase_addr);
			for(afe=0;afe<N_AFES;afe++)
			{
				for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
					xil_printf("%4d ",tam_ojos_completo[(minibase_addr-1)*N_AFES*N_CHANNELS_PER_AFE+afe*N_CHANNELS_PER_AFE+channel]);
				xil_printf(" ");
			}

		}
		xil_printf("\n\r");
	}

	return ret;


}



int afe_remote_align(u8 minibase_addr)
{
	u16 value;
	u32 i;
	hw_uint32_ptr afe5808a_0;
	ctrl_afe_t afe_ctrl_reg;

    diagrama_ojos_t diagrama_ojos;
    u8 delays[N_CHANNELS],tam_ojos[N_CHANNELS];

    afe5808a_0=MCBCC_activate_emu(minibase_addr,AFE_BUSSAR_SUBMOD_ADDR);

    //afe_ctrl_reg.REG = afe5808a_0[SPI_CTRL_REG_OFFSET];
    //xil_printf("\n\rAFE_CTRL_REG: 0x%08X\n\r",afe_ctrl_reg.REG);


    afe_ctrl_reg.REG = 0x0;



    afe_ctrl_reg.BIT.a0 = 1;
    afe_ctrl_reg.BIT.a1 = 1;
    afe_ctrl_reg.BIT.a2 = 1;
    afe_ctrl_reg.BIT.a3 = 1;

    afe_ctrl_reg.BIT.pwdn_glb = 0;
    afe_ctrl_reg.BIT.samples_ce = 1;

    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;
    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;

    xil_printf("\n\rAFE_CTRL_REG: 0x%08X\n\r",afe5808a_0[SPI_CTRL_REG_OFFSET]);

    afe5808a_basic_set_up(afe5808a_0);
/*
    afe_ctrl_reg.BIT.afe_rst = 0;

    afe5808a_0[SPI_CTRL_REG_OFFSET] = afe_ctrl_reg.REG;

    xil_printf("\n\rAFE_CTRL_REG: 0x%08X\n\r",afe5808a_0[SPI_CTRL_REG_OFFSET]);

    afe5808a_reset(afe5808a_0);
    for(i=0;i<1000000;i++);
*/

    afe5808a_0[4]=0xF5FAF5AA;
    afe5808a_select((u32)afe5808a_0,0x0f);  //selecciono 4 afes con cs
    afe5808a_spi_write((u32)afe5808a_0,22,1); // Deshabilitar demodulador

    afe5808a_select((u32)afe5808a_0,0x00);  //deselecciono 4 afes cs

	for(i=1; i<=8 ; i=i<<1)
	{
		afe5808a_select((u32)afe5808a_0,i);  //deselecciono 4 afes cs
		afe5808a_spi_read((u32)afe5808a_0,22,&value); // Deshabilitar demodulador
		xil_printf("Resultado AFE %X: %04X\r\n",i,(u32)value);
	}


	for(i=1; i<=8 ; i=i<<1)
	{
		afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
		xil_printf("CALIB_REG AFE %X: %04X\r\n",i,afe5808a_0[3]);
	}
 	diagrama_de_ojos((u32)afe5808a_0,&diagrama_ojos);
 	print_diagrama_de_ojos(&diagrama_ojos);
 	calc_delays(&diagrama_ojos,delays,tam_ojos);
 	xil_printf("Iniciando AFEs...\n\r");
 	afe5808a_complete_skew_align_delays((u32)afe5808a_0,delays);

	for(i=1; i<=8 ; i=i<<1)
	{
		afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
		xil_printf("CALIB_REG AFE %X: %04X\r\n",i,afe5808a_0[3]);
	}
	xil_printf("AFEs iniciados.\r\n");


	for(i=1; i<=8 ; i=i<<1)
	{
		afe5808a_select((u32)afe5808a_0,i);  //selecciono afe
		config_afe((u32)afe5808a_0);
		xil_printf("\r\n");
		print_current_afe_regs((u32)afe5808a_0);
	}
	for(i=1; i<=8 ; i=i<<1)
	{
		u16 dato_leido;
		afe5808a_select((u32)afe5808a_0,i);

		afe5808a_test_pattern(afe5808a_0,2);
		afe5808a_spi_read(afe5808a_0,2,&dato_leido);
		afe5808a_test_pattern(afe5808a_0,0);
		afe5808a_spi_read(afe5808a_0,2,&dato_leido);
		afe5808a_test_pattern(afe5808a_0,5);
		afe5808a_spi_read(afe5808a_0,2,&dato_leido);
	}
	//afe_set_dig_gain((u32)afe5808a_0,0xff);
	MCBCC_deactivate_emu();



    return 0;
}
