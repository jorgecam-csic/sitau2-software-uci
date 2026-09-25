# SITAU2 — software de la UCI

Firmware bare-metal para los dos Cortex-A9 de la unidad de control e interfaz (UCI) de SITAU2. CPU0 gestiona Ethernet y la carga de programas; CPU1 controla la adquisición y los periféricos FPGA.

**CPU1 se envía por red y se ejecuta en RAM; nunca se incluye en la imagen de flash.** El repositorio contiene los fuentes, las recetas de generación, los artefactos hardware necesarios y las entregas software numeradas de output. Vitis trabaja en una carpeta externa que se puede reconstruir desde cero.

## Índice

- [Herramientas necesarias](#herramientas-necesarias)
- [Descargar en un directorio limpio](#descargar-en-un-directorio-limpio)
- [Generar el workspace](#generar-el-workspace)
- [Compilar y empaquetar](#compilar-y-empaquetar)
- [Recorrido completo sin GUI](#recorrido-completo-sin-gui)
- [Generar una versión para entregar](#generar-una-versión-para-entregar)
- [Trabajar desde Vitis](#trabajar-desde-vitis)
- [Trabajo diario y cambios de rama](#trabajo-diario-y-cambios-de-rama)
- [Dependencias hardware y lwIP](#dependencias-hardware-y-lwip)
- [Diagnóstico y limpieza](#diagnóstico-y-limpieza)
- [Contenido y validación](#contenido-y-validación)

## Herramientas necesarias

El flujo está preparado y probado para **Windows y Vitis Classic 2022.2**. No se ha migrado a Vitis 2025.2.

| Herramienta | Para qué se utiliza |
| --- | --- |
| Git para Windows | Clonar, cambiar de rama y registrar cambios. |
| Windows PowerShell 5.1 | Ejecutar los scripts; los BAT lo seleccionan explícitamente. |
| Vitis Classic 2022.2 | IDE, XSCT, compilador ARM, BSP y herramientas de empaquetado. |
| Bootgen 2022.2 | Crear las imágenes BIN; forma parte de la instalación Xilinx utilizada. |
| Vivado | Produce el XSA en el proyecto hardware externo. No hay que abrirlo ni reconstruir ese proyecto para compilar este repositorio. |

### Instalar Xilinx

1. Abrir la [página oficial de descargas 2022.2 de AMD](https://www.amd.com/en/support/downloads/adaptive-socs-and-fpgas/development-tools/2022-2.html).
2. Descargar **AMD/Xilinx Unified Installer 2022.2: Windows Self Extracting Web Installer**. El acceso a la descarga puede requerir una cuenta AMD.
3. Ejecutar el instalador y seleccionar **Vitis / Vitis Unified Software Platform**, manteniendo los componentes asociados que requiera el instalador, incluido Vivado. No basta con instalar únicamente Vivado.
4. Incluir soporte para **Zynq-7000** (la UCI usa XC7Z045). Para trabajar con una sonda, incluir también los controladores de cable. Model Composer, plataformas Alveo/Kria y familias ajenas a este hardware no son requisitos de este flujo.
5. Los lanzadores del repositorio utilizan por defecto **`E:\Xilinx\Vitis\2022.2`**. Para usar directamente los BAT sin parámetros, instalar bajo `E:\Xilinx`. Si se elige otra unidad, utilizar las órdenes con `-VitisHome` indicadas más abajo.
6. Verificar que existen `bin/vitis.bat`, `bin/xsct.bat`, `bin/bootgen.bat` y `data/embeddedsw` dentro de la instalación Vitis.

La [guía de instalación Vitis 2022.2](https://docs.amd.com/r/2022.2-English/ug1400-vitis-embedded/Installation) describe la instalación oficial. El XSA activo fue exportado con **Vivado 2022.2.2**; ese dato describe el productor del hardware, no una orden de actualizar Vitis. No hacen falta PetaLinux, un proyecto Vivado hermano ni una placa conectada para generar y compilar este software.

Si hay otras versiones Xilinx instaladas, conservarlas en sus directorios y seleccionar explícitamente la 2022.2 para este proyecto. Los scripts construyen un PATH reducido solo para el proceso lanzado; no cambian el PATH global de Windows. Esto evita el bloqueo de arranque observado con el PATH habitual del equipo.

## Descargar en un directorio limpio

Los ejemplos usan `D:\sitau2`. Se puede elegir otro directorio local con permisos de escritura; conviene una ruta corta, sin espacios ni caracteres especiales. Habrá que poder crear también una carpeta hermana del repositorio. Evitar reutilizar un workspace heredado o mezclar archivos del respaldo con el clon.

En PowerShell:

```powershell
New-Item -ItemType Directory -Force D:\sitau2 | Out-Null
Set-Location D:\sitau2
git clone https://github.com/jorgecam-csic/sitau2-software-uci.git
Set-Location .\sitau2-software-uci
```

El clon normal obtiene la rama predeterminada de GitHub. **Mientras esta PR no esté fusionada**, para usar este flujo:

```powershell
git switch artifacts-refactor
```

Para otra rama o versión, hacer el cambio antes de generar. Comprobar con `git status` que se está en el directorio y rama previstos. Los XSA y el paquete MK32 están incluidos en el repositorio; no se necesita copiar nada del ordenador del programador. Utilizar un clon Git: el build y las entregas registran el commit de origen, por lo que un ZIP sin historial Git no sirve para este recorrido.

Comprobar la dependencia UCI antes de abrir Xilinx:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Verify
```

Debe aparecer `Dependencia verificada: imported-3ed161d88aff` con el lock actual. `Verify` comprueba los hashes del paquete seleccionado y del XSA; no genera el workspace ni prueba la instalación de Vitis.

## Generar el workspace

Con Vitis cerrado, ejecutar desde la raíz:

```powershell
.\generar-workspace.bat
```

También se puede abrir el BAT con doble clic. **No admite parámetros y siempre genera desde cero.**

- Si no existe el workspace, lo crea.
- Si existe, muestra su ruta y la del archivo propuesto y pregunta `[s/N]`. `s` conserva el anterior como `workspace-<fecha>` y crea uno nuevo. Cualquier otra respuesta cancela.
- Si el workspace está abierto o no pertenece al repositorio esperado, se detiene. No eliminar sus marcadores para forzar la operación.

La disposición resultante es:

```text
D:/sitau2/
  sitau2-software-uci/             # Git: fuentes, recetas, artifacts, output y documentos
  sitau2-software-uci-work/        # entorno local, fuera de Git
    workspace/                    # Platform, CPU0, CPU1 y proyectos de sistema
      software-repository/        # biblioteca lwIP personalizada
    logs/                         # logs de generación y compilación
    workspace-<fecha>/            # entornos anteriores, si se archivaron
    workflow.lock                 # control de ejecuciones simultáneas
```

La carpeta de trabajo se calcula como `../<nombre-del-repositorio>-work`. Las carpetas antiguas de auditoría o referencia pueden existir en equipos usados para el refactor; no son necesarias en una descarga limpia.

El generador verifica el XSA, prepara `lwip211 1.08.s`, crea `Platform`, sus BSP y el FSBL, y crea las aplicaciones con los fuentes enlazados a `src/`. La primera ejecución puede tardar bastantes minutos. Consultar el log cuya ruta se muestra al principio, sin lanzar otra generación en paralelo.

El éxito se confirma con **`SITAU_OK:setup`** y la ruta final del workspace. Esta acción prepara y compila la plataforma/FSBL; **todavía no equivale a compilar CPU0/CPU1 ni a generar los paquetes de entrega**.

### Si Vitis está instalado en otra ruta

El motor interno permite seleccionar la instalación sin editar el BAT:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Setup -VitisHome 'C:\Xilinx\Vitis\2022.2'
```

Tiene el mismo comportamiento de generación desde cero y confirmación antes de archivar. Añadir el mismo `-VitisHome` a las órdenes posteriores de `Open`, `Build` y al empaquetado directo. También existe `-WorkRoot` para un entorno externo alternativo; si se utiliza, hay que repetirlo en todas las acciones. [Referencia de parámetros](scripts/README.md).

## Compilar y empaquetar

Guardar los cambios y **cerrar Vitis antes de ejecutar el flujo por consola**, para no abrir dos procesos sobre el mismo workspace. Desde la raíz del repositorio:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Build
```

`Build` requiere un workspace ya generado y coherente con las recetas actuales. Limpia y recompila CPU0 y CPU1 en **Debug**, comprueba los ELF, registra la procedencia del build y ejecuta el empaquetado. No genera ni actualiza automáticamente un workspace desactualizado. El build por consola recompila las aplicaciones desde cero para registrar una procedencia fiable; la plataforma se preparó al generar el workspace. El script elimina los objetos y dependencias de las aplicaciones antes de compilar; evita `app clean`, que en Vitis también limpia la plataforma y puede invalidarla. Las compilaciones de desarrollo desde GUI pueden ser incrementales.

El log de XSCT debe contener **`SITAU_OK:build`**. La operación completa debe terminar además con **`Paquetes verificados: ...`**; el marcador de compilación por sí solo no confirma el empaquetado.

### Dónde quedan los resultados

Dentro del workspace:

| Ruta | Resultado |
| --- | --- |
| `CPU0/Debug/CPU0.elf` | Ejecutable de CPU0. |
| `CPU1/Debug/CPU1.elf` | Ejecutable de CPU1. |
| `Platform/export/Platform/sw/Platform/boot/fsbl.elf` | FSBL generado de la plataforma. |
| `packages/<fecha>/cpu0-boot.bin` | FSBL + bitstream UCI + CPU0: imagen de arranque para flash. |
| `packages/<fecha>/cpu1-network.bin` | Solo CPU1: entrega por red para ejecutar en RAM. |
| `packages/<fecha>/*.bif` | Recetas de empaquetado con las rutas de esa ejecución. |
| `packages/<fecha>/manifest.json` | Hashes de entradas/salidas y particiones verificadas. |
| `packages/latest.txt` | Identifica el último empaquetado que terminó correctamente. |

Se genera una carpeta nueva en cada empaquetado. Conservar el manifiesto junto a los BIN cuando se prepare una entrega y anotar también el commit del software y la selección de hardware.

**No usar el `BOOT.BIN` automático de `CPU1_system` para enviarlo por red:** puede incluir FSBL y bitstream. Si el cliente exige un archivo llamado `BOOT.bin`, utilizar una copia de `cpu1-network.bin` con ese nombre. Los scripts crean y verifican archivos; no programan flash ni envían nada al equipo.

## Recorrido completo sin GUI

Sí: se puede clonar, generar el entorno, compilar y preparar una entrega **sin abrir manualmente Vitis gráfico**. Es obligatorio tener Vitis Classic 2022.2 instalado: XSCT utiliza sus herramientas y servicios internos. No ejecutar `abrir-vitis.bat` para este recorrido.

En PowerShell, con la instalación predeterminada y un directorio nuevo:

```powershell
Set-Location D:\sitau2
git clone https://github.com/jorgecam-csic/sitau2-software-uci.git
Set-Location .\sitau2-software-uci
# Mientras el refactor no este fusionado, seleccionar su rama:
git switch artifacts-refactor
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Verify
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Setup
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Build
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/generar-nueva-version.ps1 -Version 0.1.0
```

Ejecutar cada orden después de comprobar el éxito de la anterior. `0.1.0` es un ejemplo: elegir un número superior a todas las versiones existentes. Con un clon nuevo Setup no pregunta nada; si el workspace ya existe pide confirmación antes de archivarlo. La variante PowerShell evita la pausa final del BAT. El número pasado con `-Version` evita la pregunta de versión. Así el primer recorrido puede ejecutarse completamente por consola sin interacción con el IDE. Para otra instalación, repetir `-VitisHome` en Setup, Build y generación de versión.

Si se han editado fuentes o recetas después de clonar, **hacer commit antes de Build**. La creación de una versión exige Git limpio y un build del mismo commit. No pasar por `git commit` entre Build y la creación de la versión: incluso un commit documental distinto exige un nuevo Build para identificar inequívocamente su origen.

## Generar una versión para entregar

Los paquetes de `workspace/packages` son resultados de trabajo. Para crear una entrega numerada, ejecutar desde la raíz:

```powershell
.\generar_nueva_version.bat
```

El BAT pregunta `mayor.menor.parche` y reutiliza el empaquetador interno, sin compilar. Acepta únicamente tres componentes numéricos, sin prefijos, sufijos ni ceros iniciales. **No permite una versión igual o inferior a ninguna existente**; 0.10.0 es posterior a 0.9.0. No sobrescribe carpetas.

Para publicar exige: Git limpio, workspace coherente, Vitis cerrado, registro `.sitau-build.json` del mismo commit y hashes de entradas/ELF/FSBL/bitstream sin cambios desde Build. Un cambio posterior, una compilación GUI que altere el ELF o un build fallido obligan a ejecutar de nuevo Build. El registro se genera automáticamente, no se debe editar a mano.

La entrega se prepara temporalmente y solo aparece en su ubicación definitiva al terminar las validaciones:

```text
output/
  README.md
  0.1.0/
    cpu0-boot.bin
    cpu1-network.bin
    manifest.json
    README.md
```

**output se incluye en Git.** El manifiesto registra versión, fecha, commit de origen, entradas y salidas por hash, versión de herramientas, lock hardware y particiones. No certifica el funcionamiento en placa.

Después de revisar la entrega:

```powershell
git add output/0.1.0
git commit -m "Publica entrega 0.1.0"
```

Ese segundo commit guarda la entrega; el manifiesto referencia el commit anterior que produjo el software. El script no hace commit, push ni tag. Actualizar la rama antes de elegir una versión, conservar inmutables las entregas compartidas y resolver versiones concurrentes entre colaboradores antes de integrarlas. [Política y contenido de output](output/README.md).

## Trabajar desde Vitis

Abrir siempre el entorno existente mediante:

```powershell
.\abrir-vitis.bat
```

El lanzador comprueba su coherencia, utiliza el PATH reducido y abre la ruta calculada. No crea un workspace. No seleccionar la raíz del repositorio como workspace ni importar a mano proyectos del respaldo.

1. Cerrar la pestaña **Welcome** si aparece y seleccionar la perspectiva **Design**.
2. En Explorer/Assistant deben aparecer `Platform`, `Both_CPUs_system` con CPU0 y `CPU1_system` con CPU1. Estos proyectos son generados y viven fuera de Git.
3. Editar los archivos enlazados de `src/`. Comprobar en las propiedades de un recurso su ubicación si hay dudas.
4. En Assistant, seleccionar **Debug → Build** para CPU0 y después CPU1. La plataforma se genera durante Setup. Para una recompilación de aplicaciones se puede usar Clean en cada aplicación seguido de Build; no se necesita regenerar la plataforma por cada cambio de C.
5. Comprobar el final de las compilaciones en la consola y los ELF de Debug. Las comprobaciones de integridad también se ejecutan mediante los makefiles generados.
6. Una compilación en la GUI no produce necesariamente nuestros paquetes de entrega. Con los ELF actualizados, ejecutar desde la raíz:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/package.ps1 -Workspace 'D:\sitau2\sitau2-software-uci-work\workspace'
```

`package.ps1` **no compila**: utiliza los ELF existentes y verifica las dependencias antes de empaquetar. Estos paquetes sirven para desarrollo; una entrega en output requiere además el registro de procedencia del build por consola. Guardar y terminar los builds antes de invocarlo. Para una entrega con menos pasos manuales, cerrar Vitis y usar `Build` por consola.

## Trabajo diario y cambios de rama

### Qué editar y qué conservar

- Mantener el código en `src/`, las opciones de aplicaciones en `config/applications.tcl`, los parámetros BSP en `config/bsp` y las recetas BIN en `config/bootimage`.
- Los fuentes abiertos en Vitis están enlazados al repositorio: **editar desde Vitis modifica Git**. Archivar o borrar el workspace no crea una copia de seguridad independiente de esos fuentes. Guardar cambios y hacer commit o stash antes de cambiar de rama.
- Las modificaciones manuales de propiedades, BSP, bibliotecas o archivos generados dentro del workspace se pierden al regenerar. Si una configuración debe perdurar, trasladarla a las recetas mantenidas y validarla.
- Conservar las codificaciones y finales de línea de los fuentes existentes. El proyecto establece Cp1252 en Vitis; `.gitattributes` protege los bytes heredados y `.editorconfig` no fuerza UTF-8 en el código. Los README nuevos se escriben en UTF-8. La normalización del firmware tiene un [plan separado](documents/plans/normalizacion-codificaciones.md).
- No subir workspaces, objetos, ELF ni logs. Los paquetes de desarrollo quedan en el workspace; solo las entregas numeradas de `output` se incorporan a Git. Los XSA/BIT/BIN de `artifacts` son entradas hardware mantenidas y sí se versionan.

### Cuándo regenerar

| Cambio | Acción |
| --- | --- |
| Editar un C/H existente | Guardar y recompilar las aplicaciones afectadas; volver a empaquetar. |
| Añadir, eliminar o mover fuentes | Regenerar para reconstruir los enlaces y después compilar. |
| Cambiar `config`, `scripts` o `dependencies-lock.json` | Cerrar Vitis, generar desde cero y compilar. |
| Cambiar includes, bibliotecas o mapa de memoria | Modificar la receta o linker script correspondiente y validar una generación/compilación nueva. |
| Mover o renombrar el clon | Generar un workspace nuevo en la ubicación final; los enlaces antiguos apuntan a la ruta anterior. |
| Cambiar solo documentación fuera de `config` y `scripts` | No requiere regeneración. |

La huella del entorno incluye **todos los archivos de `config` y `scripts`, también sus README**, y el lock hardware. Por tanto, esta actualización documental puede invalidar un workspace creado con las recetas anteriores. La huella incluye también la lista de archivos fuente: si cambia su estructura se detecta y hay que regenerar.

Después de `git pull` o `git switch`, revisar qué cambió y ejecutar:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Check
```

Si aparece `Workspace desactualizado`, cerrar Vitis y ejecutar `generar-workspace.bat`. No reutilizar productos de otra rama para una entrega. Para trabajar simultáneamente en varias ramas, utilizar clones/worktrees en carpetas distintas con sus propios workspaces externos; no compartir un workspace activo.

## Dependencias hardware y lwIP

### UCI y MK32

`artifacts/dependencies-lock.json` selecciona el XSA UCI por ruta y SHA-256. Con él se reconstruyen la plataforma, BSP y FSBL; el bitstream exportado se utiliza en el paquete de arranque. No se consulta el checkout ni la rama de ningún proyecto Vivado externo.

Para actualizar hardware, añadir un paquete nuevo con manifiesto, seleccionar su XSA en el lock, ejecutar `Verify`, generar desde cero y compilar. No sobrescribir paquetes publicados. Para volver a una dependencia anterior, cambiar únicamente la selección del lock: **no hace falta retroceder el código software a otro commit**. [Detalle de artifacts](artifacts/README.md).

`artifacts/mk32/0.0.0` conserva juntos BIN, BIT y LTX del conjunto MK32 seleccionado. Es una entrega hardware independiente: no es CPU1 y no se incluye en el build ni en el BOOT de la UCI. `0.0.0` identifica esta entrega inicial; no certifica validación en placa ni revela el commit del productor.

### Biblioteca Ethernet personalizada

El generador copia la biblioteca Xilinx original 1.8 de Vitis a un repositorio software del workspace, aplica los dos fuentes auditados y la registra como **`lwip211 1.08.s`**. El BSP pide esa versión exacta. No se modifica la instalación Xilinx.

Regenerar el BSP recupera los cambios desde esa biblioteca personalizada. Los controles comprueban su identidad, selección y hashes tanto en el repositorio software como en el BSP. Si falta la variante o cambian los archivos, el flujo se detiene; se probó que una petición de `1.08.s` sin la variante disponible no selecciona silenciosamente la `1.8` original. [Funcionamiento y mantenimiento](config/lwip211/README.md).

## Diagnóstico y limpieza

| Síntoma | Comprobación o siguiente paso |
| --- | --- |
| Vitis tarda o parece bloqueado al arrancar | Usar nuestros lanzadores con PATH reducido; comprobar el log antes de iniciar otra instancia. |
| `No se encuentra Vitis` | Revisar instalación y `-VitisHome`. |
| `Falta dependencia` o hash incorrecto | Restaurar el paquete correspondiente al lock; no desactivar la verificación. |
| `Workspace desactualizado` | Cerrar Vitis y generar uno nuevo; incluye cambios en README de config/scripts. |
| Error de lwIP personalizada | Ejecutar `Check`; reconstruir desde las recetas verificadas en vez de seleccionar la original manualmente. |
| Mensajes rojos `NativeCommandError` o `RemoteException` | PowerShell 5 puede envolver notas/avisos enviados a stderr. Leer el diagnóstico real y comprobar el marcador final y el código de salida; el color no determina el resultado. |
| `Nothing to be done` | Un build incremental no encontró tareas; no acredita una recompilación desde cero. |
| Error al generar | Conservar el log, corregir la causa y generar de nuevo; un workspace incompleto no debe marcarse manualmente como listo. |
| Git sucio al generar una versión | Revisar y hacer commit de los cambios. Si cambia el commit, repetir Build antes de generar la versión. |
| Falta registro o cambió el commit/ELF | Ejecutar Build desde consola con el workspace coherente y Vitis cerrado. No editar el registro para saltar el control. |
| Versión existente o inferior | Elegir una versión superior a todas las entregas de output; no borrar carpetas para reutilizar números. |

Los logs del motor están en `<repo>-work/logs`; Vitis también escribe `workspace/IDE.log` y `workspace/.metadata/.log`. Los avisos heredados conocidos se describen en los informes de validación.

Con Vitis y los procesos de compilación cerrados, se pueden retirar workspaces archivados y resultados antiguos cuando no se necesiten. Conservar antes cualquier entrega o ajuste manual que interese. El archivo `workflow.lock` puede quedar en disco al terminar: el bloqueo real es el handle abierto durante una ejecución, no la mera existencia del archivo. El respaldo del proyecto original es independiente y no es necesario para compilar un clon limpio.

## Contenido y validación

Cada directorio versionable tiene un README con su propósito, archivos y contenido esperado.

| Carpeta | Contenido esperado |
| --- | --- |
| [src](src/README.md) | Código y linker scripts mantenidos de CPU0/CPU1. |
| [config](config/README.md) | Aplicaciones, parámetros BSP, parches lwIP y plantillas BIF. |
| [scripts](scripts/README.md) | Automatización PowerShell/XSCT del flujo reproducible. |
| [artifacts](artifacts/README.md) | Paquetes hardware inmutables y lock del XSA activo. |
| [output](output/README.md) | Entregas software versionadas e inmutables, incluidas en Git. |
| [tests](tests/README.md) | Pruebas aisladas de versiones y procedencia. |
| [documents](documents/README.md) | Planes e informes fechados con evidencias de validación. |

En la raíz, `generar-workspace.bat` genera el entorno, `abrir-vitis.bat` lo abre y `generar_nueva_version.bat` prepara una entrega numerada. `.gitignore` excluye productos/restos; `.gitattributes` y `.editorconfig` establecen las políticas de archivos. No hay proyectos Eclipse/Vitis mantenidos en la raíz: se reconstruyen en el workspace.

Se han probado generación limpia, compilación de ambas CPU, empaquetado, reconstrucción del BSP, rechazo de lwIP ausente/alterada y publicación de entregas numeradas con sus controles. Consultar [validación del refactor](documents/reports/validacion-refactor.md), [empaquetado](documents/reports/empaquetado-red-cpu1.md), [lwIP](documents/reports/lwip-version-sitau2.md) y [versionado de output](documents/reports/versionado-output.md). Los informes son históricos: sus rutas antiguas describen las pruebas de su fecha, no instrucciones vigentes.

Quedan fuera de la validación de compilación: funcionamiento en placa, preparación de depuración JTAG portable y, si se necesita, recarga de CPU1 en caliente. La normalización de codificaciones sigue pendiente. No se afirma que compilar y validar cabeceras equivalga a haber probado la entrega en hardware.
