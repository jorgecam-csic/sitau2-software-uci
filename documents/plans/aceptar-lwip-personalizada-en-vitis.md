# Aceptar la personalización conocida de lwIP en la instalación Vitis

Fecha: 2026-10-05. Rama: `accept_modified_lwip`, base `73bf163`.
Estado: implementado y validado el 2026-10-05. Véanse el [informe de ejecución](../reports/lwip-origen-personalizado.md) y sus evidencias. El resto del documento conserva el análisis y los criterios previos al cambio.

## Objetivo y evidencia

Permitir generar y compilar con Vitis 2022.2 cuando los dos archivos comprobados de su biblioteca `lwip211_v1_8` sean los originales auditados o las personalizaciones SITAU2 conocidas. Mantener el rechazo de cualquier contenido distinto y no modificar la instalación de Vitis.

Los archivos recibidos en `D:\sitau2\tmp\lwip-originales` coinciden byte por byte con las copias mantenidas en `config/lwip211`:

| Archivo | SHA-256 original (`stockSha256`) | SHA-256 SITAU2 (`sha256`) |
| --- | --- | --- |
| `xadapter.c` | `1eb5fbba3427b9874c07378248b0667735595b2aa6735157852bab2c69529099` | `1ea10744bd801f479540b73ec93ee71536d4fefd12b7b7ded5d6cd745de25e6b` |
| `xemacpsif_physpeed.c` | `2e05d7ae1ebe0094dab10742cc29868a1bef7b91a3a888f030ef96f4da68e7ad` | `a141a9e70197200d7bade9902518f97aeec71e06a98995aec4807d4317914f3d` |

La comprobación actual de `scripts/setup.ps1` solo admite `stockSha256` y se detiene antes de generar el workspace. El generador copia la biblioteca a un repositorio software externo y después reemplaza ambos archivos por nuestras copias mantenidas. Admitir las mismas copias en el origen no requiere cambiar ese proceso ni la identidad `lwip211 1.08.s`.

La coincidencia demuestra el estado de estos dos archivos. No certifica que el resto de la biblioteca del otro ordenador esté intacto.

## Cambio propuesto

1. Mantener `config/lwip211/manifest.json` sin cambios: ya contiene los dos hashes necesarios por archivo.
2. En la validación previa de `setup.ps1`, comprobar primero que la copia mantenida en `config/lwip211` coincide con `sha256`.
3. Calcular el SHA-256 del archivo de la instalación y aceptar exclusivamente `stockSha256` o `sha256`. Evaluar cada archivo por separado: se admiten original/original, original/SITAU2, SITAU2/original y SITAU2/SITAU2.
4. Mostrar por archivo si se encontró el original o la personalización conocida. Si falta o no coincide, mostrar ruta, hash observado y hashes permitidos; detenerse antes de crear o archivar un workspace.
5. Mantener las comparaciones exactas de bytes. No normalizar finales de línea para aceptar archivos, no actualizar hashes automáticamente ni habilitar una opción de omitir validación.
6. Conservar la copia al repositorio del workspace, la sustitución de los dos fuentes, la selección `1.08.s` y las comprobaciones estrictas del BSP generado. No modificar Vitis, firmware, MSS, XSA, empaquetado ni entregas existentes.

Para probar esta lógica aisladamente se puede extraer la validación a un pequeño helper en `scripts`, invocado por `setup.ps1`. No añadirlo a las entradas de generación/compilación: valida las mismas entradas y no transforma el firmware. Revisar esta decisión si la implementación acaba cambiando el proceso de generación.

## Impacto en la release actual

La release presente es `output/0.0.0`, esquema 2, con commit de firmware y empaquetado `f5090d0c2aeadbd85a490e66d5a77d6e42164997`. Se han comprobado correctamente contra su manifiesto:

- `cpu0-boot.bin` y `cpu1-network.bin`.
- `programming/fsbl.elf`.
- `debug/CPU0.elf` y `debug/CPU1.elf`.

Sus entradas de firmware coinciden con `Get-BuildInputs` de la rama actual. `hardwareValidated` sigue siendo `false`; la integridad de archivos no acredita pruebas en placa.

| Elemento | Efecto esperado del cambio limitado a la validación |
| --- | --- |
| Release `0.0.0` ya guardada | Ninguno: conservar todos sus archivos, hashes, manifiesto y procedencia. No sobrescribirla ni volver a generarla. |
| Uso de la release para grabación | La validación de la entrega comprueba sus archivos y configuración; no depende de estos hashes de los fuentes lwIP instalados ni del workspace. Se mantienen los requisitos habituales de herramientas y hardware. |
| Workspace compatible existente | Su huella debe permanecer idéntica: `setup.ps1` y los helpers de validación no forman parte de `Get-WorkspaceRecipeFiles`. |
| Build válido existente | Sus entradas deben permanecer idénticas. `Assert-BuildRecord` actual compara entradas, productos y metadatos de herramientas; ya no exige que HEAD sea el mismo commit del build. |
| Nueva publicación | Sigue exigiendo Git limpio y una versión superior a las existentes. Conserva el commit del firmware y registra por separado el commit de empaquetado. No es necesario crear otra release solo por ampliar esta validación. |
| Firmware generado en el otro PC | Se espera la misma variante de los dos archivos, pues siempre se reemplazan por las copias mantenidas. No se garantiza identidad de todo el build sin comprobar el resto de la instalación y los resultados. |

**No debería invalidar la release actual ni forzar regeneración por sí mismo.** Esta conclusión depende de no cambiar `create-workspace.tcl`, `config/lwip211/manifest.json`, los fuentes personalizados, los MSS u otras entradas reales. El manifiesto lwIP sí forma parte de la huella y del inventario de firmware: añadir allí metadatos innecesarios cambiaría el impacto.

## Estado previo del workspace local

La comprobación de lectura realizada en `D:\sitau2\sitau2-software-uci-work\workspace` antes de implementar esta propuesta encontró:

- `setup.ps1 -Action Check`: workspace desactualizado.
- `Assert-BuildRecord`: entrada no versionada `src/cpu0/shared_mem_def.h.bak` en el registro antiguo.
- Huella actual calculada para las recetas de esta rama: `B29213E65F35B5867CE0CFF5213CDD9BE6EC9C2C462C158486312CA6BDB05D2B`.

Ese workspace no sirve hoy como evidencia de reutilización de un build válido. Su recuperación requerirá el flujo normal de generación y compilación si se quiere trabajar con él; es una situación previa e independiente de esta propuesta. No modificar sus marcadores o el registro para hacerlos pasar. La release autocontenida sigue teniendo hashes correctos.

## Pruebas y criterios de aceptación

1. Guardar una referencia de hashes de todos los archivos de `output/0.0.0`, la huella del workspace y los mapas de entradas de firmware/empaquetado antes del cambio.
2. Pruebas aisladas con archivos temporales: las cuatro combinaciones original/SITAU2 aceptadas; archivo ausente, byte alterado, contenido desconocido y copia personalizada del repositorio alterada rechazados. No tocar la instalación real para fabricar los casos.
3. Verificar que un error aparece antes de archivar o generar el workspace y que los mensajes identifican archivo y causa.
4. Confirmar que ambos orígenes aceptados producen los mismos dos archivos personalizados y MLD `1.08.s` en entornos de prueba. Mantener el rechazo de modificaciones posteriores del BSP y de una biblioteca personalizada ausente.
5. Ejecutar las suites existentes de procedencia y del flujo afectado. Probar Setup/Build en entornos aislados con instalación original y con los dos archivos ya personalizados; comparar las entradas y productos. Si los binarios difieren, explicar la diferencia antes de afirmar equivalencia.
6. Comprobar que la huella y los mapas de entradas permanecen iguales antes/después; probar reutilización sobre un workspace compatible, no sobre el entorno local obsoleto. La compatibilidad de registros anteriores no debe relajarse.
7. Confirmar que ningún archivo de `output/0.0.0` ni de la instalación Vitis se ha modificado. No programar hardware ni publicar una release como parte de estas pruebas.
8. Actualizar los README de scripts y lwIP con la política de dos hashes conocidos y el límite de la comprobación. Registrar resultados reales en `documents/reports` al ejecutar el plan.

## Entrega del cambio

Implementar y revisar en `accept_modified_lwip`. La PR debe explicar que se amplía la aceptación de una instalación previamente personalizada, sin cambiar la variante generada ni la release `0.0.0`. Si las pruebas obligan a modificar entradas del firmware, revisar este análisis de impacto antes de continuar. No hacer merge como parte de este trabajo.

[Volver](README.md)
