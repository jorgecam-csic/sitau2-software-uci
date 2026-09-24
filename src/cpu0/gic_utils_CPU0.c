/*
 * gic_utils.c
 *
 *  Created on: 18/11/2016
 *      Author: csic
 */

#include "gic_utils.h"
#include "xil_cache.h"

XScuGic Gic_instance;
static XScuGic_Config *GicConfig;    /* The configuration parameters of the
                                       controller */
#define N_INTS 96
static void DistributorInit(XScuGic *InstancePtr, u32 CpuID)
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
	for (Int_Id = 32U; Int_Id < N_INTS; Int_Id=Int_Id+16U) {
		/*
		 * Each INT_ID uses two bits, or 16 INT_ID per register
		 * Set them all to be level sensitive, active HIGH.
		 */
		XScuGic_DistWriteReg(InstancePtr,
					XSCUGIC_INT_CFG_OFFSET_CALC(Int_Id),
					0U);
	}


#define DEFAULT_PRIORITY    0xa0a0a0a0U
	for (Int_Id = 0U; Int_Id < N_INTS; Int_Id=Int_Id+4U) {
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


	for (Int_Id = 32U; Int_Id<N_INTS;Int_Id=Int_Id+4U) {
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

	for (Int_Id = 0U; Int_Id<N_INTS;Int_Id=Int_Id+32U)
	{
		/*
		 * 4. Enable the SPI using the enable_set register. Leave all
		 * disabled for now.
		 */
		XScuGic_DistWriteReg(InstancePtr,
		XSCUGIC_EN_DIS_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET, Int_Id),
			0xFFFFFFFFU);

	}
	for (Int_Id = 0U; Int_Id<N_INTS;Int_Id=Int_Id+32U)
	{
		/*
		 * 4. Enable the SPI using the enable_set register. Leave all
		 * disabled for now.
		 */
		XScuGic_DistWriteReg(InstancePtr,(0x0100U+(Int_Id/32U)*4U),0xFFFFFFFFU);

	}

	XScuGic_DistWriteReg(InstancePtr, XSCUGIC_DIST_EN_OFFSET,
						XSCUGIC_EN_INT_MASK);
	return;
}
static void CPUInitialize(XScuGic *InstancePtr)
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

static void StubHandler(void *CallBackRef) {
	/*
	 * verify that the inputs are valid
	 */
	Xil_AssertVoid(CallBackRef != NULL);

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
	u32 Cpu_Id = (u32)XPAR_CPU_ID + (u32)1;
	(void) EffectiveAddr;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	if(InstancePtr->IsReady != XIL_COMPONENT_IS_READY)
	{

		InstancePtr->IsReady = 0;
		InstancePtr->Config = ConfigPtr;


		if(0)//for (Int_Id = 0U; Int_Id<N_INTS;Int_Id++)
		{
			/*
			* Initalize the handler to point to a stub to handle an
			* interrupt which has not been connected to a handler. Only
			* initialize it if the handler is 0 which means it was not
			* initialized statically by the tools/user. Set the callback
			* reference to this instance so that unhandled interrupts
			* can be tracked.
			*/
			//if(Int_Id==8) continue;
			if 	((InstancePtr->Config->HandlerTable[Int_Id].Handler == NULL)) {
				InstancePtr->Config->HandlerTable[Int_Id].Handler =
									StubHandler;
			}
			InstancePtr->Config->HandlerTable[Int_Id].CallBackRef =
								InstancePtr;
		}

		//DistributorInit(InstancePtr, Cpu_Id);
		CPUInitialize(InstancePtr);

		InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	}

	return XST_SUCCESS;
}

int gic_utils_init(void)
{
	u16 DeviceId = INTC_DEVICE_ID;
	int Status;

	Xil_DCacheDisable();

	GicConfig = XScuGic_LookupConfig(DeviceId);
	if (NULL == GicConfig) {
		return XST_FAILURE;
	}

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

	Xil_DCacheEnable();
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;

}
int gic_utils_register_interrupt(u32 interrupt_id,Xil_ExceptionHandler interrupt_function)
{
	int ret;
	Xil_DCacheDisable();
	ret = XScuGic_Connect(&Gic_instance, interrupt_id,interrupt_function,(void *)&Gic_instance);
	Xil_DCacheEnable();
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

