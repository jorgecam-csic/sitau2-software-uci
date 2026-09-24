# artifacts

Paquetes hardware recibidos de otros proyectos. El lock selecciona la entrada XSA de compilación de la UCI; los paquetes MK32 se conservan por versión de entrega.

## Archivos

| Archivo | Función |
| --- | --- |
| [dependencies-lock.json](dependencies-lock.json) | Selecciona la versión de herramientas y el paquete XSA mediante ruta y SHA-256. |

## Subcarpetas

- [uci](uci/README.md): Paquetes XSA de la UCI. Solo el paquete señalado por el lock es una entrada activa.

- [mk32](mk32/README.md): BIN, BIT y LTX de MK32; versión inicial `0.0.0`, independiente del build UCI.

## Actualizar o volver a otra dependencia UCI

1. Añadir un paquete nuevo con manifiesto y SHA-256, sin sobrescribir los anteriores.
2. Actualizar la ruta y el hash del XSA en `dependencies-lock.json`.
3. Ejecutar `scripts/setup.ps1 -Action Verify` desde la raíz.
4. Cerrar Vitis y ejecutar `generar-workspace.bat`; confirmar el archivo del entorno anterior.
5. Compilar y validar sobre el equipo antes de declarar compatible la selección.

Para volver atrás, seleccionar el paquete anterior y repetir la generación; los cambios
de software permanecen. No se consultan ramas ni salidas de repositorios Vivado hermanos.
Los identificadores imported proceden del contenido, no de versiones funcionales.

## Contenido esperado de un paquete

Los payloads de artifacts se guardan sin conversión de finales de línea mediante `.gitattributes`, incluidos LTX y ps7_init. Sus bytes deben coincidir con los hashes también después de clonar en Windows. Los README, manifiestos y el lock siguen siendo texto normalizado.

Cada entrega ocupa una carpeta propia e inmutable con `README.md`, `manifest.json` y los archivos recibidos. El manifiesto identifica el paquete y sus rutas/hashes SHA-256, registra la procedencia conocida y deja explícitos los datos desconocidos, como el commit productor. Los README explican uso y límites; una versión de carpeta no equivale a validación funcional.

El lock actual solo activa el paquete UCI `imported-3ed161d88aff`; los otros XSA conservan referencias históricas. `Verify` comprueba el paquete UCI activo, no todos los paquetes almacenados. La selección MK32 `0.0.0` es independiente y sus hashes están en su propio manifiesto.

Aquí no deben aparecer ELF de CPU0/CPU1 ni BOOT generados por este repositorio. Esas salidas van a `workspace/packages`. Tampoco se deben enlazar salidas de un Vivado vecino: copiar una entrega conocida permite compilar sin depender de su rama de trabajo.

[Volver](../README.md)
