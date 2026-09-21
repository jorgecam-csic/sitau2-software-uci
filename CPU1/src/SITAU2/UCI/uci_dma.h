#ifndef uci_dmaH
#define uci_dmaH

#include "mcbcc_mst_driver.h"
#include "sleep.h"
#include "global.h"
#include "timer_util.h"
#include "datamover_driver.h"
#include "ttimer.h"
#include "cons_prod_util.h"
#include "log.h"
#include "xil_types.h"



#ifdef CODIGO_A_ELIMINAR
int DMA_Set_Datamover_Write16(int n_words_16_image, int *n_buffer_data);
int DMA_Write16(u16);
int DMA_Write16_from_Amplia_wait_end(float);
int DMA_Prepare_Write16_from_Amplia(int ndatos);

#endif //CODIGO_A_ELIMINAR

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================
int DMA_init(datamov_instance_t *datamover_instance_pointer);

int DMA_Datamover_wait_until_end();
int DMA_close_and_send_to_host(float us_timeout);
u32 DMA_Get_bytes_available(void);
int DMA_Set_Datamover_Write32(u32 n_data, u32 *n_buffer_data);

int DMA_Write32(u32 data);
int DMA_Prepare_Write32_from_BCC_after_n_words(u32 n_data,u32 n_words_to_wait);
int DMA_Prepare_Write32_from_BCC_after_proc(u32 n_data);
int DMA_Write32_from_BCC_wait_end(float us_timeout);
int DMA_memcpy_remote_base_int_no_block(u32 n_data,u8 minibase,u32 remote_addr);
int DMA_memcpy_remote_base_int_no_block_last_ctrl(u32 n_data,u8 minibase,u32 remote_addr,u8 last_behavior);
int DMA_asymmetrical_memcpy_remote_base_int_no_block(u32 n_data_DMA,u32 n_data_remote,u8 minibase,u32 remote_addr);
int DMA_asymmetrical_memcpy_remote_base_int_no_block_last_ctrl(u32 n_data_DMA,u32 n_data_remote,u8 minibase,u32 remote_addr,u8 last_behavior);
int DMA_asymmetrical_memcpy_remote_base_int_no_block_sum(u32 n_data_DMA,u32 n_data_remote,u32 remote_addr);
int DMA_asymmetrical_memcpy_remote_base_int_no_block_sum_last_ctrl(u32 n_data_DMA,u32 n_data_remote,u32 remote_addr,u8 last_behavior);

#endif

