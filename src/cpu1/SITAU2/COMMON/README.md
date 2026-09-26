# COMMON

Utilidades numéricas, registro de errores y temporización.

## Archivos

| Archivo | Función |
| --- | --- |
| [calc.c](calc.c) | Implementación: Conversiones numéricas, límites de tipos y funciones de redondeo. |
| [calc.h](calc.h) | Cabecera: Conversiones numéricas, límites de tipos y funciones de redondeo. |
| [log.c](log.c) | Implementación: Registro de errores mediante log_error. |
| [log.h](log.h) | Cabecera: Registro de errores mediante log_error. |
| [ttimer.c](ttimer.c) | Implementación: Abstracción de temporización: inicio, parada, esperas y timeouts. |
| [ttimer.h](ttimer.h) | Cabecera: Abstracción de temporización: inicio, parada, esperas y timeouts. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
