#ifndef vchH
#define vchH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include "vch_tad.h"
#include "protocol.h"

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern int gb_firmware_type;
extern TVCH gb_fp_virtual_channel[MAX_VIRTUAL_CHANNELS];

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================

int VCH_PrintConfig(TVCH *vch);

int VCH_Process_NoHardware(int offset, unsigned short n_samples);

int VCH_Process_NoHardware_sitau2 (unsigned short n_samples,signed short offset);

int VCH_Reset (void);

int VCH_UpdateAcquisitionSize (TVCH *vch);

int VCH_Set_Config_v0 (TMSG_Config_v0 *msg);

int VCH_Set_Config_v1 (TMSG_Config_v1 *msg);

int VCH_Set_Config (TMSG_Config *msg);

int VCH_Set_EmissionFocalLaws (TMSG_EmissionFocalLaws *msg);

int VCH_Set_Beamformer (TMSG_Beamformer *msg);

int VCH_Set_TGC (TMSG_TGC *msg);

int VCH_Set_FIR (TMSG_FIR *msg);


#endif

