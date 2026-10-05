# Instalaciones Vitis con lwIP previamente personalizada

Fecha: 2026-10-05. Rama de trabajo: `accept_modified_lwip`, base `73bf163`.

## Cambio implementado

`scripts/lwip-source.ps1`, invocado por `setup.ps1` antes de tocar el workspace, valida primero las dos copias mantenidas del repositorio. Después acepta, por cada archivo instalado, únicamente el hash original auditado o el de la personalización SITAU2. Informa del estado encontrado y rechaza ausencias y contenidos desconocidos con ruta y hashes en el diagnóstico.

Se mantienen los hashes del manifiesto, las recetas Tcl, los MSS, los fuentes personalizados, el hardware y el empaquetado. La biblioteca generada sigue siendo `lwip211 1.08.s`: los dos archivos siempre se sustituyen por las copias mantenidas, independientemente del estado aceptado del origen. La instalación Xilinx no se modifica.

Los dos archivos recibidos en `D:/sitau2/tmp/lwip-originales` coinciden exactamente con nuestras personalizaciones. No se recibió ni se auditó el resto de la instalación del otro ordenador.

## Pruebas rápidas y controles del workspace

- `tests/lwip-source-tests.ps1`: 18 casos correctos con Windows PowerShell 5.1. Incluyen las cuatro combinaciones de origen, archivos ausentes/desconocidos, cambio de finales de línea, copias mantenidas alteradas y rechazo de Setup/Build/Open antes de modificar un workspace. Un Setup inválido tampoco crea un directorio de trabajo nuevo.
- `tests/release-tests.ps1`: 52 pruebas de procedencia correctas.
- `tests/clean-app-tests.tcl`: `CLEAN_APP_TESTS_OK`, ejecutado con Tcl mediante Python/Tkinter y un TEMP de pruebas en `D:/sitau2/tmp`.
- En el workspace real de pruebas ya compilado, `Check` y `Assert-BuildRecord` pasan. Alterar un byte del `xadapter.c` del BSP provoca rechazo; retirar temporalmente el MLD de la biblioteca personalizada también. Tras restaurar exactamente ambos archivos, los controles vuelven a pasar.

## Integración con Vitis 2022.2

Evidencias locales en `D:/sitau2/tmp/lwip-acceptance-20261005`.

1. `stock-work`: Setup y Build con `E:/Xilinx/Vitis/2022.2`. Generación, CPU0, CPU1 y ambos paquetes Bootgen correctos; salida final 0.
2. `modified-work`: Setup y Build correctos con una copia separada de `lwip211_v1_8` que difiere de la instalación únicamente en los dos archivos personalizados conocidos. Se generan CPU0, CPU1 y ambos paquetes Bootgen; salida final 0. `Check` y `Assert-BuildRecord` posteriores también pasan.

Para el segundo caso, los lanzadores de ensayo delegan en los ejecutables de la instalación real; el argumento de origen de la receta apunta a la copia personalizada. Un primer intento de reubicar también los lanzadores Xilinx falló al cargar `xv_commontasks.dll`; se conservó en `modified-launch-failed`. Este ajuste afecta únicamente al montaje de la prueba, no a los scripts del repositorio.

Las bibliotecas personalizadas generadas coinciden en sus 393 archivos, incluido el MLD `1.08.s`. Los productos se compararon por SHA-256 y, para los ELF, por contenido, tamaño y dirección de cada sección:

| Producto | Comparación entre ambos orígenes |
| --- | --- |
| CPU0 ELF | Código ejecutable idéntico. En las secciones cargables solo cambian cuatro bytes de `.rodata`, correspondientes a la hora del mensaje de arranque (`16:28:15` frente a `16:50:27`). También difieren secciones de depuración. |
| CPU1 ELF | Todas las secciones cargables idénticas. Solo difieren secciones de depuración. |
| FSBL ELF | Todas las secciones cargables idénticas. Solo difieren secciones de depuración. |
| `cpu0-boot.bin` | Mismo tamaño (13.622.280 bytes). Solo difieren cuatro bytes de la hora de CPU0, en los offsets 13.589.439, 13.589.440, 13.589.442 y 13.589.443. |
| `cpu1-network.bin` | Idéntico byte por byte (366.352 bytes); SHA-256 `77dd1036395cc0c3cf470622ba869e4e11be9f120d82c323d4202a7b17b2a266`. |

La hora procede del `__TIME__` ya existente en `src/cpu0/main.c`, no de este cambio. Sustituyendo únicamente esas dos cadenas horarias por un mismo valor en memoria, las imágenes de CPU0 coinciden exactamente. No se alteraron los binarios guardados. No se afirma reproducibilidad byte a byte de los ELF ni de CPU0 entre recompilaciones; sí se ha acotado la diferencia cargable a esa cadena. No se ha programado ninguna placa.

Los hashes, la comparación de secciones y los controles negativos están en [la evidencia JSON](lwip-origen-personalizado.json). Paquetes locales: `stock-work/workspace/packages/20261005-163042-589` y `modified-work/workspace/packages/20261005-165248-088`.

Bootgen conserva los avisos de solapamiento entre FSBL, bitstream y CPU0 ya documentados en la [validación anterior](lwip-version-sitau2.md). Se mantienen las recetas y el diseño de memoria; este cambio no resuelve ni introduce esa configuración.

## Release y compatibilidad

La comparación con el inventario anterior al cambio confirma:

- Los siete archivos de `output/0.0.0`, incluidos manifiesto y README, conservan sus hashes.
- Las 194 entradas de firmware y las cuatro de empaquetado son idénticas. Las entradas de firmware siguen coincidiendo con las registradas en la release.
- Huella de workspace idéntica: `B29213E65F35B5867CE0CFF5213CDD9BE6EC9C2C462C158486312CA6BDB05D2B`.
- Los 393 archivos de la biblioteca lwIP instalada en `E:/Xilinx/Vitis/2022.2` permanecen intactos.

Este cambio no invalida la release `0.0.0` ni exige regenerar un workspace compatible. La release conserva `hardwareValidated: false`; integridad y compilación no equivalen a validación en placa. No se publica una nueva versión de firmware por este cambio.

El workspace habitual `D:/sitau2/sitau2-software-uci-work/workspace` ya estaba desactualizado antes de comenzar, según el [plan](../plans/aceptar-lwip-personalizada-en-vitis.md). No se ha tocado ni alterado su registro para eludir controles.

[Volver](README.md)
