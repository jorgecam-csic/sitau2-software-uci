# output

Entregas software versionadas, incluidas en Git. Cada directorio `mayor.menor.parche` contiene CPU0 para arranque, CPU1 para carga por red, manifiesto y README. No guardar aquí resultados intermedios ni cada compilación de desarrollo.

## Generar una entrega

1. Guardar y hacer commit de los cambios del software y de las recetas. El árbol Git debe quedar limpio.
2. Con Vitis cerrado, generar el workspace si cambió su configuración y ejecutar `scripts/setup.ps1 -Action Build` desde la raíz. Esto limpia y recompila ambas CPU y registra procedencia en el workspace.
3. Ejecutar `generar_nueva_version.bat`. Pide una versión y no compila. Para automatizar, usar `scripts/generar-nueva-version.ps1 -Version 0.1.0`.
4. Revisar `output/<versión>` y hacer un segundo commit que incluya la entrega. El manifiesto identifica el commit de origen del software, anterior al commit de la propia entrega. No se hace commit, push ni tag automáticamente.

Solo se aceptan tres componentes numéricos entre 0 y 2147483647, sin ceros iniciales, prefijos ni sufijos. Se comparan numéricamente: 0.10.0 supera 0.9.0. La nueva versión debe superar todas las carpetas de versión presentes; una existente nunca se sobrescribe. Actualizar la rama antes de publicar para incorporar las versiones que hayan añadido otros colaboradores. El bloqueo evita publicaciones locales simultáneas, no coordina clones de distintos equipos.

## Contenido de cada versión

| Archivo | Uso |
| --- | --- |
| `cpu0-boot.bin` | FSBL + bitstream UCI + CPU0; imagen de arranque para flash. |
| `cpu1-network.bin` | Solo CPU1; envío por red y ejecución en RAM, nunca flash. |
| `manifest.json` | Versión, fecha UTC, commit limpio, hashes de entradas de build y binarios, herramientas, lock hardware y particiones verificadas. No contiene rutas absolutas del equipo. |
| `README.md` | Identificación y uso de esa entrega. |

El empaquetado se prepara en el workspace; la copia final se monta en `.pending-*` y solo se renombra a la versión al terminar todas las comprobaciones. Git ignora únicamente esas carpetas temporales y `.release.lock`, no las entregas. Un fallo no publica una versión parcial.

Las entregas son inmutables: no editar ni borrar versiones ya compartidas para reutilizar sus números. Los BIN incrementan el tamaño del historial Git. Antes de distribuirlos, validar sobre el equipo; `hardwareValidated: false` indica que el generador no certifica esas pruebas.

Los cambios de fuente posteriores al build, otro commit, un ELF recompilado desde GUI o una dependencia alterada invalidan la entrega. Ejecutar de nuevo el build por consola. Una compilación GUI sigue siendo útil para desarrollo, pero no crea por sí misma el registro de procedencia exigido para publicar.

[Volver](../README.md)
