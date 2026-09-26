/*
 * bcc_bussar_error_code.h
 *
 *  Created on: 8 abr. 2019
 *      Author: Cruza
 */

#ifndef SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_ERROR_CODE_H_
#define SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_ERROR_CODE_H_


#define EBCCBUSSAR_NONE	0

#define bccbussar_error_code_offset          -100

#define EBCC_EN_PROC					(bccbussar_error_code_offset - 1)
#define charEBCC_EN_PROC				"Error, al intentar poner en procesamiento, el bus ya se encuentra en procesamiento"

#define EBCC_NO_PROC_FEEDBACK					(bccbussar_error_code_offset - 2)
#define charEBCC_NO_PROC_FEEDBACK				"Error, no ha habido realimentación de PRO=1 o ha llegado pasado el tiempo de espera"

#define EBCCBUSSAR_COMMNDBUFF_OVF					(bccbussar_error_code_offset - 3)
#define charEBCCBUSSAR_COMMNDBUFF_OVF				"Error, sobrepasado límite de datos en el buffer de envío de comandos al BCC"

#define EBCC_NO_FLAG_IN_TIME					(bccbussar_error_code_offset - 4)
#define charEBCC_NO_FLAG_IN_TIME				"Error, tiempo de espera superado esperando banderas del MCBCC_mst"

#define EBCC_MCBCC_MST_WR_ERROR					(bccbussar_error_code_offset - 5)
#define charEBCC_MCBCC_MST_WR_ERROR				"Error, tiempo de espera superado la escritura en el BCC"

#define EBCC_MCBCC_DATAMOVER_ERROR					(bccbussar_error_code_offset - 6)
#define charEBCC_MCBCC_DATAMOVER_ERROR				"Error, datamover no terminó adecuadamente en el envio de comandos al BCC"

#define EBCC_MCBCC_READ_NO_ACK_ERROR					(bccbussar_error_code_offset - 7)
#define charEBCC_MCBCC_READ_NO_ACK_ERROR				"Error, se realizó una lectura, pero el bit ACK de la cabecera de la respuesta no llegó a 1"

#define EBCC_INTERRUPT_LOST					(bccbussar_error_code_offset - 8)
#define charEBCC_INTERRUPT_LOST				"Error, se esperaba una interrupción que no se produjo, pero al leer el registro, la bandera de la interrupción YA estaba activada"

#define EBCC_INTERRUPT_LOST_TIMEOUT					(bccbussar_error_code_offset - 9)
#define charEBCC_INTERRUPT_LOST_TIMEOUT					"Error, se esperaba una interrupción, pero se superó el tiempo de espera y al leer el registro, la bandera de la interrupción estaba activada"

#define EBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_END					(bccbussar_error_code_offset - 10)
#define charEBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_END						"Error, no se pudo confirmar la finalización del datamover remoto"

#define EBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_OK					(bccbussar_error_code_offset - 11)
#define charEBCC_UCI2MINIBASE_REMOTE_DATAMOVER_NOT_OK						"Error, el datamover remoto no ha finalizado correctamente"


#endif /* SRC_SITAU2_BCC_BUSSAR_BCC_BUSSAR_ERROR_CODE_H_ */
