# Entregas versionadas en output

Fecha: 2026-09-25.

## Cambios

`generar_nueva_version.bat` solicita una versión y llama a `scripts/generar-nueva-version.ps1`. Para ejecución sin preguntas se utiliza el PS1 con `-Version mayor.menor.parche`. La versión debe ser estrictamente superior a todas las carpetas de versiones existentes; nunca se sobrescribe una entrega.

`output` se incluye en Git. Cada entrega contiene `cpu0-boot.bin`, `cpu1-network.bin`, `manifest.json` y un README. CPU0 incluye FSBL y bitstream; CPU1 contiene exclusivamente la aplicación destinada a carga por red. No se programa ni se transmite nada al equipo.

El manifiesto identifica el commit compilado, la versión de herramientas, las entradas y salidas mediante SHA-256, el lock hardware y las particiones. No incluye rutas absolutas de la máquina. La generación no certifica funcionamiento en placa.

## Procedencia y publicación

Build limpia y compila ambas aplicaciones y registra `.sitau-build.json` fuera del repositorio. Se eliminan los objetos `.o` y dependencias `.d` de cada aplicación, conservando el BSP. La prueba detectó que `app clean` limpiaba también la plataforma y podía dejarla inválida; se retiró esa llamada. Se comprueba que las entradas y el commit no cambien durante el build.

La publicación exige Git limpio, entradas seguidas por Git, workspace coherente, IDE cerrado y un registro del mismo commit cuyos hashes sigan coincidiendo. Los bloqueos impiden publicar simultáneamente sobre el mismo output o mientras otra operación de los scripts utiliza el mismo workspace. Las carpetas temporales se excluyen de Git y se retiran si falla la publicación.

El orden de trabajo es: commit de fuentes/recetas, generación del workspace si hace falta, Build, generación de versión y segundo commit con output. El script no hace commit, push ni tag. La coordinación de números entre clones diferentes corresponde a los colaboradores.

## Validación

- Las 26 pruebas de `tests/release-tests.ps1` pasan en Windows PowerShell 5.1. Cubren orden numérico, formato, duplicados, versiones anteriores, Git sucio, entradas no versionadas, cambios de fuentes y binarios, commit, herramientas, registro ausente y bloqueo del IDE.
- Análisis sintáctico de los PS1 y `git diff --check`: correctos.
- `tests/clean-app-tests.tcl` pasa con Tcl 8.6: retira objetos/dependencias anidados y conserva fuentes, makefiles y objetos BSP bajo `_sdk`.
- Los 41 README están presentes y sus enlaces locales resuelven correctamente.
- La publicación durante un Build activo se rechaza por `workflow.lock`, sin crear la versión solicitada.
- Setup y Build nativos con Vitis 2022.2 terminan correctamente en la copia aislada `D:\sitau2\uci-release-test-20260925`, con workspace externo. Se generan ambos ELF y los dos BIN verificados sin abrir manualmente la GUI.
- La entrega de prueba `0.1.0` se publica correctamente en esa copia. Se verifican hashes y ausencia de rutas absolutas en el manifiesto. No se crea ninguna versión en el repositorio principal.
- La integración rechaza Git sucio, un ELF alterado, registro ausente, fallo de Bootgen, versión duplicada, versión anterior y una nueva entrega cuando la previa sigue sin commit. El fallo de Bootgen no publica una carpeta parcial. Las entradas alteradas deliberadamente se restauran al terminar cada caso.
- Un segundo Build en el mismo workspace termina correctamente, vuelve a generar ambos BIN y mantiene un registro válido. Los 78 objetos de aplicación tienen fecha posterior al inicio de ese segundo build: se recompilaron realmente. Su log es `D:\sitau2\release-build-repeat-20260925.log`.

La primera ejecución se interrumpió por un reinicio del ordenador. Al retomarla se detectó el problema de `app clean`; la validación satisfactoria corresponde a un workspace regenerado con la limpieza corregida. Vitis puede volver a ejecutar reglas de bibliotecas durante `app build`; retirar `app clean` no significa que sus archivos de biblioteca nunca cambien. Las comprobaciones de la variante lwIP siguen activas.

Los [resultados y hashes](versionado-output.json) corresponden al commit de la copia de prueba, no a una entrega oficial del repositorio principal. Persisten los avisos heredados de compilación/Bootgen; esta validación no modifica el firmware ni sustituye las pruebas en placa.

Logs locales: `D:\sitau2\release-setup-fixed-20260925.log`, `D:\sitau2\release-build-fixed-20260925.log` y `D:\sitau2\release-integration-20260925.log`. El detalle de cada prueba de publicación queda en `D:\sitau2\release-evidence-20260925`.

[Flujo completo por consola](../../README.md) · [Política de entregas](../../output/README.md)

## Corrección de respaldos locales

En el repositorio de trabajo aparecieron cuatro `*.bak` heredados, ignorados por Git. El registro los recogía como entradas y la publicación los rechazaba por no estar versionados. El inventario y la huella ahora utilizan una enumeración común que excluye exclusivamente la extensión `.bak`; no se excluyen indiscriminadamente los archivos ignorados por Git.

Los cuatro respaldos se conservan y sus hashes coinciden con el registro anterior. La suite ampliada pasa 29 pruebas: añadir, modificar o retirar respaldos no afecta a las entradas; una cabecera ignorada sin versionar sigue bloqueando la publicación. `Verify` del paquete hardware también pasa. No se ha repetido un build nativo tras esta corrección: el workspace habitual debe regenerarse y compilarse para producir un registro con el nuevo criterio. No se modifican los registros anteriores para hacerlos pasar.

## Exclusión de documentación y auxiliares

El inventario compartido por la huella del workspace y el registro de build excluye ahora Markdown (`.md` y `.markdown`), `README.txt`, `.bak`, `.log`, `.tmp`, `.swp`, `.swo`, nombres terminados en `~`, `.DS_Store`, `Thumbs.db` y `desktop.ini`. Los archivos no se borran. Se conservan en el control los formatos ambiguos, como otros `.txt`, `.json`, `.html` e `.in`, además de fuentes, recetas y artefactos.

Esta regla sustituye el criterio anterior que incluía README en la huella. Exige regenerar una vez los workspaces anteriores, pero las siguientes ediciones de documentación no exigirán regenerarlos. Los requisitos de Git limpio y mismo commit para publicar se mantienen; un nuevo commit documental requiere Build para publicar con esa identidad.

La suite ampliada pasa 77 pruebas en Windows PowerShell 5.1. Comprueba altas, modificaciones y bajas de cada tipo de auxiliar en las cuatro raíces de entrada; también verifica la detección de fuentes C/ASM, cabeceras, linker scripts, especificaciones, JSON, MSS, BIF, PS1/Tcl, XSA/BIT/BIN, TXT de recursos y HTML. No se ha repetido la compilación nativa para este cambio de selección de entradas.

[Volver](README.md)
