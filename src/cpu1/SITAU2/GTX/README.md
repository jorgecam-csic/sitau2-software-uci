# GTX

Control de enlaces GTX y de sus interfaces de datos.

## Archivos

| Archivo | Función |
| --- | --- |
| [axi2gtx.c](axi2gtx.c) | Implementación: Inicialización y envío por la FIFO de interfaz AXI a GTX. |
| [axi2gtx.h](axi2gtx.h) | Cabecera: Inicialización y envío por la FIFO de interfaz AXI a GTX. |
| [gtx_control.c](gtx_control.c) | Implementación: Configuración, encaminamiento, cabeceras y diagnóstico de enlaces GTX locales/remotos. |
| [gtx_control.h](gtx_control.h) | Cabecera: Configuración, encaminamiento, cabeceras y diagnóstico de enlaces GTX locales/remotos. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
