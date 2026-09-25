# scripts

Automatización mantenida para Windows PowerShell 5.1 y Vitis Classic 2022.2. Aquí deben vivir las recetas portables de generación, apertura, compilación y empaquetado; sus resultados y logs se escriben fuera de Git. No guardar aquí scripts generados por una sesión del IDE ni rutas del ordenador del programador.

## Archivos

| Archivo | Función |
| --- | --- |
| [build-record.ps1](build-record.ps1) | Funciones de procedencia de build, hashes, Git limpio, estado de IDE y comparación numérica de versiones. |
| [generar-nueva-version.ps1](generar-nueva-version.ps1) | Publica una entrega validada en output/versión, con Git limpio y registro de build coincidente. No compila ni hace commit. |
| [create-workspace.tcl](create-workspace.tcl) | Receta XSCT: crea plataforma, BSP/FSBL, lwIP personalizada y aplicaciones con fuentes enlazados; en modo build compila y comprueba los ELF. Invocarla mediante setup.ps1. |
| [generar-workspace.ps1](generar-workspace.ps1) | Entrada de generación sin parámetros, utilizada por el BAT de la raíz. |
| [setup.ps1](setup.ps1) | Motor de acciones y controles: paquetes, huella del entorno, integridad lwIP, bloqueo de ejecuciones y lanzamiento Xilinx. |
| [package.ps1](package.ps1) | Empaqueta ELF existentes con BIF propios, valida particiones y registra hashes. No compila ni programa hardware. |

## Acciones del motor

Desde la raíz del repositorio:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Verify
```

Cambiar la acción según lo necesario:

| Acción | Precondición y resultado |
| --- | --- |
| `Verify` | Verifica el manifiesto/XSA seleccionado en el lock. No necesita workspace ni comprueba la instalación Vitis. |
| `Setup` | Genera siempre desde cero. Si existe un workspace propio, pide archivarlo; si se cancela, lo deja intacto. Requiere cerrar Vitis. |
| `Check` | Verifica un workspace ya preparado: estado, huella, selección y hashes de lwIP. No compila ni corrige archivos. |
| `Open` | Comprueba y abre el workspace existente. No genera automáticamente. |
| `Build` | Comprueba el entorno, limpia y recompila CPU0/CPU1 en Debug, registra procedencia y empaqueta. Cerrar Vitis para evitar acceso simultáneo. No crea el workspace. |

Los lanzadores habituales están en la raíz:

- `generar-workspace.bat`: genera el entorno desde cero.
- `compilar.bat`: ejecuta `setup.ps1 -Action Build` sobre el workspace existente y genera los paquetes de desarrollo.
- `generar_nueva_version.bat`: prepara una entrega numerada en output, sin compilar.
- `abrir-vitis.bat`: abre el IDE y permite pasar opciones para un entorno alternativo.

Los tres primeros no admiten parámetros y hacen una pausa al terminar. Para automatización sin pausa o rutas alternativas, usar directamente los PS1. `setup.ps1` sigue siendo el motor interno de varias acciones, no solo de compilación.

## Rutas alternativas

Los valores predeterminados son Vitis `E:/Xilinx/Vitis/2022.2` y la carpeta hermana `<nombre-del-repositorio>-work`. Para otra instalación o un entorno independiente:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Setup -VitisHome 'C:\Xilinx\Vitis\2022.2' -WorkRoot 'D:\sitau2\uci-prueba-work'
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/setup.ps1 -Action Build -VitisHome 'C:\Xilinx\Vitis\2022.2' -WorkRoot 'D:\sitau2\uci-prueba-work'
.\abrir-vitis.bat -VitisHome 'C:\Xilinx\Vitis\2022.2' -WorkRoot 'D:\sitau2\uci-prueba-work'
```

Ejecutar las órdenes por separado; cerrar Vitis antes de Build/Setup. Los parámetros no se guardan como preferencias: repetir las mismas rutas en cada operación. WorkRoot debe estar fuera del repositorio y el workspace será su subcarpeta `workspace`. El entorno archivado queda como `workspace-<fecha>` dentro del mismo WorkRoot.

## Empaquetar después de compilar en la GUI

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/package.ps1 -Workspace 'D:\sitau2\sitau2-software-uci-work\workspace'
```

Para la instalación alternativa, añadir `-VitisHome 'C:\Xilinx\Vitis\2022.2'`. El workspace debe ser el generado por Setup. Guardar fuentes y completar ambos builds antes de empaquetar: este comando no detecta por sí mismo si los ELF están al día respecto a cada edición de código.

Se resuelven las plantillas de [config/bootimage](../config/bootimage/README.md) contra FSBL, bitstream y ELF del workspace. Cada ejecución escribe `packages/<fecha>/`; solo si ambos productos se validan se actualiza `packages/latest.txt`. No publicar una carpeta parcial de una ejecución fallida.

## Controles y mantenimiento

- La huella incluye `dependencies-lock.json` y todos los archivos de `config` y `scripts`, incluidos sus README. Modificarlos requiere generar otra vez. La huella incluye también la lista de archivos de src: un alta, baja o movimiento exige regenerar sus enlaces.
- lwIP debe ser exactamente `lwip211 1.08.s`; se verifican el MLD, los dos MSS y los parches del repositorio software y del BSP. Los makefiles de las aplicaciones/sistemas incluyen el mismo control para las compilaciones GUI.
- Se auditan los archivos originales de la instalación Xilinx antes de construir la variante; no se escribe sobre Vitis.
- Setup no archiva workspaces ajenos ni abiertos. `workflow.lock` evita operaciones simultáneas del motor; además debe cerrarse la GUI al usar XSCT por consola.
- El PATH reducido incluye también el directorio de Git para los controles de procedencia y solo dura durante la ejecución. Los logs XSCT se guardan en WorkRoot/logs. Se exige código de salida correcto y marcador `SITAU_OK:<acción>`; Build exige además empaquetado correcto.
- No cambiar manualmente `.sitau-workspace.json` para eludir una comprobación. Cambiar las entradas mantenidas y generar desde cero.

## Versionar una entrega sin GUI

El BAT de raíz `generar_nueva_version.bat` no admite parámetros y pregunta la versión. Por consola, el equivalente automatizable es:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/generar-nueva-version.ps1 -Version 0.1.0
```

Admite `-VitisHome` y `-WorkRoot` con los mismos valores que Setup/Build; si se omite Version, pregunta. Requiere Git limpio, workspace coherente, Vitis cerrado y `.sitau-build.json` con commit, entradas y productos coincidentes. Todos los archivos de entrada deben estar versionados. El registro se elimina al comenzar Build y solo se escribe tras una compilación correcta con entradas estables. No editarlo ni copiar uno de otro entorno.

Build elimina los objetos `.o` y dependencias `.d` de las aplicaciones y exige un ELF nuevo, conservando sus BSP. No utiliza `app clean`, que puede invalidar la plataforma en Vitis 2022.2; las compilaciones GUI pueden seguir siendo incrementales para desarrollo. El empaquetador interno mantiene `workspace/packages` y admite `-PassThru` para devolver al publicador el directorio que acaba de generar. Las entregas oficiales se guardan en [output](../output/README.md), con números estrictamente crecientes.

Las comprobaciones rápidas de versiones y procedencia están en [tests](../tests/README.md).

[Guía completa de uso](../README.md) · [Volver](../README.md)
