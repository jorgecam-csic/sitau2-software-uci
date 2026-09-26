/*
 * qspi_simple.c
 *
 *  Created on: 22/03/2016
 *      Author: csic
 */
#include "qspi_simple.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPI_DEVICE_ID		XPAR_PS7_QSPI_0_DEVICE_ID

/*
 * The following constants define the commands which may be sent to the FLASH
 * device.
 */
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD			0x02
#define READ_CMD			0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID				0x9F

/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* FLASH instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define DATA_OFFSET		4 /* Start of Data for Read/Write */
#define DUMMY_OFFSET		4 /* Dummy byte offset for fast, dual and quad
				     reads */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
				     quad reads */
#define RD_ID_SIZE		4 /* Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1 /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4 /* Sector Erase command + Sector address */

/*
 * The following constants specify the extra bytes which are sent to the
 * FLASH on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

#if (XPAR_PS7_QSPI_0_QSPI_MODE==2)
#define ADDRESS_SCALE	2
#else
#define ADDRESS_SCALE	1
#endif


/************************** Variable Definitions *****************************/

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XQspiPs QspiInstance;

/*
 * The following variable allows a test value to be added to the values that
 * are written to the FLASH such that unique values can be generated to
 * guarantee the writes to the FLASH were successful
 */
int Test = 5;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuffer[PAGE_SIZE + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[PAGE_SIZE + DATA_OFFSET];

XQspiPs *QspiInstancePtr;

void print_inicio(void)
{
	#define NLIN 64
	char cadena[NLIN*0x10+1];
	int len_cad,i,j;
	len_cad=NLIN*0x10;
	FlashRead_simple(0x00000,cadena,len_cad);
	cadena[len_cad]='\0';
	xil_printf("\r\n\r\n");
	for(i=0;i<NLIN;i++)
	{
		for(j=0;j<0x10;j++)
		{
			xil_printf("%02X ",cadena[i*0x10+j]);
			if(!((j+1)%4)) xil_printf(" ");
		}
		xil_printf("\r\n");
	}
}

int InitQspi(void)
{
	int Status;
	XQspiPs_Config *QspiConfig;
	u16 QspiDeviceId=QSPI_DEVICE_ID;

	QspiInstancePtr=&QspiInstance;
	/*
	 * Initialize the QSPI driver so that it's ready to use
	 */
	QspiConfig = XQspiPs_LookupConfig(QspiDeviceId);
	if (NULL == QspiConfig)
	{
		return XST_FAILURE;
	}

	Status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig,	QspiConfig->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
#if	(XPAR_PS7_QSPI_0_QSPI_MODE==2)
	/*
	 * Enable two flash memories on seperate buses
	 */
	XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_QSPI_CONFIG_WRITE);

	/*
	 * Set the QSPI device as a master and enable manual CS, manual start
	 * and flash interface mode options and drive HOLD_B pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr,  XQSPIPS_FORCE_SSELECT_OPTION |
					    XQSPIPS_MANUAL_START_OPTION |
					    XQSPIPS_HOLD_B_DRIVE_OPTION);

#endif

	/*
	 * Set Manual Start and Manual Chip select options and drive HOLD_B
	 * pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_MANUAL_START_OPTION |
			XQSPIPS_FORCE_SSELECT_OPTION |
			XQSPIPS_HOLD_B_DRIVE_OPTION);

	/*
	 * Set the prescaler for QSPI clock
	 */
	XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);

	/*
	 * Assert the FLASH chip select.
	 */
	XQspiPs_SetSlaveSelect(QspiInstancePtr);
/*
	xil_printf("SECTOR_SIZE: %0X\r\n",SECTOR_SIZE);
	xil_printf("NUM_SECTORS: %0X\r\n",NUM_SECTORS);
	xil_printf("NUM_PAGES:   %0X\r\n",NUM_PAGES);
	xil_printf("PAGE_SIZE:   %0X\r\n",PAGE_SIZE);
	print_inicio();*/
	return XST_SUCCESS;
}

int FlashErase_simple(u32 address, u32 bytecount)
{
	int ret;
	ret=FlashErase(QspiInstancePtr, address, bytecount);
	return ret;
}

int FlashWrite_simple(u32 address, const u8* source, u32 bytecount)
{
	int ret;
	u32 bytes_left,bytes,address_align;
	int primera=1,segunda=0;
	bytes_left=bytecount;

	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	if(address%PAGE_SIZE)
	{
		address_align=(((u32)(address/PAGE_SIZE))*PAGE_SIZE)+1;
		bytes=(address_align-address);
		memcpy(&WriteBuffer[DATA_OFFSET],source,bytes);
		source+=bytes;
		ret=FlashWrite(QspiInstancePtr, address/ADDRESS_SCALE, bytes, WRITE_CMD);
		bytes_left-=bytes;
	}
	else
		address_align=address;

	while(bytes_left!=0)
	{
		bytes=(bytes_left<PAGE_SIZE?bytes_left:PAGE_SIZE);
		memcpy(&WriteBuffer[DATA_OFFSET],source,bytes);
		source+=bytes;
		do{
			ret=FlashWrite(QspiInstancePtr, address_align/ADDRESS_SCALE, bytes, WRITE_CMD);
			if(ret) xil_printf("Error escribiendo en QSPI, intentando de nuevo...");
		}while(ret!=0);
		address_align+=PAGE_SIZE;
		bytes_left-=bytes;
	}

	return 0;

}


int FlashRead_simple(u32 address, u8* dst, u32 bytecount)
{
	int ret;
	u32 bytes_left,bytes,address_align;
	bytes_left=bytecount;
	/*
	 * Write the data in the write buffer to the serial FLASH a page at a
	 * time, starting from TEST_ADDRESS
	 */
	if(address%PAGE_SIZE)
	{
		address_align=(((u32)(address/PAGE_SIZE))*PAGE_SIZE)+1;
		bytes=(address_align-address);
		ret=FlashRead(QspiInstancePtr, address/ADDRESS_SCALE, bytes, FAST_READ_CMD);
		memcpy(dst,&ReadBuffer[DATA_OFFSET + DUMMY_SIZE],bytes);
		dst+=bytes;
		bytes_left-=bytes;
	}
	else
		address_align=address;

	while(bytes_left!=0)
	{
		bytes=(bytes_left<PAGE_SIZE?bytes_left:PAGE_SIZE);
		bytes_left-=bytes;
		ret=FlashRead(QspiInstancePtr, address_align/ADDRESS_SCALE, bytes, FAST_READ_CMD);
		memcpy(dst,&ReadBuffer[DATA_OFFSET + DUMMY_SIZE],bytes);
		dst+=bytes;
		address_align+=PAGE_SIZE;
	}
	return ret;

}

/******************************************************************************
*
*
* This function writes to the  serial FLASH connected to the QSPI interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to write data to in the FLASH.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPI
*		device supports only Page Program command to write data to the
*		flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	int ret;
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];
#if	(XPAR_PS7_QSPI_0_QSPI_MODE==2)
	/*
	 * Enable two flash memories on seperate buses
	 */
	XQspiPs_SetLqspiConfigReg(QspiInstancePtr, DUAL_QSPI_CONFIG_WRITE);

	/*
	 * Set the QSPI device as a master and enable manual CS, manual start
	 * and flash interface mode options and drive HOLD_B pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr,  XQSPIPS_FORCE_SSELECT_OPTION |
					    XQSPIPS_MANUAL_START_OPTION |
					    XQSPIPS_HOLD_B_DRIVE_OPTION);

#endif
	/*
	 * Send the write enable command to the FLASH so that it can be
	 * written to, this needs to be sent as a seperate transfer before
	 * the write
	 */
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));


	/*
	 * Setup the write command with the specified address and data for the
	 * FLASH
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	/*
	 * Send the write command, address, and data to the FLASH to be
	 * written, no receive buffer is specified since there is nothing to
	 * receive
	 */
	XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
				ByteCount + OVERHEAD_SIZE);

	/*
	 * Wait for the write command to the FLASH to be completed, it takes
	 * some time for the data to be written
	 */
	while (1) {
		/*
		 * Poll the status register of the FLASH to determine when it
		 * completes, by sending a read status command and receiving the
		 * status byte
		 */
		ret=XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the
		 * device and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being
		 * read
		 */
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
	return ret;
}

/******************************************************************************
*
* This function reads from the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to read data from in the FLASH.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash. QSPI
*		device supports one of the Read, Fast Read, Dual Read and Fast
*		Read commands to read data from the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	int ret;

	/*
	 * Setup the write command with the specified address and data for the
	 * FLASH
	 */
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
	    (Command == QUAD_READ_CMD)) {
		ByteCount += DUMMY_SIZE;
	}
	/*
	 * Send the read command to the FLASH to read the specified number
	 * of bytes from the FLASH, send the read command and address and
	 * receive the specified number of bytes of data in the data buffer
	 */
	ret=XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, ReadBuffer,
				ByteCount + OVERHEAD_SIZE);
	return ret;
}



/******************************************************************************
*
*
* This function erases the sectors in the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
int FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];
	int Sector,ret=0;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command
	 */
	if (ByteCount == (NUM_SECTORS * SECTOR_SIZE)) {
		/*
		 * Send the write enable command to the FLASH so that it can be
		 * written to, this needs to be sent as a seperate transfer
		 * before the erase
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		/*
		 * Setup the bulk erase command
		 */
		WriteBuffer[COMMAND_OFFSET]   = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					BULK_ERASE_SIZE);

		/*
		 * Wait for the erase command to the FLASH to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			ret=XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting; if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		return ret;
	}

	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */
	for (Sector = 0; Sector < ((ByteCount / SECTOR_SIZE) + 1); Sector++) {
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a seperate transfer
		 * before the write
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and data
		 * for the FLASH
		 */
		WriteBuffer[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)(Address >> 16);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)(Address >> 8);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					SEC_ERASE_SIZE);

		/*
		 * Wait for the sector erse command to the FLASH to be completed
		 */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read
			 */
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		Address += SECTOR_SIZE;
	}
	return ret;
}

/******************************************************************************
*
* This function reads serial FLASH ID connected to the SPI interface.
*
* @param	None.
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashReadID(void)
{
	int Status;

	/*
	 * Read ID in Auto mode.
	 */
	WriteBuffer[COMMAND_OFFSET]   = READ_ID;
	WriteBuffer[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
	WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(&QspiInstance, WriteBuffer, ReadBuffer,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("FlashID=0x%x 0x%x 0x%x\r\n", ReadBuffer[1], ReadBuffer[2],
		   ReadBuffer[3]);

	return XST_SUCCESS;
}
