#ifndef uci_defineH
#define uci_defineH

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// =============================================================================

// _____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// =============================================================================

//#define UCI_N_MAX_FOCAL_LAW         32//512
#define UCI_N_MAX_SUBSYSTEM         4
#define UCI_N_ENCODER               NUM_ENCODERS

//// Timers
//#define ID_TIMER_MUX 0 //!< Timer asociado al retardo de los multiplexores
//#define ID_TIMER_PRF 1 //!< Timer asociado a la PRF

#define FCLK4  250.        //!< (MHz) Frecuencia de muestreo interpolada
#define FCLK   62.5        //!< (MHz) Frecuencia de muestreo
#define PCLK   1.0 / FCLK  //!< (us) Periodo de muestreo

// Tipo de fuentes de disparo
#define TRG_SCAN_PRF    0
#define TRG_SCAN_EXT    1
#define TRG_SCAN_ENC    2
//#define TRG_SW          0
//#define TRG_SCAN_ENC    1
//#define TRG_SCAN_SW     2
//#define TRG_SCAN_EXT    3
//#define TRG_SCAN_PRF    4

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================

#endif
