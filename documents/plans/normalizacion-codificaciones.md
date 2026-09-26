# Plan de normalización de codificaciones

- **Proyecto:** SITAU2 — software de la UCI.
- **Fecha del análisis:** 2026-09-21.
- **Estado:** propuesta pendiente de ejecución. No se han convertido los fuentes.
- **Referencia inicial:** commit `751b302cb8e8ca2e7d21f8cfb4f437a6ca66fe09`.

**Actualización del 22 de septiembre:** ya está instalado Vitis Classic 2022.2 y se ha
obtenido una [compilación de referencia](../reports/referencia-compilacion.md). El refactor
traslada `CPU0/src` y `CPU1/src` a `src/cpu0` y `src/cpu1`; mantiene las personalizaciones
BSP en `config/lwip211`. Las reglas `-text` de esas rutas conservan los bytes originales,
incluidos sus finales de línea. Las cifras y rutas del análisis siguiente describen el
estado inicial. La normalización sigue pendiente y no se ejecuta como parte del refactor.

## 1. Objetivo y decisión actual

Establecer una política de texto uniforme para trabajar en Windows, Vitis y GitHub, preservando el comportamiento del firmware y los bytes de sus interfaces externas.

**No convertir las codificaciones de los fuentes hasta disponer de una compilación de referencia y medios de validación.** Mientras tanto, se pueden realizar inventarios y documentar la configuración existente mediante operaciones de solo lectura.

El objetivo futuro es UTF-8 sin BOM para los fuentes mantenidos por el proyecto, con finales de línea LF. La codificación de las cadenas que el firmware genera o transmite se decidirá por separado: adoptar UTF-8 en el archivo fuente no implica adoptar UTF-8 en los protocolos o mensajes del equipo.

## 2. Estado comprobado

El proyecto corresponde a Vitis Classic 2022.2, con aplicaciones C para los dos Cortex-A9 de un Zynq. La última revisión del equipo local encontró Vitis 2025.2, pero no Vitis 2022.2. No se ha realizado una compilación de referencia; debe comprobarse de nuevo el entorno cuando se ejecute este plan.

### 2.1 Inventario de fuentes de las aplicaciones

Se ha repetido una lectura binaria de los archivos bajo `CPU0/src/` y `CPU1/src/`. Se seleccionaron las extensiones `.c`, `.h`, `.S`, `.ld`, `.tcl`, `.bif`, `.project`, `.cproject`, `.prj`, `.sprj`, `.spr`, `.mss` y `.md`: 173 archivos en total. Las copias `.bak` y otras extensiones de respaldo no forman parte de estas cifras.

| Clasificación de caracteres | Archivos |
|---|---:|
| Solo ASCII, compatible con UTF-8 | 108 |
| UTF-8 válido con caracteres no ASCII | 12 |
| No decodificables como UTF-8 estricto | 53 |

| Finales de línea en los archivos locales | Archivos |
|---|---:|
| CRLF | 144 |
| LF | 29 |
| Mezcla de CRLF y LF | 0 |

Ejemplos que no son UTF-8 válido: `CPU0/src/main.c`, `CPU0/src/cons_prod_util.c`, `CPU0/src/image_mover_mod.c` y `CPU0/src/tcpcom.c`.

Estas cifras **no identifican la codificación original de los 53 archivos**. Windows-1252 o ISO-8859-1 son posibilidades por investigar, no resultados confirmados. Una decodificación que no arroje errores tampoco demuestra que el texto sea correcto; pueden existir secuencias ya deterioradas o codificaciones mezcladas dentro de un archivo.

El inventario no cubre todo el repositorio. Hay 2756 archivos versionados con extensiones `.c`, `.h`, `.S` o `.ld`, incluyendo BSP, FSBL y bibliotecas. Es necesario clasificar su procedencia antes de ampliar una conversión.

### 2.2 Reglas ya incorporadas

- [`.gitattributes`](../../.gitattributes) establece `* text=auto eol=lf`, con CRLF para `.bat` y `.cmd`, y tratamiento binario para los artefactos correspondientes.
- [`.editorconfig`](../../.editorconfig) establece LF y deja `charset=unset` para los archivos existentes. Solo los tres archivos de políticas del repositorio tienen UTF-8 fijado explícitamente.
- Git conserva la codificación de caracteres actual: no hay reglas `working-tree-encoding` que hagan conversiones implícitas.

Al crear el commit inicial se verificó que los 5561 archivos anteriores examinados, excluyendo `.git` y `.metadata`, conservaban sus hashes en disco. De los 3110 archivos incorporados al índice, 2791 eran idénticos a sus archivos locales y 319 solo diferían en la normalización CRLF → LF. No se detectaron otras modificaciones de bytes.

Por tanto, el commit de referencia conserva la codificación de caracteres, pero no necesariamente los finales de línea originales de cada archivo. Un nuevo checkout puede usar LF aunque la copia local previa al primer commit todavía use CRLF.

`charset=unset` evita imponer una codificación desde EditorConfig; **no determina cómo Vitis u otro editor interpretan el archivo**. La visualización incorrecta en una terminal tampoco prueba que el archivo esté deteriorado.

## 3. Riesgos que deben controlarse

| Ubicación de caracteres no ASCII | Riesgo y comprobación necesaria |
|---|---|
| Comentarios | Una conversión correcta conserva su significado. Revisar delimitadores, continuaciones de línea y otros detalles léxicos; no aplicar sustituciones globales sin revisión. |
| Cadenas y literales de carácter | Pueden cambiar bytes, longitudes, `sizeof`, comparaciones, checksums, tablas o tramas. Revisar compilador y consumidores. |
| Identificadores, macros, ensamblador y rutas | Pueden afectar al compilador, preprocesador, herramientas o búsqueda de archivos. Examinar individualmente. |
| Mensajes de UART, logs o terminal | El receptor puede esperar una codificación antigua. Compilar correctamente no garantiza que se vean igual. |
| Texto mezclado o ya deteriorado | Una conversión automática puede consolidar la corrupción. Conservar evidencia y resolver cada caso. |
| BSP, FSBL y bibliotecas | Una edición puede perderse al regenerar la plataforma. Identificar modificaciones propias y el origen que debe mantenerse. |

Por ejemplo, `ñ` ocupa un byte en Windows-1252 y dos en UTF-8. Si esos bytes se emiten directamente, una conversión puede modificar el resultado. El efecto real depende también de la codificación de entrada y de ejecución configuradas en el compilador.

## 4. Fase A: diagnóstico sin modificar fuentes

Puede ejecutarse antes de disponer del entorno de compilación.

1. Crear un inventario por ruta con procedencia, tamaño, SHA-256, BOM, finales de línea, validez UTF-8 y codificación candidata cuando exista evidencia.
2. Revisar por separado aplicaciones propias, BSP/FSBL, bibliotecas de terceros, archivos generados y respaldos históricos.
3. Consultar las preferencias de codificación del workspace, los proyectos y los archivos de Vitis. Revisar también las opciones efectivas de compilación. La ausencia de una preferencia explícita no demuestra UTF-8.
4. Localizar los caracteres no ASCII y clasificarlos como comentarios, cadenas, literales, identificadores, macros u otros usos. No utilizar una expresión regular simplista como única garantía de clasificación del código C.
5. Contrastar las codificaciones candidatas con texto español conocido y, cuando exista, documentación del entorno original. Registrar los casos ambiguos sin convertirlos.
6. Identificar cadenas expuestas por UART, Ethernet u otras interfaces, y cómo las interpreta el software del host.

**Resultado:** inventario y lista de archivos convertibles, casos pendientes y comprobaciones necesarias. Ninguna modificación de bytes en los fuentes.

## 5. Fase B: obtener una referencia reproducible

Condiciones previas a cualquier conversión:

1. Disponer de la versión de Vitis adecuada y registrar su versión exacta, compilador, opciones y bibliotecas.
2. Identificar y fijar el XSA, la configuración BSP y los firmwares externos que correspondan a la referencia funcional.
3. Compilar desde la referencia conservada. Cualquier ajuste de rutas o configuración necesario para lograrlo debe ir en un cambio independiente de la normalización.
4. Guardar los resultados y advertencias de esa compilación en una ubicación de artefactos, junto con el commit de origen y las dependencias utilizadas.
5. Obtener una referencia funcional del equipo para las interfaces afectadas. Si no se puede validar una cadena expuesta externamente, posponer su conversión.

No mezclar esta fase con una migración a Vitis 2025.2, refactorizaciones, cambios de hardware o correcciones funcionales.

## 6. Fase C: conversión controlada

1. Trabajar en una rama específica, con el árbol limpio y la referencia anterior identificada.
2. Empezar por un lote pequeño de fuentes propios cuya codificación se conozca. Priorizar archivos cuyos caracteres no ASCII estén exclusivamente en comentarios.
3. Decodificar con la codificación original confirmada, en modo estricto, y escribir UTF-8 sin BOM. Nunca usar modos que sustituyan o descarten caracteres inválidos.
4. Verificar que la conversión conserva la secuencia de caracteres interpretada y que puede reconstruirse el original con su codificación y finales de línea registrados. Esta comprobación prueba la transformación elegida, no que se haya identificado correctamente la codificación original.
5. Revisar el diff sin mezclar formato, indentación, correcciones de texto ni cambios funcionales. Preservar los finales de línea en la conversión local o registrar su cambio explícitamente como una operación separada.
6. Tratar los archivos con cadenas o literales no ASCII individualmente. Decidir primero los bytes de ejecución que deben conservarse. Revisar las opciones de codificación del compilador; no imponer opciones globales para resolver un único caso.
7. No convertir masivamente BSP o bibliotecas generadas. Aplicar las modificaciones necesarias en su fuente mantenida o mecanismo de personalización, con una estrategia que sobreviva a su regeneración.
8. Crear commits pequeños de normalización, separados de cualquier modificación del comportamiento.

## 7. Verificación y criterios de aceptación

Para cada lote:

- Todos los archivos convertidos son UTF-8 válido sin BOM y no hay caracteres sustituidos o perdidos.
- Los cambios de Git corresponden únicamente al lote previsto; los artefactos binarios conservan sus hashes.
- CPU0, CPU1 y los componentes afectados compilan con la misma configuración de referencia, sin nuevas advertencias atribuibles a la conversión.
- Las cadenas, tamaños y bytes observables relevantes se mantienen, salvo un cambio funcional autorizado por separado.
- Las pruebas sobre el equipo cubren las interfaces afectadas, especialmente tramas, respuestas y mensajes interpretados por el host.

No exigir igualdad binaria de todo el ELF como única prueba: rutas de depuración, marcas temporales y macros como `__DATE__` o `__TIME__` pueden introducir diferencias. Comparar las secciones, símbolos y datos relevantes para el cambio, además de las observaciones funcionales.

Una compilación correcta no basta por sí sola para validar compatibilidad de protocolos o interpretación de mensajes.

## 8. Fase D: fijar la política definitiva

Solo cuando la normalización del ámbito elegido esté validada:

1. Establecer UTF-8 sin BOM en `.editorconfig` para ese ámbito y configurarlo explícitamente en los proyectos o archivos de Vitis correspondientes.
2. Mantener excepciones documentadas para dependencias que deban conservar otra codificación. No extender la regla a todo el repositorio mientras queden archivos heredados sin resolver.
3. Añadir una comprobación de UTF-8, BOM y finales de línea para los fuentes propios normalizados. La comprobación debe informar y fallar, no reescribir archivos automáticamente.
4. Documentar cómo abrir y guardar los archivos heredados restantes, y validar las reglas en un checkout limpio en Windows.

No utilizar conversiones transparentes mediante `working-tree-encoding` en esta primera normalización: añadirían diferencias entre lo almacenado y lo editado y exigirían verificar el soporte de todos los clientes Git utilizados.

## 9. Recuperación

Cada lote debe poder revertirse por separado. Si ya se ha compartido, revertir el commit de normalización y reconstruir con la referencia conocida; evitar volver todo el repositorio a un estado anterior que oculte otros cambios de software.

Antes de revertir, preservar cualquier trabajo pendiente. Cuando haya commits posteriores sobre los mismos archivos, revisar posibles conflictos en lugar de restaurar indiscriminadamente toda una carpeta.

## 10. Siguiente acción prevista

Ejecutar únicamente la **fase A**, cuando se decida retomar este trabajo. Las fases de conversión quedan pendientes de la compilación de referencia y de la validación de comportamiento.

## Referencias

- [Git: atributos de texto, finales de línea y codificación](https://git-scm.com/docs/gitattributes).
- [EditorConfig: propiedades y soporte en editores](https://editorconfig.org/).
- [Configuración de atributos del repositorio](../../.gitattributes).
- [Configuración de edición del repositorio](../../.editorconfig).
