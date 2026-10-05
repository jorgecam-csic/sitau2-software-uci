# reports

Resultados de auditoría, inventarios, hashes y validaciones. Aquí se esperan informes fechados y evidencias JSON, sin incorporar workspaces ni logs masivos. Los catálogos antiguos conservan nombres y rutas de su fecha; no implican que esos archivos sigan presentes. La compilación comprobada no equivale a validación sobre placa.

## Archivos

| Archivo | Función |
| --- | --- |
| [auditoria-bsp.md](auditoria-bsp.md) | Conclusiones de la comparación del BSP/FSBL heredado con una generación limpia. |
| [baseline-manifest.json](baseline-manifest.json) | Inventario de referencia y hashes del estado heredado. |
| [bootimage-archived.json](bootimage-archived.json) | Mapa de las siete recetas/salidas archivadas: origen, destino externo y SHA-256. |
| [bootimage-clasificacion.json](bootimage-clasificacion.json) | Clasificación detallada, particiones, duplicados y conjuntos MK32 del inventario original. |
| [bootimage-inventory.json](bootimage-inventory.json) | Inventario inicial del bootimage, con tamaños, hashes y estado de seguimiento Git. |
| [bsp-audit.json](bsp-audit.json) | Detalle de la auditoría de diferencias de BSP. |
| [bsp-final-comparison.json](bsp-final-comparison.json) | Resultado de la comparación final del BSP reproducido. |
| [build-manifest.json](build-manifest.json) | Manifiesto de entradas/salidas de la compilación de validación del refactor. |
| [carga-cpu1-desde-cpu0.md](carga-cpu1-desde-cpu0.md) | Recorrido del cargador TCP de CPU0 y confirmación del envío de CPU1 por red. |
| [cierre-refactor.md](cierre-refactor.md) | Comprobación documental, conservación de bytes en checkout y estado final de la rama. |
| [clasificacion-bootimage.md](clasificacion-bootimage.md) | Clasificación legible de imágenes, variantes MK32, configuraciones y duplicados. |
| [clean-build-manifest.json](clean-build-manifest.json) | Manifiesto de la validación realizada desde una copia limpia de Git. |
| [elf-comparison.json](elf-comparison.json) | Comparación de los ELF generados y las referencias heredadas. |
| [empaquetado-red-cpu1.md](empaquetado-red-cpu1.md) | Implementación y pruebas de los paquetes separados de arranque y carga por red. |
| [limpieza-mk32.md](limpieza-mk32.md) | Selección MK32 0.0.0, limpieza autorizada y pendientes conservados. |
| [limpieza-mk32.json](limpieza-mk32.json) | Rutas y hashes de los archivos copiados y retirados. |
| [limpieza-workspace-heredado.md](limpieza-workspace-heredado.md) | Registro de la retirada de restos del workspace antiguo. |
| [lwip-version-sitau2.md](lwip-version-sitau2.md) | Implementación y pruebas de lwip211 1.08.s: ausencia, controles, regeneración y compilación. |
| [lwip-origen-personalizado.md](lwip-origen-personalizado.md) | Aceptación de originales o personalizaciones conocidas en Vitis, pruebas e impacto en la release 0.0.0. |
| [lwip-origen-personalizado.json](lwip-origen-personalizado.json) | Hashes antes/después, comparación de los dos builds y controles negativos del BSP y la biblioteca. |
| [lwip-build-manifest.json](lwip-build-manifest.json) | Entradas, paquetes y hashes de la compilación con la variante lwIP. |
| [packaging-build-manifest.json](packaging-build-manifest.json) | Entradas, salidas, particiones y hashes del build que validó el empaquetado nuevo. |
| [referencia-compilacion.md](referencia-compilacion.md) | Evidencias de compilación del workspace heredado antes del refactor. |
| [validacion-refactor.md](validacion-refactor.md) | Validación de fuentes, BSP, compilación y entorno reproducible, con sus límites. |
| [versionado-output.md](versionado-output.md) | Entregas numeradas en Git, procedencia del build y validación del flujo por consola. |
| [versionado-output.json](versionado-output.json) | Commit de la copia de pruebas, hashes, particiones y resultados de la entrega aislada. |

[Volver](../README.md)
