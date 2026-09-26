# AFE

Configuración y calibración del frontal analógico AFE.

## Archivos

| Archivo | Función |
| --- | --- |
| [afe5808a.c](afe5808a.c) | Implementación: Configuración SPI, calibración, alineamiento y retardos de los convertidores AFE. |
| [afe5808a.h](afe5808a.h) | Cabecera: Configuración SPI, calibración, alineamiento y retardos de los convertidores AFE. |
| [afe_bussar.c](afe_bussar.c) | Implementación: Comandos de programación y alineamiento remoto de AFE mediante BUSSAR. |
| [afe_bussar.h](afe_bussar.h) | Cabecera: Comandos de programación y alineamiento remoto de AFE mediante BUSSAR. |
| [afe_error_code.h](afe_error_code.h) | Cabecera: Códigos de error del subsistema AFE. |
| [delays_afe.h](delays_afe.h) | Cabecera: Datos de retardos para la configuración/alineamiento del AFE. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
