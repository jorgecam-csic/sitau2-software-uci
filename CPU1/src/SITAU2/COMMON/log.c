// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   C O M P I L A D O R
// ============================================================================

#include <stdio.h>
#ifdef _PCDebugger
	#include <string.h>
	#include <time.h>
#endif

// ____________________________________________________________________________
// ------ L I B R E R I A S   P R O P I A S   D E L   P R O Y E C T O
// ============================================================================

#include "log.h"
#include "calc.h"
#include "shared_mem.h"
#ifndef _PCDebugger
	#include "cons_prod_util.h"
#endif

// ____________________________________________________________________________
// ------ V A R I A B L E S   G L O B A L E S
// ============================================================================

//T_LOG_ErrorRegister gb_log_error;

int gb_log_enabled = 1;

// -----------------------------------------------------------------------------
/**
@param[in]
@param[out]
@return
*/
// -----------------------------------------------------------------------------
int log_error (char *msg_err, int code_parameter, unsigned int line, char *file)
{
	unsigned char e_idx, s_idx;
	char *file_name = NULL;
	int i;

	if (gb_log_enabled == 0) return code_parameter;
	if (code_parameter < 0)
	{
		if (file != NULL) file_name = (strrchr(file, '/') != NULL ? strrchr(file, '/') + 1 : file);

		if (msg_err != NULL || gb_shared->log.count == 0)
		{
			if (gb_shared->log.count >= SHRD_LOG_ERROR_SIZE)
			{
				gb_shared->log.count = 0;
				gb_shared->log.e[0].count = 0;
				gb_shared->log.e[0].s[0].code = 0;
				gb_shared->log.e[0].s[0].line = 0;
				gb_shared->log.e[0].s[0].file[0] = 0;
			}
			e_idx = gb_shared->log.count;
			gb_shared->log.e[e_idx].msg[0] = 0;
			if (msg_err != NULL)
			{
				for (i=0; msg_err[i] != 0 && i<SHRD_LOG_ERROR_MSG_SIZE; i++) gb_shared->log.e[e_idx].msg[i] = msg_err[i];
				gb_shared->log.e[e_idx].msg[i] = 0;
			}
			gb_shared->log.e[e_idx].code = code_parameter;
			gb_shared->log.e[e_idx].line = line;
			gb_shared->log.e[e_idx].file[0] = 0;
			if (file_name != NULL)
			{
				for (i=0; file_name[i] != 0 && i<SHRD_LOG_ERROR_FILE_SIZE; i++) gb_shared->log.e[e_idx].file[i] = file_name[i];
				gb_shared->log.e[e_idx].file[i] = 0;
			}
			gb_shared->log.count++;
		}
		else
		{
			e_idx = gb_shared->log.count - 1;
			if (gb_shared->log.e[e_idx].count >= SHRD_LOG_ERROR_STACK_SIZE) gb_shared->log.e[e_idx].count = 0;
			s_idx = gb_shared->log.e[e_idx].count;
			gb_shared->log.e[e_idx].s[s_idx].code = code_parameter;
			gb_shared->log.e[e_idx].s[s_idx].line = line;
			gb_shared->log.e[e_idx].s[s_idx].file[0] = 0;
			if (file_name != NULL)
			{
				for (i=0; file_name[i] != 0 && i<SHRD_LOG_ERROR_FILE_SIZE; i++) gb_shared->log.e[e_idx].s[s_idx].file[i] = file_name[i];
				gb_shared->log.e[e_idx].s[s_idx].file[i] = 0;
			}
			gb_shared->log.e[e_idx].count++;
		}

		xil_printf("\r\n(App Error Code:  %d) ", code_parameter);
		if (msg_err != NULL) xil_printf("%s",msg_err);
		if (file_name != NULL) xil_printf("\r\n     FILE: %s ", file_name);
		if (line != 0) xil_printf("\r\n     LINE: %u ", line);
	}
	return code_parameter;
}
