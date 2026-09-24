/*
 * datamover_error_code.h
 *
 *  Created on: 3 abr. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_DATAMOVER_DATAMOVER_ERROR_CODE_H_
#define SRC_SITAU2_DATAMOVER_DATAMOVER_ERROR_CODE_H_

#define afe_error_code_offset   -2000

// OK, resultado de la operacion correcto, no se ha producido ningun error.
#define DATAMOVER_NONE                  0

// Error, tratando de programar las muestras a adquirir por el AFE
#define ERROR_NUM_SAMPLES_AFE                 (afe_error_code_offset - 1)
#define charERROR_NUM_SAMPLES_AFE            "Error, demasiadas muestras de adquisición programadas"


#endif /* SRC_SITAU2_DATAMOVER_DATAMOVER_ERROR_CODE_H_ */
