# DATAMOVER

Movimiento de datos entre memoria y streams, local y remoto.

## Archivos

| Archivo | Función |
| --- | --- |
| [datamover_driver.c](datamover_driver.c) | Implementación: Transferencias memoria/stream mediante DataMover y gestión de sus interrupciones. |
| [datamover_driver.h](datamover_driver.h) | Cabecera: Transferencias memoria/stream mediante DataMover y gestión de sus interrupciones. |
| [datamover_error_code.h](datamover_error_code.h) | Cabecera: Códigos de error del DataMover. |
| [datamover_remote.c](datamover_remote.c) | Implementación: Construcción de comandos y direcciones de transferencias DataMover remotas. |
| [datamover_remote.h](datamover_remote.h) | Cabecera: Construcción de comandos y direcciones de transferencias DataMover remotas. |
| [datamover_structs.h](datamover_structs.h) | Cabecera: Estructuras y campos de control utilizados por DataMover. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
