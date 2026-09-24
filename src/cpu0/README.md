# cpu0

CPU0 gestiona Ethernet, comunicación con el host y carga por red de CPU1 en RAM. Incluye código derivado de FSBL para el cargador secundario; no es el proyecto FSBL generado de la plataforma.

## Archivos

| Archivo | Función |
| --- | --- |
| [README.txt](README.txt) | Texto heredado del ejemplo lwIP TCP Perf Server; no describe el flujo actual de SITAU2. |
| [Xilinx.spec](Xilinx.spec) | Especificación GCC de archivos de inicio del enlace (crti y crtbegin). |
| [config_net_apps.h](config_net_apps.h) | Cabecera: Selección de las aplicaciones de red habilitadas. |
| [cons_prod_util.c](cons_prod_util.c) | Implementación: Buffers productor/consumidor para intercambio de datos y notificaciones entre procesadores. |
| [cons_prod_util.h](cons_prod_util.h) | Cabecera: Buffers productor/consumidor para intercambio de datos y notificaciones entre procesadores. |
| [fsbl.h](fsbl.h) | Cabecera: Definiciones del cargador derivadas de FSBL: estados, registros y rutinas de arranque. |
| [fsbl_debug.h](fsbl_debug.h) | Cabecera: Macros y niveles de mensajes del cargador. |
| [fsbl_handoff.S](fsbl_handoff.S) | Transferencia de ejecución en ensamblador ARM al programa cargado. |
| [fsbl_hooks.c](fsbl_hooks.c) | Implementación: Puntos de extensión antes/después de cargar el bitstream y antes de transferir el control. |
| [fsbl_hooks.h](fsbl_hooks.h) | Cabecera: Puntos de extensión antes/después de cargar el bitstream y antes de transferir el control. |
| [gic_utils.h](gic_utils.h) | Cabecera: Interfaz de inicialización, registro y habilitación de interrupciones GIC. |
| [gic_utils_CPU0.c](gic_utils_CPU0.c) | Implementación: Inicialización adaptada del GIC y gestión de interrupciones para CPU0. |
| [global.h](global.h) | Cabecera: Definiciones globales e integración de utilidades del procesador. |
| [image_mover_mod.c](image_mover_mod.c) | Implementación: Lectura y validación de cabeceras, traslado de particiones y recogida de direcciones de ejecución. |
| [image_mover_mod.h](image_mover_mod.h) | Cabecera: Lectura y validación de cabeceras, traslado de particiones y recogida de direcciones de ejecución. |
| [lscript.ld](lscript.ld) | Mapa de memoria, secciones, pila y heap del enlace; ubica la aplicación CPU0 en su región DDR. |
| [main.c](main.c) | Implementación: Entrada de CPU0: inicializa memoria compartida, QSPI, plataforma y red; ejecuta el bucle de comunicaciones. |
| [md5.c](md5.c) | Implementación: Cálculo MD5 utilizado por las rutinas de comprobación de imágenes. |
| [md5.h](md5.h) | Cabecera: Cálculo MD5 utilizado por las rutinas de comprobación de imágenes. |
| [mmu_config.c](mmu_config.c) | Implementación: Configuración de regiones de memoria y atributos de caché/MMU. |
| [mmu_config.h](mmu_config.h) | Cabecera: Configuración de regiones de memoria y atributos de caché/MMU. |
| [mutex_utils.c](mutex_utils.c) | Implementación: Inicialización y operaciones de bloqueo/desbloqueo de los mutex de hardware. |
| [mutex_utils.h](mutex_utils.h) | Cabecera: Inicialización y operaciones de bloqueo/desbloqueo de los mutex de hardware. |
| [nand.c](nand.c) | Implementación: Rutinas heredadas de inicialización y lectura NAND; su presencia no confirma uso en el equipo. |
| [nand.h](nand.h) | Cabecera: Rutinas heredadas de inicialización y lectura NAND; su presencia no confirma uso en el equipo. |
| [net_params.c](net_params.c) | Implementación: Parámetros de red, valores por defecto y recuperación de configuración. |
| [net_params.h](net_params.h) | Cabecera: Parámetros de red, valores por defecto y recuperación de configuración. |
| [net_rst_button.c](net_rst_button.c) | Implementación: Tratamiento del botón asociado al restablecimiento de red. |
| [net_rst_button.h](net_rst_button.h) | Cabecera: Tratamiento del botón asociado al restablecimiento de red. |
| [network.c](network.c) | Implementación: Inicialización lwIP/Ethernet, aplicaciones de red, temporizadores y supervisión del enlace. |
| [network.h](network.h) | Cabecera: Inicialización lwIP/Ethernet, aplicaciones de red, temporizadores y supervisión del enlace. |
| [nor.c](nor.c) | Implementación: Rutinas heredadas de acceso NOR; no constituyen el flujo confirmado de carga de CPU1. |
| [nor.h](nor.h) | Cabecera: Rutinas heredadas de acceso NOR; no constituyen el flujo confirmado de carga de CPU1. |
| [pcap.c](pcap.c) | Implementación: Inicialización PCAP, transferencia de datos y carga de particiones de lógica programable. |
| [pcap.h](pcap.h) | Cabecera: Inicialización PCAP, transferencia de datos y carga de particiones de lógica programable. |
| [platform.c](platform.c) | Implementación: Inicialización y limpieza de recursos de plataforma del procesador. |
| [platform.h](platform.h) | Cabecera: Inicialización y limpieza de recursos de plataforma del procesador. |
| [platform_config.h](platform_config.h) | Cabecera: Constantes de configuración hardware de la plataforma de red. |
| [qspi_simple.c](qspi_simple.c) | Implementación: Inicialización, lectura, escritura y borrado QSPI; CPU1 se carga por red, no se persiste aquí. |
| [qspi_simple.h](qspi_simple.h) | Cabecera: Inicialización, lectura, escritura y borrado QSPI; CPU1 se carga por red, no se persiste aquí. |
| [rsa.c](rsa.c) | Implementación: Rutinas de autenticación RSA heredadas del cargador; no implican que las imágenes actuales estén firmadas. |
| [rsa.h](rsa.h) | Cabecera: Rutinas de autenticación RSA heredadas del cargador; no implican que las imágenes actuales estén firmadas. |
| [sd.c](sd.c) | Implementación: Rutinas heredadas de acceso a tarjeta SD. |
| [sd.h](sd.h) | Cabecera: Rutinas heredadas de acceso a tarjeta SD. |
| [shared_mem.c](shared_mem.c) | Implementación: Inicialización o actualización del estado compartido entre CPU0 y CPU1. |
| [shared_mem.h](shared_mem.h) | Cabecera: Inicialización o actualización del estado compartido entre CPU0 y CPU1. |
| [shared_mem_def.h](shared_mem_def.h) | Cabecera: Direcciones y distribución de las regiones de memoria compartida. |
| [shared_mem_type.h](shared_mem_type.h) | Cabecera: Tipos y estructuras del estado compartido entre procesadores. |
| [ssbl.c](ssbl.c) | Implementación: Cargador secundario desde RAM: procesa la imagen y señala el arranque de CPU1. |
| [ssbl.h](ssbl.h) | Cabecera: Cargador secundario desde RAM: procesa la imagen y señala el arranque de CPU1. |
| [tcpcom.c](tcpcom.c) | Implementación: Servidor de comandos TCP: transferencia a RAM, LOAD_N_RUN, acceso QSPI y comunicación con CPU1. |
| [tcpcom.h](tcpcom.h) | Cabecera: Servidor de comandos TCP: transferencia a RAM, LOAD_N_RUN, acceso QSPI y comunicación con CPU1. |
| [tcpimage.c](tcpimage.c) | Implementación: Envío TCP de datos de imagen y control de su buffer. |
| [tcpimage.h](tcpimage.h) | Cabecera: Envío TCP de datos de imagen y control de su buffer. |
| [version_utils.c](version_utils.c) | Implementación: Registro, consulta y presentación de versiones software/hardware. |
| [version_utils.h](version_utils.h) | Cabecera: Registro, consulta y presentación de versiones software/hardware. |
| [xcugic_mod_hw.c](xcugic_mod_hw.c) | Implementación: Rutinas adaptadas de inicialización del controlador de interrupciones GIC. |
| [xcugic_mod_hw.h](xcugic_mod_hw.h) | Cabecera: Rutinas adaptadas de inicialización del controlador de interrupciones GIC. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
