#ifndef uci_dataH
#define uci_dataH

#include "xil_types.h"
#include "global.h"
// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================
#ifdef CODIGO_A_ELIMINAR
int UCI_MemoryWrite_Data16(unsigned short data);
#endif

int UCI_MemoryWrite_int32 (s32 data);
int UCI_MemoryWrite_uint32 (u32 data);
int UCI_MemoryWrite_BufferData (u32 *data, u32 n_data);
int UCI_MemoryWrite_AcqCounter (void);
int UCI_MemoryWrite_TimeStamp (void);
int UCI_MemoryWrite_Encoder (int id_encoder, int id_vch);
int UCI_MemoryWrite_EncoderValue (s32 value);

#endif

