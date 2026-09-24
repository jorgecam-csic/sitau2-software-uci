#ifndef vch_prgH
#define vch_prgH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include "vch_tad.h"

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================

int VCH_PRG_EmissionFocalLaws(TVCH *vch);

int VCH_PRG_Registers_FMC(TVCH *vch);

int VCH_PRG_Registers_PA_DMA(TVCH *vch);

int VCH_PRG_Registers_TFM(TVCH *vch);

//int VCH_PRG_Registers(TVCH *vch);

int VCH_PRG_BeamformerRegisters(TVCH *vch);

int VCH_PRG_BeamformerFocalLaws(TVCH *vch);

int VCH_PRG_ForwardFocalLaws(TVCH *vch);

int VCH_PRG (TVCH *vch);

int VCH_PRG_PA_no_DDR_Registers(TVCH *vch);

int VCH_PRG_BeamformerRegisters_no_DDRACQ(TVCH *vch);



#endif

