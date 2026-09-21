/*
 * axil2stream_fifo_driver.c
 *
 *  Created on: 15 sept. 2020
 *      Author: Cruza
 */

#include "axil2stream_fifo_driver.h"
#include "xllfifo.h"
#include "xparameters.h"
#include "xstatus.h"

static XLlFifo FifoInstance;
static XLlFifo *InstancePtr=&FifoInstance;
static const u16 DeviceId = XPAR_AXI_FIFO_0_DEVICE_ID;


int axil2stream_fifo_init()
{
	XLlFifo_Config *Config;
	int Status;
	int i;
	int Error=0;
	Status = XST_SUCCESS;

	/* Initialize the Device Configuration Interface driver */
	Config = XLlFfio_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XLlFifo_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}

	/* Check for the Reset value */
	Status = XLlFifo_Status(InstancePtr);
	XLlFifo_IntClear(InstancePtr,0xffffffff);
	Status = XLlFifo_Status(InstancePtr);
	if(Status != 0x0) {
		xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
				"Expected : 0x0\n\r",
				XLlFifo_Status(InstancePtr));
		return XST_FAILURE;
	}
}

int axil2stream_fifo_send(u32  *SourceAddr, u32 Numwords)
{

	int i;
	int j;
	volatile register int temp_volatile_reg;
	//xil_printf(" Transmitting Buffer ... \r\n");
	u32 total_words_left = Numwords+0;//6692572 bytes + relleno, 0 en este caso
	u32 words_left_to_send,words_to_send,words_to_send_block;

	u32 MAX_WORDS_PER_BLOCK=(1<<(23-2))-1;
	u32  *temp_src=SourceAddr;
	//XLlFifo_TxReset(InstancePtr);

	while(total_words_left)
	{
		if(total_words_left>MAX_WORDS_PER_BLOCK)
			words_to_send_block=MAX_WORDS_PER_BLOCK;
		else
			words_to_send_block=total_words_left;

		total_words_left-=words_to_send_block;

		//XLlFifo_iTxSetLen(InstancePtr, words_to_send_block);


		while(words_to_send_block)
		{
			words_to_send=XLlFifo_iTxVacancy(InstancePtr);

			if(words_to_send>words_to_send_block)
				words_to_send=words_to_send_block;

			words_to_send_block-=words_to_send;
			words_left_to_send=words_to_send;
			if(words_left_to_send)
			{
				do
				{
					//if(XLlFifo_iTxVacancy(InstancePtr))
					{
						XLlFifo_TxPutWord(InstancePtr,*temp_src++);
						words_left_to_send--;
					}
				}while(words_left_to_send);
				XLlFifo_iTxSetLen(InstancePtr, words_to_send*4);
			}
			else
			{
				for(temp_volatile_reg=0;temp_volatile_reg<1000;temp_volatile_reg++); //espera un poco
			}

		}

	}

	/* Start Transmission by writing transmission length into the TLR */


	/* Check for Transmission completion */
	while( !(XLlFifo_IsTxDone(InstancePtr)) ){

	}
	/* Transmission Complete */
	return XST_SUCCESS;
}

int axil2stream_fifo_fill(u32  data, u32 Numwords)
{

	int i;
	int j;
	volatile register int temp_volatile_reg;
	u32 total_words_left = Numwords;
	u32 words_left_to_send,words_to_send,words_to_send_block;

	u32 MAX_WORDS_PER_BLOCK=(1<<(23-2))-1;


	while(total_words_left)
	{
		if(total_words_left>MAX_WORDS_PER_BLOCK)
			words_to_send_block=MAX_WORDS_PER_BLOCK;
		else
			words_to_send_block=total_words_left;

		total_words_left-=words_to_send_block;

		//XLlFifo_iTxSetLen(InstancePtr, words_to_send_block);


		while(words_to_send_block)
		{
			words_to_send=XLlFifo_iTxVacancy(InstancePtr);

			if(words_to_send>words_to_send_block)
				words_to_send=words_to_send_block;

			words_to_send_block-=words_to_send;
			words_left_to_send=words_to_send;
			if(words_left_to_send)
			{
				do
				{
					//if(XLlFifo_iTxVacancy(InstancePtr))
					{
						XLlFifo_TxPutWord(InstancePtr,data);
						words_left_to_send--;
					}
				}while(words_left_to_send);
				XLlFifo_iTxSetLen(InstancePtr, words_to_send*4);
			}
			else
			{
				for(temp_volatile_reg=0;temp_volatile_reg<1000;temp_volatile_reg++); //espera un poco
			}

		}

	}
	/* Start Transmission by writing transmission length into the TLR */


	/* Check for Transmission completion */
	while( !(XLlFifo_IsTxDone(InstancePtr)) ){

	}

	/* Transmission Complete */
	return XST_SUCCESS;
}

