# BCC_BUSSAR

Comunicación y programación de módulos remotos mediante BCC/BUSSAR.

## Archivos

| Archivo | Función |
| --- | --- |
| [bcc_bussar.c](bcc_bussar.c) | Implementación: Acceso a registros y comandos remotos BUSSAR/BCC, señales PRO y comprobaciones de enlace. |
| [bcc_bussar.h](bcc_bussar.h) | Cabecera: Acceso a registros y comandos remotos BUSSAR/BCC, señales PRO y comprobaciones de enlace. |
| [bcc_bussar_error_code.h](bcc_bussar_error_code.h) | Cabecera: Códigos de error de comunicación BCC/BUSSAR. |
| [bussar_addr.h](bussar_addr.h) | Cabecera: Mapa de direcciones y registros remotos BUSSAR. |
| [mcbcc_mst_driver.c](mcbcc_mst_driver.c) | Implementación: Control del maestro MCBCC: transferencias, cabeceras, interrupciones y señales de control. |
| [mcbcc_mst_driver.h](mcbcc_mst_driver.h) | Cabecera: Control del maestro MCBCC: transferencias, cabeceras, interrupciones y señales de control. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
