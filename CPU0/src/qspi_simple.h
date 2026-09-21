/*
 * qspi_simple.h
 *
 *  Created on: 22/03/2016
 *      Author: csic
 */

#ifndef QSPI_SIMPLE_H_
#define QSPI_SIMPLE_H_


#include "xparameters.h"	/* SDK generated parameters */
#include "xqspips.h"		/* QSPI device driver */
#include "xil_printf.h"
/*
 * The following constants specify the page size, sector size, and number of
 * pages and sectors for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#if	(XPAR_PS7_QSPI_0_QSPI_MODE==2)
#define SECTOR_SIZE		0x20000
#define NUM_SECTORS		0x100
#define NUM_PAGES		0x10000
#define PAGE_SIZE		512
#else
#define SECTOR_SIZE		0x10000
#define NUM_SECTORS		0x100
#define NUM_PAGES		0x10000
#define PAGE_SIZE		256
#endif
/*
 * The following defines are for dual flash interface.
 */
#define LQSPI_CR_FAST_QUAD_READ		0x0000006B /* Fast Quad Read output */
#define LQSPI_CR_1_DUMMY_BYTE		0x00000100 /* 1 Dummy Byte between
						     address and return data */
#define DUAL_QSPI_CONFIG_WRITE		(XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 XQSPIPS_LQSPI_CR_SEP_BUS_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

#define DUAL_QSPI_CONFIG_QUAD_READ	(XQSPIPS_LQSPI_CR_LINEAR_MASK | \
					 XQSPIPS_LQSPI_CR_TWO_MEM_MASK | \
					 XQSPIPS_LQSPI_CR_SEP_BUS_MASK | \
					 LQSPI_CR_1_DUMMY_BYTE | \
					 LQSPI_CR_FAST_QUAD_READ)

/************************** Function Prototypes ******************************/

int InitQspi(void);

int FlashErase_simple(u32 address, u32 bytecount);

int FlashWrite_simple(u32 address, const u8* source, u32 bytecount);

int FlashRead_simple(u32 address, u8* dst, u32 bytecount);

int FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);

int FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

int FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

int FlashReadID(void);

int QspiFlashPolledExample(XQspiPs *QspiInstancePtr, u16 QspiDeviceId);


#endif /* QSPI_SIMPLE_H_ */
