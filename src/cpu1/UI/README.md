# UI

Protocolo y lógica de control de adquisición y canales virtuales. UI aquí no es una interfaz gráfica de escritorio.

## Archivos

| Archivo | Función |
| --- | --- |
| [fppa_uci_acq.c](fppa_uci_acq.c) | Implementación: Rutina BF_UCI_Beamform_FP_PA de adquisición/conformación FP/PA. |
| [protocol.c](protocol.c) | Implementación: Decodificación de comandos recibidos y envío de respuestas/estado UCI. |
| [protocol.h](protocol.h) | Cabecera: Decodificación de comandos recibidos y envío de respuestas/estado UCI. |
| [uci_acquire.c](uci_acquire.c) | Implementación: Coordinación de adquisición, leyes focales y beamforming. |
| [uci_acquire.h](uci_acquire.h) | Cabecera: Coordinación de adquisición, leyes focales y beamforming. |
| [uci_fsm.c](uci_fsm.c) | Implementación: Máquina de estados principal de control de adquisición. |
| [uci_fsm.h](uci_fsm.h) | Cabecera: Máquina de estados principal de control de adquisición. |
| [uci_phased_array.c](uci_phased_array.c) | Implementación: Utilidad TimeToTicks para convertir tiempos a ticks del reloj FCLK4. |
| [uci_phased_array.h](uci_phased_array.h) | Cabecera reservada de phased array, actualmente sin declaraciones funcionales. |
| [uci_process.h](uci_process.h) | Cabecera: Cabecera reservada, actualmente sin declaraciones funcionales. |
| [vch.c](vch.c) | Implementación: Configuración y estado de canales virtuales, FIR, TGC, leyes focales y tamaño de adquisición. |
| [vch.h](vch.h) | Cabecera: Configuración y estado de canales virtuales, FIR, TGC, leyes focales y tamaño de adquisición. |
| [vch_prg.c](vch_prg.c) | Implementación: Traducción de la configuración de canales virtuales a registros y leyes focales hardware. |
| [vch_prg.h](vch_prg.h) | Cabecera: Traducción de la configuración de canales virtuales a registros y leyes focales hardware. |
| [vch_tad.h](vch_tad.h) | Cabecera: Tipos, límites y estructuras de canales virtuales y modos de adquisición. |

## Subcarpetas

- [ACQ](ACQ/README.md): Implementaciones de los modos de adquisición FMC, PA y TFM.

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
