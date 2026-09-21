/*
 * fir_filter.h
 *
 *  Created on: 1 sept. 2020
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_FILTRO_FIR_FIR_FILTER_H_
#define SRC_SITAU2_FILTRO_FIR_FIR_FILTER_H_

#include "xil_types.h"

#define FIR_NUM_COEF_SIMETRICOS	32

extern s16 filtro_unidad_casi[FIR_NUM_COEF_SIMETRICOS];
extern s16 filtro_barbaro[FIR_NUM_COEF_SIMETRICOS];
extern s16 hilbert_coef[FIR_NUM_COEF_SIMETRICOS];


int calc_interleaving(int dec_beamformer);

int config_interleaving(u8 interleaving,u8 video);

int config_FIR_coeficients(s16* coefficients);

int config_HIL_coeficients(s16* coefficients);

int config_FIR_coeficients_remote_get_commands(s16* coefficients,commnd_buffer_t *commnd_buf);

int config_FIR_interleaving_remote_get_commands(commnd_buffer_t *commnd_buf);

int config_FIR_bit_shift_remote_get_commands(u8 bit_shift,commnd_buffer_t *commnd_buf);



#endif /* SRC_SITAU2_FILTRO_FIR_FIR_FILTER_H_ */
