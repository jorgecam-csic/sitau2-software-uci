#ifndef uci_setH
#define uci_setH

#include "protocol.h"
#include "xparameters.h"

#define ALARM1_CHANNEL	1
#define ALARM1_MASK		0x10
#define ALARM1_GPIO_BASEADDRESS		XPAR_MISCELLANEOUS_GPIO_0_BASEADDR
#define ALARM1_TRISTATE_REG	(((ALARM1_CHANNEL-1)*2)+1)
#define ALARM1_DATAVAL_REG	(((ALARM1_CHANNEL-1)*2))

#define ALARM2_CHANNEL	1
#define ALARM2_MASK		0x20
#define ALARM2_GPIO_BASEADDRESS		XPAR_MISCELLANEOUS_GPIO_0_BASEADDR
#define ALARM2_TRISTATE_REG	(((ALARM2_CHANNEL-1)*2)+1)
#define ALARM2_DATAVAL_REG	(((ALARM2_CHANNEL-1)*2))


#define SYNC_CHANNEL	1
#define SYNC_MASK		0x1
#define SYNC_GPIO_BASEADDRESS		XPAR_MISCELLANEOUS_GPIO_0_BASEADDR
//Del product guide del gpio:
#define SYNC_TRISTATE_REG	(((SYNC_CHANNEL-1)*2)+1)
#define SYNC_DATAVAL_REG	(((SYNC_CHANNEL-1)*2))

#define SW2HWTRG_CHANNEL	1
#define SW2HWTRG_MASK		0x2
#define SW2HWTRG_GPIO_BASEADDRESS		XPAR_MISCELLANEOUS_GPIO_0_BASEADDR
//Del product guide del gpio:
#define SW2HWTRG_TRISTATE_REG	(((SW2HWTRG_CHANNEL-1)*2)+1)
#define SW2HWTRG_DATAVAL_REG	(((SW2HWTRG_CHANNEL-1)*2))

#define RST_DATAPATH_CHANNEL	1
#define RST_DATAPATH_MASK		0x4
#define RST_DATAPATH_GPIO_BASEADDRESS		XPAR_MISCELLANEOUS_GPIO_0_BASEADDR
//Del product guide del gpio:
#define RST_DATAPATH_TRISTATE_REG	(((RST_DATAPATH_CHANNEL-1)*2)+1)
#define RST_DATAPATH_DATAVAL_REG	(((RST_DATAPATH_CHANNEL-1)*2))

#define SYNC_MUX_SEL_CHANNEL	1
#define SYNC_MUX_SEL_MASK		0x8
#define SYNC_MUX_SEL_GPIO_BASEADDRESS		XPAR_MISCELLANEOUS_GPIO_0_BASEADDR
//Del product guide del gpio:
#define SYNC_MUX_SEL_TRISTATE_REG	(((SYNC_MUX_SEL_CHANNEL-1)*2)+1)
#define SYNC_MUX_SEL_DATAVAL_REG	(((SYNC_MUX_SEL_CHANNEL-1)*2))

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================

int UCI_Alarm1(int data);

int UCI_Alarm2(int data);

int UCI_SW2HW_trig(void);

int UCI_Sync(int data);

int UCI_set_Sync_is_SW (void);

int UCI_set_Sync_is_PRO (void);

int UCI_datapath_clear(void);

int UCI_SetEncoderZero(unsigned char id_encoder);

int UCI_SetAllEncoderZero (void);

int UCI_SetAcqCounterZero(void);

int UCI_IncAcqCounter (void);

int UCI_SetPulseAmplitude (unsigned char data);

int UCI_Set_TriggerSource_v0 (TMSG_TriggerSource_v0 *msg);
int UCI_Set_TriggerSource_v1 (TMSG_TriggerSource_v1 *msg);
int UCI_Set_TriggerSource_v2 (TMSG_TriggerSource_v2 *msg);
int UCI_Set_TriggerSource_v3 (TMSG_TriggerSource_v3 *msg);
int UCI_Set_TriggerSource (TMSG_TriggerSource *msg);

#endif

