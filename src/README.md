# src

Fuentes mantenidos de las dos aplicaciones bare-metal Cortex-A9. Se enlazan al workspace externo y conservan sus codificaciones heredadas.

## Subcarpetas

- [cpu0](cpu0/README.md): CPU0 gestiona Ethernet, comunicación con el host y carga por red de CPU1 en RAM. Incluye código derivado de FSBL para el cargador secundario; no es el proyecto FSBL generado de la plataforma.
- [cpu1](cpu1/README.md): CPU1 controla adquisición y periféricos del SITAU2. Se compila como ELF y se empaqueta por separado para entrega por red.

## Contenido esperado y mantenimiento

Separar las aplicaciones por CPU. Los fuentes, cabeceras y linker scripts son mantenidos aquí; las copias de librerías Xilinx, BSP y ELF se generan fuera del repositorio. Preservar las codificaciones heredadas. Los recursos de Vitis están enlazados a este árbol, por lo que sus ediciones modifican Git.

Cambios en archivos existentes requieren recompilar; altas, bajas o movimientos requieren además generar de nuevo el workspace. Si cambia la organización de aplicaciones o sus includes, revisar `config/applications.tcl`.

[Volver](../README.md)
