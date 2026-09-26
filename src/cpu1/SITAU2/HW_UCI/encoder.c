/*
 * encoder.c
 *
 *  Created on: 6 de jul. de 2017
 *      Author: csic
 */

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "encoder.h"
#include "uci_error_code.h"
#include "log.h"

// ____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ============================================================================

u32* Encoders_base[NUM_ENCODERS]={	(u32*)XPAR_ENCODER_TOP_AXILITE_1_BASEADDR,
									(u32*)XPAR_ENCODER_TOP_AXILITE_2_BASEADDR,
									(u32*)XPAR_ENCODER_TOP_AXILITE_3_BASEADDR,
									(u32*)XPAR_ENCODER_TOP_AXILITE_4_BASEADDR};

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ENC_set_pos_to_zero(u8 id_encoder)
{
u32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = Encoders_base[id_encoder];
	enc_base[ENC_POS_OFFSET] = ENC_RESET_POS_MASK;
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ENC_config (  u8 id_encoder,
                  u8 set_to_zero,
                  u8 enable_count,
                  u8 enable_trigger,
                  u32 trig_steps,
                  u32 reverse_clearance_control)
{
u32* enc_base;
u32 temp;

   if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	if ((trig_steps + reverse_clearance_control) > STEPS_ADD_REV_MAX) 
         return ELOG(charEUCI_Parameter, EUCI_Parameter);

	enc_base = Encoders_base[id_encoder];
	temp = 0;
	if (enable_count) temp |= ENC_ENABLE_MASK;
	if (enable_trigger) temp |= ENC_TRIGGER_EN_MASK;
	temp |= trig_steps;
	enc_base[ENC_TRIG_OFFSET] = temp;
	enc_base[ENC_HOLG_OFFSET] = reverse_clearance_control;

	if (set_to_zero) enc_base[ENC_POS_OFFSET]=ENC_RESET_POS_MASK;
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int ENC_filter_config (u8 id_encoder, u32 filter_prescaler, u32 filter_samples)
{
u32* enc_base;

   if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = Encoders_base[id_encoder];
	enc_base[ENC_DEC_FACTOR_OFFSET] = filter_prescaler;
	enc_base[ENC_FIL_NUM_OFFSET] = filter_samples;
	return 0;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
s32 ENC_get_pos(u8 id_encoder)
{
s32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = (s32*)Encoders_base[id_encoder];
	return enc_base[ENC_POS_OFFSET];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
u32 ENC_get_dir_changes(u8 id_encoder)
{
u32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = (u32*)Encoders_base[id_encoder];
	return enc_base[ENC_DIR_CHG_CNT_OFFSET];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
u32 ENC_get_channel_a_valid_edges(u8 id_encoder)
{
u32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = (u32*)Encoders_base[id_encoder];
	return enc_base[ENC_A_TRN_CNT_OFFSET];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
u32 ENC_get_channel_b_valid_edges(u8 id_encoder)
{
u32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = (u32*)Encoders_base[id_encoder];
	return enc_base[ENC_B_TRN_CNT_OFFSET];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
u32 ENC_get_channel_a_filtered_glitches(u8 id_encoder)
{
u32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = (u32*)Encoders_base[id_encoder];
	return enc_base[ENC_A_FIL_CNT_OFFSET];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
u32 ENC_get_channel_b_filtered_glitches(u8 id_encoder)
{
u32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = (u32*)Encoders_base[id_encoder];
	return enc_base[ENC_B_FIL_CNT_OFFSET];
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
void ENC_reset_debug_counters(u8 id_encoder)
{
u32* enc_base;

	if (id_encoder >= NUM_ENCODERS) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
	enc_base = (u32*)Encoders_base[id_encoder];
   enc_base[ENC_RST_DBG_OFFSET] = 1;
}
