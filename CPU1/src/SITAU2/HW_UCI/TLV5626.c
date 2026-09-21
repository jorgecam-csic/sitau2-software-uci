#include <stdio.h>

#include <xparameters.h>
#include <xenv.h>
#include <xbasic_types.h>
#include <xstatus.h>

#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xbasic_types.h"

#include "TLV5626.h"

/********* TLV5626 **********/
#define STARTUP_GAIN	0
#define FAST_MODE		0
#define REF_VAL_EXTR	0x0
#define REF_VAL_1024	0x1
#define REF_VAL_2048	0x2
#define REF_VAL			REF_VAL_2048
static void wait_for(u32 delay)
{
	while(delay--);
}
void TLV5626_ini(u32* dac_baseaddress)
{
	volatile u32 *TLV5626=dac_baseaddress;
	TLV5626[0] = 0x80009000|FAST_MODE<<14|REF_VAL;
	wait_for(100);
	TLV5626[0] = 0x80000000 | (0x8000) | (0x0ff0 & (STARTUP_GAIN<<4))|FAST_MODE<<14;
}

void TLV5626_value(u32* dac_baseaddress,u8 value)
{
	volatile u32 *TLV5626=dac_baseaddress;
	u32 temp;
	temp = 0x80000000 | (0x8000) | (0x0ff0 & ((u16)value<<4)) | (FAST_MODE<<14);
	TLV5626[0] = temp;
}
