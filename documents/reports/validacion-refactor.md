# Validación del workspace reproducible

Fecha: 22 de septiembre de 2026. Herramientas: Vitis Classic 2022.2 en Windows,
Windows PowerShell 5.1 y configuración Debug.

## Entradas y conservación

- Hardware seleccionado: `artifacts/uci/imported-3ed161d88aff/design_1_wrapper.xsa`.
  El lock y el manifiesto verifican su SHA-256 antes de generar o compilar.
- Se conservan también los otros dos XSA distintos encontrados, con identificadores
  derivados de su hash. No se seleccionan automáticamente.
- Los 177 archivos de fuentes versionados en `src/` coinciden byte a byte con los
  del respaldo `sitau2_sw_uci`. No se convierten sus codificaciones.
  `-text` evita normalizaciones al pasar por Git. Algunos blobs del commit inicial
  tenían LF normalizado; el diff puede reflejar finales de línea al conservar ahora
  los CRLF originales. La comparación binaria se hizo contra los archivos locales.
- Se mantienen las configuraciones BSP y dos personalizaciones lwIP que la generación
  estándar no reproduce. Véase [auditoría BSP](auditoria-bsp.md).
  La [comparación final](bsp-final-comparison.json) encuentra 2601 archivos idénticos
  a la referencia y siete equivalentes con distinto orden de bloques MSS o declaraciones
  de `xparameters.h`. Solo falta el MSS auxiliar heredado `system_1.mss`, no seleccionado.
- Los archivos de ambos `_ide/bootimage` permanecen en sus ubicaciones y sin cambios;
  el [inventario](bootimage-inventory.json) identifica los versionados y los locales.
- El respaldo `D:\sitau2\sitau2_sw_uci` no se modifica. La referencia de la compilación
  heredada se conserva fuera del repo, en `../sitau2-software-uci-work/reference`.

## Pruebas del generador

Se exportó un árbol de Git mediante un índice temporal y `git archive`, sin utilizar
los proyectos, BSP, ejecutables ni metadatos Eclipse locales. La copia de prueba se
creó en `D:\sitau2\sitau2-software-uci-validation` y genera su entorno en la carpeta
hermana `sitau2-software-uci-validation-work`.

También se ejecutó la receta desde el repositorio de trabajo, usando su carpeta
hermana `sitau2-software-uci-work`. Ambas compilaciones terminaron con código 0 y
confirmación `SITAU_OK:build`. Se comprobaron las cinco salidas y los enlaces a fuentes
del checkout correspondiente, sin referencias antiguas `C:/PV/` en los archivos de
construcción generados. Los manifiestos conservan tamaños y SHA-256:

- [Entorno principal](build-manifest.json).
- [Copia limpia de Git](clean-build-manifest.json).

| Salida | Resultado |
|---|---|
| CPU0/Debug/CPU0.elf | Generado y verificado en ambos entornos |
| CPU1/Debug/CPU1.elf | Generado y verificado en ambos entornos |
| Platform/zynq_fsbl/fsbl.elf | Generado; secciones cargables iguales a la referencia |
| Both_CPUs_system/Debug/sd_card/BOOT.BIN | 13 622 280 bytes; BIF: FSBL + bitstream + CPU0 |
| CPU1_system/Debug/sd_card/BOOT.BIN | 13 786 128 bytes; BIF: FSBL + bitstream + CPU1 |

Se repitió `Setup` sin `-Fresh`: reutiliza el workspace y termina sin invocar XSCT.
`Check` también terminó correctamente. La última comprobación confirmó que siguen
intactos los 112 archivos locales inventariados de `_ide/bootimage`.
Finalmente se compararon los 194 archivos de construcción de `src`, `config`, `scripts`
y `artifacts` de la copia validada contra sus blobs del índice final de Git: coinciden
byte a byte. Se excluye de esa cifra únicamente el README explicativo de `artifacts`.

Las comprobaciones negativas realizadas rechazan un XSA ausente o modificado,
una configuración distinta de la usada para generar el workspace y una carpeta
de trabajo situada dentro del repositorio.
La prueba del control previo desde `make` devuelve 0 con entradas válidas y 2 cuando
la receta rechaza la configuración; el fallo no queda ignorado como un prebuild de CDT.

Durante la prueba se corrigieron tres problemas del entorno de consola:

1. XSCT no añadía `gnuwin/bin` al PATH y Eclipse no encontraba `make`.
2. `make` podía heredar el `PSModulePath` de PowerShell 7 al lanzar Windows
   PowerShell 5. Se cargan explícitamente los módulos de la instancia en ejecución.
3. XSCT podía terminar sin error pese a un fallo de Eclipse. La receta exige que
   se vuelvan a generar ELF válidos y los BOOT.BIN; no acepta salidas antiguas.

Se corrigió además el verificador ELF: Tcl interpreta de forma inesperada el literal
hexadecimal seguido de `ELF`; se comprueban los cuatro bytes con `binary scan` y el
valor `7f454c46`. La prueba aislada confirmó el resultado con el ELF de CPU0.
La generación desde cero y la compilación se probaron por fases. Tras esta corrección
del verificador se conservaron las plataformas ya generadas, verificando primero que
el código de generación, las entradas, configuraciones y fuentes no habían cambiado.
La actualización puntual de las huellas de estos dos entornos de desarrollo está
registrada en `../sitau2-software-uci-work/audit/verifier-fix-workspaces.json`; no existe
una opción de usuario para saltarse la comprobación de configuración.

El PATH se limita al proceso de las herramientas. El PATH global de Windows y las
instalaciones Xilinx no se modifican. Los temporales de XSCT se generan en el workspace.

## Comparación y límites

En la [comparación final de ELF](elf-comparison.json), todas las secciones cargables del FSBL coinciden
con la referencia heredada. CPU0 y CPU1 compilan con los fuentes y opciones originales,
pero sus ELF no son idénticos byte a byte: cambian las rutas de depuración, cadenas
`__FILE__` y la hora de compilación de CPU0. Estos cambios también pueden desplazar
datos y referencias dentro de los ejecutables.
La compilación de diagnóstico produjo las mismas advertencias que la referencia:
79 en CPU0 y 402 en CPU1, sin errores. Su corrección queda para un trabajo específico.

Esto valida la reconstrucción, no el comportamiento sobre el equipo. Quedan por probar
arranque, Ethernet, adquisición, interacción entre CPU0/CPU1 y depuración JTAG en hardware.
Las recetas personalizadas de `_ide/bootimage` siguen pendientes de revisión y no se
sustituyen por los BOOT.BIN automáticos.

Los directorios heredados se retiraron primero del índice y se conservaron localmente
durante la transición. El 23 de septiembre se eliminaron también de la copia local,
tras comprobar en la GUI los enlaces a fuentes y los Build de CPU0 y CPU1.
Un checkout nuevo contiene únicamente la estructura mantenida y genera su workspace
externo mediante `scripts/setup.ps1`.
