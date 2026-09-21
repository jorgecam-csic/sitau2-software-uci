# Plan de reorganización de artefactos y dependencias

- **Proyecto:** SITAU2 — software de la UCI.
- **Fecha:** 2026-09-21.
- **Estado:** propuesta pendiente de ejecución. Este documento no mueve ni elimina archivos.
- **Referencia del análisis:** `b0a0af077f14a428bf4309f56960d2de86bcd459`.
- **Referencia inicial del software:** `751b302cb8e8ca2e7d21f8cfb4f437a6ca66fe09`.

## 1. Objetivo y límites

Centralizar en `artifacts/` las entradas externas del proyecto, conservar versiones identificables e inmutables y eliminar del árbol versionado las copias dispersas y resultados regenerables. La compilación y depuración habituales seguirán realizándose desde la interfaz gráfica de Vitis Classic 2022.2.

El software debe consumir versiones seleccionadas explícitamente, nunca los resultados variables de un repositorio hardware hermano. Cambiar una dependencia no debe requerir retroceder el código del software a otro commit.

Se conservarán los contenidos cuya procedencia o uso no estén resueltos. Sin una compilación de referencia no se retirarán las rutas necesarias para el workspace actual ni se excluirán en bloque las fuentes de BSP/FSBL.

## 2. Inventario comprobado

### 2.1 XSA y bitstream de la UCI

Hay cuatro XSA versionados, correspondientes a tres contenidos distintos:

| Ruta actual | SHA-256 abreviado | Marca de exportación interna | Tratamiento previsto |
|---|---|---|---|
| `Platform/export/design_1_wrapper.xsa` | `3ed161d88aff` | 2026-03-02 12:09:06 | Candidato inicial: entrada referenciada por `platHandOff`. |
| `Platform/hw/design_1_wrapper.xsa` | `3ed161d88aff` | 2026-03-02 12:09:06 | Idéntico al anterior; copia interna referenciada por `platIntHandOff`. |
| `Platform/resources/design_1_wrapper.xsa` | `001cf3e70d22` | 2026-03-02 10:33:39 | Versión distinta: conservar e identificar. |
| `design_1_wrapper.xsa` en la raíz | `9484b63770f0` | 2024-11-08 15:14:05 | Versión distinta: conservar e identificar. |

Los tres contenidos identifican Vivado 2022.2.2 y el dispositivo `xc7z045ffg676-2`. Cada uno incluye un bitstream distinto. La marca interna es evidencia de exportación, no una versión funcional ni una garantía de compatibilidad.

Se ha comprobado que `Platform/hw/design_1_wrapper.bit` coincide byte a byte con el bitstream incluido en el XSA `3ed161d88aff`. Puede convertirse en una copia derivada, pero solo después de asegurar que se obtiene del XSA seleccionado y que los consumidores utilizan esa copia.

Hashes completos de los XSA:

```text
3ed161d88aff0e0c9244fbacf1fc15ac816d9b40662f01281b2efc3adf6b8286
001cf3e70d225843f370a690234267ac825517597ba467143eab7a8280a4a413
9484b63770f0f0c8f5e1460a17835291d69a9f02fd3910db64356133a3b1ddab
```

La plataforma configurada apunta al primer contenido, pero todavía no se ha demostrado que sea el hardware correcto para una instalación concreta del equipo ni que los BSP y ELF existentes procedan de él.

### 2.2 Directorio de firmware e históricos

`CPU1_system/_ide/bootimage/BIN/` contiene 105 archivos versionados y aproximadamente 390,5 MiB. Incluye:

- Variantes de `MK32CH_basics_wrapper.bin`, `.bit` y `.ltx`.
- Imágenes `BOOT.bin`, `BOOT_UCI.bin`, `BOOT_old.bin` y variantes con sufijos históricos.
- Archivos comprimidos `impl_1.zip` e `impl_1.rar`.
- Configuraciones XML, textos, CSV y una hoja de cálculo.

Entre sus 92 archivos con extensiones `.bin`, `.bit`, `.ltx`, `.bin_old` y `.binmal` hay 71 contenidos distintos y 14 grupos con duplicados exactos. Igualdad de hash permite deduplicar bytes; nombres parecidos no permiten deducir equivalencia ni compatibilidad. Tampoco debe emparejarse un `.ltx` con un `.bit` únicamente por su nombre.

En todo el repositorio hay 99 archivos versionados de las extensiones de artefactos revisadas, con un total aproximado de 411,6 MiB. Las cifras describen el árbol de archivos, no el tamaño comprimido del historial de Git.

### 2.3 Consumidores y resultados del workspace

- `Platform/platform.spr` referencia XSA en `export/` y `hw/`.
- Los proyectos, recetas BIF y scripts de depuración contienen rutas históricas `C:/PV/...` y `C:\PV\...`.
- `Platform/platform.tcl` contiene una secuencia histórica de operaciones; no debe asumirse que es una receta mínima y reproducible sin revisarlo.
- Las aplicaciones dependen de los BSP y bibliotecas de `Platform`.
- Los directorios `_ide/bitstream/` y `_ide/psinit/` de las aplicaciones contienen copias derivadas usadas en lanzamiento y depuración.
- El BIF personalizado de `Both_CPUs_system` combina FSBL, bitstream y CPU0. El de `CPU1_system` contiene solo CPU1. El empaquetado automático de `Debug/` tiene otras recetas. Deben mantenerse diferenciados.
- CPU1 recibe firmware de las bases mediante el protocolo en ejecución. La ubicación local de esos archivos puede ser consumida por herramientas del host que no están en este repositorio; esas referencias también deben identificarse.

## 3. Modelo propuesto

### 3.1 Paquetes inmutables y selección explícita

Cada productor hardware entrega un paquete versionado. Una versión publicada nunca se modifica: cualquier cambio genera otra versión. Las ramas de los productores pueden estar en desarrollo; lo necesario es identificar el commit y los bytes exactos del paquete.

Para la importación inicial se usarán identificadores como `imported-<hash-corto>`, sin inventar versiones semánticas, commits ni estados de validación. Las entregas futuras podrán usar versiones de release definidas por el productor.

La selección se guarda en `artifacts/dependencies-lock.json`. Cambiar esa selección y reaplicar la plataforma permite probar el software actual con otra dependencia. La compatibilidad debe validarse; el archivo de selección no la garantiza.

### 3.2 Estructura objetivo

```text
artifacts/
├── README.md
├── dependencies-lock.json
├── uci-hw/
│   ├── imported-3ed161d88aff/
│   │   ├── manifest.json
│   │   └── design_1_wrapper.xsa
│   ├── imported-001cf3e70d22/
│   │   ├── manifest.json
│   │   └── design_1_wrapper.xsa
│   └── imported-9484b63770f0/
│       ├── manifest.json
│       └── design_1_wrapper.xsa
├── mk32-hw/
│   └── <version-identificada>/
│       ├── manifest.json
│       ├── MK32CH_basics_wrapper.bin
│       └── debug/                 # .bit/.ltx, si se necesitan y están asociados
└── legacy/
    ├── inventory.json
    └── <grupos-pendientes-de-clasificar>/

scripts/
├── verify-artifacts.ps1
├── apply-artifacts.ps1
└── apply-platform.tcl

documents/plans/
└── refactor-artifacts.md
```

Los scripts son una propuesta de implementación futura; no existen como resultado de este documento. Los grupos desconocidos de MK32 permanecerán en `legacy/` hasta identificarlos, sin simular una selección válida en el lock.

`artifacts/legacy/` conserva provisionalmente resultados antiguos y documentación asociada cuya procedencia no está resuelta. No es una fuente de dependencias activas. No se eliminarán originales históricos solo porque sus nombres incluyan `old`, `mal` o `tmp`.

Las salidas nuevas, como ELF, imágenes de arranque y paquetes de entrega de software, permanecerán en directorios generados e ignorados, separados de `artifacts/`. El BSP será un derivado del XSA y de la configuración mantenida, una vez comprobada su regeneración.

### 3.3 Manifiestos

Cada paquete registrará, al menos:

- Versión del formato de manifiesto, identificador del paquete y versión inmutable.
- Tipo: hardware UCI, firmware MK32 u otra categoría explícita.
- Lista de archivos con rutas relativas, tamaños y hashes SHA-256 completos.
- Repositorio y commit productor cuando se conozcan; `null` y una explicación cuando no se conozcan.
- Versión de Vivado observada, dispositivo y revisión de placa cuando estén documentados.
- Evidencia de procedencia: rutas originales, commit de importación y marca de exportación, distinguiendo hechos de inferencias.
- Estado de validación: importado sin verificar, compilado o probado sobre un equipo identificado.
- Notas de compatibilidad y asociación de archivos de depuración, si están confirmadas.

El lock seleccionará cada paquete mediante su ruta y versión, y fijará también el hash del manifiesto. El verificador comprobará ese hash y los hashes de los archivos. Una dependencia pendiente debe aparecer explícitamente como no resuelta y bloquear la operación que la necesite; nunca se sustituirá por `latest` ni por el primer archivo encontrado.

La compilación del C requiere la dependencia UCI y la plataforma correspondiente. Un paquete MK32 sin seleccionar no tiene por qué bloquear esa compilación, pero sí debe bloquear una preparación de entrega o programación que afirme incluir dicho firmware.

Los hashes verifican identidad e integridad, no compatibilidad funcional ni autenticidad del productor por sí solos.

## 4. Integración con Vitis y Windows

Se conservará la compilación y depuración desde el IDE. El flujo objetivo tiene dos operaciones distintas:

1. **Aplicar dependencias:** leer el lock, verificar todos los archivos requeridos, actualizar la plataforma al XSA seleccionado y regenerar los componentes derivados antes de compilar las aplicaciones.
2. **Comprobar dependencias:** verificar, antes de compilar, que la plataforma preparada sigue correspondiendo al lock y a sus entradas.

La aplicación de cambios será explícita desde una acción del IDE o herramienta auxiliar. No se regenerará la plataforma dentro del pre-build de CPU0 o CPU1: ambas dependen de ella y el orden de construcción podría ser incorrecto.

La integración deberá probarse con Vitis 2022.2 y un workspace abierto. No se ejecutarán simultáneamente una construcción del IDE y una actualización externa de plataforma. Primero puede usarse la actualización manual del XSA desde el IDE; la automatización se incorporará cuando se comprueben refresco, bloqueo del workspace y orden de generación.

El pre-build existente `a9-linaro-pre-build-step` se conservará. La comprobación añadida deberá probar que un fallo detiene la construcción y se aplica a CPU0 y CPU1, en Debug y Release. No basta con imprimir un aviso.

El estado de aplicación será local y generado: registrará el lock aplicado, los hashes de las entradas y la configuración pertinente. Un marcador aislado no prueba que los BSP estén actualizados; se verificará también la plataforma real y la finalización de su generación. Si el proceso falla, no se marcará la nueva selección como aplicada.

Los scripts resolverán las rutas a partir de su propia ubicación o del workspace, con soporte para espacios. Se eliminarán dependencias de directorios personales. Los metadatos que Vitis requiera con rutas absolutas podrán generarlas a partir de esa raíz; no se forzarán rutas relativas en formatos que no las admitan.

## 5. Mapa de migración

| Origen | Destino o tratamiento | Condición para retirar la ruta versionada original |
|---|---|---|
| XSA de `Platform/export/` y `Platform/hw/` | Un paquete UCI `imported-3ed161d88aff` | Verificar hashes, actualizar plataforma y comprobar checkout limpio. Las copias internas podrán seguir existiendo ignoradas. |
| XSA de raíz y `Platform/resources/` | Dos paquetes UCI importados independientes | Registrar procedencia y comprobar que no quedan consumidores de las rutas antiguas. |
| `Platform/hw/design_1_wrapper.bit` | Extraído del XSA seleccionado cuando se necesite | Validar extracción, hash y rutas de empaquetado/depuración. |
| Firmware MK32 identificado | `artifacts/mk32-hw/<version>/` | Confirmar versión, asociación entre archivos y referencias del software host. |
| Variantes MK32 sin identificar | `artifacts/legacy/` con inventario | Conservar todos los contenidos únicos y el mapa de rutas originales. |
| `BOOT*.bin` y variantes históricas dentro de `BIN/` | Archivo histórico en `artifacts/legacy/` | No presentarlos como entradas hardware ni descartarlos hasta conocer su procedencia. |
| ZIP/RAR en `BIN/` | Conservación provisional en `legacy/` | Inspeccionar su contenido antes de deduplicar o eliminar. |
| XML, CSV, TXT y XLSX de `BIN/` | Preservar junto con su contexto en `legacy/`; clasificar después | Identificar si son configuración operativa, documentación o resultados. No convertir formatos ni codificaciones. |
| BIF, launch y Tcl útiles de `_ide/` | Mantener o convertir en recetas mantenidas | Actualizar referencias y validar cada modo de lanzamiento y empaquetado. |
| BSP, FSBL y bibliotecas generadas | Derivados ignorados en una fase posterior | Extraer personalizaciones, guardar configuración y demostrar regeneración equivalente. |

No se eliminará `_ide/` entero ni se aplicarán reglas globales para ignorar `.bin`, `.bit` o `.xsa`.

## 6. Ejecución por fases

### Fase 0 — Congelar y documentar la referencia

1. Comprobar que el árbol no contiene trabajo pendiente ajeno y trabajar en una rama del refactor.
2. Registrar el commit inicial, las rutas versionadas e ignoradas relevantes, hashes, tamaños, referencias y herramientas.
3. Obtener una compilación y un arranque de referencia cuando el entorno esté disponible. Registrar qué XSA y firmwares se utilizaron realmente.
4. Si aún no se puede compilar, limitarse al inventario y al diseño; no retirar entradas activas.

### Fase 1 — Incorporar paquetes sin romper el workspace

1. Crear `artifacts/` y sus convenciones documentadas.
2. Importar los tres XSA como paquetes independientes, sin alterar sus bytes.
3. Inventariar los contenidos de `BIN/`, agrupando duplicados exactos y preservando asociaciones y rutas originales.
4. Incorporar solo los paquetes MK32 cuya identidad esté resuelta. Preservar el resto en el área histórica.
5. Preparar el lock con el XSA referenciado actualmente como selección inicial pendiente de validación. No elegir un MK32 por nombre o fecha.
6. Verificar paquetes y manifiestos. Mantener temporalmente las rutas antiguas mientras se valida la transición.

### Fase 2 — Hacer efectiva la selección

1. Implementar primero un verificador de solo lectura: rutas válidas, archivos presentes, hashes, selección y diagnósticos claros.
2. Probar la actualización manual de plataforma desde el paquete seleccionado.
3. Preparar el mecanismo de aplicación y extracción del bitstream; no editar a ciegas `platform.spr` ni ejecutar todo el historial de `platform.tcl`.
4. Actualizar las referencias en proyectos, recetas de empaquetado y depuración. Resolver las antiguas rutas personales y las referencias a plataformas que ya no existen.
5. Registrar la selección aplicada y añadir la comprobación previa a las aplicaciones.
6. Comprobar qué imágenes necesita realmente cada flujo: arranque CPU0, carga posterior CPU1 y paquetes automáticos del sistema.

### Fase 3 — Validar y retirar duplicados del árbol versionado

1. Compilar y empaquetar la selección de referencia. Comprobar el bitstream efectivo y los ELF incluidos en cada imagen.
2. Probar lanzamiento/depuración y, cuando corresponda, carga de firmware de las bases con el host.
3. Retirar del seguimiento las copias derivadas en las rutas antiguas, conservándolas localmente si el workspace las necesita hasta regenerarlas.
4. Mover los contenidos externos únicos a sus paquetes o al archivo histórico. Deduplicar solo después de registrar el mapa completo de procedencia.
5. Retirar la carpeta antigua `BIN/` del árbol versionado cuando todos sus archivos tengan un destino o una decisión documentada y los consumidores hayan sido actualizados.
6. Ajustar `.gitignore`: excluir íntegramente `Platform/export/` y retirar su excepción de XSA solo cuando el paquete sea la entrada mantenida y se pueda reconstruir la plataforma. Excluir otras copias generadas de forma específica y probada.
7. Mantener `.gitattributes` para preservar los artefactos como binarios.

### Fase 4 — Limpiar BSP/FSBL generados

1. Comparar las fuentes actuales con las generadas por las herramientas de referencia.
2. Separar modificaciones propias, configuración BSP y recetas necesarias.
3. Demostrar que se reconstruyen desde las entradas versionadas.
4. Solo entonces retirar del seguimiento el código que sea realmente generado y ajustar las exclusiones correspondientes.

Esta fase puede hacerse en otro cambio. No es requisito para centralizar los XSA y firmwares y no debe ampliar innecesariamente el primer refactor.

## 7. Validación y criterios de aceptación

- Cada entrada activa tiene paquete, manifiesto, hashes y selección explícita.
- Ningún artefacto importado cambia sus bytes; toda deduplicación conserva el mapa de nombres y procedencias.
- El bitstream utilizado corresponde al XSA seleccionado.
- La construcción no depende de ramas o directorios de trabajo de otros proyectos ni de las antiguas rutas `C:/PV/...`.
- CPU0, CPU1 y los componentes requeridos se construyen desde un checkout limpio siguiendo las instrucciones documentadas.
- Los modos de arranque y depuración mantienen su composición y comportamiento esperado.
- Cambiar el lock sin aplicar la plataforma impide construir con una selección incoherente.
- Un paquete ausente, alterado o no resuelto genera un error claro en las operaciones que lo requieren.
- Cambiar A → B → A no modifica fuentes del software; deja la plataforma y copias derivadas correspondientes a la selección final.
- Aplicar la misma selección dos veces es idempotente y no produce cambios versionados inesperados.
- Una construcción normal no añade XSA, bitstreams ni otros resultados fuera de las ubicaciones previstas en Git.
- El material histórico ambiguo sigue disponible y documentado; no se confunde con dependencias activas.

Las pruebas negativas de hashes, paquetes ausentes y cambio de selección se harán en copias temporales. No se alterarán deliberadamente los paquetes conservados como referencia.

## 8. Commits, recuperación y tamaño del repositorio

Separar los commits por propósito: inventario y paquetes; selección y verificación; integración con Vitis; retirada de copias; limpieza posterior de generados. No mezclar normalización de caracteres ni cambios funcionales.

Si hay que recuperar el flujo anterior, revertir los cambios de integración y limpieza de forma controlada y volver a aplicar la plataforma correspondiente. Cambiar solo el lock no restaura por sí mismo los BSP ni el estado del IDE.

Eliminar archivos en commits futuros limpia el árbol actual, pero **no reduce automáticamente el historial ya publicado en GitHub**. No usar `filter-repo`, rebase del historial publicado ni push forzado como parte de este plan.

Inicialmente se mantendrán los paquetes seleccionados y el histórico necesario en Git por sencillez. Si el crecimiento lo justifica, evaluar Git LFS o almacenamiento de releases como un trabajo independiente, manteniendo el lock, los hashes y una descarga reproducible. No introducir esa infraestructura durante la primera reorganización.

## 9. Decisiones pendientes y siguiente paso

1. Confirmar cuál es la combinación UCI/MK32 actualmente utilizada y validada en el equipo.
2. Identificar personalizaciones de BSP/FSBL y herramientas del host que referencien `BIN/`.
3. Obtener la compilación de referencia con las herramientas adecuadas.
4. Decidir qué históricos deben permanecer disponibles como paquetes y cuáles solo como archivo de referencia, sin descartarlos antes de clasificarlos.

El siguiente paso ejecutable es el inventario detallado de la fase 0. La creación y migración real de `artifacts/`, así como la retirada de archivos actuales, quedan pendientes de ejecutar este plan.

## Referencias

- [Plan de normalización de codificaciones](normalizacion-codificaciones.md).
- [Configuración actual de la plataforma](../../Platform/platform.spr).
- [Reglas actuales de exclusión](../../.gitignore).
- [Atributos de archivos binarios y texto](../../.gitattributes).
- [AMD: actualización de hardware mediante XSCT en Vitis 2022.2](https://docs.amd.com/r/2022.2-English/ug1400-vitis-embedded/platform-config).

## 10. Tarea final: README del repositorio

Al completar el refactor, escribir un `README.md` en la raíz del repositorio, en español y conciso. Debe permitir entender qué contiene el proyecto y cómo trabajar con él, sin reproducir este plan ni convertirse en un manual extenso.

Incluir:

- Descripción general de la UCI de SITAU2 y responsabilidades de CPU0, CPU1 y Platform.
- Herramientas y versiones necesarias, y pasos mínimos para preparar el workspace y compilar desde Vitis.
- Estructura esencial del repositorio y función de `artifacts/`.
- Metodología de paquetes versionados e inmutables, manifiestos y selección mediante `artifacts/dependencies-lock.json`.
- Pasos breves para incorporar una versión, seleccionarla, aplicar la plataforma y verificarla; explicar cómo volver a una dependencia anterior manteniendo el software actual.
- Distinción entre entradas versionadas y resultados generados, y enlaces a documentación adicional cuando sea necesaria.

Describir únicamente el flujo realmente implementado y validado, con nombres y acciones concretos. Si queda alguna operación manual o limitación, indicarla brevemente. La redacción del README será el último paso de documentación del refactor.
