/*
 * afe5808a.c
 *
 *  Created on: 11/09/2014
 *      Author: csic
 */

#include "afe5808a.h"
#include "afe_bussar.h"
#include <stdio.h>


// CONFIGURACION INICIAL AFES:

/*********  Reg 51  *********/
#define PGA_24_N_30				0				//Programmable Gain Amplifier 24dB(x15.84) o 30dB(x31.62)
#define LFP_15_NONE_20_30_10    0  				//Low Pass Filter 10 15 20 30 MHz previo al ADC
#define PGA_INTEGRATOR_DISABLE	0  				//con 0 habilita el integrador del PGA
/*********  Reg 52  *********/
#define LNA_GAIN_18_24_12_NONE	2  				//Low Noise Amplifier 24dB(x15.84) 18dB(x7.94) 12dB(x3.98) support 250mVpp-1Vpp input signal
#define ACTIVE_TERMINATION_ENABLE	0			//1 Habilita terminación
#define PRESET_ACTIVE_TERM_50_100_200_400 0     //50,100,20,400 ohms si la deshabilitamos la terminación que impedancia tiene la entrada?
#define LNA_INTEGRATOR_DISABLED		0			//0 Habilita integrador
/*********  Reg 54  *********/
#define CW_SUM_AMP_GAIN_CNTL_R250_1 0
#define CW_SUM_AMP_GAIN_CNTL_R250_2	0
#define CW_SUM_AMP_GAIN_CNTL_R500	0
#define CW_SUM_AMP_GAIN_CNTL_R1000	0
#define CW_SUM_AMP_GAIN_CNTL_R2000	0
#define CW_16X_CLK_SEL_DIFF			0
#define CW_1X_CLK_SEL_CMOS			0
#define TGC_CW_SEL					0
#define CW_SUM_AMP_DISABLE			0
#define CW_CLK_MODE_SEL_16_8_4_1	0
/*********  Reg 59  *********/
#define HPF_LNA_100_50_200_150 0				//HPF_LNA 100,50,200,150
#define DIG_TGC_ATT		0						//0 disable digital TGC attenuator, 1 enable digital TGC attenuator
#define DIG_TGC_GAIN	3             			//3 bits 000 0dB att, 001 6dB att , N (1-7)=Nx6dB => 111 = 42dB
#define CW_SUM_AMP_PWUP 0
#define PGA_TEST_MODE	0

#define OLD_CALIB 0
Afe_enable = 0;

void afe_set_dig_gain(u32 baseaddr,u8 gain)
{
	u32 afe5808a_0 = (baseaddr);
	afe5808a_select(afe5808a_0,0xF);


	afe5808a_spi_write(afe5808a_0, 13, gain<<11); //Activo ganancia digital ch1
	afe5808a_spi_write(afe5808a_0, 15, gain<<11); //Activo ganancia digital ch2
	afe5808a_spi_write(afe5808a_0, 17, gain<<11); //Activo ganancia digital ch3
	afe5808a_spi_write(afe5808a_0, 17, gain<<11); //Activo ganancia digital ch4
	afe5808a_spi_write(afe5808a_0, 25, gain<<11); //Activo ganancia digital ch5
	afe5808a_spi_write(afe5808a_0, 27, gain<<11); //Activo ganancia digital ch6
	afe5808a_spi_write(afe5808a_0, 29, gain<<11); //Activo ganancia digital ch7
	afe5808a_spi_write(afe5808a_0, 31, gain<<11); //Activo ganancia digital ch8

	afe5808a_spi_write(afe5808a_0, 3, 1<<12); //Activo ganancia digital

}
void aplicar_retardos(u32 baseaddr,retardos_t *retardos_p)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u8 afe,channel,tap,n_test;
	u32 ret;
	for(afe=0;afe<N_AFES;afe++)
	{
		afe5808a_select(baseaddr,1<<afe);
		afe5808a_dly_rst(baseaddr);
	}
	for(afe=0;afe<N_AFES;afe++)
	{
		afe5808a_select(baseaddr,1<<afe);

		for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
		{
			for(tap=0;tap<retardos_p->taps[afe];tap++)
				ret=afe5808a_dly_inc(baseaddr, channel, 1);
		}
		xil_printf("Retardo aplicado a AFE %d = %d\r\n",afe,retardos_p->taps[afe]);
		//xil_printf("Retardo devuelto a AFE %d = %d\r\n\r\n",afe,ret);
	}
}
u8 afe5808a_dly_get(u32 baseaddr,u8 afe,u8 channel)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	return (u8) ((afe5808a_0[ALL_SAMPLES_OFFSET+channel+afe*N_CHANNELS_PER_AFE]>>21)&0x1F);
}
u8 afe5808a_dly_set(u32 baseaddr,u8 afe,u8 channel,u8 dly)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u32 register_value;
	register_value=afe5808a_0[ALL_SAMPLES_OFFSET+channel+afe*N_CHANNELS_PER_AFE];
	register_value&=0xFFE0FFFF;
	register_value|=dly<<16;
	afe5808a_0[ALL_SAMPLES_OFFSET+channel+afe*N_CHANNELS_PER_AFE]=register_value;
	afe5808a_dly_rst(baseaddr);
	return (u8) ((afe5808a_0[ALL_SAMPLES_OFFSET+channel+afe*N_CHANNELS_PER_AFE]>>21)&0x1F);
}

u8 afe5808a_dly_check(u32 baseaddr,u8 afe,u8 channel)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	return (u8) ((afe5808a_0[ALL_SAMPLES_OFFSET+channel+afe*N_CHANNELS_PER_AFE]>>21)&0x1F);
}

void diagrama_de_ojos(u32 baseaddr,diagrama_ojos_t *diagrama_ojos_p)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u8 afe,channel,tap,tap_real,tap_ini;
	u16 n_test;
	u16 value;
	u16 sample;
	u16 sample_first;


	for(afe=0;afe<N_AFES;afe++)
	{
		afe5808a_select(baseaddr,1<<afe);
		afe5808a_dly_rst(baseaddr);
		afe5808a_test_pattern(baseaddr,PAT_DESKEW);
		afe5808a_spi_read(baseaddr, 2, &value);

		for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
		{
			tap_ini=afe5808a_dly_check(baseaddr,afe,channel);
			tap=tap_ini;
			do
			{
				diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]=0;
				diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]=0;
				diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]=0;
				for(n_test=0;n_test<N_TESTS;n_test++)
				{
					sample_first = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
					sample 		 = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];

					if(sample==0x1555) diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]++;
					else if(sample==0x2AAA) diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]++;
					else diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]++;
					wait_for(5);
				}
				afe5808a_dly_inc(baseaddr, channel, 1);
				tap=(tap+1)%0x20;
				tap_real=afe5808a_dly_get(baseaddr, afe, channel);
				if(tap_real!=tap)
					xil_printf("\n\r---------------------DELAY INCORRECTO: afe %d, canal %d, tap REAL %2d, tap TEORICO %2d----------------\n\r",afe,channel,tap_real,tap);

			}while(tap!=tap_ini);
		}
	}
}

void calc_delays(diagrama_ojos_t *diagrama_ojos_p,u8* delays_p,u8* tam_ojo)
{
	u8 afe=0,channel=5,tap,n_test;

	for(afe=0;afe<N_AFES;afe++)
	{
		for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
		{
			u8 ini_ojo_actual=0,tam_ojo_actual=0,tam_ojo_candidato=0,ojo_1555=0,ojo_2AAA=0,centro_ojo_candidato=0;
			for(tap=0;tap<N_TAPS;tap++)
			{
				if(ojo_1555!=0)
				{
					if(
							diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]!=0 &&
							diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]==0 &&
							diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]==0
					)
					{
						tam_ojo_actual++;
					}
					else if(
							diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]==0 &&
							diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]!=0 &&
							diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]==0
					)
					{
						ojo_1555=0;
						ojo_2AAA=1;
						ini_ojo_actual=tap;
						tam_ojo_actual=0;
					}
				}
				else if(ojo_2AAA!=0)
				{
					if(
							diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]!=0 &&
							diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]==0 &&
							diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]==0
					)
					{
						ojo_1555=1;
						ojo_2AAA=0;
						ini_ojo_actual=tap;
						tam_ojo_actual=0;
					}
					else if(
							diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]==0 &&
							diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]!=0 &&
							diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]==0
					)
					{
						tam_ojo_actual++;
					}
				}
				else
				{
					if(
							diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]!=0 &&
							diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]==0 &&
							diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]==0
					)
					{
						ojo_1555=1;
						ojo_2AAA=0;
						ini_ojo_actual=tap;
						tam_ojo_actual=0;
					}
					else if(
							diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]==0 &&
							diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]!=0 &&
							diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]==0
					)
					{
						ojo_1555=0;
						ojo_2AAA=1;
						ini_ojo_actual=tap;
						tam_ojo_actual=0;
					}

				}
				if((ojo_1555 || ojo_2AAA)&&tam_ojo_actual>tam_ojo_candidato)
				{
					tam_ojo_candidato = tam_ojo_actual;
					centro_ojo_candidato = ini_ojo_actual+(tam_ojo_actual>>1);
				}
			}
			delays_p[afe*N_CHANNELS_PER_AFE+channel]=centro_ojo_candidato;
			tam_ojo[afe*N_CHANNELS_PER_AFE+channel]=tam_ojo_candidato;

		}
	}
}

void print_diagrama_de_ojos(diagrama_ojos_t *diagrama_ojos_p)
{
	u8 afe=0,channel=5,tap,n_test;

	for(afe=0;afe<N_AFES;afe++)
	{
		for(channel=0;channel<N_CHANNELS_PER_AFE;channel++)
		{

			xil_printf("AFE %d,canal %d:\r\nret: ",afe,channel);

			for(tap=0;tap<N_TAPS;tap++)
				xil_printf("%5d",tap);
			xil_printf("\r\n1555 ");

			for(tap=0;tap<N_TAPS;tap++)
				xil_printf("%5d",diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]);
			xil_printf("\r\n2AAA ");
			for(tap=0;tap<N_TAPS;tap++)
				xil_printf("%5d",diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]);
			xil_printf("\r\nOTRO ");
			for(tap=0;tap<N_TAPS;tap++)
				xil_printf("%5d",diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]);
			xil_printf("\r\n");

		}
	}
}


void wait_for(u32 delay)
{
	while(delay--);
}


void usleep(u32 delay)
{
	while(delay--);
}

void afe5808a_select(u32 baseaddr,u8 afe_select_byte)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u32 tmp;
	Afe_enable=afe_select_byte;
	tmp = afe5808a_0[AFE_CTRL_REG_OFFSET];
	afe5808a_0[AFE_CTRL_REG_OFFSET] = (tmp&0xfffff0ff)|(afe_select_byte<<8);
	wait_for(100);
}

void afe5808a_select0(u32 baseaddr,u8 afe_select_byte)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u32 tmp;
	tmp = afe5808a_0[AFE_CTRL_REG_OFFSET];
	afe5808a_0[AFE_CTRL_REG_OFFSET] = (tmp&0xfffff0ff)|(1<<8);
}

u32 spi_transfer(u32 baseaddr, u32 tx_reg)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u32 rx_reg,afe_ctrl_val;

	afe5808a_0[AFE_TX_REG_OFFSET] = tx_reg;
	wait_for(10);
	do{
		afe_ctrl_val=afe5808a_0[AFE_CTRL_REG_OFFSET];
	}while(!(afe_ctrl_val & 1<<16));
	rx_reg = afe5808a_0[AFE_RX_REG_OFFSET] & 0x0000ffff;
	wait_for(10);

	return (rx_reg);
}

u32 afe5808a_spi_write(u32 baseaddr, u8 addr, u16 data)
{
	u32 reg = ((((u32) (addr))<<16 | (u32) (data))) & (0x00ffffff);
	spi_transfer(baseaddr, reg);

	return 0;
}

u32 afe5808a_spi_read(u32 baseaddr, u8 addr, u16 * data)
{
	u32 reg = ((((u32) (addr))<<16) & 0x00ff0000);

	spi_transfer(baseaddr, REGISTER_READOUT_ENABLE);
	*data = (u16) spi_transfer(baseaddr, reg);
	spi_transfer(baseaddr, 0);

	return 0;
}

void config_afe(u32 baseaddr)
{
	u16 send_word,recv_word;

	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	//Se configuran todos los afes a la vez:
	//afe5808a_select(afe_baseaddress,0xf);
	/*********  Reg 51  *********/
	send_word = (PGA_24_N_30<<13)|(PGA_INTEGRATOR_DISABLE<<4)|(LFP_15_NONE_20_30_10<<1);
	afe5808a_spi_write(afe5808a_0, 51, send_word);
	afe5808a_spi_read(afe5808a_0, 51, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",51,recv_word);
	/*********  Reg 52  *********/
	send_word = (LNA_GAIN_18_24_12_NONE<<13)|(LNA_INTEGRATOR_DISABLED<<12)|(ACTIVE_TERMINATION_ENABLE<<8)|(PRESET_ACTIVE_TERM_50_100_200_400<<6);
	afe5808a_spi_write(afe5808a_0, 52, send_word);
	afe5808a_spi_read(afe5808a_0, 52, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",52,recv_word);
	/*********  Reg 54  *********/
	send_word = (CW_CLK_MODE_SEL_16_8_4_1<<10)|(CW_SUM_AMP_DISABLE<<9)|(TGC_CW_SEL<<8)|(CW_1X_CLK_SEL_CMOS<<6)|(CW_16X_CLK_SEL_DIFF<<5)|
				(CW_SUM_AMP_GAIN_CNTL_R2000<<4)|
				(CW_SUM_AMP_GAIN_CNTL_R1000<<3)|
				(CW_SUM_AMP_GAIN_CNTL_R500<<2)|
				(CW_SUM_AMP_GAIN_CNTL_R250_2<<1)|
				(CW_SUM_AMP_GAIN_CNTL_R250_1<<0);
	afe5808a_spi_write(afe5808a_0, 54, send_word);
	afe5808a_spi_read(afe5808a_0, 54, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",54,recv_word);
	/*********  Reg 59  *********/
	send_word = (DIG_TGC_ATT<<7)|(DIG_TGC_GAIN<<4)|(HPF_LNA_100_50_200_150<<2)|(CW_SUM_AMP_PWUP<<8)|(PGA_TEST_MODE<<9);
	afe5808a_spi_write(afe5808a_0, 59, send_word);
	afe5808a_spi_read(afe5808a_0, 59, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",59,recv_word);
}

void print_afe_regs(AFE_SPI_UT_t afe_SPI_regs)
{
	/*********  Reg 51  *********/
	xil_printf("REG %d: 0x%04X\r\n",51,afe_SPI_regs.reg51);
	/*********  Reg 52  *********/
	xil_printf("REG %d: 0x%04X\r\n",52,afe_SPI_regs.reg52);
	/*********  Reg 59  *********/
	xil_printf("REG %d: 0x%04X\r\n",59,afe_SPI_regs.reg59);
	/*********  Reg 61  *********/
	xil_printf("REG %d: 0x%04X\r\n",61,afe_SPI_regs.reg61);
}

void print_current_afe_regs(u32 baseaddr)
{
	u16 send_word,recv_word;

	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	//afe5808a_select(afe_baseaddress,0xf);
	/*********  Reg 51  *********/
	afe5808a_spi_read(afe5808a_0, 51, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",51,recv_word);
	/*********  Reg 52  *********/
	afe5808a_spi_read(afe5808a_0, 52, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",52,recv_word);
	/*********  Reg 59  *********/
	afe5808a_spi_read(afe5808a_0, 59, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",59,recv_word);
	/*********  Reg 61  *********/
	afe5808a_spi_read(afe5808a_0, 61, &recv_word);
	xil_printf("REG %d: 0x%04X\r\n",61,recv_word);
}

AFE_SPI_UT_t get_SPI_afe_regs(u32 baseaddr)
{
	AFE_SPI_UT_t returned_spi_regs;
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	afe5808a_spi_read(afe5808a_0, 51, &returned_spi_regs.reg51);
	afe5808a_spi_read(afe5808a_0, 52, &returned_spi_regs.reg52);
	afe5808a_spi_read(afe5808a_0, 59, &returned_spi_regs.reg59);
	afe5808a_spi_read(afe5808a_0, 61, &returned_spi_regs.reg61);

	return returned_spi_regs;
}

void afe5808a_reset(u32 baseaddr)
{
	afe5808a_spi_write(baseaddr, 0, 0x0001);
}

u32 afe5808a_ramp(u32 baseaddr)
{
	const u16 test_pattern = (7<<13);
	u16 value;

	// Ramp: Setting Register 2[15:13]=111 causes all the channels to output a repeating full-scale ramp pattern.
	// The ramp increments from zero code to full-scale code in steps of 1LSB every clock cycle. After hitting the
	// full-scale code, it returns back to zero code and ramps again.
	afe5808a_spi_write(baseaddr, 2, test_pattern);
	afe5808a_spi_read(baseaddr, 2, &value);
	if (value != test_pattern)
		return -1;

	//xil_printf("ramp test mode!\r\n");

	return 0;
}

u32 afe5808a_dly_rst(u32 baseaddr)
{
	hw_uint32_ptr adc = (hw_uint32_ptr) (baseaddr);

	adc[AFE_CALIB_REG_OFFSET] = 0x02;

	adc[AFE_CALIB_REG_OFFSET] = 0x00;

	wait_for(10000);
	//while((adc[AFE_CALIB_REG_OFFSET] & 0x01) != 0x01);

	return 0;
}



u32 afe5808a_dly_inc(u32 baseaddr, u32 channel, u32 dly_inc)
{
	static u32 dly_tap[8] = {0};
	u32 dly_ce;

	hw_uint32_ptr adc = (hw_uint32_ptr) (baseaddr);

	u32 calib_reg = 0;

	channel &= 0x07;

	dly_inc &= 0x01;
	dly_inc = dly_inc<<channel;
	dly_inc <<= 16;

	dly_ce = 0x01<<channel;
	dly_ce <<= 8;

	calib_reg = (dly_inc | dly_ce);

	adc[AFE_CALIB_REG_OFFSET] = calib_reg;

	if (dly_inc)
	{
		if (dly_tap[channel] == 31)	dly_tap[channel] = 0;
		else						dly_tap[channel] ++;
		//xil_printf("---->\r\n");
	}
	else
	{
		if (dly_tap[channel] == 0)	dly_tap[channel] = 31;
		else						dly_tap[channel] --;
		//xil_printf("<----\r\n");
	}
	wait_for(100);

	return dly_tap[channel];
}

u32 afe5808a_bitslip(u32 baseaddr, u32 channel)
{
	hw_uint32_ptr adc = (hw_uint32_ptr) (baseaddr);

	channel &= 0x07;

	u32 bitslip = 0x01;

	bitslip <<= channel;
	bitslip <<= 24;

	adc[AFE_CALIB_REG_OFFSET] = bitslip;
	wait_for(10000);
	//adc[AFE_CALIB_REG_OFFSET] = 0;
	wait_for(100);
	return 0;
}
u32 afe5808a_bitslip_all(u32 baseaddr)
{
	hw_uint32_ptr adc = (hw_uint32_ptr) (baseaddr);

	u32 bitslip = 0xFF;

	bitslip <<= 24;

	adc[AFE_CALIB_REG_OFFSET] = bitslip;
	adc[AFE_CALIB_REG_OFFSET] = 0;
	return 0;
}
u32 look_for_estable(u32 baseaddr, u8 channel, u16 * data)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	const u8 n_dly_tap = 32;
	u8 dly_tap = 0;

	const u8 n_tap_vld = 13;
	u8 tap_vld = 0;

	const u8 n_equal_for_vld = 100;
	u8 equal_for_vld = 0;

	const u16 sample_vld[2] = {0x2aaa, 0x1555};
	u16 candidate = 0;

	bool estable = false;

	const u16 sample_mask = (1<<14)-1;
	u16 sample = 0;

	//xil_printf("Buscando zona estable\r\n");

	while(!estable)
	{
		sample = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
		sample &= sample_mask;

		//if (sample == sample_vld[0] || sample == sample_vld[1])
		{
			if (tap_vld == 0)
				candidate = sample;
		}
		tap_vld++;
		for(equal_for_vld=0;equal_for_vld<n_equal_for_vld;equal_for_vld++)
		{
			sample = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
			sample &= sample_mask;
			if (sample != candidate)
			{
				tap_vld = 0;
				break;
			}
			wait_for(10);
		}

		if(tap_vld == n_tap_vld)
			estable = true;
		else
			dly_tap = afe5808a_dly_inc(baseaddr, channel, 1);

		//xil_printf("sample = 0x%04x\r\n", sample);
		//xil_printf("candidate = 0x%04x\r\n", candidate);
	}

	*data = candidate;
	return (dly_tap < n_tap_vld ? dly_tap + n_dly_tap - n_tap_vld : dly_tap - n_tap_vld);
}

u32 look_for_metaestable(u32 baseaddr, u8 channel, u16 data)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	const u8 n_dly_tap = 32;
	u8 dly_tap = 0;

	bool metaestable = false;

	const u16 sample_mask = (1<<14)-1;
	u16 sample = 0;

	//xil_printf("Buscando metaestabilidades\r\n");

	while(!metaestable)
	{
		sample = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
		sample &= sample_mask;

		if (sample == data)
			dly_tap = afe5808a_dly_inc(baseaddr, channel, 1);
		else
			metaestable = true;

		wait_for(10);

		//xil_printf("sample = 0x%04x\r\n", sample);
	}

	return (dly_tap == 0 ? n_dly_tap - 1 : dly_tap - 1);
}

u32 afe5808a_calib(u32 baseaddr)
{
	const u16 test_pattern = (1<<14);
	u16 value;

	//hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	// Set AFE in Deskew training mode.

	// Deskew Patten: When 2[15:13]=010; this mode replaces the 14-bit ADC output with the 01010101010101 word.
	afe5808a_spi_write(baseaddr, 2, test_pattern);
	afe5808a_spi_read(baseaddr, 2, &value);
	xil_printf("test_pattern = %04X\r\nvalue        = %04x\r\n",test_pattern,value);
	if (value != test_pattern)
		return -1;

	const u8 n_dly_tap = 32;
	u8 dly_tap;
	u8 t0, t1;
	u8 t_gap;
	u8 i;

	const u8 n_channel = 8;
	u8 channel;

	afe5808a_dly_rst(baseaddr);

	for(channel = 0; channel < n_channel; channel++)
	{
		//xil_printf("calibrating channel %d\r\n", channel);

		dly_tap = look_for_estable(baseaddr, channel, &value);

		//xil_printf("dato estable [0x%04x] at %d\r\n", value, dly_tap);

		dly_tap = look_for_metaestable(baseaddr, channel, value);

		//xil_printf("metaestabilidad encontrada at %d\r\n", dly_tap);

		t0 = look_for_estable(baseaddr, channel, &value);

		//xil_printf("dato estable [0x%04x] at %d\r\n", value, t0);

		t1 = look_for_metaestable(baseaddr, channel, value);

		//xil_printf("metaestabilidad encontrada at %d\r\n", t1);

		t_gap = (t1 > t0 ? t1 - t0 : t1 + n_dly_tap - t0);

		//xil_printf("efective bit: %d\r\n", t_gap);

		for(i = 0; i<t_gap/2; i++)
			afe5808a_dly_inc(baseaddr, channel, 0);

		xil_printf("calibration done: CHANNEL %d, delay = %d\r\n",channel,t_gap/2);
	}



	return 0;
}

u32 afe5808a_calib_fixed(u32 baseaddr,u8 num_inc)
{
	int channel,i;

	afe5808a_dly_rst(baseaddr);
	for(channel = 0; channel < 8; channel++)
	{
		for(i = 0; i<num_inc; i++)
			afe5808a_dly_inc(baseaddr, channel, 1);
	}


}

u32 afe5808a_calib_channel(u32 baseaddr,u8 num_inc,u8 channel)
{
	int i;

	for(i = 0; i<num_inc; i++)
		afe5808a_dly_inc(baseaddr, channel, 1);
}

u32 afe5808a_calib_print(u32 baseaddr)
{
	const u16 test_pattern = (2<<13);
	u16 value;

	//hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	// Set AFE in Deskew training mode.

	// Deskew Patten: When 2[15:13]=010; this mode replaces the 14-bit ADC output with the 01010101010101 word.
	afe5808a_spi_write(baseaddr, 2, test_pattern);
	afe5808a_spi_read(baseaddr, 2, &value);
	xil_printf("test_pattern = %04X\r\nvalue        = %04x\r\n",test_pattern,value);
	if (value != test_pattern)
		return -1;

	const u8 n_dly_tap = 32;
	u8 dly_tap;
	u8 t0, t1;
	u8 t_gap;
	u8 i;

	const u8 n_channel = 8;
	u8 channel;

	for(channel = 0; channel < n_channel; channel++)
	{
		xil_printf("calibrating channel %d\r\n", channel);

		dly_tap = look_for_estable(baseaddr, channel, &value);

		xil_printf("dato estable [0x%04x] at %d\r\n", value, dly_tap);

		dly_tap = look_for_metaestable(baseaddr, channel, value);

		xil_printf("metaestabilidad encontrada at %d\r\n", dly_tap);

		t0 = look_for_estable(baseaddr, channel, &value);

		xil_printf("dato estable [0x%04x] at %d\r\n", value, t0);

		t1 = look_for_metaestable(baseaddr, channel, value);

		xil_printf("metaestabilidad encontrada at %d\r\n", t1);

		t_gap = (t1 > t0 ? t1 - t0 : t1 + n_dly_tap - t0);

		xil_printf("efective bit: %d\r\n", t_gap);

		for(i = 0; i<t_gap/2; i++)
			afe5808a_dly_inc(baseaddr, channel, 0);
	}

	xil_printf("calibration done!\r\n");

	return 0;
}

u8 num_ones(u16 num)
{
	int ones=0;
	while(num!=0)
	{
		if(num&0x1) ones++;
		num>>=1;
	}
	return ones;
}

void afe5808a_ojos_1_canal(u32 baseaddr,u8 afe,u8 channel,diagrama_ojos_t* diagrama_ojos_p)
{
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u8 tap_ini,tap,n_test,tap_real;

	u16 sample_first,sample;

	tap_ini=0;
	tap=tap_ini;
	tap_real=afe5808a_dly_set(baseaddr, afe, channel,tap);

	if(tap_real!=0)
		xil_printf("\n\r---------------------DELAY INCORRECTO: afe %d, canal %d, tap REAL %2d, tap TEORICO %2d----------------\n\r",afe,channel,tap_real,tap);


	do
	{
		diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]=0;
		diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]=0;
		diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]=0;
		for(n_test=0;n_test<N_TESTS;n_test++)
		{
			sample_first = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
			sample 		 = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];

			if(sample==0x1555) diagrama_ojos_p->res_1555[afe*N_CHANNELS_PER_AFE+channel][tap]++;
			else if(sample==0x2AAA) diagrama_ojos_p->res_2AAA[afe*N_CHANNELS_PER_AFE+channel][tap]++;
			else diagrama_ojos_p->res_otro[afe*N_CHANNELS_PER_AFE+channel][tap]++;
			wait_for(5);
		}
		afe5808a_dly_inc(baseaddr, channel, 1);
		tap=(tap+1)%0x20;
		tap_real=afe5808a_dly_get(baseaddr, afe, channel);
		if(tap_real!=tap)
			xil_printf("\n\r---------------------DELAY INCORRECTO: afe %d, canal %d, tap REAL %2d, tap TEORICO %2d----------------\n\r",afe,channel,tap_real,tap);

	}while(tap!=tap_ini);

}


int test_7_ones(u32 baseaddr,u8 afe)
{

	volatile u16 align_sample;
	const u8 n_channel = 8;
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	int ret = 0;
	u8 channel;

	for(channel = 0; channel < n_channel; channel++)
	{
		//xil_printf("aligning channel %d\r\n", channel);
		align_sample = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
		align_sample &= (1<<14)-1;

		if(num_ones(align_sample)!=7)
			xil_printf("Channel = %d, align_sample = 0x%08x, CHANNEL DESKEW ERROR, NUM ONES = %2d\r\n",channel, align_sample,num_ones(align_sample));
		else
			xil_printf("Channel = %d, align_sample = 0x%08x, NUM ONES = %2d\r\n",channel, align_sample,num_ones(align_sample));

		wait_for(100);
	}

	xil_printf("alignation done!\r\n\r\n\r\n");

	return ret;
}


int afe5808a_align(u32 baseaddr,u8 afe)
{
#define swap7(x) (((x & 0x3f80) >> 7) | ((x & 0x007f) << 7))
#define rol14(x) (((x << 1) & 0x3ffe) | ((x >> 13) & 0x0001))

	const u16 align_pattern = 0x3F80;
	volatile u16 align_sample;
	bool aligned;
	const u8 n_channel = 8;
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	int deskew_errors;
	int ret = 0;
	u8 channel;
	u8 bitslips_dados;

	// Sync Pattern: When 2[15:13]=001, the normal ADC output is replaced by a fixed 11111110000000 word.
	afe5808a_test_pattern(baseaddr,1);

	for(channel = 0; channel < n_channel; channel++)
	{

		bitslips_dados=0;
		aligned = false;
		deskew_errors =0;
		while(1)
		{
			align_sample = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
			align_sample &= (1<<14)-1;

			if(num_ones(align_sample)==0 || num_ones(align_sample)==14)
			{
				xil_printf("Channel = %d, align_sample = 0x%08x, bitslips_dados = %2d CHANNEL DESKEW ERROR = %d, NUM ONES = %2d\r\n",channel, align_sample,bitslips_dados,deskew_errors,num_ones(align_sample));
				break;
			}

			if(num_ones(align_sample)!=7)
			{
				//xil_printf("Channel = %d, align_sample = 0x%08x, bitslips_dados = %2d CHANNEL DESKEW ERROR, NUM ONES = %2d\r\n",channel, align_sample,bitslips_dados,num_ones(align_sample));
				deskew_errors++;
				afe5808a_bitslip(baseaddr, channel);
				bitslips_dados++;
				wait_for(10000);
				if(deskew_errors>50)
				{
					xil_printf("Channel = %d, align_sample = 0x%08x, bitslips_dados = %2d CHANNEL DESKEW ERROR = %d, NUM ONES = %2d\r\n",channel, align_sample,bitslips_dados,deskew_errors,num_ones(align_sample));
					break;
				}
				else
					continue;
			}
			if(align_sample != align_pattern)
			{
				afe5808a_bitslip(baseaddr, channel);
				bitslips_dados++;
				wait_for(100);
			}
			else
			{
				aligned = true;
				//xil_printf("Channel = %d, align_sample = 0x%08x, bitslips_dados = %2d     ALIGNED!\r\n",channel, align_sample,bitslips_dados);
				break;
			}
		}
	}

	xil_printf("alignation done!\r\n\r\n\r\n");

	return ret;
}


u32 afe5808a_align_mod(u32 baseaddr)
{
#define swap7(x) (((x & 0x3f80) >> 7) | ((x & 0x007f) << 7))
#define rol14(x) (((x << 1) & 0x3ffe) | ((x >> 13) & 0x0001))

	const u16 test_pattern = (1<<13);
	const u16 align_pattern = 0x3f80;
	u16 value;
	volatile u16 align_sample;
	bool aligned;
	const u8 n_channel = 8;
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);

	// Sync Pattern: When 2[15:13]=001, the normal ADC output is replaced by a fixed 11111110000000 word.
	afe5808a_spi_write(baseaddr, 2, test_pattern);
	afe5808a_spi_read(baseaddr, 2, &value);
	if (value != test_pattern)
		return -1;


	u8 channel=0;


	//xil_printf("align_pattern = 0x%04x\r\n", align_pattern);

	//for(channel = 0; channel < n_channel; channel++)
	{
		//xil_printf("aligning channel %d\r\n", channel);

		aligned = false;
		while(!aligned)
		{
			align_sample = afe5808a_0[AFE_SAMPLE_0_REG_OFFSET+channel];
			align_sample &= (1<<14)-1;

			if(align_sample != align_pattern)
				afe5808a_bitslip_all(baseaddr);

			else
				aligned = true;
				//xil_printf("Channel = %d, align_sample = 0x%08x\r\n",channel, align_sample);

			wait_for(1000);
		}
	}

	//xil_printf("alignation done!\r\n");

	return 0;
}

int afe5808a_delay_samples(u32 baseaddr)
{
	u16 all_ch_samples[32];
	u16 sample_min,sample_max,sample;
	bool aligned;
	hw_uint32_ptr afe5808a_0 = (hw_uint32_ptr) (baseaddr);
	u8 print;
	u8 ch;
	u32 reg_val;
	int ret;

	print=1;

	afe5808a_select(baseaddr,0xF);
	afe5808a_ramp(baseaddr);
	afe5808a_spi_write((u32)baseaddr,10,0x10); // Sincroniza rampa

	wait_for(1000);

	afe5808a_0[AFE_CTRL_REG_OFFSET] |=  (1<<25); //RSC Register Samples clock Enable = 1
	afe5808a_0[AFE_CTRL_REG_OFFSET] &= ~(1<<25); //RSC Register Samples clock Enable = 0

	sample=(u16)afe5808a_0[ALL_SAMPLES_OFFSET]&0xFFFF;
	sample_min=sample;
	sample_max=sample;
	for(ch=0;ch<32;ch++)
	{
		all_ch_samples[ch]=(u16)(afe5808a_0[ALL_SAMPLES_OFFSET+ch]&0xFFFF);
		if(all_ch_samples[ch]==0x3FFF||all_ch_samples[ch]==0x0000) continue;
		if(all_ch_samples[ch]>sample_max) sample_max=all_ch_samples[ch];
		if(all_ch_samples[ch]<sample_min) sample_min=all_ch_samples[ch];
	}
	xil_printf("sample_max: %d\n\r",sample_max);
	xil_printf("sample_min: %d\n\r",sample_min);
	if((sample_max-sample_min)>1)
	{
		xil_printf("¡¡¡¡¡¡¡¡Ajuste de retardos mayor que 1!!!!!\n\r");
		ret=-1;
	}
	else
		ret=0;

/*
  	for(ch=0;ch<32;ch++)
	{
		if(all_ch_samples[ch]==0x3FFF||all_ch_samples[ch]==0x0000) continue;
		reg_val = afe5808a_0[ALL_SAMPLES_OFFSET+ch]&0x0FFFFFFF;

		afe5808a_0[ALL_SAMPLES_OFFSET+ch]=((u32)(all_ch_samples[ch]-sample_min))<<28|reg_val;
	}
	*/


	afe5808a_0[AFE_CTRL_REG_OFFSET] |=  (1<<25); //RSC Register Samples clock Enable = 1
	afe5808a_0[AFE_CTRL_REG_OFFSET] &= ~(1<<25); //RSC Register Samples clock Enable = 0

	if(print)
	{
		u8 afe,adc;
		ch=0;
		xil_printf("\r\nR, OD, ID,  VAL\n\r");
		for(afe=0;afe<4;afe++)
		{
			for(adc=0;adc<8;adc++,ch++)
			{
				xil_printf("%d, %2d, %2d, %04X\n\r",(afe5808a_0[ALL_SAMPLES_OFFSET+ch]>>28)&0xF,(afe5808a_0[ALL_SAMPLES_OFFSET+ch]>>21)&0x1F,(afe5808a_0[ALL_SAMPLES_OFFSET+ch]>>16)&0x1F,afe5808a_0[ALL_SAMPLES_OFFSET+ch]&0xFFFF);
			}
			xil_printf("\r\n");
		}
	}

	return ret;

}
u32 afe5808a_enable(u32 baseaddr)
{
	const u16 test_pattern = (0<<13);
	u16 value;

	// Normal operation.
	afe5808a_spi_write(baseaddr, 2, test_pattern);
	//afe5808a_spi_read(baseaddr, 2, &value);
	//if (value != test_pattern) return -1;

	return 0;
}

u32 afe5808a_test_pattern(u32 baseaddr,u8 test)
{
	/*
		000: Normal operation;
		001: Sync;
		010: De-skew;
		011: Custom;
		100: All 1's;
		101: Toggle;
		110: All 0's;
		111: Ramp
	*/
	u16 test_pattern = (test<<13);
	u16 value;

	// Normal operation.
	afe5808a_spi_write(baseaddr, 2, test_pattern);

	return 0;
}
void afe5808a_basic_set_up(u32 baseaddr)
{
	u8 afe_index,channel_index;
	volatile u32 *aferegs;
	aferegs=(u32*)baseaddr;
	aferegs[AFE_CTRL_REG_OFFSET]&=0xFFFF0FFF;

	aferegs[AFE_CTRL_REG_OFFSET]|= (1<<25);

	aferegs[AFE_CTRL_REG_OFFSET]|= (1<<15);
	wait_for(10000);
	aferegs[AFE_CTRL_REG_OFFSET]&=~(1<<15);
	wait_for(10000);
//
//	aferegs[AFE_CTRL_REG_OFFSET]|= (1<<27);
//	wait_for(10000000);
//	aferegs[AFE_CTRL_REG_OFFSET]&=~(1<<27);
//	wait_for(10000000);

	aferegs[AFE_CTRL_REG_OFFSET]|= (1<<26);
	wait_for(100000);
	aferegs[AFE_CTRL_REG_OFFSET]&=~(1<<26);
	wait_for(100000);

	//afe5808a_select(baseaddr,0xf);  //Reset a los AFES
	//afe5808a_spi_write(baseaddr,0,1); // RESET AFE OBLIGATORIO
	//afe5808a_spi_write((u32)baseaddr,10,0x10); // Sincroniza PATRONES DE TEST ENTRE CANALES

	for(afe_index=0;afe_index<N_AFES;afe_index++)	//cambia el sentido de bits de MSB-LSB a LSB-MSB
	{
		short sample;
		afe5808a_select(baseaddr,1<<afe_index);

		afe5808a_test_pattern(baseaddr,4);//100: All 1's Se pone el AFE en modo "siempre unos"
		wait_for(10000);

		sample=0;
		for(channel_index=3;channel_index<4;channel_index++)
			sample|=aferegs[AFE_SAMPLE_0_REG_OFFSET+channel_index]; //Se hace or de todas los canales

		if(sample==0)//Si todas las muestras son cero... Es que es el 5809 y no está desactivado el demodulador, así que lo desactivamos
		    afe5808a_spi_write(baseaddr,22,1); // Deshabilitar demodulador
		else
			xil_printf("5808\n\r");

		wait_for(1000);
	}

	afe5808a_select(baseaddr,0xF);
	afe5808a_spi_write(baseaddr, 4, 1<<4);
	wait_for(100);

	if(0)
	{//test de lectura a varios AFES
		u16 dato;
		afe5808a_select(baseaddr,0xF);
		afe5808a_spi_read(baseaddr,4,&dato);
		xil_printf("\n\rTest de lectura:\n\rREGISTO 4: %d\n\r",dato);
	}


}

int afe5808a_complete_skew_align(u32 baseaddr)
{
	u8 afe;
	u8 ini=0,fin=4;
	volatile u32 *aferegs;
	aferegs=(u32*)baseaddr;
	aferegs[AFE_CTRL_REG_OFFSET]&=0xFFFF0FFF;

	aferegs[AFE_CTRL_REG_OFFSET]|= (1<<15);
	wait_for(1000);
	aferegs[AFE_CTRL_REG_OFFSET]&=~(1<<15);
	wait_for(1000);


	aferegs[AFE_CTRL_REG_OFFSET]|= (1<<26);
	wait_for(1000);
	aferegs[AFE_CTRL_REG_OFFSET]&=~(1<<26);
	wait_for(1000);

	//afe5808a_select(baseaddr,0xf);  //Reset a los AFES
	//afe5808a_spi_write(baseaddr,0,1); // RESET AFE OBLIGATORIO
	//afe5808a_spi_write((u32)baseaddr,10,0x10); // Sincroniza PATRONES DE TEST ENTRE CANALES


	afe5808a_select(baseaddr,0xf);  //selecciono 4 afes con cs
    afe5808a_spi_write(baseaddr,22,1); // Deshabilitar demodulador

	for(afe=ini;afe<fin;afe++)	//cambia el sentido de bits de MSB-LSB a LSB-MSB
	{
		afe5808a_select(baseaddr,1<<afe);
		afe5808a_spi_write(baseaddr, 4, 1<<4);
		wait_for(100);
	}


	for(afe=ini;afe<fin;afe++)
	{
		int delay[]=P_DELAYS_APLICADOS;
		afe5808a_select(baseaddr,1<<afe);
		xil_printf("\r\nAFE: %d, IDELAY: %d\r\n",afe+1,delay[afe]);
#if CALIB_AFES_DYN
		afe5808a_calib(baseaddr);
#else
		afe5808a_calib_fixed(baseaddr,delay[afe]);
#endif

		wait_for(100);
	}

	for(afe=ini;afe<fin;afe++)
	{
		afe5808a_select(baseaddr,1<<afe);
		afe5808a_align(baseaddr,afe);
		wait_for(100);
	}


	//afe5808a_delay_samples(baseaddr);

/*
  	for(i=ini;i<fin;i++)
	{
		afe5808a_select(baseaddr,1<<i);
		afe5808a_spi_write(baseaddr, 4, 4);
		wait_for(100);
	}
	*/
	afe5808a_enable(baseaddr);


	//////////////////////PRUEBA///////////////
}
int delays_test_7_ones(u32 baseaddr,u8 *delays)
{
	int i,ret=0;
	u8 ini=0,fin=4,afe,channel,delay;
	volatile u32 *aferegs;

	aferegs=(u32*)baseaddr;
	for(i=ini;i<fin;i++)
	{
		afe5808a_select(baseaddr,1<<i);
		//afe5808a_config_sys(baseaddr);
		afe5808a_test_pattern(baseaddr,1);

		wait_for(100);
	}

	{
		int ch=0;
		for(afe=0;afe<N_AFES;afe++)
			for(channel=0;channel<8;channel++,ch++)
				aferegs[ALL_SAMPLES_OFFSET+ch]=(delays[ch]<<16);
		afe5808a_select(baseaddr,0xF);
		afe5808a_dly_rst(baseaddr);
	}


	for(afe=ini;afe<fin;afe++)
	{
		afe5808a_select(baseaddr,1<<afe);
		//xil_printf("\r\nAFE: %d\r\n",i+1);
		if(test_7_ones(baseaddr,afe))
			ret = -1;
		//afe5808a_align_mod(baseaddr);
		wait_for(100);
	}


	//afe5808a_delay_samples(baseaddr);


}


int afe5808a_complete_skew_align_delays(u32 baseaddr,u8 *delays)
{
	int i,ret=0;
	u8 ini=0,fin=4,afe,channel,delay;
	volatile u32 *aferegs;

	aferegs=(u32*)baseaddr;


	if(1)
	{
		int ch=0;
		for(afe=0;afe<N_AFES;afe++)
			for(channel=0;channel<8;channel++,ch++)
				aferegs[ALL_SAMPLES_OFFSET+ch]=(delays[ch]<<16);
		afe5808a_select(baseaddr,0xF);
		afe5808a_dly_rst(baseaddr);

	}

	for(afe=ini;afe<fin;afe++)
	{
		afe5808a_select(baseaddr,1<<afe);
		if(afe5808a_align(baseaddr,afe))
			ret = -1;
		wait_for(100);
	}

	//afe5808a_delay_samples(baseaddr);

/*
  	for(i=ini;i<fin;i++)
	{
		afe5808a_select(baseaddr,1<<i);
		afe5808a_spi_write(baseaddr, 4, 4);
		wait_for(100);
	}
	*/
	afe5808a_enable(baseaddr);

	return ret;

	//////////////////////PRUEBA///////////////
}
int afe5808a_complete_skew_align_delays_ANTIGUA(u32 baseaddr,u8 *delays)
{
	int i,ret=0;
	u8 ini=0,fin=4,afe,channel,delay;
	volatile u32 *aferegs;

	aferegs=(u32*)baseaddr;
	//aferegs[AFE_CTRL_REG_OFFSET]&=0xFFFF0FFF;
	if(0)
	{
		aferegs[AFE_CTRL_REG_OFFSET]|= (1<<15);
		wait_for(1000);
		aferegs[AFE_CTRL_REG_OFFSET]&=~(1<<15);
		wait_for(1000);


		aferegs[AFE_CTRL_REG_OFFSET]|= (1<<26);
		wait_for(1000);
		aferegs[AFE_CTRL_REG_OFFSET]&=~(1<<26);
		wait_for(1000);

		//afe5808a_select(baseaddr,0xf);  //Reset a los AFES
		//afe5808a_spi_write(baseaddr,0,1); // RESET AFE OBLIGATORIO
		//afe5808a_spi_write((u32)baseaddr,10,0x10); // Sincroniza PATRONES DE TEST ENTRE CANALES


		afe5808a_select(baseaddr,0xf);  //selecciono 4 afes con cs
		afe5808a_spi_write(baseaddr,22,1); // Deshabilitar demodulador
	}
//	for(i=ini;i<fin;i++)	//cambia el sentido de bits de MSB-LSB a LSB-MSB
//	{
//		afe5808a_select(baseaddr,1<<i);
//		afe5808a_spi_write(baseaddr, 4, 1<<4);
//		wait_for(100);
//	}
	for(i=ini;i<fin;i++)
	{
		afe5808a_select(baseaddr,1<<i);
		//afe5808a_config_sys(baseaddr);
		afe5808a_test_pattern(baseaddr,1);

		wait_for(100);
	}

	{
		int ch=0;
		for(afe=0;afe<N_AFES;afe++)
			for(channel=0;channel<8;channel++,ch++)
				aferegs[ALL_SAMPLES_OFFSET+ch]=(delays[ch]<<16);
		afe5808a_select(baseaddr,0xF);
		afe5808a_dly_rst(baseaddr);

	}


	for(afe=ini;afe<fin;afe++)
	{
		afe5808a_select(baseaddr,1<<afe);
		//xil_printf("\r\nAFE: %d\r\n",i+1);
		if(afe5808a_align(baseaddr,afe))
			ret = -1;
		//afe5808a_align_mod(baseaddr);
		wait_for(100);
	}
	for(i=ini;i<fin;i++)
	{
		afe5808a_select(baseaddr,1<<i);
		//afe5808a_config_sys(baseaddr);
		afe5808a_test_pattern(baseaddr,2);

		wait_for(100);
	}

	afe5808a_delay_samples(baseaddr);

/*
  	for(i=ini;i<fin;i++)
	{
		afe5808a_select(baseaddr,1<<i);
		afe5808a_spi_write(baseaddr, 4, 4);
		wait_for(100);
	}
	*/
	afe5808a_enable(baseaddr);

	return ret;

	//////////////////////PRUEBA///////////////
}

