/*
 * xcugic_mod_hw.c
 *
 *  Created on: 15 de feb. de 2017
 *      Author: csic
 */

#include "xil_types.h"
#include "xil_assert.h"
#include "xscugic.h"
#include "xparameters.h"
#include "gic_utils.h"
#include "global.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/



#define MSK_TRG	0x03
#define MSK_PRI	0xFF
#define MSK_CPU	0xFF
#define MSK_ENA	0x01

#define DEF_TRG	0x00
#define DEF_PRI	0xA0
#define DEF_CPU	0x01
#define DEF_ENA	0x00

#define BIT_TRG	2
#define BIT_PRI	8
#define BIT_CPU	8
#define BIT_ENA	1


void Set_Up_Gic_Interrupt(XScuGic *InstancePtr, u32 CpuID, u16 Int_Id)
{
	u32 LocalCpuID = CpuID;
	u8 *u8_ptr;
	u32 *u32_ptr;
	u32 u32_tmp;
	u32 u32_mask;

	XScuGic_DistWriteReg(InstancePtr, XSCUGIC_DIST_EN_OFFSET, 0U);


	/*
	 * Set the security domains in the int_security registers for
	 * non-secure interrupts
	 * All are secure, so leave at the default. Set to 1 for non-secure
	 * interrupts.
	 */

	/*
	 * For the Shared Peripheral Interrupts INT_ID[MAX..32], set:
	 */

	/*
	 * 1. The trigger mode in the int_config register
	 * Only write to the SPI interrupts, so start at 32
	 */


	{
		/*
		 * Each INT_ID uses two bits, or 16 INT_ID per register
		 * Set them all to be level sensitive, active HIGH.
		 */
		u32_tmp=XScuGic_DistReadReg(InstancePtr,XSCUGIC_INT_CFG_OFFSET_CALC(Int_Id));
		u32_mask=MSK_TRG<<(BIT_TRG*(Int_Id%(32/BIT_TRG)));
		u32_tmp&=(~u32_mask);
		u32_mask=DEF_TRG<<(BIT_TRG*(Int_Id%(32/BIT_TRG)));
		u32_tmp|=(u32_mask);
		XScuGic_DistWriteReg(InstancePtr,XSCUGIC_INT_CFG_OFFSET_CALC(Int_Id),u32_tmp);
	}


	{
		/*
		 * 2. The priority using int the priority_level register
		 * The priority_level and spi_target registers use one byte per
		 * INT_ID.
		 * Write a default value that can be changed elsewhere.
		 */
		u32_tmp=XScuGic_DistReadReg(InstancePtr,XSCUGIC_PRIORITY_OFFSET_CALC(Int_Id));
		u32_mask=MSK_PRI<<(BIT_PRI*(Int_Id%(32/BIT_PRI)));
		u32_tmp&=(~u32_mask);
		u32_mask=DEF_PRI<<(BIT_PRI*(Int_Id%(32/BIT_PRI)));
		u32_tmp|=(u32_mask);
		XScuGic_DistWriteReg(InstancePtr,XSCUGIC_PRIORITY_OFFSET_CALC(Int_Id),u32_tmp);

	}

	{
		/*
		 * 3. The CPU interface in the spi_target register
		 * Only write to the SPI interrupts, so start at 32
		 */

		u32_tmp=XScuGic_DistReadReg(InstancePtr,XSCUGIC_SPI_TARGET_OFFSET_CALC(Int_Id));
		u32_mask=MSK_CPU<<(BIT_CPU*(Int_Id%(32/BIT_CPU)));
		u32_tmp&=(~u32_mask);
		u32_mask=CpuID<<(BIT_CPU*(Int_Id%(32/BIT_CPU)));
		u32_tmp|=(u32_mask);
		XScuGic_DistWriteReg(InstancePtr,XSCUGIC_SPI_TARGET_OFFSET_CALC(Int_Id),u32_tmp);

	}

	{
		/*
		 * 4. Enable the SPI using the enable_set register. Leave all
		 * disabled for now.
		 */

		u32_tmp=XScuGic_DistReadReg(InstancePtr,XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,Int_Id));
		u32_mask=MSK_ENA<<(BIT_ENA*(Int_Id%(32/BIT_ENA)));
		u32_tmp&=(~u32_mask);
		u32_mask=DEF_ENA<<(BIT_ENA*(Int_Id%(32/BIT_ENA)));
		u32_tmp|=(u32_mask);
		XScuGic_DistWriteReg(InstancePtr,XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,Int_Id),u32_mask);
	}

	XScuGic_DistWriteReg(InstancePtr, XSCUGIC_DIST_EN_OFFSET,XSCUGIC_EN_INT_MASK);
}

static void DistInit(XScuGic_Config *Config, u32 CpuID);
static void CPUInit(XScuGic_Config *Config);
static XScuGic_Config *LookupConfigByBaseAddress(u32 CpuBaseAddress);

/************************** Variable Definitions *****************************/

extern XScuGic_Config XScuGic_ConfigTable[XPAR_XSCUGIC_NUM_INSTANCES];


static void DistInit(XScuGic_Config *Config, u32 CpuID)
{
	u32 Int_Id;
	u32 LocalCpuID = CpuID;

#if USE_AMP==1
	#warning "Building GIC for AMP"

	/*
	 * The distrubutor should not be initialized by FreeRTOS in the case of
	 * AMP -- it is assumed that Linux is the master of this device in that
	 * case.
	 */
	return;
#endif

	XScuGic_WriteReg(Config->DistBaseAddress, XSCUGIC_DIST_EN_OFFSET, 0U);

	/*
	 * Set the security domains in the int_security registers for non-secure
	 * interrupts. All are secure, so leave at the default. Set to 1 for
	 * non-secure interrupts.
	 */


	/*
	 * For the Shared Peripheral Interrupts INT_ID[MAX..32], set:
	 */

	/*
	 * 1. The trigger mode in the int_config register
	 * Only write to the SPI interrupts, so start at 32
	 */
	for (Int_Id = 32U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+16U) {
	/*
	 * Each INT_ID uses two bits, or 16 INT_ID per register
	 * Set them all to be level sensitive, active HIGH.
	 */
		XScuGic_WriteReg(Config->DistBaseAddress,
			XSCUGIC_INT_CFG_OFFSET_CALC(Int_Id), 0U);
	}


#define DEFAULT_PRIORITY	0xa0a0a0a0U
	for (Int_Id = 0U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+4U) {
		/*
		 * 2. The priority using int the priority_level register
		 * The priority_level and spi_target registers use one byte per
		 * INT_ID.
		 * Write a default value that can be changed elsewhere.
		 */
		XScuGic_WriteReg(Config->DistBaseAddress,
				XSCUGIC_PRIORITY_OFFSET_CALC(Int_Id),
				DEFAULT_PRIORITY);
	}

	for (Int_Id = 32U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+4U) {
		/*
		 * 3. The CPU interface in the spi_target register
		 * Only write to the SPI interrupts, so start at 32
		 */
		LocalCpuID |= LocalCpuID << 8U;
		LocalCpuID |= LocalCpuID << 16U;

		XScuGic_WriteReg(Config->DistBaseAddress,
				XSCUGIC_SPI_TARGET_OFFSET_CALC(Int_Id), LocalCpuID);
	}

	for (Int_Id = 0U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+32U) {
	/*
	 * 4. Enable the SPI using the enable_set register. Leave all disabled
	 * for now.
	 */
		XScuGic_WriteReg(Config->DistBaseAddress,
		XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,
		Int_Id),
		0xFFFFFFFFU);

	}

	XScuGic_WriteReg(Config->DistBaseAddress, XSCUGIC_DIST_EN_OFFSET,
						XSCUGIC_EN_INT_MASK);

}

static void CPUInit(XScuGic_Config *Config)
{
	/*
	 * Program the priority mask of the CPU using the Priority mask
	 * register
	 */
	XScuGic_WriteReg(Config->CpuBaseAddress, XSCUGIC_CPU_PRIOR_OFFSET,
									0xF0U);

	/*
	 * If the CPU operates in both security domains, set parameters in the
	 * control_s register.
	 * 1. Set FIQen=1 to use FIQ for secure interrupts,
	 * 2. Program the AckCtl bit
	 * 3. Program the SBPR bit to select the binary pointer behavior
	 * 4. Set EnableS = 1 to enable secure interrupts
	 * 5. Set EnbleNS = 1 to enable non secure interrupts
	 */

	/*
	 * If the CPU operates only in the secure domain, setup the
	 * control_s register.
	 * 1. Set FIQen=1,
	 * 2. Set EnableS=1, to enable the CPU interface to signal secure .
	 * interrupts Only enable the IRQ output unless secure interrupts
	 * are needed.
	 */
	XScuGic_WriteReg(Config->CpuBaseAddress, XSCUGIC_CONTROL_OFFSET, 0x07U);

}

s32 XScuGic_DeviceInitialize_mod(u32 DeviceId)
{
	XScuGic_Config *Config;
	u32 Cpu_Id = (u32)XPAR_CPU_ID + (u32)1;
	int i;

	Config = &XScuGic_ConfigTable[(u32 )DeviceId];

	Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, 54);
	Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, 55);
	//Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, 77);
	//Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, 78);

	//for(i=0;i<61;i++)
		//Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, i);
	//for(i=69;i<84;i++)
		//Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, i);

	//Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, 92);
	//DistInit(Config, Cpu_Id);
	//for(i=0;i<96;i++)
		//Set_Up_Gic_Interrupt(&Gic_instance,THIS_CPU_GIC_MASK, i);


	CPUInit(Config);

	return XST_SUCCESS;
}
