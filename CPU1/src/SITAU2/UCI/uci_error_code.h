#ifndef uci_error_codeH
#define uci_error_codeH

#define uci_error_code_offset       0

// OK, resultado de la operacion correcto, no se ha producido ningun error.
#define EUCI_NONE                   0

#define EUCI_BASE_Test              (uci_error_code_offset - 1)
#define charEUCI_BASE_Test          "Error: Una base no se ha configurado correctamente"

#define EUCI_MOD_TEST               (uci_error_code_offset - 2)
#define charEUCI_MOD_TEST           "Error: Un mï¿½dulo no se ha configurado correctamente"

#define EUCI_Parameter              (uci_error_code_offset - 3)
#define charEUCI_Parameter          "Error: Rango de datos no vï¿½lido"

#define EUCI_Timeout                (uci_error_code_offset - 4)
#define charEUCI_Timeout            "Se ha salido de un bucle por timeout"

#define EUCI_Channel                (uci_error_code_offset - 5)
#define charEUCI_Channel            "Error nï¿½mero de canales, el numero de canales es distinto al configurado por la funciï¿½n ST_OpenSys()"

#define EUCI_NoMemory               (uci_error_code_offset - 6)
#define charEUCI_NoMemory           "Error en la reserva de memoria dinï¿½mica"

#define EUCI_RUN_NoData             (uci_error_code_offset - 7)
#define charEUCI_RUN_NoData         "Error intento de disparo con cero muestras programado"

#define EUCI_MemoryOverflow         (uci_error_code_offset - 8)
#define charEUCI_MemoryOverflow     "Error desbordamiento de la memoria interna del equipo"

#define EUCI_ZeroDivision		      (uci_error_code_offset - 9)
#define charEUCI_ZeroDivision	      "Se va a intentar dividir por cero. Se aborta la operaciï¿½n"

#define EUCI_InvalidAccess          (uci_error_code_offset - 10)
#define charEUCI_InvalidAccess      "Error Se ha intentado acceder a una posiciï¿½n no vï¿½lida del vector"

#define EUCI_AD_TEST                (uci_error_code_offset - 11)
#define charEUCI_AD_TEST            "Error en los conversores AD de un mï¿½dulo con el patrï¿½n de test"

#define EUCI_DataInconsistency      (uci_error_code_offset - 12)
#define charEUCI_DataInconsistency 	"Error, inconsistencia entre los datos"

#define EUCI_PtrAccess_NULL			(uci_error_code_offset - 13)
#define charEUCI_PtrAccess_NULL     "ERROR: Acceso a puntero NULL."

#define EUCI_MathFunction           (uci_error_code_offset - 14)
#define charEUCI_MathFunction       "Error en el cï¿½lculo de una funciï¿½n matemï¿½tica."

#define EUCI_IdEncoder              (uci_error_code_offset - 15)
#define charEUCI_IdEncoder			   "Error de nï¿½mero del encoder, el encoder al que se desea acceder esta fuera de rango"

#define EUCI_IdTimer                (uci_error_code_offset - 16)
#define charEUCI_IdTimer			   "Error de nï¿½mero del encoder, el encoder al que se desea acceder esta fuera de rango"

#define EUCI_ReceiverDataNumber     (uci_error_code_offset - 17)
#define charEUCI_ReceiverDataNumber	"Error, el nï¿½mero de datos recibido es diferente al tamaï¿½o del tipo de datos correspondiente"

#define EUCI_Start_DMA_not_end     (uci_error_code_offset - 18)
#define charEUCI_Start_DMA_not_end	"Error, se ha intentado iniciar la DMA sin haber finalizado la transacciï¿½n anterior"

#define EUCI_xxxxx_ERROR_LIBRE_USAR     (uci_error_code_offset - 19)
#define charEUCI_xxxxx_ERROR_LIBRE_USAR	"Error, xxxxx_ERROR_LIBRE_USAR"

#define EUCI_no_mem_img_buffer     (uci_error_code_offset - 20)
#define charEUCI_no_mem_img_buffer	"Sin memoria en el buffer"

#define EUCI_max_bytes_datamover     (uci_error_code_offset - 21)
#define charEUCI_max_bytes_datamover	"Superado tamaï¿½o mï¿½ximo del datamover"

#define EUCI_MCBCC_MST_rd_stream_timeout     (uci_error_code_offset - 22)
#define charEUCI_MCBCC_MST_rd_stream_timeout	"Superado tiempo de espera en la funciï¿½n esperando a recibir datos en rï¿½faga desde el mï¿½dulo del amplia al MCBCC mst."

#define EUCI_MCBCC_MST_rd_stream_more_data_on_stream     (uci_error_code_offset - 23)
#define charEUCI_MCBCC_MST_rd_stream_more_data_on_stream	"Despuï¿½s de una recepciï¿½n en rï¿½faga completa, hay mï¿½s datos en el stream de entrada. Puede deberse a un error de configuraciï¿½n UCI-SISTEMA o puede ser premeditado."

#define EUCI_DMA_not_set     (uci_error_code_offset - 24)
#define charEUCI_DMA_not_set	"Tratando de iniciar transacciï¿½n de AMPLIA a DMA sin haber iniciado DMA"

#define EUCI_DMA_too_much_data     (uci_error_code_offset - 25)
#define charEUCI_DMA_too_much_data	"El cantidad de datos solicitada desde AMPLIA a la DMA es mayor que la queda en la DMA"

#define EUCI_DMA_MCBCC_Write_word16_busy     (uci_error_code_offset - 26)
#define charEUCI_DMA_MCBCC_Write_word16_busy	"MCBCC se encuentra en medio de una transacciï¿½n de lectura en rï¿½faga cuando se ha intentado escribir una palabra al stream"

#define EUCI_DMA_Write_word16_Stream_nrdy     (uci_error_code_offset - 27)
#define charEUCI_DMA_Write_word16_Stream_nrdy	"El stream to_sswich no estï¿½ listo para escritura (RDY=0) al intentar escribir una palabra"

#define EUCI_DMA_Closing_not_zero_word16_left     (uci_error_code_offset - 28)
#define charEUCI_DMA_Closing_not_zero_word16_left	"Se ha intentado cerrar la imagen para el envï¿½o, pero todavï¿½a quedan datos por enviar"

#define EUCI_DMA_Closing_Datamover_S2MM_not_end     (uci_error_code_offset - 29)
#define charEUCI_DMA_Closing_Datamover_S2MM_not_end	"Se ha intentado cerrar la imagen para el envï¿½o, pero todavï¿½a el driver del datamover no ha indicado que ha terminado el canal S2MM"

#define EUCI_MCBCC_MST_rd_stream_not_end     (uci_error_code_offset - 30)
#define charEUCI_MCBCC_MST_rd_stream_not_end	"En la funciï¿½n esperando a recibir datos en rï¿½faga desde el mï¿½dulo del amplia al MCBCC mst, se ha hecho un sondeo con timeout=0 y aï¿½n no ha terminado"

#define EUCI_DMA_Closing_Datamover_S2MM_end_with_error     (uci_error_code_offset - 31)
#define charEUCI_DMA_Closing_Datamover_S2MM_end_with_error	"DMA S2MM ERROR: TLAST comes early or late or never"

#define EUCI_DMA_Closing_Datamover_S2MM_end_with_unknown_error     (uci_error_code_offset - 32)
#define charEUCI_DMA_Closing_Datamover_S2MM_end_with_unknown_error	"DMA S2MM ERROR: Unkown error."

#define EUCI_TRIG_Trigger_timeout     (uci_error_code_offset - 33)
#define charEUCI_TRIG_Trigger_timeout	"TRIGGER ERROR: Waiting Trigger function timeout."

#define EUCI_IdVirtualChannel    (uci_error_code_offset - 34)
#define charEUCI_IdVirtualChannel			   "Error de nï¿½mero de canal virtual, el canal virtual al que se desea acceder esta fuera de rango"

#define EUCI_SamplesNumber    (uci_error_code_offset - 35)
#define charEUCI_SamplesNumber			   "Error en el nï¿½mero de muestras."

#define EUCI_AScanNumber    (uci_error_code_offset - 36)
#define charEUCI_AScanNumber			   "Error en el nï¿½mero de A-Scan."

#define EUCI_IdBASE                 (uci_error_code_offset - 37)
#define charEUCI_IdBASE			      "Error, BASE Index Overflow"

#define EUCI_IdMODULE                 (uci_error_code_offset - 38)
#define charEUCI_IdMODULE			      "Error, MODULE Index Overflow"

#define EUCI_IdPASys                 (uci_error_code_offset - 39)
#define charEUCI_IdPASys			      "Error, PA System Index Overflow"

#define EUCI_AMPLIACommand_Number             (uci_error_code_offset - 40)
#define charEUCI_AMPLIACommand_Number          "Error en el nÃºmero de comandos AMPLIA"

#define EUCI_BCC_PRO_RISE            (uci_error_code_offset - 41)
#define charEUCI_BCC_PRO_RISE          "Error, time out del flag PORF (Pro Rise Flag)"

#define EUCI_BCC_PRO_FALL            (uci_error_code_offset - 42)
#define charEUCI_BCC_PRO_FALL          "Error, time out del flag POFF (Pro Fall Flag)"

#define EUCI_ETH_STUCK	            (uci_error_code_offset - 43)
#define charEUCI_ETH_STUCK          "Error, time out en la espera a la Ethernet, el buffer no se vacía"

#define EUCI_IsRunning	            (uci_error_code_offset - 44)
#define charEUCI_IsRunning         "Error, configuration attempt when the system was running"

#endif
