/*
 * afe5808a.h
 *
 *  Created on: 11/09/2014
 *      Author: csic
 */

#ifndef AFE5808A_H_
#define AFE5808A_H_

#include "xil_types.h"
#include "xparameters.h"
#include "mcbcc_mst_driver.h"

#define AFES_BASEADDRESS	(XPAR_MCBCC_MST_AXI_TOP_0_BASEADDR+BUSSAR_AXIL_BASE)

#define DELAYS_PLACA_NUEVA	{8,0,4,4}
#define DELAYS_PLACA_NUEVB	{8,8,0,8}
#define DELAYS_PLACA_VIEJA	{5,5,7,5}
#define P_DELAYS_APLICADOS	DELAYS_PLACA_VIEJA

typedef volatile u32 * hw_uint32_ptr;
typedef char bool;

#define true 1
#define false (!true)

#define SOFTWARE_RESET 			(0<<16 | 1<<0)
#define REGISTER_READOUT_ENABLE	(0<<16 | 1<<1)

#define PAT_NORMAL	0
#define PAT_SYNC	1
#define PAT_DESKEW	2
#define PAT_CUSTOM	3
#define PAT_ONES	4
#define PAT_TOGGLE	5
#define PAT_ZEROS	6
#define PAT_RAMP	7


#define ADC_COMPLETE_PDN				(1<<16 | 1<<0)
#define LVDS_OUTPUT_DISABLE				(1<<16 | 1<<1)
#define ADC_PDN_CH0						(1<<16 | 1<<2)
#define ADC_PDN_CH1						(1<<16 | 1<<3)
#define ADC_PDN_CH2						(1<<16 | 1<<4)
#define ADC_PDN_CH3						(1<<16 | 1<<5)
#define ADC_PDN_CH4						(1<<16 | 1<<6)
#define ADC_PDN_CH5						(1<<16 | 1<<7)
#define ADC_PDN_CH6						(1<<16 | 1<<8)
#define ADC_PDN_CH7						(1<<16 | 1<<9)
#define PARTIAL_PDN						(1<<16 | 1<<10)
#define LOW_FREQUENCY_NOISE_SUPPRESSION	(1<<16 | 1<<11)
#define EXT_REF							(1<<16 | 1<<13)
#define LVDS_OUTPUT_RATE_2X				(1<<16 | 1<<14)
#define SINGLE_ENDED_CLK_MODE			(1<<16 | 1<<15)

#define POWER_DOWN_LVDS0	(2<<16 | 1<<3)
#define POWER_DOWN_LVDS1	(2<<16 | 1<<4)
#define POWER_DOWN_LVDS2	(2<<16 | 1<<5)
#define POWER_DOWN_LVDS3	(2<<16 | 1<<6)
#define POWER_DOWN_LVDS4	(2<<16 | 1<<7)
#define POWER_DOWN_LVDS5	(2<<16 | 1<<8)
#define POWER_DOWN_LVDS6	(2<<16 | 1<<9)
#define POWER_DOWN_LVDS7	(2<<16 | 1<<10)
#define AVERAGING_ENABLE	(2<<16 | 1<<11)
#define LOW_LATENCY			(2<<16 | 1<<12)
#define TEST_PATTERN_MODES0	(2<<16 | 1<<13)
#define TEST_PATTERN_MODES1	(2<<16 | 1<<14)
#define TEST_PATTERN_MODES2	(2<<16 | 1<<15)

#define INVERT_CHANNELS0					(3<<16 | 1<<0)
#define INVERT_CHANNELS1					(3<<16 | 1<<1)
#define INVERT_CHANNELS2					(3<<16 | 1<<2)
#define INVERT_CHANNELS3					(3<<16 | 1<<3)
#define INVERT_CHANNELS4					(3<<16 | 1<<4)
#define INVERT_CHANNELS5					(3<<16 | 1<<5)
#define INVERT_CHANNELS6					(3<<16 | 1<<6)
#define INVERT_CHANNELS7					(3<<16 | 1<<7)
#define CHANNEL_OFFSET_SUBSTRACTION_ENABLE	(3<<16 | 1<<8)
#define DIGITAL_GAIN_ENABLE					(3<<16 | 1<<12)
#define SERIALIZED_DATA_RATE0				(3<<16 | 1<<13)
#define SERIALIZED_DATA_RATE1				(3<<16 | 1<<14)
#define ENABLE_EXTERNAL_REFERENCE_MODE		(3<<16 | 1<<15)

#define ADC_RESOLUTION_SELECT	(4<<16 | 1<<0)
#define ADC_OUTPUT_FORMAT		(4<<16 | 1<<1)
#define LSB_MSB_FIRST			(4<<16 | 1<<2)

#define N_AFES				4
#define N_CHANNELS_PER_AFE	8
#define N_CHANNELS			(N_AFES*N_CHANNELS_PER_AFE)
#define N_TAPS				32
#define N_TESTS				250

typedef struct diagrama_ojos_s
{
	u16 res_1555[N_CHANNELS][N_TAPS];
	u16 res_2AAA[N_CHANNELS][N_TAPS];
	u16 res_otro[N_CHANNELS][N_TAPS];
}diagrama_ojos_t;

typedef struct retardos_s
{
	u8 taps[N_AFES];
}retardos_t;

#define AFE_CTRL_REG_OFFSET			0
#define AFE_TX_REG_OFFSET			1
#define AFE_RX_REG_OFFSET			2
#define AFE_CALIB_REG_OFFSET		3
#define AFE_ACQ_REG_OFFSET			4
#define AFE_INT_ENABLE_REG_OFFSET	5
#define AFE_INT_FLAG_REG_OFFSET		6
#define AFE_WATER_DELAY_REG_OFFSET	7
#define AFE_SAMPLE_0_REG_OFFSET		8
#define AFE_SAMPLE_1_REG_OFFSET		9
#define AFE_SAMPLE_2_REG_OFFSET		10
#define AFE_SAMPLE_3_REG_OFFSET		11
#define AFE_SAMPLE_4_REG_OFFSET		12
#define AFE_SAMPLE_5_REG_OFFSET		13
#define AFE_SAMPLE_6_REG_OFFSET		14
#define AFE_SAMPLE_7_REG_OFFSET		15

#define ALL_SAMPLES_OFFSET			32

#define AFE_MAX_SAMPLES ((1<<17)-1)

void afe_set_dig_gain(u32 baseaddr,u8 gain);

void aplicar_retardos(u32 baseaddr,retardos_t *retardos_p);

u32 afe5808a_test_pattern(u32 baseaddr,u8 test);

void diagrama_de_ojos(u32 baseaddr,diagrama_ojos_t *diagrama_ojos_p);

void print_diagrama_de_ojos(diagrama_ojos_t *diagrama_ojos_p);

void wait_for(u32 delay);

void usleep(u32 delay);

void afe5808a_select(u32 baseaddr,u8 afe_select_byte);

u32 spi_transfer(u32 baseaddr, u32 tx_reg);

u32 afe5808a_spi_write(u32 baseaddr, u8 addr, u16 data);

u32 afe5808a_spi_read(u32 baseaddr, u8 addr, u16 * data);

void config_afe(u32 baseaddr);

void afe5808a_reset(u32 baseaddr);

u32 afe5808a_ramp(u32 baseaddr);

u32 afe5808a_dly_rst(u32 baseaddr);

u32 afe5808a_dly_inc(u32 baseaddr, u32 channel, u32 dly_inc);

u32 afe5808a_bitslip(u32 baseaddr, u32 channel);

u32 look_for_estable(u32 baseaddr, u8 channel, u16 * data);

u32 look_for_metaestable(u32 baseaddr, u8 channel, u16 data);

u32 afe5808a_calib(u32 baseaddr);

int afe5808a_align(u32 baseaddr,u8 afe);

u32 afe5808a_enable(u32 baseaddr);

int afe5808a_delay_samples(u32 baseaddr);

u32 afe5808a_calib_channel(u32 baseaddr,u8 num_inc,u8 channel);

void calc_delays(diagrama_ojos_t *diagrama_ojos_p,u8* delays_p,u8* tam_ojo);

void afe5808a_skew_align(u32 baseaddr,u8 afe_select);

int afe5808a_complete_skew_align(u32 baseaddr);
int main_afe(void);

void print_current_afe_regs(u32 baseaddr);

int afe5808a_complete_skew_align_delays(u32 baseaddr,u8 *delays);

u32 afe5808a_calib_channel(u32 baseaddr,u8 num_inc,u8 channel);

void afe5808a_ojos_1_canal(u32 baseaddr,u8 afe,u8 channel,diagrama_ojos_t* diagrama_ojos_p);

void afe5808a_basic_set_up(u32 baseaddr);

u8 afe5808a_dly_get(u32 baseaddr,u8 afe,u8 channel);

u8 afe5808a_dly_set(u32 baseaddr,u8 afe,u8 channel,u8 dly);

#endif /* AFE5808A_H_ */
