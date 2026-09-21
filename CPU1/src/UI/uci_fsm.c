/*
 * uci_fsm.c
 *
 *  Created on: 20 sept. 2022
 *      Author: csic
 */
#include "calc.h"
#include "uci_reg.h"
#include "uci_fsm.h"
#include "vch.h"
#include "ACQ_FMC.h"
#include "ACQ_PA.h"
#include "ACQ_TFM.h"
#include "trigger.h"
#include "xil_types.h"
#include "timestamp.h"

void UCIFSM_main(void)
{
	int enc_index,ch_index,result;
	u32 waited_trigs;
	int modo_seguro=0;
	float us_acq,us_gtx,us_tot;
	volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;

	switch(gb_sitau_status)
	{
		case ST_None:
			TRIG_reset_waited_triggers();
			break;
		case ST_ConfigAcquisition:
			waited_trigs=TRIG_get_waited_triggers();
			TRIG_reset_waited_triggers();
			//if (waited_trigs != 0)
			{
				if (gb_log_acquiring == 1) LOG_ACQUIRING("\r\nTRIGGER Full Parallel");
				for (enc_index=0; enc_index<UCI_N_ENCODER; enc_index++)
				{
					if (enc_index == gb_uci.encoder_trigger)
						gb_encoder_trigger_value[enc_index] = ENC_get_pos(enc_index) + gb_encoder_trigger_value_offset;
					else gb_encoder_trigger_value[enc_index] = ENC_get_pos(enc_index);
					gb_encoder_channel_a_valid_edges_counter[enc_index] = ENC_get_channel_a_valid_edges(enc_index);
					gb_encoder_channel_b_valid_edges_counter[enc_index] = ENC_get_channel_b_valid_edges(enc_index);
					gb_encoder_channel_a_filtered_glitches_counter[enc_index] = ENC_get_channel_a_filtered_glitches(enc_index);
					gb_encoder_channel_b_filtered_glitches_counter[enc_index] = ENC_get_channel_b_filtered_glitches(enc_index);
					gb_encoder_sign_changes_counter[enc_index] = ENC_get_dir_changes(enc_index);
				}

				if (gb_fp_virtual_channel[gb_uci.active_virtual_channel].enabled == 1)
				{
					if(gb_fp_virtual_channel[gb_uci.active_virtual_channel].hardware_set_up == 0)
					{
						switch(gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquisition_type)
						{
						case AQUISITION_TYPE_PA:
							PA_set_up(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]);
							break;
						case AQUISITION_TYPE_MC:
							ACQ_FMC_set_up(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]);
							break;
						case AQUISITION_TYPE_PA_MC:
							xil_printf("\n\rMODO PA FMC AUN NO SOPORTADO\n\r");
							break;
						case AQUISITION_TYPE_TFM:
							TFM_set_up(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]);
							break;
						default:
							xil_printf("\n\rMODO %d DESCONOCIDO\n\r",gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquisition_type);
							break;
						}
					}
					gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquiring = 1;
					gb_sitau_status = ST_SystemArmed;
					SHRD_CPU1_Status(gb_sitau_status);
				}

				//TRIG_set_interrupt_source(TRIGGER_PRFTIMER2_MASK);
				TRIG_reset_waited_triggers();
			}

			TRIG_reset_waited_triggers();
			//break;
		//case ST_SystemArmed:
			switch(gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquisition_type)
			{
			case AQUISITION_TYPE_PA:
				PA_start_trigger();
				break;
			case AQUISITION_TYPE_MC:
				ACQ_FMC_start_trigger();
				break;
			case AQUISITION_TYPE_PA_MC:
				xil_printf("\n\rMODO PA FMC AUN NO SOPORTADO\n\r");
				break;
			case AQUISITION_TYPE_TFM:
				TFM_FSM(); // Se manda a la FSM porque ella misma se encarga de iniciar
				break;
			default:
				xil_printf("\n\rMODO %d DESCONOCIDO\n\r",gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquisition_type);
				break;
			}
			gb_sitau_status = ST_TransferData;
			SHRD_CPU1_Status(gb_sitau_status);

			break;
		case ST_TransferData:
//			if(ACQ_end==0)
//				break;
//			else
//			{
//				if(gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquisition_type==AQUISITION_TYPE_PA)
//				{
//					gb_sitau_status = Beamforming;
//					break;
//				}
//			}
			if (gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquiring == 1)
			{
				switch(gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquisition_type)
				{
				case AQUISITION_TYPE_PA:
					PA_transfer(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]);
					break;
				case AQUISITION_TYPE_MC:
					FMC_transfer(&gb_fp_virtual_channel[gb_uci.active_virtual_channel]);
					break;
				case AQUISITION_TYPE_PA_MC:
					xil_printf("\n\rMODO PA FMC AUN NO SOPORTADO\n\r");
					break;
				case AQUISITION_TYPE_TFM:
					TFM_FSM();
					break;
				default:
					xil_printf("\n\rMODO %d DESCONOCIDO\n\r",gb_fp_virtual_channel[gb_uci.active_virtual_channel].acquisition_type);
					break;
				}
			}
			else
			{
				u64 tamanio_total_imagen,tamanio_total_adquisicion;
				u32 TAMANIO_CABECERA=32;
				u32 izquierda,centro,derecha;

				gb_sitau_status = ST_WaitStop;
				SHRD_CPU1_Status(gb_sitau_status);

				tamanio_total_imagen=(u64)((gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_bases*gb_fp_virtual_channel[gb_uci.active_virtual_channel].afe.AFE_BUSSAR_UT.num_samples*32*2)*gb_fp_virtual_channel[gb_uci.active_virtual_channel].n_efl)+TAMANIO_CABECERA;
				tamanio_total_adquisicion=tamanio_total_imagen*gb_uci.ind_acquisitions;

				convert_ulong_to_int(tamanio_total_adquisicion,&izquierda,&centro,&derecha);

				us_acq=timestamp_get_us_float(Timstamp_ini, Timstamp_acq);
				us_gtx=timestamp_get_us_float(Timstamp_acq, Timstamp_end);
				us_tot=timestamp_get_us_float(Timstamp_ini, Timstamp_end);

				LOG_ACQUIRING("\n\r");
				LOG_ACQUIRING("Tiempo ACQ: %d us\n\r",(int)us_acq);
				LOG_ACQUIRING("Tiempo GTX: %d us\n\r",(int)us_gtx);
				LOG_ACQUIRING("Tiempo TOT: %d us\n\r",(int)us_tot);
				LOG_ACQUIRING("Número de disparos %d, tiempo entre disparos: %d us\n\r",ACQ->n_pros,(int)us_acq/(ACQ->n_pros));
				LOG_ACQUIRING("Tamaño total de la imagen %lu Bytes\n\r",tamanio_total_imagen);
				LOG_ACQUIRING("Tamaño total de la adquisicion, %d%09d%09d Bytes\n\r",izquierda,centro,derecha);
				LOG_ACQUIRING("Tamaño total de la adquisicion, %d MB\n\r",tamanio_total_adquisicion,(int)tamanio_total_adquisicion/1024.0/1024.0);
				LOG_ACQUIRING("Velocidad media de transferencia (tiempo ACQ + GTX) MB/s %d\n\r",(int)(((float)tamanio_total_adquisicion)/us_tot));
				LOG_ACQUIRING("Velocidad media de transferencia (solo tiempo  GTX) MB/s %d\n\r",(int)(((float)tamanio_total_adquisicion)/us_gtx));
				LOG_ACQUIRING("\n\r");
			}

			break;
		case ST_WaitStop:
			xil_printf("\r\nWaiting Stop...");
			UCI_Stop();
			//UCI_Send_MSG_STATUS();
			break;
	}
}


