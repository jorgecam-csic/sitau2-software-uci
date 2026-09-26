# ACQ

Implementaciones de los modos de adquisición FMC, PA y TFM.

## Archivos

| Archivo | Función |
| --- | --- |
| [ACQ_FMC - copia.c_previoaPAUSA](ACQ_FMC%20-%20copia.c_previoaPAUSA) | Copia histórica anterior a los cambios de pausa; no es la unidad C activa ACQ_FMC.c. |
| [ACQ_FMC.c](ACQ_FMC.c) | Implementación: Control de adquisición FMC: preparación, triggers, pausa/parada y transferencia de datos. |
| [ACQ_FMC.h](ACQ_FMC.h) | Cabecera: Control de adquisición FMC: preparación, triggers, pausa/parada y transferencia de datos. |
| [ACQ_PA.c](ACQ_PA.c) | Implementación: Control de adquisición phased array: configuración, triggers y envío de cabeceras/imágenes. |
| [ACQ_PA.h](ACQ_PA.h) | Cabecera: Control de adquisición phased array: configuración, triggers y envío de cabeceras/imágenes. |
| [ACQ_TFM.c](ACQ_TFM.c) | Implementación: Máquina de estados y transferencias de adquisición TFM, ráfagas y leyes focales. |
| [ACQ_TFM.h](ACQ_TFM.h) | Cabecera: Máquina de estados y transferencias de adquisición TFM, ráfagas y leyes focales. |
| [ACQ_common.c](ACQ_common.c) | Implementación: Utilidades comunes de adquisición, incluido diagnóstico de beamformer. |
| [ACQ_common.h](ACQ_common.h) | Cabecera: Utilidades comunes de adquisición, incluido diagnóstico de beamformer. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
