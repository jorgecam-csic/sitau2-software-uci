# cpu1

CPU1 controla adquisición y periféricos del SITAU2. Se compila como ELF y se empaqueta por separado para entrega por red.

## Archivos

| Archivo | Función |
| --- | --- |
| [Xilinx.spec](Xilinx.spec) | Especificación GCC de archivos de inicio del enlace (crti y crtbegin). |
| [ge_sitau_uci.c](ge_sitau_uci.c) | Implementación: Rutinas auxiliares heredadas para preparar adquisición y esperar su finalización. |
| [ge_sitau_uci.h](ge_sitau_uci.h) | Cabecera: Rutinas auxiliares heredadas para preparar adquisición y esperar su finalización. |
| [lscript.ld](lscript.ld) | Mapa de memoria, secciones, pila y heap del enlace; CPU1 comienza en DDR 0x18000000. |
| [main_uci.c](main_uci.c) | Implementación: Entrada de CPU1: inicialización del equipo y ejecución de su lógica de control. |

## Subcarpetas

- [SITAU2](SITAU2/README.md): Controladores y servicios de hardware organizados por bloque funcional.
- [UI](UI/README.md): Protocolo y lógica de control de adquisición y canales virtuales. UI aquí no es una interfaz gráfica de escritorio.

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
