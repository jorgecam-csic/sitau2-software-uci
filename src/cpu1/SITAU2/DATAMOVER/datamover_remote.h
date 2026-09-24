/*
 * datamover_remote.h
 *
 *  Created on: 3 abr. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_DATAMOVER_DATAMOVER_REMOTE_H_
#define SRC_SITAU2_DATAMOVER_DATAMOVER_REMOTE_H_

#include "datamover_structs.h"
#include "bcc_bussar.h"

status_t get_afe2mem_datamover_status(u8 minibase);
status_t get_mem2beamformer_datamover_status(u8 minibase);
status_t get_mem2emi_prom_beamformer_datamover_status(u8 minibase);
status_t get_beamformer2mem_datamover_status(u8 minibase);
status_t get_uci_buffer2minibase_datamover_status(u8 minibase);
status_t get_minibase2uci_buffer_datamover_status(u8 minibase);
u32 get_mem2beamformer_datamover_next_addr(u8 minibase);
u32 get_afe2mem_datamover_next_addr(u8 minibase);
u32 get_beamformer2mem_next_addr(u8 minibase);
u32 get_mem2bf_prom_next_addr(u8 minibase);

int mem2beamformer_inmediate_command(u32 btt,u32 autostart,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 sec_trigger,u8 circular_mem,u8 minibase);

int config_mem2beamformer_sec_trig_circular(u32 start_addr,u32 end_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 sec_trigger,u8 circular_mem,u8 minibase,commnd_buffer_t *commnd_buf);

int set_afe2mem_256bit_datamover_next_addr(u32 start_addr,u8 minibase);
int set_mem2beamformer_256bit_datamover_next_addr(u32 start_addr,u8 minibase);

int config_afe2mem(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 minibase,commnd_buffer_t *commnd_buf);
int config_mem2beamformer(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 minibase,commnd_buffer_t *commnd_buf);
int config_minibase2uci_buffer(u32 start_addr,u32 btt,u8 minibase,commnd_buffer_t *commnd_buf);
int config_beamformer2mem(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 minibase,commnd_buffer_t *commnd_buf,u8 writeonly0_readwrite1);
int config_mem2emi_prom_beamformer2mem(u32 res_addr,u32 res_btt,u32 res_autoinc,u32 scratch_addr,u32 scratch_btt,u32 scratch_autoinc,u8 minibase,commnd_buffer_t *commnd_buf);
int config_beamformer2mem_circular(u32 start_addr,u32 end_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 circular_mem,u8 minibase,commnd_buffer_t *commnd_buf,u8 writeonly0_readwrite1);
int config_remote_256bit_datamover_stream2mem(u32 start_addr,u32 end_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 circular_mem,u8 minibase,commnd_buffer_t *commnd_buf);
int config_remote_datamover32_disable_read(u8 minibase,commnd_buffer_t *commnd_buf);
int config_remote_datamover32_disable_write(u8 minibase,commnd_buffer_t *commnd_buf);
u32 get_mem2lvds_next_addr(u8 minibase);
u32 get_datamover2mem_next_addr(u8 minibase);
int config_mem2beamformer_sec_trig(u32 start_addr,u32 btt,u32 auto_inc,u8 external_trigger,u8 trig_now,u8 sec_trigger,u8 minibase,commnd_buffer_t *commnd_buf);
void BF0_datamover_remote_write_inmediate_MM2S_addr(u32 MM2S_remote_addr,u8 minibase);

#endif /* SRC_SITAU2_DATAMOVER_DATAMOVER_REMOTE_H_ */
