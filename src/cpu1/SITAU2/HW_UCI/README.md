# HW_UCI

Servicios de CPU1: interrupciones, memoria compartida, caché, temporizadores, encoders y triggers.

## Archivos

| Archivo | Función |
| --- | --- |
| [TLV5626.c](TLV5626.c) | Implementación: Inicialización y escritura de valores en el DAC TLV5626. |
| [TLV5626.h](TLV5626.h) | Cabecera: Inicialización y escritura de valores en el DAC TLV5626. |
| [cons_prod_util.c](cons_prod_util.c) | Implementación: Buffers productor/consumidor para intercambio de datos y notificaciones entre procesadores. |
| [cons_prod_util.h](cons_prod_util.h) | Cabecera: Buffers productor/consumidor para intercambio de datos y notificaciones entre procesadores. |
| [encoder.c](encoder.c) | Implementación: Configuración, lectura de posición, filtrado y contadores de diagnóstico de encoders. |
| [encoder.h](encoder.h) | Cabecera: Configuración, lectura de posición, filtrado y contadores de diagnóstico de encoders. |
| [gic_utils.h](gic_utils.h) | Cabecera: Interfaz de inicialización, registro y habilitación de interrupciones GIC. |
| [gic_utils_CPU1.c](gic_utils_CPU1.c) | Implementación: Inicialización adaptada del GIC y gestión de interrupciones para CPU1. |
| [global.c](global.c) | Implementación: Definiciones globales e integración de utilidades del procesador. |
| [global.h](global.h) | Cabecera: Definiciones globales e integración de utilidades del procesador. |
| [mmu_config.c](mmu_config.c) | Implementación: Configuración de regiones de memoria y atributos de caché/MMU. |
| [mmu_config.h](mmu_config.h) | Cabecera: Configuración de regiones de memoria y atributos de caché/MMU. |
| [mutex_utils.c](mutex_utils.c) | Implementación: Inicialización y operaciones de bloqueo/desbloqueo de los mutex de hardware. |
| [mutex_utils.h](mutex_utils.h) | Cabecera: Inicialización y operaciones de bloqueo/desbloqueo de los mutex de hardware. |
| [platform.c](platform.c) | Implementación: Inicialización y limpieza de recursos de plataforma del procesador. |
| [platform.h](platform.h) | Cabecera: Inicialización y limpieza de recursos de plataforma del procesador. |
| [shared_mem.c](shared_mem.c) | Implementación: Inicialización o actualización del estado compartido entre CPU0 y CPU1. |
| [shared_mem.h](shared_mem.h) | Cabecera: Inicialización o actualización del estado compartido entre CPU0 y CPU1. |
| [shared_mem_def.h](shared_mem_def.h) | Cabecera: Direcciones y distribución de las regiones de memoria compartida. |
| [shared_mem_type.h](shared_mem_type.h) | Cabecera: Tipos y estructuras del estado compartido entre procesadores. |
| [timer_util.c](timer_util.c) | Implementación: Control de temporizadores hardware, cuentas periódicas, esperas e interrupciones. |
| [timer_util.h](timer_util.h) | Cabecera: Control de temporizadores hardware, cuentas periódicas, esperas e interrupciones. |
| [timestamp.c](timestamp.c) | Implementación: Contador temporal: inicialización, reinicio y lectura de ticks o tiempo. |
| [timestamp.h](timestamp.h) | Cabecera: Contador temporal: inicialización, reinicio y lectura de ticks o tiempo. |
| [trigger.c](trigger.c) | Implementación: Fuentes de disparo, callbacks de interrupción y contadores de triggers. |
| [trigger.h](trigger.h) | Cabecera: Fuentes de disparo, callbacks de interrupción y contadores de triggers. |
| [version_utils.c](version_utils.c) | Implementación: Registro, consulta y presentación de versiones software/hardware. |
| [version_utils.h](version_utils.h) | Cabecera: Registro, consulta y presentación de versiones software/hardware. |

Editar los fuentes aquí, no las copias generadas del workspace. Las cabeceras describen interfaces y datos; su presencia no garantiza que todas las rutinas se ejecuten en el flujo habitual.

## Contenido esperado y mantenimiento

Conservar aquí los fuentes, cabeceras y recursos mantenidos del módulo descrito arriba. Añadir archivos junto a los del mismo cometido y actualizar este inventario. Los objetos, bibliotecas compiladas y ejecutables se generan en el workspace, no en esta carpeta.

Las ediciones desde los recursos enlazados de Vitis afectan a estos archivos. Mantener su codificación heredada; al añadir, eliminar o mover archivos, generar de nuevo el workspace para reconstruir los enlaces. Los cambios en registros, direcciones o protocolos deben contrastarse con el hardware seleccionado.

[Volver](../README.md)
