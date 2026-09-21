// -----------------------------------------------------------------------------
/**
@file ttimer.c

@brief Implementaci�n de las funciones de asociadas el tipo \c T_Timer.<br>

Este archivo contiene la implementaci�n de las fgunciones necesarias para manejar 
los timers definidos por el tipo \c T_Timer.

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

#include "ttimer.h"
#include "uci_error_code.h"
#include "calc.h"
#include "log.h"

// _____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// =============================================================================

// _____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// =============================================================================

T_Timer gb_timer[N_MAX_TIMER];
T_Timer gb_timer_sleep;

// _____________________________________________________________________________
// ------ I M P L E M E N T A C I O N   D E   L A S   F U N C I O N E S
// =============================================================================

// -----------------------------------------------------------------------------
/**
Configura el timer gen�rico asociado a la funci�n TIMER_Sleep().

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int TIMER_Init(void)
{

	timer_init_all_default();
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un timer con los registros hardware correspondientes. 

@param[in] data      Tiempo con el que se inicia el timer.
@param[in] unit      Unidades del par�metro 'data'.
@param[in] timer_id  Identificador del timer.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int TIMER_set_one_count(float useconds,u8 timer_id)
{
	if (timer_id >= (NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH))
	   return ELOG(charEUCI_IdTimer, EUCI_IdTimer);
	timer_set_one_count(useconds/1e6,timer_id);

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura un timer con los registros hardware correspondientes.

@param[in] data      Tiempo con el que se inicia el timer.
@param[in] unit      Unidades del par�metro 'data'.
@param[in] timer_id  Identificador del timer.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int TIMER_set_periodic_count(float useconds,u8 timer_id)
{
	if (timer_id >= (NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH))
	   return ELOG(charEUCI_IdTimer, EUCI_IdTimer);

	timer_set_periodic_count(useconds/1e6,timer_id);

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Inicia un timer con un tiempo determinado.

@param[in] data      Tiempo con el que se inicia el timer.
@param[in] unit      Unidades del par�metro 'data'.
@param[in] timer_id  Identificador del timer.

@return Ver c�digos de error.
*/
// -----------------------------------------------------------------------------
int TIMER_Start(u8 timer_id)
{
	if (timer_id >= (NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH))
	   return ELOG(charEUCI_IdTimer, EUCI_IdTimer);
	timer_start(timer_id);
	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Consulta si ha finalizado un timer determinado.

@param[in] timer_id  Identificador del timer.

@return
   0 -> El timer no ha finalizado.\n
   1 -> El timer ha finalizado.\n
   (Ver c�digos de error)
*/
// -----------------------------------------------------------------------------
int TIMER_Timeout(u8 timer_id)
{
	if (timer_id >= (NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH))
	   return ELOG(charEUCI_IdTimer, EUCI_IdTimer);

	return Timer_flag[timer_id];
}
// -----------------------------------------------------------------------------
/**
Espera la finalizaci�n de un timer previamente programado e iniciado.

@param[in] timer_id  Identificador del timer.

@return (Ver c�digos de error)
*/
// -----------------------------------------------------------------------------
int TIMER_Wait(u8 timer_id)
{
	if (timer_id >= (NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH))
	   return ELOG(charEUCI_IdTimer, EUCI_IdTimer);

	while (!Timer_flag[timer_id]);
	   WFI_timer();

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Configura e inicia un timer y espera la finalizaci�n del timer.

@param[in] data      Tiempo con el que se inicia el timer.
@param[in] unit      Unidades del par�metro 'data'.
@param[in] timer_id  Identificador del timer.

@return (Ver c�digos de error)
*/
// -----------------------------------------------------------------------------
int TIMER_Delay(float useconds,u8 timer_id)
{
int result = EUCI_NONE;

	if ((result = TIMER_set_one_count(useconds, timer_id)) < 0) return RLOG(result);
	if ((result = TIMER_Start(timer_id)) < 0) return RLOG(result);
   if ((result = TIMER_Wait(timer_id)) < 0) return RLOG(result);

	return EUCI_NONE;
}
// -----------------------------------------------------------------------------
/**
Espera un tiempo determinado.

@param[in] data      Tiempo con el que se inicia el timer.
@param[in] unit      Unidades del par�metro 'data'.

@return (Ver c�digos de error)
*/
// -----------------------------------------------------------------------------
int TIMER_Sleep(float useconds)
{
	usleep_timer((u32)useconds);
	return EUCI_NONE;
}


int TIMER_Stop(u8 timer_id)
{
	if (timer_id >= (NUM_TIMERS_PERIPH*NUM_TIMERS_PER_PERIPH))
	   return ELOG(charEUCI_IdTimer, EUCI_IdTimer);
	timer_stop(timer_id);
	return EUCI_NONE;
}

