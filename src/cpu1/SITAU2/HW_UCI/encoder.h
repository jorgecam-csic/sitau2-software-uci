/*
 * encoder.h
 *
 *  Created on: 6 de jul. de 2017
 *      Author: csic
 */
// Documentación en registros encoder.xlsx y Documentación encoder.docx en el directorio raiz del IP del encoder.

#ifndef SRC_ENCODER_H_
#define SRC_ENCODER_H_

#include "xil_types.h"
#include "xparameters.h"

#define ENC_TRIG_OFFSET 	      0
#define ENC_POS_OFFSET		      1
#define ENC_HOLG_OFFSET          2
#define ENC_DEC_FACTOR_OFFSET    3
#define ENC_FIL_NUM_OFFSET		   4
#define ENC_DIR_CHG_CNT_OFFSET   5 // Contador de cambios de dirección
#define ENC_RST_DBG_OFFSET		   7
#define ENC_A_TRN_CNT_OFFSET		8 // Contador de transiciones canal A
#define ENC_B_TRN_CNT_OFFSET		9 // Contador de transiciones canal B
#define ENC_A_FIL_CNT_OFFSET		10 // Contador de glitches filtrados (máximo 1 por transición) en canal A
#define ENC_B_FIL_CNT_OFFSET		11 // Contador de glitches filtrados (máximo 1 por transición) en canal B

#define ENC_RESET_POS_MASK	0X80000000
#define ENC_TRIGGER_EN_MASK	0X80000000
#define ENC_ENABLE_MASK		0X40000000
#define	STEPS_ADD_REV_MASK	0x00000FFF
#define STEPS_ADD_REV_MAX	STEPS_ADD_REV_MASK

#define NUM_ENCODERS	4


s32 ENC_get_pos(u8 id_encoder);
u32 ENC_get_dir_changes(u8 id_encoder);
u32 ENC_get_channel_a_valid_edges(u8 id_encoder);
u32 ENC_get_channel_b_valid_edges(u8 id_encoder);
u32 ENC_get_channel_a_filtered_glitches(u8 id_encoder);
u32 ENC_get_channel_b_filtered_glitches(u8 id_encoder);
void ENC_reset_debug_counters(u8 id_encoder);
int ENC_set_pos_to_zero(u8 id_encoder);
int ENC_filter_config (u8 id_encoder, u32 filter_prescaler, u32 filter_samples);
int ENC_config(u8 id_encoder,u8 set_to_zero,u8 enable_count,u8 enable_trigger,u32 trig_steps,u32 reverse_clearance_control);

#endif /* SRC_ENCODER_H_ */
