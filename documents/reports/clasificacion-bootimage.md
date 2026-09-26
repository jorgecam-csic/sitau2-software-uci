# Clasificación del bootimage heredado

**Estado actual (24/09/2026):** este documento es un inventario histórico. Se eliminaron las imágenes CPU restantes y las carpetas de variantes MK32; el trío de la raíz BIN se copió como `artifacts/mk32/0.0.0` y posteriormente se eliminó BIN completo por autorización del usuario. Véase [limpieza y pendientes](limpieza-mk32.md).

Actualización posterior: las siete recetas/salidas de nivel superior se han archivado fuera del repositorio al sustituir el empaquetado por las recetas de `config/bootimage`. Véase `bootimage-archived.json` para las rutas y hashes. El inventario siguiente conserva las ubicaciones originales; en esa fecha los 105 archivos de `CPU1_system/_ide/bootimage/BIN` permanecían intactos.

Fecha: 23 de septiembre de 2026. Clasificación completada sin mover, borrar ni modificar los archivos originales. Se comprobaron todos los SHA-256 contra el inventario anterior.

Ámbito: **112 archivos**, **416.78 MiB**, 107 presentes en el índice Git. Los scripts y launch de otros directorios de _ide quedan fuera de esta clasificación.

## Familias y destino propuesto

Los destinos son una propuesta para el traslado posterior, no carpetas creadas ni nuevas dependencias activas. El archivo histórico no debe añadirse al lock activo por el mero hecho de conservarlo.

| Familia | Archivos | Tratamiento propuesto |
| --- | ---: | --- |
| archivo-comprimido | 2 | Archivo histórico: archives; verificar antes de deduplicar. |
| auxiliar-pendiente | 2 | Archivo histórico: auxiliary; revisar utilidad con el programador. |
| configuracion-equipo | 7 | Archivo histórico: equipment-config; confirmar qué aplicación los consume. |
| hardware-mk32 | 87 | Paquetes de hardware MK32 por contenido; mantener juntos bin, bit y ltx. |
| imagen-cpu0 | 2 | Archivo histórico: images/cpu0-boot; conservar ambas imágenes. |
| imagen-cpu1 | 6 | Archivo histórico: images/cpu1-load; conservar todas hasta identificar la utilizada. |
| receta | 2 | Conservar BIF originales como referencia; futuras recetas portables en config/bootimage. |
| registro-prueba | 4 | Archivo histórico: diagnostics; fuera de las dependencias de compilación. |

## Imágenes de software

Las ocho imágenes inspeccionadas tienen cabeceras de partición con suma válida. Esto no demuestra que funcionen ni que correspondan al firmware actual. Todas tienen SHA-256 distintos; ninguna es un duplicado exacto de otra.

| Ruta | Bytes | SHA-256 abreviado | Particiones |
| --- | ---: | --- | --- |
| Both_CPUs_system/_ide/bootimage/BOOT.bin | 13622280 | 62ff4e08b5ff | 0x10 @ 0x00000000; 0x20 @ 0x00000000; 0x10 @ 0x00100000 |
| Both_CPUs_system/_ide/bootimage/Flash_v2_0_8.bin | 13622280 | 85d2ec8e46c0 | 0x10 @ 0x00000000; 0x20 @ 0x00000000; 0x10 @ 0x00100000 |
| CPU1_system/_ide/bootimage/BIN/BOOT.bin | 349968 | 9acb0d5f70e4 | 0x10 @ 0x18000000 |
| CPU1_system/_ide/bootimage/BIN/BOOT.bin_old | 349968 | cd842051051f | 0x10 @ 0x18000000 |
| CPU1_system/_ide/bootimage/BIN/BOOT.binmal | 349968 | 89c9b116902a | 0x10 @ 0x18000000 |
| CPU1_system/_ide/bootimage/BIN/BOOT_old.bin | 317200 | 199b6e21ed7e | 0x10 @ 0x18000000 |
| CPU1_system/_ide/bootimage/BIN/BOOT_UCI.bin | 317200 | d7ce4d795de5 | 0x10 @ 0x18000000 |
| CPU1_system/_ide/bootimage/BOOT.bin | 349968 | 88e63509656b | 0x10 @ 0x18000000 |

Atributo 0x10: software PS; 0x20: lógica PL. Both_CPUs_system contiene FSBL + PL + CPU0; las seis imágenes de CPU1 contienen una partición con entrada 0x18000000. Los sufijos old, mal y la etiqueta v2_0_8 son nombres heredados, no estados validados. Véase [el análisis del cargador](carga-cpu1-desde-cpu0.md).

## Variantes MK32

Hay 29 carpetas con tríos completos .bin/.bit/.ltx y 23 conjuntos distintos por contenido. Se agrupan por los tres hashes; compartir solo un LTX no implica que dos bitstreams sean iguales. Los nombres no permiten elegir la variante de producción.

| ID de conjunto | Carpetas originales bajo CPU1_system/_ide/bootimage/BIN |
| --- | --- |
| 06185b15b86d | MK32_debug_addr_ida_0fallos |
| 1d5956e227b2 | MK32_arreglos y debug conformadores |
| 248bb9f8bcf0 | MK32_conformador_arreglado_error_datos_entrada_acabados |
| 34cbd7bd7b10 | Mailyn; MK32_pocodebug_1lineamal |
| 5075d959faf5 | MK32_pocodebug_7lineamal |
| 50e7ce059259 | MK32_ce_prom_emi_beamformer |
| 64b8889aa85d | MK32_debugprom2lineasmal |
| 70e59c4c6613 | MK32_con2DMAs |
| 736cb3958cdb | MK32_debug_mem; (raíz BIN) |
| 780b5e75ed77 | MK32_intento_arreglo_promediado |
| 7db9e9408792 | Bitstream_mod_beamformers |
| 7dbee4cce520 | MK2_mucho_Debug |
| 8f07644f9450 | MK32_con_debug_conformador_sin_fallos |
| 9398430fad19 | MK32_confarreglado |
| 9ff48254c7f3 | MK32_mem_ida_0fallos |
| b74306f3f15f | impl_1; MK32_debugpromediado; MK32_previo_a_arregloconformadores |
| bc0dc11ad755 | MK32_a_ver |
| bd82e447a165 | MK32_DATAMOVER_PARA_TFM_8_FALLOS; MK32_funciona_ulti; tmp |
| d0ebb3161e82 | MK32_ce_prom_emi_beamformer_debug_4fallostemporales |
| d6b75fbe8dd7 | impl_1_zip |
| e01731956439 | MK32_con_debug_alineamiento |
| e091725c1060 | MK32_debug_512bits |
| e7fd574ed97e | MK32_debug_T0_I |

El conjunto de la raíz BIN es idéntico al de MK32_debug_mem. Debe preservarse la asociación del LTX con su bitstream incluso cuando algún LTX esté repetido.

## Comprimidos, configuraciones y pruebas

- impl_1.zip contiene un trío cuyos tres miembros coinciden por SHA-256 con impl_1_zip. El directorio impl_1 contiene otra variante; no se debe sustituir por impl_1_zip por similitud de nombre.
- impl_1.rar queda pendiente de inspección interna; no se considera duplicado del ZIP.
- SITAU.XML contiene configuración de registros bajo SITAU2. Los otros cinco XML describen el equipo bajo SITAU; _SITAU.XML y SITAU_hypertronix.XML son idénticos.
- SITAU.TXT es un informe de configuración de hardware. Su vigencia y consumidor quedan pendientes.
- LOG_SERVER_SEND.TXT y output_dasel_2024-03-11T14-23-36.csv son registros de comunicaciones/datos de una prueba. Los dos writebif.log conservan trazas de generación y rutas antiguas.
- orden_canales.csv contiene una tabla de canales. Table.xlsx se conserva como auxiliar sin interpretar su contenido.

## Duplicados exactos

La igualdad se ha comprobado por SHA-256. No se ha eliminado ninguna copia. Las rutas originales completas y los hashes están en [el catálogo JSON](bootimage-clasificacion.json).

- 7dfe34c579d3: _SITAU.XML; SITAU_hypertronix.XML
- 48c32d376caf: impl_1/MK32CH_basics_wrapper.bin; MK32_debugpromediado/MK32CH_basics_wrapper.bin; MK32_previo_a_arregloconformadores/MK32CH_basics_wrapper.bin
- f3e105d12a77: impl_1/MK32CH_basics_wrapper.bit; MK32_debugpromediado/MK32CH_basics_wrapper.bit; MK32_previo_a_arregloconformadores/MK32CH_basics_wrapper.bit
- 7bab51eb5f0b: impl_1/MK32CH_basics_wrapper.ltx; MK32_debugpromediado/MK32CH_basics_wrapper.ltx; MK32_previo_a_arregloconformadores/MK32CH_basics_wrapper.ltx
- 45af77f2b4b4: Mailyn/MK32CH_basics_wrapper.bin; MK32_pocodebug_1lineamal/MK32CH_basics_wrapper.bin
- 01c1cbd3f123: Mailyn/MK32CH_basics_wrapper.bit; MK32_pocodebug_1lineamal/MK32CH_basics_wrapper.bit
- 12fa2695c2c7: Mailyn/MK32CH_basics_wrapper.ltx; MK32_pocodebug_1lineamal/MK32CH_basics_wrapper.ltx
- ca1d2ecb5e0e: MK32_a_ver/MK32CH_basics_wrapper.ltx; MK32_debug_addr_ida_0fallos/MK32CH_basics_wrapper.ltx; MK32_mem_ida_0fallos/MK32CH_basics_wrapper.ltx
- c393a72c2593: MK32_arreglos y debug conformadores/MK32CH_basics_wrapper.ltx; MK32_intento_arreglo_promediado/MK32CH_basics_wrapper.ltx
- 0905638d7ebe: MK32_DATAMOVER_PARA_TFM_8_FALLOS/MK32CH_basics_wrapper.bin; MK32_funciona_ulti/MK32CH_basics_wrapper.bin; tmp/MK32CH_basics_wrapper.bin
- aec58e75d956: MK32_DATAMOVER_PARA_TFM_8_FALLOS/MK32CH_basics_wrapper.bit; MK32_funciona_ulti/MK32CH_basics_wrapper.bit; tmp/MK32CH_basics_wrapper.bit
- 79151d4b188f: MK32_DATAMOVER_PARA_TFM_8_FALLOS/MK32CH_basics_wrapper.ltx; MK32_funciona_ulti/MK32CH_basics_wrapper.ltx; tmp/MK32CH_basics_wrapper.ltx
- 51c409777735: MK32_debug_mem/MK32CH_basics_wrapper.bin; MK32CH_basics_wrapper.bin
- dcd88f6c880c: MK32_debug_mem/MK32CH_basics_wrapper.bit; MK32CH_basics_wrapper.bit
- 75ba1420ebea: MK32_debug_mem/MK32CH_basics_wrapper.ltx; MK32CH_basics_wrapper.ltx

## Orden de traslado propuesto

1. Conservar este catálogo como mapa de procedencia y mantener el respaldo original.
2. Separar recetas portables de imágenes ya compiladas: config/bootimage para las primeras y un archivo histórico para las segundas.
3. Archivar las variantes MK32 como conjuntos indivisibles identificados por hash, conservando sus nombres antiguos como alias. Solo la variante confirmada debe promocionarse a una dependencia activa de artifacts.
4. Separar configuraciones, auxiliares y diagnósticos de las entradas de compilación.
5. Tras confirmar el flujo de carga, automatizar por separado el arranque CPU0 y el paquete de carga CPU1; no sustituir el segundo por el BOOT automático de Vitis.

No se han decidido eliminaciones ni la ubicación definitiva del archivo histórico (repositorio o almacenamiento externo). La clasificación permite hacerlo sin confundir variantes, salidas de compilación y datos de pruebas.
