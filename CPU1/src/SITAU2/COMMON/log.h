#ifndef logH
#define logH

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <stdio.h>

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

//extern T_LOG_ErrorRegister gb_log_error;

// _____________________________________________________________________________
// ------ M A C R O S
// =============================================================================

#define ELOG(msg_err,code) log_error(msg_err,code,(unsigned long)__LINE__,__FILE__);
#define RLOG(code) log_error(NULL,code,(unsigned long)__LINE__,__FILE__);

//#define LOG_TERM xil_printf
#define LOG_TERM void_printf

//#define WARNING_PRINTF xil_printf
#define WARNING_PRINTF void_printf

#define LOG_ACQUIRING_FRAME xil_printf
//#define LOG_ACQUIRING_FRAME void_printf

#define LOG_BEAMFORMING xil_printf
//#define LOG_ACQUIRING_FRAME void_printf

#define LOG_ACQUIRING xil_printf
//#define LOG_ACQUIRING void_printf

#define LOG_PROTOCOL xil_printf
//#define LOG_PROTOCOL void_printf

#define LOG_BASECfg xil_printf
//#define LOG_BASECfg void_printf

#define LOG_MODCfg xil_printf
//#define LOG_MODCfg void_printf

#define LOG_SITAUCfg xil_printf
//#define LOG_SITAUCfg void_printf

// Presenta en el terminal la secuencia de programación de los registros de los MODULOS y de las BASES
#define LOG_PRG_REG xil_printf
//#define LOG_PRG_REG void_printf

// Presenta en el terminal la secuencia de programación de los registros de las leyes focales de los MODULOS
#define LOG_PRG_WMPAR xil_printf
//#define LOG_PRG_WMPAR void_printf

// Presenta en el terminal la configuración del sistema
//#define LOG_CFG

int log_error (char *msg_err, int code_parameter, unsigned int line, char *file);

#endif


