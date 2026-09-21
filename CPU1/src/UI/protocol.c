	// -----------------------------------------------------------------------------
/**
@file protocol.c

@brief Este archivo contiene la implementaciï¿½n de las funciones de la clase \c FL_Aperture.<br>

Esta clase tienen la funcionalidad necesaria para definir las aperturas que componen
un barrido.

@author (rg) Ricardo Gonzï¿½lez

<pre>
MODIFICATION HISTORY:

Ver   Who  Date       Changes
----- ---- ---------- -----------------------------------------------------------
1.00a (rg) 16/01/2017 First release
</pre>
*/

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

#include "protocol.h"
#include "log.h"
#include "uci_set.h"
#include "xparameters.h"
#include "xllfifo.h"
#include "bcc_bussar.h"
#include "ttimer.h"
#include "TLV5626.h"
#include "switch_driver.h"
#include "axil2stream_fifo_driver.h"
#include "uci_error_code.h"
#include "version_utils.h"
#include "vch.h"
#include "uci_reg.h"
#include "ACQ_FMC.h"
#include "trigger.h"
#include "shared_mem.h"

u32 datosFPGA[1500000];

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================

// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del n�mero de datos de la trama.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
unsigned short ChecksumFletcher16(unsigned char *data, int n_bytes)
{
unsigned char sum1 = 0;
unsigned char sum2 = 0;
int index;

//   for (index = 0; index < n_bytes; ++index)
//   {
//      sum1 = sum1 + (uint8_t)data[index];
//      sum2 = sum2 + sum1;
//   }
   return 0;//(sum2 << 8) | sum1;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_VIRTUAL_CHANNEL_BEAMFORMER(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_Beamformer))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_Beamformer));
		cons_refresh_read (	HOST2UCI_cons_p,cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF) 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VIRTUAL_CHANNEL_BEAMFORMER: %d", sizeof(T_AMPLIACommand)*2);
		}
		else 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VIRTUAL_CHANNEL_BEAMFORMER: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_Beamformer);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_Beamformer((TMSG_Beamformer *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_BeamformerExtended))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_BeamformerExtended));
		cons_refresh_read (	HOST2UCI_cons_p,cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF)
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT: %d", sizeof(T_AMPLIACommand)*2);
		}
		else
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_BeamformerExtended);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_Beamformer((TMSG_BeamformerExtended *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_TFM_FMC_FORWARD(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_TFM_FMC_FORWARD (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_TFM_FMC_Forward))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_TFM_FMC_FORWARD (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_TFM_FMC_Forward));
		cons_refresh_read (	HOST2UCI_cons_p,cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_TFM_FMC_FORWARD (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF)
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_TFM_FMC_FORWARD: %d", sizeof(T_AMPLIACommand)*2);
		}
		else
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_TFM_FMC_FORWARD: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_TFM_FMC_Forward);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_TFM_FMC_Forward((TMSG_TFM_FMC_Forward *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_TFM_FMC_FORWARD (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_TFM_FMC_FORWARD (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_TFM_PWI_FORWARD(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_TFM_PWI_FORWARD (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_TFM_PWI_Forward))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_TFM_PWI_FORWARD (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_TFM_PWI_Forward));
		cons_refresh_read (	HOST2UCI_cons_p,cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_TFM_PWI_FORWARD (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF)
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_TFM_PWI_FORWARD: %d", sizeof(T_AMPLIACommand)*2);
		}
		else
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_TFM_PWI_FORWARD: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_TFM_PWI_Forward);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_TFM_PWI_Forward((TMSG_TFM_PWI_Forward *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_TFM_PWI_FORWARD (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_TFM_PWI_FORWARD (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n de los registros de una BASE, y configura
el hardware correspondiente.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_PULSE_AMPLITUDE(T_AMPLIACommand command)
{
T_MSG_PulseAmplitude msg;
u32 n_data, n_bytes_ref = 0;
int result;
   
	if (command.BIT.DAT != sizeof(T_MSG_PulseAmplitude))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_PULSE_AMPLITUDE != %d)",
            command.BIT.DAT, 
            sizeof(T_MSG_PulseAmplitude));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_PULSE_AMPLITUDE");
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_PULSE_AMPLITUDE: %d", sizeof(T_AMPLIACommand));
      n_bytes_ref = sizeof(T_MSG_PulseAmplitude);
      cons_memcpy(HOST2UCI_cons_p, n_bytes_ref, (unsigned char *)&msg);

      if ((result = UCI_SetPulseAmplitude(msg.data)) < 0) {RLOG(result);}
   }
   else
   {
	   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_PULSE_AMPLITUDE (%d < %d)",
			   n_data,
			   command.BIT.DAT);
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_PRINT_VCH_CONFIG(T_AMPLIACommand command)
{
	int i;

	if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_PRINT_VCH_CONFIG");
	for (i=0; i<MAX_VIRTUAL_CHANNELS; i++)
	{
		VCH_PrintConfig(&gb_fp_virtual_channel[i]);
	}
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n del evento de disparo.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];
   
	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {         
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0 (Auxiliar Command)");
         return 0;
      }         
   }
   else {size_msg = command.BIT.DAT;}
   
	if (size_msg != (unsigned long)sizeof(TMSG_Config_v0))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0 (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_Config_v0));
		cons_refresh_read (	HOST2UCI_cons_p,
									cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0 (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF) 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0: %d", sizeof(T_AMPLIACommand)*2);
		}
		else 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_Config_v0);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_Config_v0((TMSG_Config_v0 *)ptr_msg)) < 0)
			{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0 (ptr_msg = NULL)");
   }
   else 
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0 (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n del evento de disparo.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v1(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];
   
	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {         
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (Auxiliar Command)");
         return 0;
      }         
   }
   else {size_msg = command.BIT.DAT;}
   
	if (size_msg != (unsigned long)sizeof(TMSG_Config_v1))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_Config_v1));
		cons_refresh_read (HOST2UCI_cons_p, cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF) 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG: %d", sizeof(T_AMPLIACommand)*2);
		}
		else 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_Config_v1);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_Config_v1((TMSG_Config_v1 *)ptr_msg)) < 0)
			{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (ptr_msg = NULL)");
   }
   else 
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n del evento de disparo.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2 (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_Config_v2))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2 (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_Config_v2));
		cons_refresh_read (HOST2UCI_cons_p, cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2 (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF)
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2: %d", sizeof(T_AMPLIACommand)*2);
		}
		else
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_Config_v2);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_Config((TMSG_Config_v2 *)ptr_msg)) < 0)
			{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2 (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2 (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n del evento de disparo.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3 (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_Config_v3))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3 (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_Config_v3));
		cons_refresh_read (HOST2UCI_cons_p, cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3 (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF)
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3: %d", sizeof(T_AMPLIACommand)*2);
		}
		else
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_Config_v3);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_Config((TMSG_Config_v3 *)ptr_msg)) < 0)
			{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3 (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3 (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n del evento de disparo.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_Config))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_Config));
		cons_refresh_read (HOST2UCI_cons_p, cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF)
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG: %d", sizeof(T_AMPLIACommand)*2);
		}
		else
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_CONFIG: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_Config);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_Config((TMSG_Config *)ptr_msg)) < 0)
			{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_CONFIG (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n del evento de disparo.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_FIR(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];
   
	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {         
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_FIR (Auxiliar Command)");
         return 0;
      }         
   }
   else {size_msg = command.BIT.DAT;}
   
	if (size_msg != (unsigned long)sizeof(TMSG_FIR))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_FIR (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_FIR));
		cons_refresh_read (	HOST2UCI_cons_p,
									cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_FIR (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF) 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_FIR: %d", sizeof(T_AMPLIACommand)*2);
		}
		else 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_FIR: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_FIR);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_FIR((TMSG_FIR *)ptr_msg)) < 0)
			{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_FIR (ptr_msg = NULL)");
   }
   else 
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_FIR (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_ACQUIRE(T_AMPLIACommand command)
{
   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_ACQUIRE (%d)", command.BIT.DAT);
   if (gb_sitau_status == ST_None)
   {
      gb_uci.n_acquisitions = command.BIT.DAT;
      gb_sitau_status = ST_Trigger;
      SHRD_CPU1_Status(gb_sitau_status);
   }
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_VCH_EMISSION_FOCAL_LAW(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VCH_EMISSION_FOCAL_LAW (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_EmissionFocalLaws))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VCH_EMISSION_FOCAL_LAW (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_EmissionFocalLaws));
		cons_refresh_read (	HOST2UCI_cons_p,cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VCH_EMISSION_FOCAL_LAW (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF) 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VCH_EMISSION_FOCAL_LAW: %d", sizeof(T_AMPLIACommand)*2);
		}
		else 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VCH_EMISSION_FOCAL_LAW: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_EmissionFocalLaws);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_EmissionFocalLaws((TMSG_EmissionFocalLaws *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VCH_EMISSION_FOCAL_LAW (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VCH_EMISSION_FOCAL_LAW (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_VCH_EMISSION_FOCAL_LAW_EXT(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VCH_EMISSION_FOCAL_LAW_EXT (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_EmissionFocalLawsExtended))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VCH_EMISSION_FOCAL_LAW_EXT (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_EmissionFocalLawsExtended));
		cons_refresh_read (	HOST2UCI_cons_p,cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VCH_EMISSION_FOCAL_LAW_EXT (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF)
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VCH_EMISSION_FOCAL_LAW_EXT: %d", sizeof(T_AMPLIACommand)*2);
		}
		else
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_VCH_EMISSION_FOCAL_LAW_EXT: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_EmissionFocalLawsExtended);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_EmissionFocalLawsExtended((TMSG_EmissionFocalLawsExtended *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VCH_EMISSION_FOCAL_LAW_EXT (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_VCH_EMISSION_FOCAL_LAW_EXT (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraciï¿½n del evento de disparo.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_TGC(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_TGC (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_TGC))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_TGC (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_TGC));
		cons_refresh_read (	HOST2UCI_cons_p,
									cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_TGC (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF) 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_TGC: %d", sizeof(T_AMPLIACommand)*2);
		}
		else 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_VIRTUAL_CHANNEL_TGC: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_TGC);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = VCH_Set_TGC((TMSG_TGC *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_VIRTUAL_CHANNEL_TGC (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_VIRTUAL_CHANNEL_TGC (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n del evento de disparo.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v0(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;

	if (command.BIT.DAT != sizeof(TMSG_TriggerSource_v0))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v0 (%d != %d)", command.BIT.DAT, sizeof(TMSG_TriggerSource_v0));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v0 (%d)", command.BIT.DAT);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_TRIGGER_SOURCE_v0: %d", sizeof(T_AMPLIACommand));
      n_bytes_ref = sizeof(TMSG_TriggerSource_v0);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = UCI_Set_TriggerSource_v0((TMSG_TriggerSource_v0 *)ptr_msg)) < 0)
				{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v0 (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_TRIGGER_SOURCE_v0 (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n del evento de disparo.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v1(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
int ch_index;

	if (command.BIT.DAT != sizeof(TMSG_TriggerSource_v1))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v1 (%d != %d)", command.BIT.DAT, sizeof(TMSG_TriggerSource_v1));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v1 (%d)", command.BIT.DAT);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_TRIGGER_SOURCE_v1: %d", sizeof(T_AMPLIACommand));
      n_bytes_ref = sizeof(TMSG_TriggerSource_v1);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = UCI_Set_TriggerSource_v1((TMSG_TriggerSource_v1 *)ptr_msg)) < 0)
				{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v1 (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_TRIGGER_SOURCE_v1 (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }

   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n del evento de disparo.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v2(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
int ch_index;

	if (command.BIT.DAT != sizeof(TMSG_TriggerSource_v2))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v2 (%d != %d)", command.BIT.DAT, sizeof(TMSG_TriggerSource_v2));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v2 (%d)", command.BIT.DAT);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_TRIGGER_SOURCE: %d", sizeof(T_AMPLIACommand));
      n_bytes_ref = sizeof(TMSG_TriggerSource_v2);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = UCI_Set_TriggerSource_v2((TMSG_TriggerSource_v2 *)ptr_msg)) < 0)
				{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE_v2 (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_TRIGGER_SOURCE_v2 (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }

   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n del evento de disparo.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v3(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
int ch_index;

	if (command.BIT.DAT != sizeof(TMSG_TriggerSource_v3))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE (%d != %d)", command.BIT.DAT, sizeof(TMSG_TriggerSource_v3));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE (%d)", command.BIT.DAT);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_TRIGGER_SOURCE: %d", sizeof(T_AMPLIACommand));
      n_bytes_ref = sizeof(TMSG_TriggerSource_v3);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = UCI_Set_TriggerSource_v3((TMSG_TriggerSource_v3 *)ptr_msg)) < 0)
				{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_TRIGGER_SOURCE (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }

   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n del evento de disparo.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
int ch_index;

	if (command.BIT.DAT != sizeof(TMSG_TriggerSource))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE (%d != %d)", command.BIT.DAT, sizeof(TMSG_TriggerSource));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE (%d)", command.BIT.DAT);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_TRIGGER_SOURCE: %d", sizeof(T_AMPLIACommand));
      n_bytes_ref = sizeof(TMSG_TriggerSource);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = UCI_Set_TriggerSource((TMSG_TriggerSource *)ptr_msg)) < 0)
				{RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_TRIGGER_SOURCE (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_TRIGGER_SOURCE (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }

   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------

int UCI_LoadFirmware(TMSG_LoadFirmware *ptr_msg)
{
u32 n_data=0,n_data32, n_data_total=0, n_data_file=0, n_data_sent=0;
u32* GPIO_CONF = (u32 *)XPAR_PROG_GPIO_BASEADDR;
u32* gpio_pwr;
u8 *ptr_data;
XLlFifo FifoInstance;
int Status,i;

	UCI_Alarm2(0);
	//En primer lugar apagamos y descargamos la fuente
	gpio_pwr = (u32*)XPAR_PWR_GPIO_BASEADDR;
	*gpio_pwr = 0x3;

	timer_set_one_count(1,4);
	timer_start(4);

	xil_printf("Descargando fuente de alta tensi�n...\r\n");
	while(!Timer_flag[4])
		WFI_timer();
	UCI_Alarm2(1);

	if (ptr_msg != NULL)
	{
		n_data_file = ptr_msg->size_file;
		n_data_total = 0;
		cons_refresh_read (HOST2UCI_cons_p, sizeof(TMSG_LoadFirmware));

		/* Transmit the Data Stream */
	  /*  for(i=0;i<8;i++)// Se pone los biestables de activaci�n todos a 1
		{
			*GPIO_CONF=0xF;
			TIMER_Sleep(10);
			*GPIO_CONF=0x7;
			TIMER_Sleep(10);
		}



		*GPIO_CONF=0xF;
		TIMER_Sleep(100000);
		*GPIO_CONF=0xD;			// Bajamos PROG_B para borrar FPGAS
		TIMER_Sleep(100000);	// Durante un rat�n
		*GPIO_CONF=0xF;
		TIMER_Sleep(100000);
	*/
		/* Transmit the Data Stream */
		for(i=0;i<16;i++)// Se pone los biestables de activaci�n todos a 1
		{
			*GPIO_CONF=0xD;
			TIMER_Sleep(10);
			*GPIO_CONF=0x5;
			TIMER_Sleep(10);
		}

		*GPIO_CONF=0xD;
		TIMER_Sleep(100000);
		//xil_printf("\r\n--------------------------------------CUIDADOOO BAJADA DE PRO COMENTADA PARA PROBAR--------------------      "); //Si se comenta linea de abajo, descomentar esta.
		*GPIO_CONF=0xF;			// Bajamos PROG_B para borrar FPGAS
		TIMER_Sleep(100000);	// Durante un rat�n
		*GPIO_CONF=0xD;
		TIMER_Sleep(100000);


		//TIMER_Sleep(300000);

		xil_printf("\r\nRECEIVING FIRMWARE...");
		n_data = 0;
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX],REGFIFO_SWITCH_MASTER_TO_FPGA_PROG_IDX,REGFIFO_SWITCH_REGFIFO_SLAVE_FROM_AXILFIFO_IDX,0);

		while (n_data_total < n_data_file)
		{
			n_data = cons_num_data_to_read(HOST2UCI_cons_p);
			n_data32=n_data>>2;
			n_data=n_data32<<2;
			n_data_total+=n_data;
			xil_printf("\r\n%d / %d Bytes", n_data_total, n_data_file);
			if ((ptr_data = cons_bufref(HOST2UCI_cons_p, n_data)) != NULL)
			Status = axil2stream_fifo_send((u32 *)ptr_data, n_data32);//6692572 bytes 53540576
			if (Status != XST_SUCCESS)
			{
				xil_printf("\r\nTransmisson of Data failed\n\r");
				switch_disable_master(stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX],REGFIFO_SWITCH_MASTER_TO_FPGA_PROG_IDX);
				SHRD_CPU1_MiniBaseLoaded(0);
				UCI_Alarm2(0);
				return XST_FAILURE;
			}
			cons_refresh_read (HOST2UCI_cons_p, n_data);
		}
		if(0)
		{
			/* Transmit the Data Stream */
			for(i=0;i<8;i++)// Se pone los biestables de activaci�n todos a 0
			{
				*GPIO_CONF=0x9;//1001
				TIMER_Sleep(100);
				*GPIO_CONF=0x1;//0001
				TIMER_Sleep(100);
			}
			*GPIO_CONF=0x5;//0101
			TIMER_Sleep(100);
			*GPIO_CONF=0xD;//1101
			TIMER_Sleep(100);
			*GPIO_CONF=0x5;//0101
			TIMER_Sleep(100);
			for(i=0;i<4;i++)// Vamos poniendo biestables de activaci�n 1 uno por uno
			{
				axil2stream_fifo_fill(0xFFFFFFFF,10000);
				xil_printf("\r\nFIRMWARE %d LOADED.",i);
				*GPIO_CONF=0x9;//1001
				TIMER_Sleep(10);
				*GPIO_CONF=0x1;//0001
				TIMER_Sleep(10);
				TIMER_Sleep(100000);
			}
		}
		else
			axil2stream_fifo_fill(0xFFFFFFFF,10000);

		if(0)
		{
			ctrl_afe_t reg_ctrl;
			reg_ctrl.REG=0x0;
			reg_ctrl.BIT.pwdn_glb=1;
			reg_ctrl.BIT.afe_clk_disable=1;
			BCC_write_reg_inmediate(0,AFE_BUSSAR_SUBMOD_ADDR,0,reg_ctrl.REG);
		}

		xil_printf("\r\nSENDING STATUS...");
		//UCI_Send_MSG_STATUS();
		xil_printf("\r\nFIRMWARE LOADED SUCCESSFULLY");

		init_bcc_and_bussar();
		switch_disable_master(stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX],REGFIFO_SWITCH_MASTER_TO_FPGA_PROG_IDX);
	}
	else cons_refresh_read (HOST2UCI_cons_p, sizeof(TMSG_LoadFirmware));
	SHRD_CPU1_MiniBaseLoaded(1);
	//Leer el numero de
	UCI_Alarm2(0);
	return EUCI_NONE;
}

/*{
u32 n_data=0,n_data32, n_data_total=0, n_data_file=0;
u32* GPIO_CONF = (u32 *)XPAR_PROG_GPIO_BASEADDR;
u32* gpio_pwr;
u8 *ptr_data;
int Status,i;

	gpio_pwr=(u32*)XPAR_PWR_GPIO_BASEADDR;
	*gpio_pwr=0x3;

	timer_set_one_count(1,4);
	timer_start(4);

	xil_printf("Descargando fuente de alta tensión...\r\n");
	while(!Timer_flag[4]) WFI_timer();

	if (ptr_msg != NULL)
	{
		n_data_file = ptr_msg->size_file;
		n_data_total = 0;
		cons_refresh_read (HOST2UCI_cons_p, sizeof(TMSG_LoadFirmware));

		 Transmit the Data Stream
		for(i=0;i<8;i++)// Se pone los biestables de activación todos a 1
		{
			*GPIO_CONF=0xD;
			TIMER_Sleep(10);
			*GPIO_CONF=0x5;
			TIMER_Sleep(10);
		}

		*GPIO_CONF=0xD;
		TIMER_Sleep(100000);
		*GPIO_CONF=0xF;			// Bajamos PROG_B para borrar FPGAS
		TIMER_Sleep(100000);	// Durante un ratín
		*GPIO_CONF=0xD;
		TIMER_Sleep(100000);

		xil_printf("\r\nRECEIVING FIRMWARE...");
		n_data = 0;
		switch_set_master_2_slave_enable_connect(stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX],REGFIFO_SWITCH_MASTER_TO_FPGA_PROG_IDX,REGFIFO_SWITCH_REGFIFO_SLAVE_FROM_AXILFIFO_IDX,0);

		while (n_data_total < n_data_file)
		{
			n_data = cons_num_data_to_read(HOST2UCI_cons_p);
			n_data32=n_data>>2;
			n_data=n_data32<<2;
			n_data_total+=n_data;
			xil_printf("\r\n%d / %d Bytes", n_data_total, n_data_file);
			if ((ptr_data = cons_bufref(HOST2UCI_cons_p, n_data)) != NULL)
				Status = axil2stream_fifo_send((u32 *)ptr_data, n_data32);//6692572 bytes 53540576
			if (Status != XST_SUCCESS)
			{
				xil_printf("\r\nTransmisson of Data failed\n\r");
				switch_disable_master(stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX],REGFIFO_SWITCH_MASTER_TO_FPGA_PROG_IDX);
				return XST_FAILURE;
			}
			cons_refresh_read (HOST2UCI_cons_p, n_data);
		}

		axil2stream_fifo_fill(0xFFFFFFFF,10000);
		xil_printf("\r\nFIRMWARE LOADED.");

		xil_printf("\r\nSENDING STATUS...");
		UCI_Send_MSG_STATUS();
		xil_printf("\r\nFIRMWARE LOADED SUCCESSFULLY");

		init_bcc_and_bussar();
		switch_disable_master(stream_switch_base_addrs[REGFIFO_SWITCH_TABLE_IDX],REGFIFO_SWITCH_MASTER_TO_FPGA_PROG_IDX);
	}
	else cons_refresh_read (HOST2UCI_cons_p, sizeof(TMSG_LoadFirmware));
	return EUCI_NONE;
}*/

// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n del evento de disparo.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_FP_LOAD_FIRMWARE(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
u32* ptr;

	if (command.BIT.DAT != sizeof(TMSG_LoadFirmware))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_LOAD_FIRMWARE (%d != %d)", command.BIT.DAT, sizeof(TMSG_LoadFirmware));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_LOAD_FIRMWARE (%d)", command.BIT.DAT);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_FP_LOAD_FIRMWARE: %d", sizeof(T_AMPLIACommand));
      n_bytes_ref = sizeof(TMSG_LoadFirmware);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = UCI_LoadFirmware((TMSG_LoadFirmware *)ptr_msg)) < 0)
				{RLOG(result);}
         else
         {
        	 	ptr=(u32*)XPAR_PWR_GPIO_BASEADDR;
        	 	*ptr=0x0;


        	    timer_set_one_count(1,4);
        	    timer_start(4);

        	    xil_printf("Encendiendo fuente de alta a (0 V)...\r\n");
        	    while(!Timer_flag[4])
        	 	   WFI_timer();

        	    TLV5626_ini((u32*)XPAR_TLV5626_AXI_LITE_TOP_0_BASEADDR);
        	    timer_set_one_count(1,4);
        	    timer_start(4);

        	    xil_printf("...");
        	    while(!Timer_flag[4])
        	 	   WFI_timer();
        	 	TLV5626_value((u32*)XPAR_TLV5626_AXI_LITE_TOP_0_BASEADDR,0);
        	 	   xil_printf("Fuente encendida\r\n");
         }
         n_bytes_ref = 0;
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_FP_LOAD_FIRMWARE (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_FP_LOAD_FIRMWARE (%d < %d)", n_data, command.BIT.DAT);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n de los registros de una BASE, y configura 
el hardware correspondiente.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_TERMINAL_LOG(T_AMPLIACommand command)
{
TMSG_TerminalLog data;

	if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_TERMINAL_LOG (0x%04X)", command.BIT.DAT);
	data.REG = command.BIT.DAT;
	gb_log_prg_base_wmpar = data.BIT.PRG_BASE_WMPAR;
	gb_log_prg_mod_wmpar = data.BIT.PRG_MOD_WMPAR;
	gb_log_amplia_send_command = data.BIT.AMPLIA_SEND_COMMAND;
	gb_log_prg_reg = data.BIT.PRG_REG;
	gb_log_acquiring = data.BIT.ACQUIRING;
	gb_log_acquiring_frame = data.BIT.ACQUIRING_FRAME;
	gb_log_protocol = data.BIT.PROTOCOL;
	gb_log_beamforming = data.BIT.BEAMFORMING;
	return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n de los registros de una BASE, y configura 
el hardware correspondiente.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_VERSION(T_AMPLIACommand command)
{
TMSG_Version msg;
int i, result;
   
   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_VERSION");
//   msg.fslb.branch = 0;//Version_control_array[VERSION_CONTROL_FSBL].branch_sw;
//   msg.fslb.major = 0;//Version_control_array[VERSION_CONTROL_FSBL].major_sw;
//   msg.fslb.minor = 0;//Version_control_array[VERSION_CONTROL_FSBL].minor_sw;
//   for (i=0; i<VERSION_DATE_SIZE; i++)
//      msg.fslb.date[i] = 0;//Version_control_array[VERSION_CONTROL_FSBL].date[i];
//   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
//      msg.fslb.device[i] = 0;//Version_control_array[VERSION_CONTROL_FSBL].description[i];
   
   msg.cpu0.branch = gb_shared->sw_version_cpu0.branch_sw;
   msg.cpu0.major = gb_shared->sw_version_cpu0.major_sw;
   msg.cpu0.minor = gb_shared->sw_version_cpu0.minor_sw;
   for (i=0; i<VERSION_DATE_SIZE; i++)
      msg.cpu0.date[i] = gb_shared->sw_version_cpu0.date[i];
   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
      msg.cpu0.device[i] = gb_shared->sw_version_cpu0.description[i];

   msg.cpu1.branch = gb_shared->sw_version_cpu1.branch_sw;
   msg.cpu1.major = gb_shared->sw_version_cpu1.major_sw;
   msg.cpu1.minor = gb_shared->sw_version_cpu1.minor_sw;
   for (i=0; i<VERSION_DATE_SIZE; i++)
      msg.cpu1.date[i] = gb_shared->sw_version_cpu1.date[i];
   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
      msg.cpu1.device[i] = gb_shared->sw_version_cpu1.description[i];

   msg.hw = gb_shared->hw_version;
   msg.mod = 0;

//   msg.cpu0.branch = Version_control_array[VERSION_CONTROL_CPU0].branch_sw;
//   msg.cpu0.major = Version_control_array[VERSION_CONTROL_CPU0].major_sw;
//   msg.cpu0.minor = Version_control_array[VERSION_CONTROL_CPU0].minor_sw;
//   for (i=0; i<VERSION_DATE_SIZE; i++)
//      msg.cpu0.date[i] = Version_control_array[VERSION_CONTROL_CPU0].date[i];
//   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
//      msg.cpu0.device[i] = Version_control_array[VERSION_CONTROL_CPU0].description[i];
//
//   msg.cpu1.branch = Version_control_array[VERSION_CONTROL_CPU1].branch_sw;
//   msg.cpu1.major = Version_control_array[VERSION_CONTROL_CPU1].major_sw;
//   msg.cpu1.minor = Version_control_array[VERSION_CONTROL_CPU1].minor_sw;
//   for (i=0; i<VERSION_DATE_SIZE; i++)
//      msg.cpu1.date[i] = Version_control_array[VERSION_CONTROL_CPU1].date[i];
//   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
//      msg.cpu1.device[i] = Version_control_array[VERSION_CONTROL_CPU1].description[i];

   //msg.hw = get_HW_version();

   //xil_printf("\r\nT_MSG_Version");
   msg.checksum = 0; //IMPORTANTE POR QUE SE CALCULA EL CHECKSUM DE TODA LA ESTRUCTURA INCLUIDO EL CAMPO CHECKSUM
   msg.checksum = ChecksumFletcher16((u8 *)&msg, sizeof(TMSG_SITAUStatus));
   if ((result = UCI_Send((u8 *)&msg, sizeof(TMSG_Version))) < 0) return RLOG(result);

	//xil_printf("\r\n\r\n%s Version v%d.%d.%d - %s", msg.fslb.device, msg.fslb.branch, msg.fslb.major, msg.fslb.minor, msg.fslb.date);

	xil_printf("\r\n%s Version v%d.%d.%d - %s", msg.cpu0.device, msg.cpu0.branch, msg.cpu0.major, msg.cpu0.minor, msg.cpu0.date);

	xil_printf("\r\n%s Version v%d.%d.%d - %s", msg.cpu1.device, msg.cpu1.branch, msg.cpu1.major, msg.cpu1.minor, msg.cpu1.date);

	xil_printf("\r\nHardware Version v0x%X\r\n", msg.hw);


	print_version();
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n de los registros de una BASE, y configura 
el hardware correspondiente.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_STATUS_v0(T_AMPLIACommand command)
{
int result;

   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_STATUS_v0");
   if ((result = UCI_Send_MSG_STATUS_v0()) < 0) return RLOG(result);
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n de los registros de una BASE, y configura
el hardware correspondiente.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_STATUS(T_AMPLIACommand command)
{
int result;
   
   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_STATUS");
   if ((result = UCI_Send_MSG_STATUS()) < 0) return RLOG(result);
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuracion de los registros de una BASE, y configura
el hardware correspondiente.

@return Ver codigos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_STOP(T_AMPLIACommand command)
{
	int i;

	xil_printf("\r\nCMD_STOP");
	if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_STOP");
	ACQ_FMC_stop_continous();
//	for (i=0; i<MAX_VIRTUAL_CHANNELS; i++)
//	{
//		if (gb_fp_virtual_channel[i].enabled == 1)
//			ACQ_FMC_hard_break(&gb_fp_virtual_channel[i]);
//	}
	return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuracion de los registros de una BASE, y configura
el hardware correspondiente.

@return Ver codigos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_PAUSE(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;

	if (command.BIT.DAT != sizeof(TMSG_Pause))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_PAUSE (%d != %d)", command.BIT.DAT, sizeof(TMSG_Pause));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= command.BIT.DAT)
	{
		cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_PAUSE (%d)", command.BIT.DAT);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_PAUSE: %d", sizeof(T_AMPLIACommand));
	  n_bytes_ref = sizeof(TMSG_Pause);

	  if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
	  {
		 if ((result = ACQ_FMC_pause_continous((TMSG_Pause *)ptr_msg)) < 0)
				{RLOG(result);}
	  }
	  else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_PAUSE (ptr_msg = NULL)");
   }
   else
   {
	  if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_PAUSE (%d < %d)", n_data, command.BIT.DAT);
	  n_bytes_ref = 0;
   }

   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuracion de los registros de una BASE, y configura
el hardware correspondiente.

@return Ver codigos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_CONTINUE(T_AMPLIACommand command)
{
   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_CONTINUE");
   ACQ_FMC_continue_continous();
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n de los registros de una BASE, y configura 
el hardware correspondiente.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_RESET_ACQ_COUNTER(T_AMPLIACommand command)
{
   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_RESET_ACQ_COUNTER");
   UCI_SetAcqCounterZero();
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_HW_SITAU_ENABLED(T_AMPLIACommand command)
{
   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_HW_SITAU_ENABLED");
   gb_hw_sitau_enabled = command.BIT.DAT > 0 ? 1 : 0;
   xil_printf("\r\nSITAU Hardware Enabled(%d)\r\n", gb_hw_sitau_enabled);
   return sizeof(T_AMPLIACommand);
}
// -----------------------------------------------------------------------------
/**

*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode_CMD_STATUS_CONFIG(T_AMPLIACommand command)
{
u32 n_data, n_bytes_ref = 0;
int result;
unsigned char *ptr_msg = NULL;
unsigned long size_msg, cmd_aux[2];

	if (command.BIT.DAT == 0xFFFF)
   {
      if (cons_num_data_to_read(HOST2UCI_cons_p) > sizeof(T_AMPLIACommand) * 2)
      {
         cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand)*2, (unsigned char *)cmd_aux);
         size_msg = cmd_aux[1];
      }
      else
      {
         if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_STATUS_CONFIG (Auxiliar Command)");
         return 0;
      }
   }
   else {size_msg = command.BIT.DAT;}

	if (size_msg != (unsigned long)sizeof(TMSG_StatusConfig))
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_STATUS_CONFIG (%d != %d)", size_msg, (unsigned long)sizeof(TMSG_StatusConfig));
		cons_refresh_read (	HOST2UCI_cons_p, cons_num_data_to_read(HOST2UCI_cons_p));
		return ELOG(charEUCI_DataInconsistency, EUCI_DataInconsistency);
	}

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
	if (n_data >= size_msg)
	{
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_STATUS_CONFIG (%d)", command.BIT.DAT);
		if (command.BIT.DAT == 0xFFFF) 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand) * 2);
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_STATUS_CONFIG: %d", sizeof(T_AMPLIACommand)*2);
		}
		else 
		{
			cons_refresh_read (HOST2UCI_cons_p, sizeof(T_AMPLIACommand));
			if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data CMD_STATUS_CONFIG: %d", sizeof(T_AMPLIACommand));
		}
      n_bytes_ref = sizeof(TMSG_StatusConfig);

      if ((ptr_msg = cons_bufref(HOST2UCI_cons_p, n_bytes_ref)) != NULL)
      {
         if ((result = UCI_Set_StatusConfig((TMSG_StatusConfig *)ptr_msg)) < 0)
            {RLOG(result);}
      }
      else if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCIv2 Command - CMD_STATUS_CONFIG (ptr_msg = NULL)");
   }
   else
   {
      if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): UCI, v2 Command - CMD_STATUS_CONFIG (%d < %d)", n_data, size_msg);
      n_bytes_ref = 0;
   }
   return n_bytes_ref;
}
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del nï¿½mero de datos de la trama.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_ReceiveDecode (T_AMPLIACommand command)
{
int n_bytes_deco = -1;

	if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_ReceiveDecode(): AMPLIA Command [0x%x] [REG:%x - TAC:%d - DIR:%d - DAT:%d]",
			command.CMD,
			command.BIT.REG,
			command.BIT.TAC,
			command.BIT.DIR,
			command.BIT.DAT);
	if (command.BIT.REG != UCI_REG_UCIv2) return n_bytes_deco;
	switch(command.BIT.DIR)
	{
	case CMD_STOP:
	    n_bytes_deco = UCI_ReceiveDecode_CMD_STOP(command);
	    break;
	case CMD_PAUSE:
	    n_bytes_deco = UCI_ReceiveDecode_CMD_PAUSE(command);
	    break;
	case CMD_CONTINUE:
	    n_bytes_deco = UCI_ReceiveDecode_CMD_CONTINUE(command);
	    break;
	case CMD_HW_SITAU_ENABLED:
		n_bytes_deco = UCI_ReceiveDecode_CMD_HW_SITAU_ENABLED(command);
		break;
	case CMD_FP_PULSE_AMPLITUDE:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_PULSE_AMPLITUDE(command);
		break;   
	case CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v0(command);
		break;         
	case CMD_FP_VIRTUAL_CHANNEL_CONFIG_v1:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v1(command);
		break;         
	case CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2(command);
		break;
	case CMD_FP_VIRTUAL_CHANNEL_CONFIG_v3:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG_v2(command);
		break;
	case CMD_FP_VIRTUAL_CHANNEL_CONFIG:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_CONFIG(command);
		break;
	case CMD_FP_VIRTUAL_CHANNEL_FIR:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_FIR(command);
		break;         
	case CMD_FP_PRINT_VCH_CONFIG:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_PRINT_VCH_CONFIG(command);
		break;         
	case CMD_FP_ACQUIRE:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_ACQUIRE(command);
		break;
	case CMD_VCH_EMISSION_FOCAL_LAW:
		n_bytes_deco = UCI_ReceiveDecode_CMD_VCH_EMISSION_FOCAL_LAW(command);
		break;
	case CMD_VCH_EMISSION_FOCAL_LAW_EXT:
		n_bytes_deco = UCI_ReceiveDecode_CMD_VCH_EMISSION_FOCAL_LAW_EXT(command);
		break;
	case CMD_FP_VIRTUAL_CHANNEL_TGC:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_VIRTUAL_CHANNEL_TGC(command);
		break;
	case CMD_FP_TRIGGER_SOURCE_v0:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v0(command);
		break;
	case CMD_FP_TRIGGER_SOURCE_v1:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v1(command);
		break;
	case CMD_FP_TRIGGER_SOURCE_v2:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v2(command);
		break;
	case CMD_FP_TRIGGER_SOURCE_v3:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE_v3(command);
		break;
	case CMD_FP_TRIGGER_SOURCE:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_TRIGGER_SOURCE(command);
		break;
	case CMD_FP_LOAD_FIRMWARE:
		n_bytes_deco = UCI_ReceiveDecode_CMD_FP_LOAD_FIRMWARE(command);
		break;
	case CMD_VIRTUAL_CHANNEL_BEAMFORMER:
		n_bytes_deco = UCI_ReceiveDecode_CMD_VIRTUAL_CHANNEL_BEAMFORMER(command);
		break;
	case CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT:
		n_bytes_deco = UCI_ReceiveDecode_CMD_VIRTUAL_CHANNEL_BEAMFORMER_EXT(command);
		break;
	case CMD_TFM_FMC_FORWARD:
		n_bytes_deco = UCI_ReceiveDecode_CMD_TFM_FMC_FORWARD(command);
		break;
	case CMD_TFM_PWI_FORWARD:
		n_bytes_deco = UCI_ReceiveDecode_CMD_TFM_PWI_FORWARD(command);
		break;
	case CMD_TERMINAL_LOG:
		n_bytes_deco = UCI_ReceiveDecode_CMD_TERMINAL_LOG(command);
		break;      
	case CMD_VERSION:
		n_bytes_deco = UCI_ReceiveDecode_CMD_VERSION(command);
		break;        
	case CMD_STATUS_v0:
		n_bytes_deco = UCI_ReceiveDecode_CMD_STATUS_v0(command);
		break;
	case CMD_STATUS:
		n_bytes_deco = UCI_ReceiveDecode_CMD_STATUS(command);
		break;        
	case CMD_RESET_ACQ_COUNTER:
		n_bytes_deco = UCI_ReceiveDecode_CMD_RESET_ACQ_COUNTER(command);
		break;      
	case CMD_STATUS_CONFIG:
		n_bytes_deco = UCI_ReceiveDecode_CMD_STATUS_CONFIG(command);
		break;
	default:
		n_bytes_deco = -1;
		break;
	}
	if (n_bytes_deco < 0)
		cons_refresh_read(HOST2UCI_cons_p, cons_num_data_to_read(HOST2UCI_cons_p));      
	else if (n_bytes_deco > 0)
	{
		cons_refresh_read(HOST2UCI_cons_p, n_bytes_deco);
		if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nFree Data: %d", n_bytes_deco);
	}
	return n_bytes_deco;
}
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del nï¿½mero de datos de la trama.

@return Ver cï¿½digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Receive (void)
{
u32 n_data;
T_AMPLIACommand command;
int result = -1;

	n_data = cons_num_data_to_read(HOST2UCI_cons_p);
   if (n_data > 0 && gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_Receive(): %d", n_data);
	if (n_data >= sizeof(T_AMPLIACommand)) // Si se ha recibido un comando
	{
		cons_memcpy(HOST2UCI_cons_p, sizeof(T_AMPLIACommand), (unsigned char *)&command.CMD);
      result = UCI_ReceiveDecode(command);
	}
	else
		xil_printf("\r\nNo Command");
	return result;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n de los registros de una BASE, y configura 
el hardware correspondiente.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Send_MSG_STATUS_v0(void)
{
TMSG_SITAUStatus_v0 msg;
int i, j, k, result;

   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_Send_MSG_STATUS(%d, %d)", gb_sitau_status, gb_uci.trigger_source);
   msg.sitau_status = gb_sitau_status;
   msg.trigger_source = gb_uci.trigger_source;
   msg.n_acquisitions = gb_uci.n_acquisitions;
   msg.ind_acquisitions = gb_uci.ind_acquisitions;
   msg.size_buffer_command = SIZE_BUFFER_COMMAND;

   // VERSION
//   msg.version.fslb.branch = 0;//Version_control_array[VERSION_CONTROL_FSBL].branch_sw;
//   msg.version.fslb.major = 0;//Version_control_array[VERSION_CONTROL_FSBL].major_sw;
//   msg.version.fslb.minor = 0;//Version_control_array[VERSION_CONTROL_FSBL].minor_sw;
//   for (i=0; i<VERSION_DATE_SIZE; i++)
//      msg.version.fslb.date[i] = 0;//Version_control_array[VERSION_CONTROL_FSBL].date[i];
//   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
//      msg.version.fslb.device[i] = 0;//Version_control_array[VERSION_CONTROL_FSBL].description[i];

   msg.version.cpu0.branch = gb_shared->sw_version_cpu0.branch_sw;
   msg.version.cpu0.major = gb_shared->sw_version_cpu0.major_sw;
   msg.version.cpu0.minor = gb_shared->sw_version_cpu0.minor_sw;
   for (i=0; i<VERSION_DATE_SIZE; i++)
      msg.version.cpu0.date[i] = gb_shared->sw_version_cpu0.date[i];
   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
      msg.version.cpu0.device[i] = gb_shared->sw_version_cpu0.description[i];

   msg.version.cpu1.branch = gb_shared->sw_version_cpu1.branch_sw;
   msg.version.cpu1.major = gb_shared->sw_version_cpu1.major_sw;
   msg.version.cpu1.minor = gb_shared->sw_version_cpu1.minor_sw;
   for (i=0; i<VERSION_DATE_SIZE; i++)
      msg.version.cpu1.date[i] = gb_shared->sw_version_cpu1.date[i];
   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
      msg.version.cpu1.device[i] = gb_shared->sw_version_cpu1.description[i];

   msg.version.hw = gb_shared->hw_version;
   msg.version.mod = 0;

//   msg.version.cpu0.branch = Version_control_array[VERSION_CONTROL_CPU0].branch_sw;
//   msg.version.cpu0.major = Version_control_array[VERSION_CONTROL_CPU0].major_sw;
//   msg.version.cpu0.minor = Version_control_array[VERSION_CONTROL_CPU0].minor_sw;
//   for (i=0; i<VERSION_DATE_SIZE; i++)
//      msg.version.cpu0.date[i] = Version_control_array[VERSION_CONTROL_CPU0].date[i];
//   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
//      msg.version.cpu0.device[i] = Version_control_array[VERSION_CONTROL_CPU0].description[i];
//
//   msg.version.cpu1.branch = Version_control_array[VERSION_CONTROL_CPU1].branch_sw;
//   msg.version.cpu1.major = Version_control_array[VERSION_CONTROL_CPU1].major_sw;
//   msg.version.cpu1.minor = Version_control_array[VERSION_CONTROL_CPU1].minor_sw;
//   for (i=0; i<VERSION_DATE_SIZE; i++)
//      msg.version.cpu1.date[i] = Version_control_array[VERSION_CONTROL_CPU1].date[i];
//   for (i=0; i<VERSION_DESCRIPTION_SIZE; i++)
//      msg.version.cpu1.device[i] = Version_control_array[VERSION_CONTROL_CPU1].description[i];
//
//   msg.version.hw = get_HW_version();

   // LOG ERROR STACK
   for (k=0; k<SHRD_LOG_ERROR_SIZE; k++)
   {
      for (i=0; i<SHRD_LOG_ERROR_MSG_SIZE; i++) msg.log_error.e[k].msg[i] = gb_shared->log.e[k].msg[i];
      for (j=0; j<SHRD_LOG_ERROR_STACK_SIZE; j++)
      {
         msg.log_error.e[k].s[j].code = gb_shared->log.e[k].s[j].code;
         msg.log_error.e[k].s[j].line = gb_shared->log.e[k].s[j].line;
         for (i=0; i<SHRD_LOG_ERROR_FILE_SIZE; i++)
            msg.log_error.e[k].s[j].file[i] = gb_shared->log.e[k].s[j].file[i];
      }
      msg.log_error.e[k].count = gb_shared->log.e[k].count;
   }
   msg.log_error.count = gb_shared->log.count;

   msg.checksum = 0; //IMPORTANTE POR QUE SE CALCULA EL CHECKSUM DE TODA LA ESTRUCTURA INCLUIDO EL CAMPO CHECKSUM
   msg.checksum = ChecksumFletcher16((u8 *)&msg, sizeof(TMSG_SITAUStatus));
   if ((result = UCI_Send((u8 *)&msg, sizeof(TMSG_SITAUStatus_v0))) < 0) {return RLOG(result);}
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Decodifica el mensaje de configuraci�n de los registros de una BASE, y configura
el hardware correspondiente.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Send_MSG_STATUS(void)
{
TMSG_SITAUStatus msg;
int i, j, k, result;
volatile ACQ_hndlr_t *ACQ = Current_ACQ_hndlr_ptr;
   
   if (gb_log_protocol == 1) LOG_PROTOCOL("\r\nUCI_Send_MSG_STATUS(%d, %d)", gb_sitau_status, gb_uci.trigger_source);
   msg.status_code = 0xF005C51CDA5EDECA;
   msg.rd_address = ACQ->read_addr_index;
   msg.wr_address = ACQ->write_addr_index;
   msg.n_overflows = ACQ->n_overflows;
   msg.sitau_status = gb_sitau_status;
   msg.trigger_source = gb_uci.trigger_source;
   msg.n_acquisitions = gb_uci.n_acquisitions;
   msg.ind_acquisitions = gb_uci.ind_acquisitions;//gb_uci.ind_acquisitions;
   msg.size_buffer_command = SIZE_BUFFER_COMMAND;
     
   // LOG ERROR STACK
   for (k=0; k<SHRD_LOG_ERROR_SIZE; k++)
   {
      for (i=0; i<SHRD_LOG_ERROR_MSG_SIZE; i++) msg.log_error.e[k].msg[i] = gb_shared->log.e[k].msg[i];
      for (j=0; j<SHRD_LOG_ERROR_STACK_SIZE; j++)
      {
         msg.log_error.e[k].s[j].code = gb_shared->log.e[k].s[j].code;
         msg.log_error.e[k].s[j].line = gb_shared->log.e[k].s[j].line;
         for (i=0; i<SHRD_LOG_ERROR_FILE_SIZE; i++)
            msg.log_error.e[k].s[j].file[i] = gb_shared->log.e[k].s[j].file[i];
      }
      msg.log_error.e[k].count = gb_shared->log.e[k].count;
   }
   msg.log_error.count = gb_shared->log.count;
   
   msg.checksum = 0; //IMPORTANTE POR QUE SE CALCULA EL CHECKSUM DE TODA LA ESTRUCTURA INCLUIDO EL CAMPO CHECKSUM
   msg.checksum = ChecksumFletcher16((u8 *)&msg, sizeof(TMSG_SITAUStatus));
   if ((result = UCI_Send((u8 *)&msg, sizeof(TMSG_SITAUStatus))) < 0) {return RLOG(result);}
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del n�mero de datos de la trama.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_Send (unsigned char *data_out, int n_bytes)
{
int num_datos_max_2_host, n_pending_bytes, n_block_bytes = 255;
unsigned char *ptr_data = NULL;

   if (data_out == NULL) return 0;
   
//   if (  Version_control_array[VERSION_CONTROL_CPU0].branch_sw <= 1 &&
//         Version_control_array[VERSION_CONTROL_CPU0].major_sw == 0 &&
//         Version_control_array[VERSION_CONTROL_CPU0].minor_sw == 0 &&
//         n_bytes > 150)
   if (  gb_shared->sw_version_cpu0.branch_sw <= 1 &&
		   gb_shared->sw_version_cpu0.major_sw == 0 &&
		   gb_shared->sw_version_cpu0.minor_sw == 0 &&
		 n_bytes > 150)
   {
      for (	n_pending_bytes = n_bytes,
    		ptr_data = data_out;
    		   n_pending_bytes > 0;
    		      n_pending_bytes -= n_block_bytes,
    		      ptr_data += n_block_bytes)
      {
         if (n_pending_bytes > 150) n_block_bytes = 150;
         else n_block_bytes = n_pending_bytes;
         
         num_datos_max_2_host = prod_num_data_to_write(UCI2HOST_prod_p, NULL);
         while (num_datos_max_2_host < n_block_bytes)
         {
            usleep(2);
            num_datos_max_2_host = prod_num_data_to_write(UCI2HOST_prod_p, NULL);
         }
         prod_memcpy(UCI2HOST_prod_p, n_block_bytes, ptr_data);
         prod_refresh_write(UCI2HOST_prod_p, n_block_bytes);
         usleep(1000);
      }
   }
   else
   {
      num_datos_max_2_host = prod_num_data_to_write(UCI2HOST_prod_p, NULL);
      while (num_datos_max_2_host < n_bytes)
      {
         usleep(2);
         num_datos_max_2_host = prod_num_data_to_write(UCI2HOST_prod_p, NULL);
      }
      prod_memcpy(UCI2HOST_prod_p, n_bytes, data_out);
      prod_refresh_write(UCI2HOST_prod_p, n_bytes);
   }
   return n_bytes;
}
