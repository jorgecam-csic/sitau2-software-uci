/*
 * afe_bussar.h
 *
 *  Created on: 21 mar. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_AFE_AFE_BUSSAR_H_
#define SRC_SITAU2_AFE_AFE_BUSSAR_H_

#pragma once

#include "xil_types.h"
#include "bcc_bussar.h"
#include "bussar_addr.h"

//SWT	NBY	ETM	STP	 	 	 	RXV	 	 	ACS	AA	NBY		IE	TXR	 	 	 	 	AE3	AE2	AE1	AE0	DEC_RATIO
#define SPI_CTRL_REG_OFFSET		0
#define TX_REG_OFFSET			1
#define RX_REG_OFFSET			2
#define CALIB_REG_OFFSET		3
#define ACQ_REG_OFFSET			4
#define INT_ENABLE_REG_OFFSET	5
#define INT_FLAG_REG_OFFSET		6
#define WATER_DELAY_REG_OFFSET	7
#define PROEMI_REG_OFFSET		16
typedef union ctrl_afe_u
{
	u32 REG;
	struct
	{
		u32 dec_ratio : 8;
		u32 a0 : 1;
		u32 a1 : 1;
		u32 a2 : 1;
		u32 a3 : 1;
		u32 pwdn_adc : 1;
		u32 pwdn_vco : 1;
		u32 pwdn_glb : 1;
		u32 afe_rst : 1;
		u32 transmission_ready : 1;
		u32 reserved2 : 7;
		u32 receive_valid : 1;
		u32 samples_ce : 1;
		u32 reset_serdes : 1;
		u32 afe_clk_disable : 1;
		u32 stop : 1;
		u32 external_trigger : 1;
		u32 not_busy : 1;
		u32 software_trigger : 1;
	} BIT;
}ctrl_afe_t;


typedef struct AFE_BUSSAR_regs_s
{
	ctrl_afe_t ctrl_afe;
	u32 tx_reg;
	u32 rx_reg;
	u32 calib_dont_use;
	u32 num_samples;
	u32 interrupts_enable_dont_use;
	u32 interrupts_flags_dont_use;
	u32 water_delay;
	u32 sample_ch0;
	u32 sample_ch1;
	u32 sample_ch2;
	u32 sample_ch3;
	u32 sample_ch4;
	u32 sample_ch5;
	u32 sample_ch6;
	u32 sample_ch7;
}AFE_BUSSAR_regs_t;

typedef struct AFE_BUSSAR_UT_s
{
	u8 external_trigger;
	u8 dec_ratio;
	u32 num_samples;
	u32 water_delay;
	u8 log2_promediados;
}AFE_BUSSAR_UT_t;

typedef union AFE_SPI_reg21_u
{
	u16 REG;
	struct
	{
		u16 DIGITAL_HPF_FILTER_ENABLE_1_4		: 1;	// Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 1 to 4 channels
		u16 DIGITAL_HPF_FILTER_K_CH_1_4			: 4;	// Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.
		u16 RESERVED0							: 11;	// Bit[15:5]: 
	} BIT;
}AFE_SPI_reg21_t;

typedef union AFE_SPI_reg33_u
{
	u16 REG;
	struct
	{
		u16 DIGITAL_HPF_FILTER_ENABLE_5_8		: 1;	// Bit[0]: 0: Disable the digital HPF filter; 1: Enable for 5 to 8 channels
		u16 DIGITAL_HPF_FILTER_K_CH_5_8			: 4;	// Bit[4:1]: Set K for the HPF (k from 2 to 10, that is 0010B to 1010B). This group of four registers controls the characteristics of a digital high-pass transfer function applied to the output data.
		u16 RESERVED0							: 11;	// Bit[15:5]: 
	} BIT;
}AFE_SPI_reg33_t;

//Note: This HPF feature is only available when the demodulation block is disabled.


typedef union AFE_SPI_reg51_u
{
	u16 REG;
	struct
	{
		u16 RESERVED0 				: 1;	// Bit[0]: no hace nada
		u16 LPF_PROGRAMMABILITY 	: 3;	// Bit[3:1]: 000: 15 MHz,010: 20 MHz,011: 30 MHz,100: 10 MHz
		u16 PGA_INTEGRATOR_DISABLE	: 1;	// Bit[4]: 1: Disables offset integrator for PGA. Mirar datasheet.
		u16 PGA_CLAMP_LEVEL			: 3;	// Bit[7:5]: In normal operation, clamp function can be set as 000 in the low noise mode. Para cualquier otra cosa, mirar datasheet.
		u16 RESERVED1				: 5;	// Bit[12:8]: no hace nada
		u16 PGA_GAIN_CONTROL		: 1;	// Bit[13]: 0:24 dB; 1:30 dB
		u16 RESERVED2				: 2;	// Bit[15:14]: no hace nada
	} BIT;
}AFE_SPI_reg51_t;

typedef union AFE_SPI_reg52_u
{
	u16 REG;
	struct
	{
		u16 ACTIVE_TERMINATION 			: 5;	// Bit[4:0]:
		u16 ACT_TER_ENA_CTRL			: 1;	// Bit[5]: 1: Enable internal active termination individual resistor control
		u16 PRESET_ACTIVE_TERMINATIONS	: 2;	// Bit[7:6]: 00: 50 Ω; 01: 100 Ω; 10: 200 Ω; 11: 400 Ω	Note: The device adjusts resistor mapping (52[4:0]) automatically. 50-Ω active termination is not supported in 12-dB LNA setting. Instead, 00 represents high-impedance mode when LNA gain is 12 dB.
		u16 ACTIVE_TERMINATION_ENABLE 	: 1;	// Bit[8]: Enable active termination
		u16 LNA_INPUT_CLAMP_SETTING		: 2;	// Bit[10:9]: 00: Auto setting; 01: 1.5 Vpp; 10: 1.15 Vpp; 11: 0.6 Vpp
		u16 RESERVED0					: 1;	// Bit[11]: Set to zero.
		u16 LNA_INTEGRATOR_DISABLE		: 1;	// Bit[12]: Disable offset integrator for LNA. Mirar datasheet
		u16 LNA_GAIN					: 2;	// Bit[14:13]: 00: 18 dB; 01: 24 dB; 10: 12 dB; 11: Reserved
		u16 LNA_INDIVIDUAL_CH_CNTL		: 1;	// Bit[15]: 1: Activa control individual de la ganancia LNA con el registro 57. En principio no parece de utilidad.
	} BIT;
}AFE_SPI_reg52_t;

typedef union AFE_SPI_reg59_u
{
	u16 REG;
	struct
	{
		u16 RESERVED0 					: 2;	// Bit[1:0]:
		u16 HPF_LNA						: 2;	// Bit[3:2]: 1: Enable internal active termination individual resistor control
		u16 DIG_TGC_ATT_GAIN			: 3;	// Bit[6:4]: 000: 0-dB attenuation, 001: 6-dB attenuation, N: About N × 6 dB attenuation when 59[7] = 1
		u16 DIG_TGC_ATT				 	: 1;	// Bit[7]: 0: Disable digital TGC attenuator (TGC por DAC), 1: Enable digital TGC attenuator
		u16 CW_SUM_AMP_PDN				: 1;	// Bit[8]: 0: Power down, 1: Normal operation Note: 59[8] is only effective in TGC test mode.
		u16 PGA_TEST_MODE				: 1;	// Bit[9]: 0: Normal CW operation, 1: PGA outputs appear at CW outputs.
		u16 RESERVED1					: 6;	// Bit[15:10]:
	} BIT;
}AFE_SPI_reg59_t;

// Note: This register is supported by AFE5809 with date code later than 2014, that is, date code >41XXXX.
typedef union AFE_SPI_reg61_u
{
	u16 REG;
	struct
	{
		u16 RESERVED0 					: 13;	// Bit[12:0]:
		u16 V2I_CLAMP					: 1;	// Bit[13]: 0: Clamp disabled
												//			1: Clamp enabled at the V2I input. An additional
												//			voltage clamp at the V2I input. This limits the
												//			amount of overload signal the PGA sees.
		u16 LPF_5MHz					: 1;	// Bit[14]: 0: 5-MHz LPF disabled
												//			1: 5-MHz LPF enabled. Suppress signals >5 MHz
												//			or high-order harmonics. The LPF Register
												//			51[3:1] needs to be set as 100, that is, 10 MHz.
		u16 PGA_CLAMP_minus6dBFS		: 1;	// Bit[15]: 0: Disable the –6-dBFS clamp. PGA_CLAMP is set by Reg51[7:5]. 1: Enable the –6-dBFS clamp. PGA_CLAMP
												//			Reg51[7:5] should be set as 000 in the lownoise
												//			mode or 100 in the low-power/mediumpower
												//			mode. In this setting, PGA output HD3 will
												//			be worsen by 3 dB at –6-dBFS ADC input. The
												//			actual PGA output is reduced to approximately 1.5
												//			Vpp, about 2.5 dB below the ADC full-scale input
												//			2 Vpp . As a result, AFE5809’s LPF is not
												//			saturated, and it can suppress harmonic signals
												//			better at PGA output. Due to PGA output
												//			reduction, the ADC output dynamic range is
												//			impacted.

	} BIT;
}AFE_SPI_reg61_t;

typedef struct AFE_SPI_UT_s
{
	AFE_SPI_reg21_t reg21;
	AFE_SPI_reg33_t reg33;
	AFE_SPI_reg51_t reg51;
	AFE_SPI_reg52_t reg52;
	AFE_SPI_reg59_t reg59;
	AFE_SPI_reg61_t reg61;	// Note: This register is supported by AFE5809 with date code later than 2014, that is, date code >41XXXX.
}AFE_SPI_UT_t;

typedef struct AFE_UT_s
{
	AFE_BUSSAR_UT_t AFE_BUSSAR_UT;
	AFE_SPI_UT_t	AFE_SPI_UT;
}AFE_UT_t;


extern AFE_SPI_UT_t Last_prog_AFE_SPI_UT[MAX_MINIBASES];
int prog_afe_SPI_regs(AFE_SPI_UT_t AFE_SPI_UT_nuevo,u8 minibase_addr);
int get_prog_afe_commands(AFE_BUSSAR_UT_t afe_ut,commnd_buffer_t *commnd_buf,u8 minibase_addr);
void print_spi_afe_regs_resumen(AFE_SPI_UT_t regs);
void print_spi_afe_regs(AFE_SPI_UT_t regs);
int all_afe_remote_align(u8 num_minibases);
void get_spi_afe_regs(AFE_SPI_UT_t *regs, u8 minibase_addr);

#endif /* SRC_SITAU2_AFE_AFE_BUSSAR_H_ */
