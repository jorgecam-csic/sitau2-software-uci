#ifndef ttimerH
#define ttimerH

#include "timer_util.h"
// ____________________________________________________________________________
// ------ D E F I N I C I O N   D E   C O N S T A N T E S
// ============================================================================

#define NANOSECONDS     0
#define MICROSECONDS    1
#define MILISECONDS     2

#define N_MAX_TIMER     10

#define TIMER_DefaultClock 25 // ns

// _____________________________________________________________________________
// ------ T I P O S   D E   D A T O S
// =============================================================================

typedef struct T_Timer {
   float data;
   int unit;
   int stop;
   int enabled;
   float clock_ns;
   unsigned long reg_time;
   unsigned long reg_status;
} T_Timer;

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

extern T_Timer gb_timer[N_MAX_TIMER]; 

// ____________________________________________________________________________
// ------ D E C L A R A C I O N   D E   L A S   F U N C I O N E S
// ============================================================================
int TIMER_Start(u8 timer_id);
int TIMER_Timeout(u8 timer_id);
int TIMER_Stop(u8 timer_id);
int TIMER_Sleep(float useconds);
int TIMER_Delay(float useconds,u8 timer_id);
int TIMER_Wait(u8 timer_id);
int TIMER_set_periodic_count(float useconds,u8 timer_id);
int TIMER_set_one_count(float useconds,u8 timer_id);
int TIMER_Init(void);

#endif
