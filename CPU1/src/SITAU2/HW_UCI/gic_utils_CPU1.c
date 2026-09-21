/*
 * gic_utils.c
 *
 *  Created on: 18/11/2016
 *      Author: csic
 */

#include "gic_utils.h"
#include "xil_cache.h"

//GIC_UTILS CPU1
XScuGic Gic_instance;
static XScuGic_Config *GicConfig;



#define MSK_TRG	0x03
#define MSK_PRI	0xFF
#define MSK_CPU	0xFF
#define MSK_ENA	0x01

#define DEF_TRG	0x00
#define DEF_PRI	0xA0
#define DEF_CPU	0x02
#define DEF_ENA	0x00

#define BIT_TRG	2
#define BIT_PRI	8
#define BIT_CPU	8
#define BIT_ENA	1


void Set_Up_Gic_Interrupt(XScuGic *InstancePtr, u32 CpuID, u16 Int_Id)
{
	//u32 LocalCpuID = CpuID;
	//u8 *u8_ptr;
	//u32 *u32_ptr;
	u32 u32_tmp;
	u32 u32_mask;

	//XScuGic_DistWriteReg(InstancePtr, XSCUGIC_DIST_EN_OFFSET, 0U);


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

		//u32_tmp=XScuGic_DistReadReg(InstancePtr,XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,Int_Id));
		u32_mask=MSK_ENA<<(BIT_ENA*(Int_Id%(32/BIT_ENA)));
		//u32_tmp&=(~u32_mask);
		//u32_mask=DEF_ENA<<(BIT_ENA*(Int_Id%(32/BIT_ENA)));
		//u32_tmp|=(u32_mask);
		XScuGic_DistWriteReg(InstancePtr,XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_ENABLE_SET_OFFSET,Int_Id),u32_mask);
		//XScuGic_DistWriteReg(InstancePtr,XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET,Int_Id),0x0);
	}

	//XScuGic_DistWriteReg(InstancePtr, XSCUGIC_DIST_EN_OFFSET,XSCUGIC_EN_INT_MASK);
}


static void DistributorInit_mod(XScuGic *InstancePtr, u32 CpuID)
{
	u32 Int_Id;
	u32 LocalCpuID = CpuID;
//#if USE_AMP==1
#if 1
	#warning "Building GIC for AMP"

	/*
	 * The distrubutor should not be initialized by FreeRTOS in the case of
	 * AMP -- it is assumed that Linux is the master of this device in that
	 * case.
	 */
	return;
#endif
	Xil_AssertVoid(InstancePtr != NULL);
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
	for (Int_Id = 32U; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id=Int_Id+16U) {
		/*
		 * Each INT_ID uses two bits, or 16 INT_ID per register
		 * Set them all to be level sensitive, active HIGH.
		 */
		XScuGic_DistWriteReg(InstancePtr,
					XSCUGIC_INT_CFG_OFFSET_CALC(Int_Id),
					0U);
	}


#define DEFAULT_PRIORITY    0xa0a0a0a0U
	for (Int_Id = 0U; Int_Id < XSCUGIC_MAX_NUM_INTR_INPUTS; Int_Id=Int_Id+4U) {
		/*
		 * 2. The priority using int the priority_level register
		 * The priority_level and spi_target registers use one byte per
		 * INT_ID.
		 * Write a default value that can be changed elsewhere.
		 */
		XScuGic_DistWriteReg(InstancePtr,
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

		XScuGic_DistWriteReg(InstancePtr,
				     XSCUGIC_SPI_TARGET_OFFSET_CALC(Int_Id),
				     LocalCpuID);
	}

	for (Int_Id = 0U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id=Int_Id+32U) {
		/*
		 * 4. Enable the SPI using the enable_set register. Leave all
		 * disabled for now.
		 */
		XScuGic_DistWriteReg(InstancePtr,
		XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET, Int_Id),
			0xFFFFFFFFU);

	}

	XScuGic_DistWriteReg(InstancePtr, XSCUGIC_DIST_EN_OFFSET,
						XSCUGIC_EN_INT_MASK);

}
static void CPUInitialize_mod(XScuGic *InstancePtr)
{
	/*
	 * Program the priority mask of the CPU using the Priority mask register
	 */
	XScuGic_CPUWriteReg(InstancePtr, XSCUGIC_CPU_PRIOR_OFFSET, 0xF0U);


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
	 * 2. Set EnableS=1, to enable the CPU interface to signal secure interrupts.
	 * Only enable the IRQ output unless secure interrupts are needed.
	 */
	XScuGic_CPUWriteReg(InstancePtr, XSCUGIC_CONTROL_OFFSET, 0x07U);

}

static void StubHandler_mod(void *CallBackRef) {
	/*
	 * verify that the inputs are valid
	 */
	Xil_AssertVoid(CallBackRef != NULL);
	xil_printf("�Interrupci�n no inicializada! Usando rutina de atenci�n por defecto.\r\n");

	/*
	 * Indicate another unhandled interrupt for stats
	 */
	((XScuGic *)((void *)CallBackRef))->UnhandledInterrupts++;
}

s32  XScuGic_CfgInitialize_mod(XScuGic *InstancePtr,
				XScuGic_Config *ConfigPtr,
				u32 EffectiveAddr)
{
	u32 Int_Id;
	//u32 Cpu_Id = (u32)XPAR_CPU_ID + (u32)1;
	(void) EffectiveAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if(InstancePtr->IsReady != XIL_COMPONENT_IS_READY)
	{

		InstancePtr->IsReady = 0;
		InstancePtr->Config = ConfigPtr;


		//for (Int_Id = 0U; Int_Id<XSCUGIC_MAX_NUM_INTR_INPUTS;Int_Id++)
		if(0)
		{
			/*
			* Initalize the handler to point to a stub to handle an
			* interrupt which has not been connected to a handler. Only
			* initialize it if the handler is 0 which means it was not
			* initialized statically by the tools/user. Set the callback
			* reference to this instance so that unhandled interrupts
			* can be tracked.
			*/
			if 	((InstancePtr->Config->HandlerTable[Int_Id].Handler == NULL)) {
				InstancePtr->Config->HandlerTable[Int_Id].Handler =
									StubHandler_mod;
			}
			InstancePtr->Config->HandlerTable[Int_Id].CallBackRef =
								InstancePtr;
		}

		//DistributorInit_mod(InstancePtr, Cpu_Id);
		CPUInitialize_mod(InstancePtr);

		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	}

	return XST_SUCCESS;
}

int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{

	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			XScuGicInstancePtr);

	Set_Up_Gic_Interrupt(XScuGicInstancePtr, THIS_CPU_GIC_MASK, XIL_EXCEPTION_ID_INT);
	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int gic_utils_init(void)
{
	u16 DeviceId = INTC_DEVICE_ID;
	int Status;

	//Xil_DCacheDisable();

	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}
	Gic_instance.IsReady = 0;

	Status = XScuGic_CfgInitialize_mod(&Gic_instance, GicConfig,GicConfig->CpuBaseAddress);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	/*
	Status = XScuGic_SelfTest(&Gic_instance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	 */

	/*
	 * Setup the Interrupt System
	 */
	Status = SetUpInterruptSystem(&Gic_instance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler performs
	 * the specific interrupt processing for the device
	 */
	XScuGic_DistWriteReg(&Gic_instance, XSCUGIC_DIST_EN_OFFSET,XSCUGIC_EN_INT_MASK);
	return Status;
#define INTC_DEVICE_INT_ID 8
/*
	Status = XScuGic_Connect(&Gic_instance, INTC_DEVICE_INT_ID,
			   (Xil_ExceptionHandler)DeviceDriverHandler,
			   (void *)&Gic_instance);
*/
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the device and then cause (simulate) an
	 * interrupt so the handlers will be called
	 */
	XScuGic_Enable(&Gic_instance, INTC_DEVICE_INT_ID);


	return Status;

	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&Gic_instance, GicConfig,
					GicConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly
	 */
	/*
	Status = XScuGic_SelfTest(&Gic_instance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
*/

	/*
	 * Setup the Interrupt System
	 */
	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			&Gic_instance);

	/*
	 * Enable interrupts in the ARM
	 */
	Xil_ExceptionEnable();

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;

}

int gic_utils_register_interrupt(u32 interrupt_id,Xil_ExceptionHandler interrupt_function)
{
	int ret;
	//Xil_DCacheDisable();
	ret = XScuGic_Connect(&Gic_instance, interrupt_id,interrupt_function,(void *)&Gic_instance);
	Set_Up_Gic_Interrupt(&Gic_instance, THIS_CPU_GIC_MASK, interrupt_id);
	//Xil_DCacheEnable();
	return ret;
}

void gic_utils_enable_interrupt(u32 interrupt_id)
{
	XScuGic_Enable(&Gic_instance, interrupt_id);
}

int gic_utils_software_interrupt(u32 interrupt_id,u8 cpu_id)
{
	int ret;
	ret = XScuGic_SoftwareIntr(&Gic_instance,interrupt_id,cpu_id);
	return ret;
}
