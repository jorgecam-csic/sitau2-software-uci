# SWITCH

Encaminamiento de streams entre bloques hardware.

## Archivos

| Archivo | Función |
| --- | --- |
| [dspdma_func.c](dspdma_func.c) | Implementación: Configuración de la conexión AXI4 a GTX. |
| [dspdma_func.h](dspdma_func.h) | Cabecera: Configuración de la conexión AXI4 a GTX. |
| [switch_driver.c](switch_driver.c) | Implementación: Selección de conexiones maestro/esclavo y comandos de switches locales/remotos. |
| [switch_driver.h](switch_driver.h) | Cabecera: Selección de conexiones maestro/esclavo y comandos de switches locales/remotos. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
