# Cierre del refactor y documentación

24/09/2026. El README principal documenta instalación de Vitis Classic 2022.2, descarga de la rama correcta, generación explícita del workspace externo, compilación, GUI, empaquetado y trabajo diario. Los 39 directorios versionables, incluida la raíz, disponen de README con propósito y contenido esperado.

## Verificación de esta revisión

- Los 181 archivos de fuentes y recursos heredados bajo `src` (sin los nuevos README) coinciden byte a byte con el respaldo `sitau2_sw_uci`.
- Se verificaron los enlaces locales de todos los README y la sintaxis PowerShell de los scripts.
- `setup.ps1 -Action Verify` pasó en el repositorio y en una copia limpia creada desde el índice Git (`D:/sitau2/uci-pr-check-20260924`).
- Los seis archivos declarados por los manifiestos hardware (tres XSA y el trío MK32) conservan sus SHA-256 tras checkout. También se compararon los bytes de todos los payloads de artifacts, incluidos ps7_init y LTX, y los parches de lwIP.
- Se amplió `.gitattributes` para impedir que Git normalice los payloads externos de artifacts. README, manifiestos y lock siguen usando texto normalizado; los fuentes heredados conservan la protección existente.
- Los espacios/finales heredados del código y de los artefactos se preservan. Las comprobaciones de formato se aplican a documentación y recetas mantenidas, sin convertir el firmware.

## Evidencias de ejecución anteriores

El flujo funcional de esta rama se validó con generación limpia, compilación CPU0/CPU1 y empaquetado: [refactor](validacion-refactor.md), [paquetes](empaquetado-red-cpu1.md), [variante lwIP y ausencia/regeneración](lwip-version-sitau2.md). Las modificaciones de cierre son documentación, políticas de conservación de bytes y eliminación de líneas vacías finales en MSS; no se ha repetido una compilación completa en esta revisión documental.

Los cambios de README en config/scripts invalidan la huella del workspace. El usuario debe generar de nuevo conscientemente antes de continuar. No se actualizó ni se marcó como válido ningún workspace existente para eludir ese control.

## Estado final

El repositorio mantiene fuentes, recetas, paquetes hardware y documentación. Los proyectos Vitis, BSP/FSBL generados y paquetes software quedan fuera de Git. Se retiraron los restos de los sistemas heredados; el respaldo original permanece independiente.

Siguen pendientes las pruebas en placa, la preparación de depuración JTAG portable y la normalización de codificaciones. La generación de paquetes no programa la flash ni valida el funcionamiento Ethernet del equipo.
