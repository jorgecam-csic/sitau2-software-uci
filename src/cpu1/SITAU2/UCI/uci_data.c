// -----------------------------------------------------------------------------
/**
@file uci.c

@brief Este archivo contiene la implementaci�n de las funciones de la clase \c FL_Aperture.<br>

Esta clase tienen la funcionalidad necesaria para definir las aperturas que componen
un barrido.

@author (rg) Ricardo Gonz�lez

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

#include "uci_data.h"
#include "uci_dma.h"
#include "uci_reg.h"
#include "uci_error_code.h"

#include "log.h"
#include "encoder.h"

//static long encoder_trg_value_last = 0;

#ifdef CODIGO_A_ELIMINAR
int UCI_MemoryWrite_uint16 (unsigned short data)
{
int result;
//unsigned long command = AMPLIA_Command(data, DIR_UCI, AMPLIA_TAC_PRI, REG_MEM_DAT);
//
//   if ((result = UCI_AMPLIACommandRun(command)) < 0) return RLOG(result);
   //if ((result = DMA_Write16(data)) < 0) return RLOG(result);
xil_printf("CODIGO_A_ELIMINAR: UCI_MemoryWrite_uint16");
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------

int UCI_MemoryWrite_Data16(unsigned short data)
{
int result;

   //if ((result = DMA_Write16(data)) < 0) return RLOG(result);
xil_printf("CODIGO_A_ELIMINAR: UCI_MemoryWrite_uint16");
   return EUCI_NONE;
}

#endif



// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
/**
Escribe un dato de 32 bits con signo en la memoria de datos (trama). 
Sirve para hacer se�ales sobre la secuencia de datos y permitir reorganizar la 
informaci�n de la misma en memoria.

@param[in] data   Valor que se coloca en la trama

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_MemoryWrite_int32 (s32 data)
{
int result; 
unsigned int *ptr = NULL;


   ptr = (unsigned int *)&data;
   if ((result = DMA_Write32(*ptr)) < 0) return RLOG(result);

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Escribe un dato de 32 bits sin signo en la memoria de datos (trama). 
Sirve para hacer se�ales sobre la secuencia de datos y permitir reorganizar la 
informaci�n de la misma en memoria.

@param[in] data   Valor que se coloca en la trama

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_MemoryWrite_uint32 (u32 data)
{
int result; 

   if ((result = DMA_Write32(data)) < 0) return RLOG(result);
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Escribe un determinado n�mero de datos en la memoria de datos (trama).

@param[in] *data   	Buffer de datos
@param[in] n_data	N�mero de datos que se escriben enla memoria.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_MemoryWrite_BufferData (u32 *data, u32 n_data)
{
	int result;

	while(n_data--)
		if ((result = DMA_Write32(*data++)) < 0) return RLOG(result);

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el contador de adquisiciones.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_MemoryWrite_AcqCounter (void)
{
int result; 

   if ((result = DMA_Write32((u32)gb_acq_counter)) < 0) return RLOG(result);
   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del 'time stamp'.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_MemoryWrite_TimeStamp (void)
{
int result; 

   if ((result = UCI_MemoryWrite_uint32(gb_time_stamp)) < 0) return RLOG(result);
   return EUCI_NONE;

}
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del encoder seleccionado por el 
par�metro 'id_encoder'

@param[in] id_encoder   Identificador del encoder.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_MemoryWrite_Encoder (int id_encoder, int id_vch)
{
int result;
s32 value;
unsigned short *ptr = NULL;

   if (id_encoder >= UCI_N_ENCODER) return ELOG(charEUCI_IdEncoder, EUCI_IdEncoder);
   value = ENC_get_pos(id_encoder);

//	if (id_vch == 0 && gb_uci.trigger_source == TRG_SCAN_ENC && id_encoder == gb_uci.encoder_trigger)
//	{
//		diff = value - encoder_trg_value_last;
//		if ( diff != 0 &&
//			 diff != gb_uci.encoder_trigger_steps)
//			xil_printf("\r\nERROR: Encoder Trigger Lost");
//		encoder_trg_value_last = value;
//	}

   if ((result = UCI_MemoryWrite_int32(value)) < 0) return RLOG(result);

   return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Escribe en la memoria de datos (trama), el valor del encoder seleccionado por el 
par�metro 'id_encoder'

@param[in] id_encoder   Identificador del encoder.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int UCI_MemoryWrite_EncoderValue (s32 value)
{
int result;
unsigned short *ptr = NULL;

	if ((result = UCI_MemoryWrite_int32(value)) < 0) return RLOG(result);
	return EUCI_NONE;
}

