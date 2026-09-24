# bsp

Parámetros MSS de los dominios BSP de CPU0, CPU1 y FSBL. CPU0 selecciona explícitamente `lwip211 1.08.s`, la variante SITAU2 construida desde la distribución Xilinx 1.8. La original no satisface esa selección exacta.

## Archivos

| Archivo | Función |
| --- | --- |
| [cpu0.mss](cpu0.mss) | Configuración del BSP standalone de CPU0, incluidas bibliotecas de red. |
| [cpu1.mss](cpu1.mss) | Configuración del BSP standalone de CPU1. |
| [fsbl.mss](fsbl.mss) | Configuración del BSP del FSBL generado por Vitis. |

[Volver](../README.md)
