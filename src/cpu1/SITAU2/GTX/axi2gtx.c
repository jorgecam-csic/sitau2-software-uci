/*
 * axi2gtx.c
 *
 *  Created on: 23 ago. 2022
 *      Author: csic
 */


#include <GTX/axi2gtx.h>
#include "xllfifo.h"
#include "xparameters.h"
#include "xstatus.h"

static XLlFifo FifoGTX_Inst;
static XLlFifo *FifoGTX_InstancePtr=&FifoGTX_Inst;
static const u16 FifoGTXDeviceId = XPAR_AXI_FIFO_1_DEVICE_ID;

int axi2gtx_fifo_init()
{
	XLlFifo_Config *Config;
	int Status;
	int i;
	int Error=0;
	Status = XST_SUCCESS;

	/* Initialize the Device Configuration Interface driver */
	Config = XLlFfio_LookupConfig(FifoGTXDeviceId);
	if (!Config) {
		xil_printf("No config found for %d\r\n", FifoGTXDeviceId);
		return XST_FAILURE;
	}

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XLlFifo_CfgInitialize(FifoGTX_InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}

	/* Check for the Reset value */
	Status = XLlFifo_Status(FifoGTX_InstancePtr);
	XLlFifo_IntClear(FifoGTX_InstancePtr,0xffffffff);
	Status = XLlFifo_Status(FifoGTX_InstancePtr);
	if(Status != 0x0) {
		xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
				"Expected : 0x0\n\r",
				XLlFifo_Status(FifoGTX_InstancePtr));
		return XST_FAILURE;
	}
}

int axi2gtx_fifo_send(u32  *SourceAddr, u32 Numwords)
{

	int i;
	int j;
	volatile register int temp_volatile_reg;
	//xil_printf(" Transmitting Buffer ... \r\n");
	u32 total_words_left = Numwords+0;//6692572 bytes + relleno, 0 en este caso
	u32 words_left_to_send,words_to_send,words_to_send_block;

	u32 MAX_WORDS_PER_BLOCK=(1<<(23-2))-1;
	u32  *temp_src=SourceAddr;
	//XLlFifo_TxReset(FifoGTX_InstancePtr);

	while(total_words_left)
	{
		if(total_words_left>MAX_WORDS_PER_BLOCK)
			words_to_send_block=MAX_WORDS_PER_BLOCK;
		else
			words_to_send_block=total_words_left;

		total_words_left-=words_to_send_block;

		//XLlFifo_iTxSetLen(FifoGTX_InstancePtr, words_to_send_block);


		while(words_to_send_block)
		{
			words_to_send=XLlFifo_iTxVacancy(FifoGTX_InstancePtr);

			if(words_to_send>words_to_send_block)
				words_to_send=words_to_send_block;

			words_to_send_block-=words_to_send;
			words_left_to_send=words_to_send;
			if(words_left_to_send)
			{
				do
				{
					//if(XLlFifo_iTxVacancy(FifoGTX_InstancePtr))
					{
						XLlFifo_TxPutWord(FifoGTX_InstancePtr,*temp_src++);
						words_left_to_send--;
					}
				}while(words_left_to_send);
				XLlFifo_iTxSetLen(FifoGTX_InstancePtr, words_to_send*4);
			}
			else
			{
				for(temp_volatile_reg=0;temp_volatile_reg<1000;temp_volatile_reg++); //espera un poco
			}

		}

	}

	/* Start Transmission by writing transmission length into the TLR */


	/* Check for Transmission completion */
	while( !(XLlFifo_IsTxDone(FifoGTX_InstancePtr)) ){

	}
	/* Transmission Complete */
	return XST_SUCCESS;
}

