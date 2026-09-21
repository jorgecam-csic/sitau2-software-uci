#include "timestamp.h"
#include "xgpio_l.h"
#include "xtmrctr_l.h"
#include "timer_util.h"


const u32 Time_stamp_timer_base_addr=XPAR_TIMESTAMP_TIMER_BASEADDR;
const u32 Time_stamp_gpio_base_addr=XPAR_MISCELLANEOUS_GPIO_0_BASEADDR;
const u16 Time_stamp_rst_int_number=XPAR_FABRIC_MISCELLANEOUS_GPIO_0_IP2INTC_IRPT_INTR;
const u8  Time_stamp_rst_gpio_channel=2;
const u32 Time_stamp_rst_gpio_mask=0x00000002;

volatile u32 Time_stamp_rst_gpio_data_last_value=0x00000000;

/* Del manual del AXI Timer.
 *
 * The following are the steps for running the 64-bit counter/timer in generate mode:
1. Clear the timer enable bits in control registers (TCSR0 and TCSR1).
2. Write the lower 32-bit timer/counter load register (TLR0).
3. Write the higher 32-bit timer/counter load register (TLR1).
4. Set the CASC bit in Control register TCSR0.
5. Set other mode control bits in control register (TCSR0) as needed.
6. Enable the timer in Control register (TCSR0).
The following are the steps for reading the 64-bit counter/timer:
1. Read the upper 32-bit timer/counter register (TCR1).
2. Read the lower 32-bit timer/counter register (TCR0).
3. Read the upper 32-bit timer/counter register (TCR1) again. If the value is different from
the 32-bit upper value read previously, go back to previous step (reading TCR0).
Otherwise 64-bit timer counter value is correct.
*/
void timestamp_init(void)
{
	volatile u32* gpio_base=(u32*)Time_stamp_gpio_base_addr;
	volatile u32* timer_base=(u32*)Time_stamp_timer_base_addr;

	////////////////////////// Iniciar GPIO //////////////////////////
	gpio_base[TIMESTAMP_RST_TRISTATE_REG]|=TIMESTAMP_RST_MASK;

	gpio_base[TIMESTAMP_RST_INTSTS_REG]|=(0x1<<(TIMESTAMP_RST_CHANNEL-1)); // Se borra la interrupci�n primero

	gic_utils_register_interrupt(XPAR_FABRIC_MISCELLANEOUS_GPIO_0_IP2INTC_IRPT_INTR,timestamp_rst_int_function);

	Time_stamp_rst_gpio_data_last_value=0x0;

	gpio_base[TIMESTAMP_RST_INTENA_REG]|=(0x1<<(TIMESTAMP_RST_CHANNEL-1)); // Se activa la interrupci�n por el canal

	gpio_base[TIMESTAMP_RST_INTGLO_REG]|=XGPIO_GIE_GINTR_ENABLE_MASK; // Se activan las interrupciones en general del gpio
	////////////////////////// Iniciar Timer 64 bits //////////////////////////
	timer_base[TCSR0] = 0;
	timer_base[TCSR1] = 0;
	timer_base[TLR0] = 0x00000000;
	timer_base[TCR0] = 0x00000000;
	timer_base[TLR1] = 0x00000000;
	timer_base[TCR1] = 0x00000000;
	timer_base[TCSR0] = XTC_CSR_CASC_MASK;
	timer_base[TCSR0] = XTC_CSR_LOAD_MASK|XTC_CSR_INT_OCCURED_MASK|XTC_CSR_CASC_MASK;
	timer_base[TCSR0] = XTC_CSR_ENABLE_TMR_MASK|XTC_CSR_ENABLE_ALL_MASK|XTC_CSR_CASC_MASK;

	gpio_base[TIMESTAMP_RST_INTSTS_REG]=gpio_base[TIMESTAMP_RST_INTSTS_REG]; // Toggle on write 1 (se borra escribiendo lo mismo)
}

void timestamp_rst(void)
{
	volatile u32* timer_base=(u32*)Time_stamp_timer_base_addr;
	timer_base[TCSR0] = XTC_CSR_LOAD_MASK|XTC_CSR_INT_OCCURED_MASK;
	timer_base[TCSR1] = XTC_CSR_LOAD_MASK|XTC_CSR_INT_OCCURED_MASK;
	timer_base[TCSR1] = 0;
	timer_base[TCSR0] = XTC_CSR_CASC_MASK;
	timer_base[TCSR0] = XTC_CSR_ENABLE_TMR_MASK|XTC_CSR_INT_OCCURED_MASK|XTC_CSR_ENABLE_ALL_MASK|XTC_CSR_CASC_MASK;
	if(0){
		u32 least_significant_word32,most_significant_word32;
		timestamp_get_ticks(&least_significant_word32,&most_significant_word32);
		xil_printf("RST TIMESTAMP!! Contador LSW = 0x%08X, Contador MSW = 0x%08X\r\n",least_significant_word32,most_significant_word32);
	}
}

void timestamp_rst_int_function()
{
	volatile u32* gpio_base=(u32*)Time_stamp_gpio_base_addr;
	static u32 times=0;
	u32 new_gpio_data_val=0x0;
	if(0){
		u32 least_significant_word32,most_significant_word32;
		timestamp_get_ticks(&least_significant_word32,&most_significant_word32);
		xil_printf("timestamp_rst_int_function(%d)!! Contador LSW = 0x%08X, Contador MSW = 0x%08X\r\n",times++,least_significant_word32,most_significant_word32);
	}
	if(gpio_base[TIMESTAMP_RST_INTSTS_REG]&(0x1<<(TIMESTAMP_RST_CHANNEL-1)))
	{
		new_gpio_data_val=gpio_base[TIMESTAMP_RST_DATAVAL_REG]&TIMESTAMP_RST_MASK;
		if( new_gpio_data_val &&(!Time_stamp_rst_gpio_data_last_value)) // solo borro si new_gpio_data_val!=0 and Time_stamp_rst_gpio_data_last_value == 0
			timestamp_rst();
		Time_stamp_rst_gpio_data_last_value=new_gpio_data_val;
	}
	// En cualquier caso, antes de salir borro las interrupciones de ambos canales.
	gpio_base[TIMESTAMP_RST_INTSTS_REG]=gpio_base[TIMESTAMP_RST_INTSTS_REG]; // Toggle on write 1 (se borra escribiendo lo mismo)

}

void timestamp_get_ticks(u32* least_significant_word32,u32* most_significant_word32)
{
	volatile u32* timer_base=(u32*)Time_stamp_timer_base_addr;
	do{
		*most_significant_word32=timer_base[TCR1];
		*least_significant_word32=timer_base[TCR0];
	}while(*most_significant_word32!=timer_base[TCR1]);
}

u64 timestamp_get_ticks_64(void)
{
	u64 ticks64;
	u32* least_significant_word32=(u32*)&ticks64;
	u32* most_significant_word32=&least_significant_word32[1];
	timestamp_get_ticks(least_significant_word32,most_significant_word32);
	return ticks64;
}

float timestamp_get_us_float(u64 ini, u64 end)
{
	//float freq_MHz=Timer[ID_TIMER_TIMESTAMP/2].Config.SysClockFreqHz/1000000;
	float freq_MHz=AXI_TIMER_FREQ/1000000;
	return (float)(end-ini)/freq_MHz;

}
u32 timestamp_get_ms_32(u64 ini, u64 end)
{
	float freq_MHz=AXI_TIMER_FREQ/1000000;
	u64 resta_ms=(end-ini)*1000/AXI_TIMER_FREQ;

	u32 ret_ms=(u32)resta_ms;


	return ret_ms;
}
