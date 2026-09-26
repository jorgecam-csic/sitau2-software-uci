# UCI

Control del equipo, registros, adquisición y transferencias DMA.

## Archivos

| Archivo | Función |
| --- | --- |
| [uci_data.c](uci_data.c) | Implementación: Escritura de datos y metadatos de adquisición: contador, tiempo y encoder. |
| [uci_data.h](uci_data.h) | Cabecera: Escritura de datos y metadatos de adquisición: contador, tiempo y encoder. |
| [uci_define.h](uci_define.h) | Cabecera: Constantes y definiciones compartidas del control UCI. |
| [uci_dma.c](uci_dma.c) | Implementación: Preparación y ejecución de transferencias DMA de adquisición y cierre de envíos al host. |
| [uci_dma.h](uci_dma.h) | Cabecera: Preparación y ejecución de transferencias DMA de adquisición y cierre de envíos al host. |
| [uci_error_code.h](uci_error_code.h) | Cabecera: Códigos de error del control UCI. |
| [uci_reg.c](uci_reg.c) | Implementación: Configuración por defecto, recepción de comandos, parada y disparos de adquisición. |
| [uci_reg.h](uci_reg.h) | Cabecera: Configuración por defecto, recepción de comandos, parada y disparos de adquisición. |
| [uci_set.c](uci_set.c) | Implementación: Programación de sincronismo, triggers, amplitud, contadores y señales de control. |
| [uci_set.h](uci_set.h) | Cabecera: Programación de sincronismo, triggers, amplitud, contadores y señales de control. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
