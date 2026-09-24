/*
 * shared_mem_def.h
 *
 *  Created on: 17/11/2016
 *      Author: csic
 */

#ifndef SHARED_MEM_DEF_H_
#define SHARED_MEM_DEF_H_
#include "cons_prod_util.h"
#include "xil_types.h"
#include "version_utils.h"
#include "shared_mem.h"

// CPU0	256MB
#define CPU0_C_START		0x00100000
#define CPU0_C_SIZE			0x0FF00000
// MEMORIA COMPARTIDA PARA BUFFERS	256MB
#define SHARED_MEM_START	0x10000000
#define SHARED_MEM_SIZE		0x08000000
// CPU1	128 MB
#define CPU1_C_START		0x18000000
#define CPU1_C_SIZE			0x08000000
// BUFFER DE IMAGENES	512MB
#define IMAGE_BUF_START		0x20000000
#define IMAGE_BUF_SIZE		0x20000000

#define HOST2UCI_BUF_START	(SHARED_MEM_START)
#define	HOST2UCI_BUF_SIZE	0x02000000

#define UCI2HOST_BUF_START	(HOST2UCI_BUF_START+HOST2UCI_BUF_SIZE)
#define	UCI2HOST_BUF_SIZE	0x02000000

#define LAST_SHRD_MB_SIZE	(1024*1024)
#define LAST_SHRD_MB_START	(SHARED_MEM_START+SHARED_MEM_SIZE-LAST_SHRD_MB_SIZE)

#define LAST_SHRD_KB_SIZE	(1024)
#define LAST_SHRD_KB_START	(SHARED_MEM_START+SHARED_MEM_SIZE-LAST_SHRD_KB_SIZE)

// ____________________________________________________________________________
// ----- SHARED MEMORY

#define SHRD_MEM_INFO_BYTES		(4)
#define SHRD_MEM_HEADER			LAST_SHRD_KB_START
#define SHRD_MEM				(SHRD_MEM_HEADER + SHRD_MEM_INFO_BYTES)

//// ----------------------------------------------------------------------------

#define CONST_PROD_S_START	LAST_SHRD_MB_START
#define CONST_PROD_S_SIZE	(SIZE_PER_STRUCT_ALIGN*MAX_CONS_PROD_STRUCTS)

#endif /* SHARED_MEM_DEF_H_ */
