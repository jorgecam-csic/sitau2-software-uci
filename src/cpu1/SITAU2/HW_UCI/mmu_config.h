/*
 * mmu_config.h
 *
 *  Created on: 20 de ene. de 2017
 *      Author: csic
 */

#ifndef SRC_MMU_CONFIG_H_
#define SRC_MMU_CONFIG_H_

#include "xil_types.h"
//#include "xil_mmu.h"
#include "shared_mem_def.h"

#define MMU_TLB_AP_FULL_ACCESS			0x00000C00		//Full access:	TLB[15]:APX=0	TLB[11]:AP1=1 TLB[10]:AP0=1
#define MMU_TLB_AP_READ_ONLY			0x00008800		//Read only:	TLB[15]:APX=1	TLB[11]:AP1=1 TLB[10]:AP0=0
#define MMU_TLB_AP_NO_ACCESS			0x00000000		//No access:	TLB[15]:APX=0	TLB[11]:AP1=0 TLB[10]:AP0=0

#define MMU_TLB_TEXCB_NO_CACHEABLE		0x00004000		//Non-cacheable:					TEX=100 c=0 B=0
#define MMU_TLB_TEXCB_WR_BACK_WR_ALLOC	0x00005004		//Write-back, write-allocate:		TEX=101 c=0 B=1
#define MMU_TLB_TEXCB_WR_THRU_WR_NALLOC	0x00006008		//Write-through, no write-allocate:	TEX=110 c=1 B=0
#define MMU_TLB_TEXCB_WR_BACK_WR_NALLOC	0x0000700C		//Write-back, no write-allocate:	TEX=111 c=1 B=0

#define MMU_TLB_SHAREABLE				0x00010000		//No access:	TLB[15]:APX=0	TLB[11]:AP1=0 TLB[10]:AP0=0

#define MMU_TLB_DOMAIN_MASK				0x000000F0		//No access:	TLB[15]:APX=0	TLB[11]:AP1=0 TLB[10]:AP0=0

void config_mmu(u32 start_address,u32 size,u32 atribbutes);
void cpu0_net_mmu_config(void);
void cpu1_uci_mmu_config(void);
void cpu1_uci_mmu_config_all_cacheable(void);
void cpu0_net_mmu_config_all_cacheable(void);


#endif /* SRC_MMU_CONFIG_H_ */
